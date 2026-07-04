'use client';

import { useState } from 'react';
import { useAvatarStore } from '@/store/avatarStore';
import { AvatarPreview } from '@/components/avatar/AvatarPreview';
import { OptionPicker } from '@/components/avatar/OptionPicker';
import { ColorSwatchPicker } from '@/components/avatar/ColorSwatchPicker';
import { SafetyRulesPanel } from '@/components/avatar/SafetyRulesPanel';
import {
  ACCESSORIES,
  EYE_COLORS,
  EYE_STYLES,
  HAIR_COLORS,
  HAIR_STYLES,
  OUTFIT_COLORS,
  OUTFIT_STYLES,
} from '@/lib/avatarPalette';
import { AvatarTab } from '@/types/avatar';

const TABS: AvatarTab[] = ['hair', 'eyes', 'outfit', 'extras'];

export default function StudioPage() {
  const {
    options,
    activeTab,
    setActiveTab,
    setName,
    setHairStyle,
    setHairColor,
    setEyeStyle,
    setEyeColor,
    setOutfitStyle,
    setOutfitColor,
    setAccessory,
    saveAvatar,
    loadAvatar,
    saveAvatarToCloud,
    loadAvatarFromCloud,
  } = useAvatarStore();
  const [message, setMessage] = useState<string | null>(null);

  return (
    <main className="min-h-screen bg-[#FFF8F0] p-6">
      <div className="mb-4 flex items-center justify-between">
        <div className="inline-flex items-center gap-2 rounded-full border border-slate-200 bg-white px-4 py-2">
          <span className="h-6 w-6 rounded-full bg-gradient-to-br from-purple-300 to-purple-500" />
          <span className="font-bold text-slate-800">Avatar Studio</span>
        </div>
      </div>

      <div className="rounded-3xl bg-white p-6 shadow-sm">
        <div className="mb-6 flex flex-wrap gap-2">
          {TABS.map((tab) => (
            <button
              key={tab}
              type="button"
              onClick={() => setActiveTab(tab)}
              className={`rounded-full px-5 py-2 font-bold capitalize ${
                tab === activeTab ? 'bg-slate-900 text-white' : 'border border-slate-200 text-slate-500'
              }`}
            >
              {tab}
            </button>
          ))}
        </div>

        <div className="mb-6 flex justify-center">
          <AvatarPreview options={options} />
        </div>

        <input
          value={options.name}
          onChange={(event) => setName(event.target.value)}
          placeholder="Name"
          maxLength={20}
          className="mb-6 w-full rounded-2xl border border-slate-200 px-4 py-3 font-bold text-slate-800"
        />

        {activeTab === 'hair' && (
          <div className="flex flex-col gap-4">
            <ColorSwatchPicker swatches={HAIR_COLORS} selected={options.hairColor} onSelect={setHairColor} />
            <OptionPicker options={HAIR_STYLES} selected={options.hairStyle} onSelect={setHairStyle} />
          </div>
        )}

        {activeTab === 'eyes' && (
          <div className="flex flex-col gap-4">
            <ColorSwatchPicker swatches={EYE_COLORS} selected={options.eyeColor} onSelect={setEyeColor} />
            <OptionPicker options={EYE_STYLES} selected={options.eyeStyle} onSelect={setEyeStyle} />
          </div>
        )}

        {activeTab === 'outfit' && (
          <div className="flex flex-col gap-4">
            <ColorSwatchPicker swatches={OUTFIT_COLORS} selected={options.outfitColor} onSelect={setOutfitColor} />
            <OptionPicker options={OUTFIT_STYLES} selected={options.outfitStyle} onSelect={setOutfitStyle} />
          </div>
        )}

        {activeTab === 'extras' && (
          <div className="flex flex-col gap-4">
            <OptionPicker options={ACCESSORIES} selected={options.accessory} onSelect={setAccessory} />
          </div>
        )}

        <div className="mt-6 flex flex-wrap items-center gap-3">
          <button
            type="button"
            onClick={() => {
              saveAvatar();
              setMessage('Avatar saved!');
            }}
            className="rounded-full bg-emerald-600 px-4 py-2 text-sm font-semibold text-white hover:bg-emerald-700"
          >
            Save Avatar
          </button>
          <button
            type="button"
            onClick={() => {
              const loaded = loadAvatar();
              setMessage(loaded ? 'Avatar loaded!' : 'No saved avatar found.');
            }}
            className="rounded-full bg-sky-600 px-4 py-2 text-sm font-semibold text-white hover:bg-sky-700"
          >
            Load Avatar
          </button>
          <button
            type="button"
            onClick={async () => {
              const result = await saveAvatarToCloud();
              setMessage(result.ok ? 'Avatar saved to Supabase!' : `Cloud save failed: ${result.error}`);
            }}
            className="rounded-full bg-[#00B398] px-4 py-2 text-sm font-semibold text-white hover:bg-[#009480]"
          >
            Save to Cloud
          </button>
          <button
            type="button"
            onClick={async () => {
              const result = await loadAvatarFromCloud();
              setMessage(result.ok ? 'Avatar loaded from Supabase!' : `Cloud load failed: ${result.error}`);
            }}
            className="rounded-full bg-[#F2A900] px-4 py-2 text-sm font-semibold text-white hover:bg-[#cf9000]"
          >
            Load from Cloud
          </button>
          {message && <span className="text-sm text-slate-600">{message}</span>}
        </div>
      </div>

      <SafetyRulesPanel />
    </main>
  );
}
