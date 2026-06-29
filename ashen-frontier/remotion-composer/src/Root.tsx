import React from 'react';
import { Composition } from 'remotion';
import { AshenFrontierTrailer } from './compositions/AshenFrontierTrailer';

// Total: 1865 frames @ 30fps ≈ 62 seconds
export const RemotionRoot: React.FC = () => (
  <>
    <Composition
      id="AshenFrontierTrailer"
      component={AshenFrontierTrailer}
      durationInFrames={1865}
      fps={30}
      width={1920}
      height={1080}
    />
  </>
);
