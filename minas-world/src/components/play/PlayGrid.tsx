'use client';

import { GRID_SIZE, WorldTile } from '@/types/world';
import { IsoTile } from '@/components/world/IsoTile';
import { isoContainerSize, isoDepthSort, isoPosition, TILE_H, TILE_W } from '@/lib/isometric';

export function PlayGrid({ tiles, playerX, playerY }: { tiles: WorldTile[]; playerX: number; playerY: number }) {
  const { width, height } = isoContainerSize(GRID_SIZE);
  const sorted = isoDepthSort(tiles);
  const playerPos = isoPosition(playerX, playerY, GRID_SIZE);

  return (
    <div className="relative select-none" style={{ width, height }}>
      {sorted.map((tile) => {
        const { left, top } = isoPosition(tile.x, tile.y, GRID_SIZE);
        return <IsoTile key={`${tile.x}-${tile.y}`} type={tile.type} left={left} top={top} />;
      })}
      <div
        aria-label="Player"
        className="absolute z-[9999] flex items-center justify-center text-lg transition-all duration-100"
        style={{
          left: playerPos.left,
          top: playerPos.top - TILE_H,
          width: TILE_W,
          height: TILE_H,
        }}
      >
        🧑
      </div>
    </div>
  );
}
