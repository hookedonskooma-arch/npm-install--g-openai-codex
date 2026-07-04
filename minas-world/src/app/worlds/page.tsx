import Link from 'next/link';
import { TileGrid } from '@/components/world/TileGrid';
import { TilePalette } from '@/components/world/TilePalette';
import { WorldToolbar } from '@/components/world/WorldToolbar';

export default function WorldsPage() {
  return (
    <main className="min-h-screen bg-[#FFF8F0] p-6">
      <div className="mb-4 flex items-center justify-between">
        <h1 className="text-2xl font-bold text-[#004F71]">Mina&apos;s World Builder</h1>
        <Link href="/play" className="text-sm font-medium text-[#004F71] underline">
          Play This World
        </Link>
      </div>
      <div className="mb-4">
        <WorldToolbar />
      </div>
      <div className="flex gap-6 overflow-x-auto">
        <TilePalette />
        <TileGrid />
      </div>
    </main>
  );
}
