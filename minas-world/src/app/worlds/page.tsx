import { TileGrid } from '@/components/world/TileGrid';
import { TilePalette } from '@/components/world/TilePalette';
import { WorldToolbar } from '@/components/world/WorldToolbar';

export default function WorldsPage() {
  return (
    <main className="min-h-screen bg-[#FFF8F0] p-6">
      <h1 className="mb-4 text-2xl font-bold text-[#004F71]">Mina&apos;s World Builder</h1>
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
