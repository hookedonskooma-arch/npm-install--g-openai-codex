export function OptionPicker<T extends string>({
  options,
  selected,
  onSelect,
}: {
  options: T[];
  selected: T;
  onSelect: (value: T) => void;
}) {
  return (
    <div className="flex flex-wrap gap-2">
      {options.map((option) => (
        <button
          key={option}
          type="button"
          onClick={() => onSelect(option)}
          className={`rounded-full px-5 py-2 text-sm font-bold capitalize ${
            option === selected
              ? 'bg-slate-900 text-white'
              : 'border border-slate-200 bg-white text-slate-600'
          }`}
        >
          {option}
        </button>
      ))}
    </div>
  );
}
