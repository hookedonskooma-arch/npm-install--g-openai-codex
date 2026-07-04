import { GRID_SIZE, TileType, WorldTile } from '@/types/world';

/**
 * Handoff 07 test world: tree border with one entrance, a path leading to a
 * house, a small interior grove, and a pond — with an open grass area left
 * for free play.
 */
export function createTestWorld(): WorldTile[] {
  const grid: TileType[][] = Array.from({ length: GRID_SIZE }, () =>
    Array.from({ length: GRID_SIZE }, () => 'grass' as TileType)
  );

  const set = (x: number, y: number, type: TileType) => {
    grid[y][x] = type;
  };

  // Tree border.
  for (let x = 0; x < GRID_SIZE; x++) {
    set(x, 0, 'tree');
    set(x, GRID_SIZE - 1, 'tree');
  }
  for (let y = 0; y < GRID_SIZE; y++) {
    set(0, y, 'tree');
    set(GRID_SIZE - 1, y, 'tree');
  }

  // Entrance gap + path leading down to the house.
  set(8, 0, 'path');
  for (let y = 1; y <= 7; y++) {
    set(8, y, 'path');
  }
  set(8, 8, 'house');

  // Small interior grove of trees.
  set(3, 3, 'tree');
  set(3, 4, 'tree');
  set(4, 3, 'tree');

  // Pond.
  for (let y = 11; y <= 14; y++) {
    for (let x = 11; x <= 14; x++) {
      set(x, y, 'water');
    }
  }

  const tiles: WorldTile[] = [];
  for (let y = 0; y < GRID_SIZE; y++) {
    for (let x = 0; x < GRID_SIZE; x++) {
      tiles.push({ x, y, type: grid[y][x] });
    }
  }
  return tiles;
}
