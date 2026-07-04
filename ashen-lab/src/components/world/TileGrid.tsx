'use client';

import { GRID_SIZE } from '@/types/world';
import { useWorldStore } from '@/store/worldStore';
import { TILE_COLORS } from '@/lib/tileColors';

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
