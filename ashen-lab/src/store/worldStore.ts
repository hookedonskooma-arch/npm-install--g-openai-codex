import { create } from 'zustand';
import { GRID_SIZE, TileType, WorldTile } from '@/types/world';
import { createTestWorld } from '@/lib/testWorld';
import { supabase } from '@/lib/supabase';

export type PaletteSelection = TileType | 'eraser';

export type CloudResult = { ok: true } | { ok: false; error: string };

const STORAGE_KEY = 'minas_world_local_save_v1';

function createDefaultGrid(): WorldTile[] {
  const tiles: WorldTile[] = [];
  for (let y = 0; y < GRID_SIZE; y++) {
    for (let x = 0; x < GRID_SIZE; x++) {
      tiles.push({ x, y, type: 'grass' });
    }
  }
  return tiles;
}

type WorldStore = {
  tiles: WorldTile[];
  selected: PaletteSelection;
  lastSavedAt: number | null;
  selectTile: (selection: PaletteSelection) => void;
  paintTile: (x: number, y: number) => void;
  saveWorld: () => void;
  loadWorld: () => boolean;
  clearWorld: () => void;
  loadTestWorld: () => void;
  saveWorldToCloud: () => Promise<CloudResult>;
  loadWorldFromCloud: () => Promise<CloudResult>;
};

export const useWorldStore = create<WorldStore>((set, get) => ({
  tiles: createDefaultGrid(),
  selected: 'grass',
  lastSavedAt: null,

  selectTile: (selection) => set({ selected: selection }),

  paintTile: (x, y) => {
    const paintType: TileType = get().selected === 'eraser' ? 'grass' : (get().selected as TileType);
    set((state) => ({
      tiles: state.tiles.map((tile) =>
        tile.x === x && tile.y === y ? { ...tile, type: paintType } : tile
      ),
    }));
  },

  saveWorld: () => {
    if (typeof window === 'undefined') return;
    window.localStorage.setItem(STORAGE_KEY, JSON.stringify(get().tiles));
    set({ lastSavedAt: Date.now() });
  },

  loadWorld: () => {
    if (typeof window === 'undefined') return false;
    const raw = window.localStorage.getItem(STORAGE_KEY);
    if (!raw) return false;
    try {
      const tiles = JSON.parse(raw) as WorldTile[];
      set({ tiles });
      return true;
    } catch {
      return false;
    }
  },

  clearWorld: () => set({ tiles: createDefaultGrid() }),

  loadTestWorld: () => set({ tiles: createTestWorld() }),

  saveWorldToCloud: async () => {
    if (!supabase) return { ok: false, error: 'Supabase is not configured.' };
    const { data: userData, error: userError } = await supabase.auth.getUser();
    if (userError || !userData.user) return { ok: false, error: 'Not signed in.' };

    const { error } = await supabase
      .from('worlds')
      .upsert({ user_id: userData.user.id, tiles: get().tiles }, { onConflict: 'user_id' });

    if (error) return { ok: false, error: error.message };
    return { ok: true };
  },

  loadWorldFromCloud: async () => {
    if (!supabase) return { ok: false, error: 'Supabase is not configured.' };
    const { data: userData, error: userError } = await supabase.auth.getUser();
    if (userError || !userData.user) return { ok: false, error: 'Not signed in.' };

    const { data, error } = await supabase
      .from('worlds')
      .select('tiles')
      .eq('user_id', userData.user.id)
      .is('deleted_at', null)
      .maybeSingle();

    if (error) return { ok: false, error: error.message };
    if (!data) return { ok: false, error: 'No cloud save found.' };

    set({ tiles: data.tiles });
    return { ok: true };
  },
}));
