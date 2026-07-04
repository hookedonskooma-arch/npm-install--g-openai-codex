import { create } from 'zustand';
import {
  Accessory,
  AvatarOptions,
  AvatarTab,
  DEFAULT_AVATAR_OPTIONS,
  EyeStyle,
  HairStyle,
  OutfitStyle,
} from '@/types/avatar';
import { supabase } from '@/lib/supabase';
import { CloudResult } from './worldStore';

const STORAGE_KEY = 'minas_avatar_local_save_v1';

type AvatarStore = {
  options: AvatarOptions;
  activeTab: AvatarTab;
  setActiveTab: (tab: AvatarTab) => void;
  setName: (name: string) => void;
  setHairStyle: (style: HairStyle) => void;
  setHairColor: (hex: string) => void;
  setEyeStyle: (style: EyeStyle) => void;
  setEyeColor: (hex: string) => void;
  setOutfitStyle: (style: OutfitStyle) => void;
  setOutfitColor: (hex: string) => void;
  setAccessory: (accessory: Accessory) => void;
  saveAvatar: () => void;
  loadAvatar: () => boolean;
  saveAvatarToCloud: () => Promise<CloudResult>;
  loadAvatarFromCloud: () => Promise<CloudResult>;
};

export const useAvatarStore = create<AvatarStore>((set, get) => ({
  options: DEFAULT_AVATAR_OPTIONS,
  activeTab: 'hair',

  setActiveTab: (tab) => set({ activeTab: tab }),
  setName: (name) => set((state) => ({ options: { ...state.options, name } })),
  setHairStyle: (hairStyle) => set((state) => ({ options: { ...state.options, hairStyle } })),
  setHairColor: (hairColor) => set((state) => ({ options: { ...state.options, hairColor } })),
  setEyeStyle: (eyeStyle) => set((state) => ({ options: { ...state.options, eyeStyle } })),
  setEyeColor: (eyeColor) => set((state) => ({ options: { ...state.options, eyeColor } })),
  setOutfitStyle: (outfitStyle) => set((state) => ({ options: { ...state.options, outfitStyle } })),
  setOutfitColor: (outfitColor) => set((state) => ({ options: { ...state.options, outfitColor } })),
  setAccessory: (accessory) => set((state) => ({ options: { ...state.options, accessory } })),

  saveAvatar: () => {
    if (typeof window === 'undefined') return;
    window.localStorage.setItem(STORAGE_KEY, JSON.stringify(get().options));
  },

  loadAvatar: () => {
    if (typeof window === 'undefined') return false;
    const raw = window.localStorage.getItem(STORAGE_KEY);
    if (!raw) return false;
    try {
      const options = JSON.parse(raw) as AvatarOptions;
      set({ options });
      return true;
    } catch {
      return false;
    }
  },

  saveAvatarToCloud: async () => {
    if (!supabase) return { ok: false, error: 'Supabase is not configured.' };
    const { data: userData, error: userError } = await supabase.auth.getUser();
    if (userError || !userData.user) return { ok: false, error: 'Not signed in.' };

    const { name, ...rest } = get().options;
    const { error } = await supabase
      .from('avatars')
      .upsert({ user_id: userData.user.id, name, options: rest }, { onConflict: 'user_id' });

    if (error) return { ok: false, error: error.message };
    return { ok: true };
  },

  loadAvatarFromCloud: async () => {
    if (!supabase) return { ok: false, error: 'Supabase is not configured.' };
    const { data: userData, error: userError } = await supabase.auth.getUser();
    if (userError || !userData.user) return { ok: false, error: 'Not signed in.' };

    const { data, error } = await supabase
      .from('avatars')
      .select('name, options')
      .eq('user_id', userData.user.id)
      .is('deleted_at', null)
      .maybeSingle();

    if (error) return { ok: false, error: error.message };
    if (!data) return { ok: false, error: 'No cloud save found.' };

    set({
      options: {
        ...DEFAULT_AVATAR_OPTIONS,
        ...(data.options as Partial<AvatarOptions>),
        name: data.name ?? DEFAULT_AVATAR_OPTIONS.name,
      },
    });
    return { ok: true };
  },
}));
