'use client';

import { GRID_SIZE, TileType } from '@/types/world';
import { useWorldStore } from '@/store/worldStore';

const TILE_COLORS: Record<TileType, string> = {
  grass: 'bg-green-400',
  path: 'bg-amber-200',
  water: 'bg-sky-400',
  house: 'bg-orange-400',
  tree: 'bg-emerald-700',
};

export function TileGrid() {
  const tiles = useWorldStore((state) => state.tiles);
  const paintTile = useWorldStore((state) => state.paintTile);

  return (
    <div
      className="grid gap-px bg-slate-300 border border-slate-300 select-none"
      style={{ gridTemplateColumns: `repeat(${GRID_SIZE}, minmax(0, 1fr))`, width: 'fit-content' }}
    >
      {tiles.map((tile) => (
        <button
          key={`${tile.x}-${tile.y}`}
          type="button"
          aria-label={`Tile ${tile.x}, ${tile.y}: ${tile.type}`}
          onClick={() => paintTile(tile.x, tile.y)}
          className={`h-6 w-6 ${TILE_COLORS[tile.type]} hover:opacity-80`}
        />
      ))}
    </div>
  );
}
