import React from 'react';
import {
  AbsoluteFill,
  Audio,
  Img,
  interpolate,
  Sequence,
  staticFile,
  useCurrentFrame,
  useVideoConfig,
} from 'remotion';

const FPS = 30;
const CROSSFADE_FRAMES = 15; // 0.5s

// ── Scene definitions ─────────────────────────────────────────────────────────
const SCENES = [
  { id: 'establish', startFrame: 0,    duration: 240, asset: 'shot_01.jpg', motion: 'zoomIn',    amount: 0.04 },
  { id: 'cold',      startFrame: 225,  duration: 300, asset: 'shot_02.jpg', motion: 'panRight',   amount: 0.02 },
  { id: 'treeline',  startFrame: 510,  duration: 300, asset: 'shot_03.jpg', motion: 'driftLeft',  amount: 0.01 },
  { id: 'fire',      startFrame: 795,  duration: 300, asset: 'shot_04.jpg', motion: 'zoomIn',     amount: 0.06 },
  { id: 'stats',     startFrame: 1080, duration: 240, asset: null,          motion: 'static',     amount: 0    },
  { id: 'title',     startFrame: 1305, duration: 300, asset: null,          motion: 'static',     amount: 0    },
  { id: 'final',     startFrame: 1590, duration: 275, asset: 'shot_07.jpg', motion: 'static',     amount: 0    },
] as const;

// Survival stats for the HUD scene
const STATS = [
  { label: 'HUNGER',      value: 0.38, color: '#f97316' },
  { label: 'THIRST',      value: 0.22, color: '#ef4444' },
  { label: 'TEMPERATURE', value: 0.15, color: '#dc2626' },
  { label: 'STAMINA',     value: 0.61, color: '#d97706' },
] as const;

// ── Film grain overlay ────────────────────────────────────────────────────────
const FilmGrain: React.FC = () => {
  const frame = useCurrentFrame();
  // Rotate through 4 pseudo-random offsets to simulate grain flicker
  const seed = frame % 4;
  return (
    <AbsoluteFill
      style={{
        backgroundImage: `url("data:image/svg+xml,%3Csvg viewBox='0 0 200 200' xmlns='http://www.w3.org/2000/svg'%3E%3Cfilter id='n'%3E%3CfeTurbulence type='fractalNoise' baseFrequency='0.9' numOctaves='4' seed='${seed}' stitchTiles='stitch'/%3E%3C/filter%3E%3Crect width='100%25' height='100%25' filter='url(%23n)'/%3E%3C/svg%3E")`,
        backgroundSize: 'cover',
        mixBlendMode: 'overlay',
        opacity: 0.12,
        pointerEvents: 'none',
      }}
    />
  );
};

// ── Vignette overlay ─────────────────────────────────────────────────────────
const Vignette: React.FC<{ intensity?: number }> = ({ intensity = 0.40 }) => (
  <AbsoluteFill
    style={{
      background: `radial-gradient(ellipse at center, transparent 30%, rgba(0,0,0,${intensity}) 100%)`,
      pointerEvents: 'none',
    }}
  />
);

// ── Photo scene with motion ───────────────────────────────────────────────────
const PhotoScene: React.FC<{
  asset: string;
  motion: string;
  amount: number;
  localFrame: number;
  totalFrames: number;
}> = ({ asset, motion, amount, localFrame, totalFrames }) => {
  const progress = localFrame / totalFrames;

  let transform = 'scale(1)';
  if (motion === 'zoomIn') {
    const scale = 1 + amount * progress;
    transform = `scale(${scale})`;
  } else if (motion === 'panRight') {
    const tx = amount * progress * 100;
    transform = `translateX(${tx}%)`;
  } else if (motion === 'driftLeft') {
    const tx = -amount * progress * 100;
    transform = `translateX(${tx}%)`;
  }

  return (
    <AbsoluteFill style={{ overflow: 'hidden' }}>
      <Img
        src={staticFile(`images/${asset}`)}
        style={{
          width: '100%',
          height: '100%',
          objectFit: 'cover',
          transform,
          transformOrigin: 'center center',
          filter: 'saturate(0.25) contrast(1.1)',
        }}
      />
    </AbsoluteFill>
  );
};

// ── Survival HUD scene ────────────────────────────────────────────────────────
const StatsScene: React.FC<{ localFrame: number; totalFrames: number }> = ({
  localFrame,
  totalFrames,
}) => {
  return (
    <AbsoluteFill
      style={{ backgroundColor: '#000', display: 'flex', alignItems: 'center', justifyContent: 'center' }}
    >
      <div style={{ display: 'flex', flexDirection: 'column', gap: 28, width: 480 }}>
        {STATS.map((stat, i) => {
          const appearFrame = i * 30; // each bar appears 1s apart
          const opacity = interpolate(localFrame, [appearFrame, appearFrame + 15], [0, 1], {
            extrapolateLeft: 'clamp',
            extrapolateRight: 'clamp',
          });
          return (
            <div key={stat.label} style={{ opacity }}>
              <div
                style={{
                  fontFamily: 'monospace',
                  fontSize: 11,
                  letterSpacing: '0.2em',
                  color: '#E8E4DC',
                  marginBottom: 6,
                  textTransform: 'uppercase',
                }}
              >
                {stat.label}
              </div>
              <div
                style={{
                  width: '100%',
                  height: 4,
                  border: '1px solid #E8E4DC',
                  position: 'relative',
                  backgroundColor: 'transparent',
                }}
              >
                <div
                  style={{
                    position: 'absolute',
                    left: 0,
                    top: 0,
                    bottom: 0,
                    width: `${stat.value * 100}%`,
                    backgroundColor: stat.color,
                  }}
                />
              </div>
            </div>
          );
        })}
      </div>
    </AbsoluteFill>
  );
};

// ── Title card scene ──────────────────────────────────────────────────────────
const TitleScene: React.FC<{ localFrame: number; totalFrames: number }> = ({
  localFrame,
  totalFrames,
}) => {
  // Letters fade in sequentially
  const TITLE = 'ASHEN FRONTIER';
  const SUBTITLE = 'Survive until morning.';
  const msPerLetter = 80;
  const fpPerLetter = (msPerLetter / 1000) * FPS;

  const titleOpacity = interpolate(localFrame, [0, 20], [0, 1], {
    extrapolateLeft: 'clamp',
    extrapolateRight: 'clamp',
  });
  const subtitleAppear = TITLE.length * fpPerLetter + 20;
  const subtitleOpacity = interpolate(
    localFrame,
    [subtitleAppear, subtitleAppear + 25],
    [0, 1],
    { extrapolateLeft: 'clamp', extrapolateRight: 'clamp' }
  );

  // Music swell
  const musicScale = interpolate(localFrame, [0, totalFrames], [1, 1.004], {
    extrapolateLeft: 'clamp',
    extrapolateRight: 'clamp',
  });

  return (
    <AbsoluteFill
      style={{
        backgroundColor: '#000',
        display: 'flex',
        flexDirection: 'column',
        alignItems: 'center',
        justifyContent: 'center',
        gap: 20,
      }}
    >
      <div
        style={{
          opacity: titleOpacity,
          fontFamily: '"Cinzel", "Times New Roman", serif',
          fontSize: 96,
          letterSpacing: '0.25em',
          color: '#E8E4DC',
          textAlign: 'center',
          transform: `scale(${musicScale})`,
        }}
      >
        {TITLE}
      </div>
      <div
        style={{
          opacity: subtitleOpacity,
          fontFamily: '"Cinzel", "Times New Roman", serif',
          fontSize: 20,
          letterSpacing: '0.3em',
          color: '#8A8070',
          textAlign: 'center',
        }}
      >
        {SUBTITLE}
      </div>
    </AbsoluteFill>
  );
};

// ── Crossfade transition ──────────────────────────────────────────────────────
const CrossfadeIn: React.FC<{ frames: number; children: React.ReactNode }> = ({
  frames,
  children,
}) => {
  const frame = useCurrentFrame();
  const opacity = interpolate(frame, [0, frames], [0, 1], {
    extrapolateLeft: 'clamp',
    extrapolateRight: 'clamp',
  });
  return <AbsoluteFill style={{ opacity }}>{children}</AbsoluteFill>;
};

// ── Main trailer composition ──────────────────────────────────────────────────
export const AshenFrontierTrailer: React.FC = () => {
  const frame = useCurrentFrame();
  const { durationInFrames } = useVideoConfig();

  // Final fade to black
  const finalFadeStart = durationInFrames - 60;
  const globalOpacity = interpolate(frame, [finalFadeStart, durationInFrames], [1, 0], {
    extrapolateLeft: 'clamp',
    extrapolateRight: 'clamp',
  });

  return (
    <AbsoluteFill style={{ backgroundColor: '#000' }}>
      <AbsoluteFill style={{ opacity: globalOpacity }}>

        {/* Photo scenes */}
        {SCENES.filter(s => s.asset !== null).map((scene) => (
          <Sequence key={scene.id} from={scene.startFrame} durationInFrames={scene.duration + CROSSFADE_FRAMES}>
            <CrossfadeIn frames={CROSSFADE_FRAMES}>
              <PhotoScene
                asset={scene.asset as string}
                motion={scene.motion}
                amount={scene.amount}
                localFrame={Math.max(0, frame - scene.startFrame - CROSSFADE_FRAMES)}
                totalFrames={scene.duration}
              />
              <Vignette />
              <FilmGrain />
            </CrossfadeIn>
          </Sequence>
        ))}

        {/* Stats HUD scene */}
        {(() => {
          const s = SCENES.find(x => x.id === 'stats')!;
          return (
            <Sequence from={s.startFrame} durationInFrames={s.duration}>
              <CrossfadeIn frames={CROSSFADE_FRAMES}>
                <StatsScene
                  localFrame={frame - s.startFrame}
                  totalFrames={s.duration}
                />
              </CrossfadeIn>
            </Sequence>
          );
        })()}

        {/* Title scene */}
        {(() => {
          const s = SCENES.find(x => x.id === 'title')!;
          return (
            <Sequence from={s.startFrame} durationInFrames={s.duration}>
              <CrossfadeIn frames={CROSSFADE_FRAMES}>
                <TitleScene
                  localFrame={frame - s.startFrame}
                  totalFrames={s.duration}
                />
              </CrossfadeIn>
            </Sequence>
          );
        })()}

      </AbsoluteFill>

      {/* Audio tracks */}
      <Audio src={staticFile('audio/music_track.mp3')} volume={0.18} />
      <Audio src={staticFile('audio/vo_track.mp3')} volume={1.0} />
      {/* Fire crackle: shot 4 = frames 795–1080 */}
      <Sequence from={795} durationInFrames={285}>
        <Audio src={staticFile('audio/fire_crackle.mp3')} volume={0.35} loop />
      </Sequence>
      {/* Heartbeat: stats scene ~frame 1140 */}
      <Sequence from={1140} durationInFrames={90}>
        <Audio src={staticFile('audio/heartbeat.mp3')} volume={0.55} />
      </Sequence>
    </AbsoluteFill>
  );
};
