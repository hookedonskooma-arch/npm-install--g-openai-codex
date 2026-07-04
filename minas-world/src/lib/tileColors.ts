import { TileType } from '@/types/world';

// Kept for any non-isometric consumers.
export const TILE_COLORS: Record<TileType, string> = {
  grass: 'bg-green-400',
  path: 'bg-amber-200',
  water: 'bg-sky-400',
  house: 'bg-orange-400',
  tree: 'bg-emerald-700',
};

export const TILE_TOP_COLORS: Record<TileType, string> = {
  grass: '#7ed957',
  path: '#f0d9a0',
  water: '#4fc3f7',
  house: '#ffb37b',
  tree: '#5fb37f',
};

export const TILE_SIDE_COLORS: Record<TileType, string> = {
  grass: '#4f9e37',
  path: '#c9a662',
  water: '#2a93c9',
  house: '#d97a3f',
  tree: '#3d7a56',
};

// How tall each tile's "block" sticks up off the floor. 0 = flat floor tile.
export const TILE_RISER_HEIGHT: Record<TileType, number> = {
  grass: 0,
  path: 0,
  water: 0,
  house: 34,
  tree: 24,
};
