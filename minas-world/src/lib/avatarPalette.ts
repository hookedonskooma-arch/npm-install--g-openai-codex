import { Accessory, EyeStyle, HairStyle, OutfitStyle } from '@/types/avatar';

export type Swatch = { hex: string; label: string };

export const HAIR_COLORS: Swatch[] = [
  { hex: '#2e2620', label: 'black' },
  { hex: '#6d4c33', label: 'brown' },
  { hex: '#7c5cff', label: 'purple' },
  { hex: '#ec4899', label: 'pink' },
  { hex: '#10b981', label: 'green' },
  { hex: '#f59e0b', label: 'orange' },
];

export const EYE_COLORS: Swatch[] = [
  { hex: '#2e2620', label: 'dark' },
  { hex: '#2563eb', label: 'blue' },
  { hex: '#10b981', label: 'green' },
  { hex: '#7c5cff', label: 'violet' },
];

export const OUTFIT_COLORS: Swatch[] = [
  { hex: '#7c5cff', label: 'purple' },
  { hex: '#d6336c', label: 'pink' },
  { hex: '#10b981', label: 'green' },
  { hex: '#f59e0b', label: 'orange' },
];

export const HAIR_STYLES: HairStyle[] = ['straight', 'curly', 'pigtails', 'spiky'];
export const EYE_STYLES: EyeStyle[] = ['round', 'sparkly', 'happy', 'wink'];
export const OUTFIT_STYLES: OutfitStyle[] = ['shirt', 'dress', 'hoodie', 'overalls'];
export const ACCESSORIES: Accessory[] = ['none', 'bow', 'glasses', 'hat'];
