'use client';

import { PaletteSelection, useWorldStore } from '@/store/worldStore';

const PALETTE_OPTIONS: { value: PaletteSelection; label: string; swatch: string }[] = [
  { value: 'grass', label: 'Grass', swatch: 'bg-green-400' },
  { value: 'path', label: 'Path', swatch: 'bg-amber-200' },
  { value: 'water', label: 'Water', swatch: 'bg-sky-400' },
  { value: 'house', label: 'House', swatch: 'bg-orange-400' },
  { value: 'tree', label: 'Tree', swatch: 'bg-emerald-700' },
  { value: 'eraser', label: 'Eraser', swatch: 'bg-white border border-slate-400' },
];

export function TilePalette() {
  const selected = useWorldStore((state) => state.selected);
  const selectTile = useWorldStore((state) => state.selectTile);

  return (
    <div className="flex flex-col gap-2">
      {PALETTE_OPTIONS.map((option) => (
        <button
          key={option.value}
          type="button"
          onClick={() => selectTile(option.value)}
          className={`flex items-center gap-2 rounded px-3 py-2 text-sm font-medium ${
            selected === option.value ? 'bg-slate-800 text-white' : 'bg-slate-100 text-slate-700'
          }`}
        >
          <span className={`h-4 w-4 rounded ${option.swatch}`} />
          {option.label}
        </button>
      ))}
    </div>
  );
}
