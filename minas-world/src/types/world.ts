export type TileType = 'grass' | 'path' | 'water' | 'house' | 'tree';

export type WorldTile = {
  x: number;
  y: number;
  type: TileType;
};

export const GRID_SIZE = 16;
