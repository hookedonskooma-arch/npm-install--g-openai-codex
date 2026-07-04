'use client';

import { GRID_SIZE } from '@/types/world';
import { useWorldStore } from '@/store/worldStore';
import { IsoHitTile, IsoTile } from './IsoTile';
import { isoContainerSize, isoDepthSort, isoPosition } from '@/lib/isometric';

export function TileGrid() {
  const tiles = useWorldStore((state) => state.tiles);
  const paintTile = useWorldStore((state) => state.paintTile);
  const { width, height } = isoContainerSize(GRID_SIZE);
  const sorted = isoDepthSort(tiles);

  return (
    <div className="relative select-none" style={{ width, height }}>
      {sorted.map((tile) => {
        const { left, top } = isoPosition(tile.x, tile.y, GRID_SIZE);
        return <IsoTile key={`${tile.x}-${tile.y}`} type={tile.type} left={left} top={top} />;
      })}
      {tiles.map((tile) => {
        const { left, top } = isoPosition(tile.x, tile.y, GRID_SIZE);
        return (
          <IsoHitTile
            key={`hit-${tile.x}-${tile.y}`}
            left={left}
            top={top}
            ariaLabel={`Tile ${tile.x}, ${tile.y}: ${tile.type}`}
            onClick={() => paintTile(tile.x, tile.y)}
          />
        );
      })}
    </div>
  );
}
