'use client';

import { TileType } from '@/types/world';
import { TILE_RISER_HEIGHT, TILE_SIDE_COLORS, TILE_TOP_COLORS } from '@/lib/tileColors';
import { TILE_H, TILE_W } from '@/lib/isometric';

/**
 * Purely decorative: renders the diamond floor face plus a riser "block"
 * for tall tiles (house/tree), sitting visually on top of and partially
 * occluding whatever is behind it - exactly like real isometric games.
 * Never receives clicks - see IsoHitTile for that.
 */
export function IsoTile({ type, left, top }: { type: TileType; left: number; top: number }) {
  const riser = TILE_RISER_HEIGHT[type];

  return (
    <div
      style={{
        position: 'absolute',
        left,
        top: top - riser,
        width: TILE_W,
        height: TILE_H + riser,
        pointerEvents: 'none',
      }}
    >
      {riser > 0 && (
        <div
          style={{
            position: 'absolute',
            top: TILE_H / 2,
            left: TILE_W * 0.08,
            width: TILE_W * 0.84,
            height: riser,
            background: TILE_SIDE_COLORS[type],
            clipPath: 'polygon(0 0, 100% 0, 88% 100%, 12% 100%)',
          }}
        />
      )}
      <div
        style={{
          position: 'absolute',
          top: 0,
          left: 0,
          width: TILE_W,
          height: TILE_H,
          background: TILE_TOP_COLORS[type],
          clipPath: 'polygon(50% 0%, 100% 50%, 50% 100%, 0% 50%)',
          border: '1px solid rgba(0,0,0,0.18)',
          boxShadow: 'inset 0 -2px 0 rgba(255,255,255,0.25)',
        }}
      />
    </div>
  );
}

/**
 * Invisible click target for the builder, positioned at the tile's
 * unraised floor position (ignoring riser height). Unraised diamonds
 * tessellate the grid perfectly with zero gaps or overlaps, so every
 * tile stays independently clickable no matter how tall its neighbors'
 * risers are.
 */
export function IsoHitTile({
  left,
  top,
  onClick,
  ariaLabel,
}: {
  left: number;
  top: number;
  onClick: () => void;
  ariaLabel: string;
}) {
  return (
    <div
      role="button"
      tabIndex={0}
      aria-label={ariaLabel}
      onClick={onClick}
      onKeyDown={(event) => {
        if (event.key === 'Enter' || event.key === ' ') onClick();
      }}
      className="hover:brightness-110"
      style={{
        position: 'absolute',
        left,
        top,
        width: TILE_W,
        height: TILE_H,
        clipPath: 'polygon(50% 0%, 100% 50%, 50% 100%, 0% 50%)',
        cursor: 'pointer',
      }}
    />
  );
}
