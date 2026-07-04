import { create } from 'zustand';
import { GRID_SIZE, TileType, WorldTile } from '@/types/world';

export type PaletteSelection = TileType | 'eraser';

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
}));
