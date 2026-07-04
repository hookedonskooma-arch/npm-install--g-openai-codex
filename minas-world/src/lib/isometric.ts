export const TILE_W = 40;
export const TILE_H = 20;
const MAX_RISER = 34;

export function isoPosition(x: number, y: number, gridSize: number) {
  const originX = ((gridSize - 1) * TILE_W) / 2;
  return {
    left: originX + (x - y) * (TILE_W / 2),
    top: (x + y) * (TILE_H / 2),
  };
}

export function isoContainerSize(gridSize: number) {
  return {
    width: gridSize * TILE_W,
    height: (gridSize - 1) * TILE_H + TILE_H + MAX_RISER + 20,
  };
}

export function isoDepthSort<T extends { x: number; y: number }>(tiles: T[]): T[] {
  return [...tiles].sort((a, b) => a.x + a.y - (b.x + b.y));
}
