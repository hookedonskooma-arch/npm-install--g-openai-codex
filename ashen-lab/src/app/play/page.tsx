'use client';

import { useEffect } from 'react';
import Link from 'next/link';
import { useWorldStore } from '@/store/worldStore';
import { LEAVES_NEEDED, usePlayStore } from '@/store/playStore';
import { PlayGrid } from '@/components/play/PlayGrid';

const MOVE_KEYS: Record<string, [number, number]> = {
  ArrowUp: [0, -1],
  ArrowDown: [0, 1],
  ArrowLeft: [-1, 0],
  ArrowRight: [1, 0],
  w: [0, -1],
  s: [0, 1],
  a: [-1, 0],
  d: [1, 0],
};

export default function PlayPage() {
  const tiles = useWorldStore((state) => state.tiles);
  const loadWorld = useWorldStore((state) => state.loadWorld);
  const { x, y, leaves, message, move, interact } = usePlayStore();

  useEffect(() => {
    loadWorld();
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

  useEffect(() => {
    function handleKeyDown(event: KeyboardEvent) {
      if (event.key in MOVE_KEYS) {
        event.preventDefault();
        const [dx, dy] = MOVE_KEYS[event.key];
        move(dx, dy, tiles);
      } else if (event.key === ' ') {
        event.preventDefault();
        interact(tiles);
      }
    }
    window.addEventListener('keydown', handleKeyDown);
    return () => window.removeEventListener('keydown', handleKeyDown);
  }, [tiles, move, interact]);

  const questComplete = leaves >= LEAVES_NEEDED;

  return (
    <main className="min-h-screen bg-[#18181b] p-6">
      <div className="mb-4 flex items-center justify-between">
        <h1 className="text-2xl font-bold text-[#c4b5fd]">Ashen Frontier Lab — Play Mode</h1>
        <Link href="/worlds" className="text-sm font-medium text-[#c4b5fd] underline">
          Back to Planner
        </Link>
      </div>

      <div className="mb-4 rounded bg-zinc-800 p-3 shadow-sm">
        <p className="font-semibold text-[#a78bfa]">
          Quest Template Tool: collect 3 marker tokens. ({Math.min(leaves, LEAVES_NEEDED)}/{LEAVES_NEEDED})
        </p>
        {questComplete && (
          <p className="mt-1 font-semibold text-emerald-400">Template test passed — all markers collected.</p>
        )}
        {message && <p className="mt-1 text-sm text-zinc-400">{message}</p>}
      </div>

      <PlayGrid tiles={tiles} playerX={x} playerY={y} />

      <p className="mt-4 text-sm text-zinc-500">
        Use arrow keys or WASD to walk. Press space near a marker node to test collection.
      </p>
    </main>
  );
}
