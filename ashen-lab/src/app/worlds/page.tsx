import Link from 'next/link';
import { TileGrid } from '@/components/world/TileGrid';
import { TilePalette } from '@/components/world/TilePalette';
import { WorldToolbar } from '@/components/world/WorldToolbar';

export default function WorldsPage() {
  return (
    <main className="min-h-screen bg-[#18181b] p-6">
      <div className="mb-4 flex items-center justify-between">
        <h1 className="text-2xl font-bold text-[#c4b5fd]">Ashen Frontier Lab — Biome/Base Planner</h1>
        <Link href="/play" className="text-sm font-medium text-[#c4b5fd] underline">
          Test This Biome
        </Link>
      </div>
      <div className="mb-4">
        <WorldToolbar />
      </div>
      <div className="flex gap-6">
        <TilePalette />
        <TileGrid />
      </div>
    </main>
  );
}
