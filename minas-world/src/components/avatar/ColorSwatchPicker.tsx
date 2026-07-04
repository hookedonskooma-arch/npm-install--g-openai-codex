import { Swatch } from '@/lib/avatarPalette';

export function ColorSwatchPicker({
  swatches,
  selected,
  onSelect,
}: {
  swatches: Swatch[];
  selected: string;
  onSelect: (hex: string) => void;
}) {
  return (
    <div className="flex flex-wrap gap-3">
      {swatches.map((swatch) => (
        <button
          key={swatch.hex}
          type="button"
          aria-label={swatch.label}
          onClick={() => onSelect(swatch.hex)}
          style={{ backgroundColor: swatch.hex }}
          className={`h-12 w-16 rounded-2xl ${
            swatch.hex.toLowerCase() === selected.toLowerCase()
              ? 'ring-4 ring-offset-2 ring-slate-900'
              : ''
          }`}
        />
      ))}
    </div>
  );
}
