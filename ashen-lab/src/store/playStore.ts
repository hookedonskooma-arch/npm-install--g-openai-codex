import { create } from 'zustand';
import { GRID_SIZE, TileType, WorldTile } from '@/types/world';

const WALKABLE: TileType[] = ['grass', 'path'];
export const LEAVES_NEEDED = 3;

function tileAt(tiles: WorldTile[], x: number, y: number) {
  return tiles.find((tile) => tile.x === x && tile.y === y);
}

type PlayState = {
  x: number;
  y: number;
  leaves: number;
  collectedTreeKeys: string[];
  message: string | null;
  spawn: (x: number, y: number) => void;
  move: (dx: number, dy: number, tiles: WorldTile[]) => void;
  interact: (tiles: WorldTile[]) => void;
};

export const usePlayStore = create<PlayState>((set, get) => ({
  x: 2,
  y: 2,
  leaves: 0,
  collectedTreeKeys: [],
  message: null,

  spawn: (x, y) => set({ x, y, leaves: 0, collectedTreeKeys: [], message: null }),

  move: (dx, dy, tiles) => {
    const { x, y } = get();
    const nx = x + dx;
    const ny = y + dy;
    if (nx < 0 || nx >= GRID_SIZE || ny < 0 || ny >= GRID_SIZE) return;
    const target = tileAt(tiles, nx, ny);
    if (!target || !WALKABLE.includes(target.type)) return;
    set({ x: nx, y: ny, message: null });
  },

  interact: (tiles) => {
    const { x, y, collectedTreeKeys } = get();
    const neighbors = [
      { x, y: y - 1 },
      { x, y: y + 1 },
      { x: x - 1, y },
      { x: x + 1, y },
    ];

    for (const neighbor of neighbors) {
      const tile = tileAt(tiles, neighbor.x, neighbor.y);
      if (tile?.type === 'tree') {
        const key = `${tile.x},${tile.y}`;
        if (collectedTreeKeys.includes(key)) {
          set({ message: 'Marker already collected.' });
          return;
        }
        set((state) => ({
          leaves: state.leaves + 1,
          collectedTreeKeys: [...state.collectedTreeKeys, key],
          message: 'Marker token collected.',
        }));
        return;
      }
    }

    set({ message: 'No marker node here.' });
  },
}));
