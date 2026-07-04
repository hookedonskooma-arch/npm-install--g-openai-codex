const RULES = ['No free text', 'Preset-only outfits'];

export function SafetyRulesPanel() {
  return (
    <div className="mt-6 rounded-2xl bg-white p-5 shadow-sm">
      <h2 className="mb-3 text-lg font-bold text-slate-400">Safety style rules</h2>
      <div className="flex flex-col gap-3">
        {RULES.map((rule) => (
          <div key={rule} className="flex items-center justify-between">
            <span className="text-slate-400">{rule}</span>
            <span className="rounded-full bg-emerald-100 px-3 py-1 text-xs font-bold text-emerald-700">
              On
            </span>
          </div>
        ))}
      </div>
    </div>
  );
}
