'use client';

import { useState } from 'react';
import { useWorldStore } from '@/store/worldStore';

export function WorldToolbar() {
  const saveWorld = useWorldStore((state) => state.saveWorld);
  const loadWorld = useWorldStore((state) => state.loadWorld);
  const clearWorld = useWorldStore((state) => state.clearWorld);
  const loadTestWorld = useWorldStore((state) => state.loadTestWorld);
  const [message, setMessage] = useState<string | null>(null);

  return (
    <div className="flex items-center gap-3">
      <button
        type="button"
        onClick={() => {
          loadTestWorld();
          saveWorld();
          setMessage('Test world loaded & saved!');
        }}
        className="rounded bg-[#004F71] px-4 py-2 text-sm font-semibold text-white hover:bg-[#00354d]"
      >
        Load Test World
      </button>
      <button
        type="button"
        onClick={() => {
          saveWorld();
          setMessage('World saved!');
        }}
        className="rounded bg-emerald-600 px-4 py-2 text-sm font-semibold text-white hover:bg-emerald-700"
      >
        Save World
      </button>
      <button
        type="button"
        onClick={() => {
          const loaded = loadWorld();
          setMessage(loaded ? 'World loaded!' : 'No saved world found.');
        }}
        className="rounded bg-sky-600 px-4 py-2 text-sm font-semibold text-white hover:bg-sky-700"
      >
        Load World
      </button>
      <button
        type="button"
        onClick={() => {
          clearWorld();
          setMessage('World cleared.');
        }}
        className="rounded bg-rose-500 px-4 py-2 text-sm font-semibold text-white hover:bg-rose-600"
      >
        Clear World
      </button>
      {message && <span className="text-sm text-slate-600">{message}</span>}
    </div>
  );
}
