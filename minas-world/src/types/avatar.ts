export type HairStyle = 'straight' | 'curly' | 'pigtails' | 'spiky';
export type EyeStyle = 'round' | 'sparkly' | 'happy' | 'wink';
export type OutfitStyle = 'shirt' | 'dress' | 'hoodie' | 'overalls';
export type Accessory = 'none' | 'bow' | 'glasses' | 'hat';
export type AvatarTab = 'hair' | 'eyes' | 'outfit' | 'extras';

export type AvatarOptions = {
  name: string;
  hairStyle: HairStyle;
  hairColor: string;
  eyeStyle: EyeStyle;
  eyeColor: string;
  outfitStyle: OutfitStyle;
  outfitColor: string;
  accessory: Accessory;
};

export const DEFAULT_AVATAR_OPTIONS: AvatarOptions = {
  name: 'Mina',
  hairStyle: 'straight',
  hairColor: '#2e2620',
  eyeStyle: 'sparkly',
  eyeColor: '#2e2620',
  outfitStyle: 'shirt',
  outfitColor: '#d6336c',
  accessory: 'bow',
};
