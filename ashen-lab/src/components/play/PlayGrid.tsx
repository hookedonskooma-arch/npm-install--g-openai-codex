'use client';

import { GRID_SIZE, WorldTile } from '@/types/world';
import { TILE_COLORS } from '@/lib/tileColors';

const TILE_PX = 24;

export function PlayGrid({ tiles, playerX, playerY }: { tiles: WorldTile[]; playerX: number; playerY: number }) {
  return (
    <div
      className="relative border border-slate-300 select-none"
      style={{ width: GRID_SIZE * TILE_PX, height: GRID_SIZE * TILE_PX }}
    >
      <div
        className="grid gap-px bg-slate-300"
        style={{ gridTemplateColumns: `repeat(${GRID_SIZE}, ${TILE_PX}px)` }}
      >
        {tiles.map((tile) => (
          <div
            key={`${tile.x}-${tile.y}`}
            className={`${TILE_COLORS[tile.type]}`}
            style={{ width: TILE_PX, height: TILE_PX }}
          />
        ))}
      </div>
      <div
        aria-label="Player"
        className="absolute flex items-center justify-center text-lg transition-all duration-100"
        style={{
          left: playerX * TILE_PX,
          top: playerY * TILE_PX,
          width: TILE_PX,
          height: TILE_PX,
        }}
      >
        🧑
      </div>
    </div>
  );
}
