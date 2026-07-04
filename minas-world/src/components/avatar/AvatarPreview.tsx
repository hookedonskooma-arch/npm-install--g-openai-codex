import { AvatarOptions } from '@/types/avatar';

const SKIN = '#fbe0c2';
const CHEEK = '#f7a8c4';
const MOUTH = '#c2554f';

function HairBack({ style, color }: { style: AvatarOptions['hairStyle']; color: string }) {
  switch (style) {
    case 'pigtails':
      return (
        <>
          <circle cx="38" cy="118" r="20" fill={color} />
          <circle cx="162" cy="118" r="20" fill={color} />
        </>
      );
    case 'curly':
      return (
        <>
          {[
            [45, 55],
            [70, 38],
            [100, 32],
            [130, 38],
            [155, 55],
            [40, 90],
            [160, 90],
          ].map(([cx, cy]) => (
            <circle key={`${cx}-${cy}`} cx={cx} cy={cy} r={22} fill={color} />
          ))}
        </>
      );
    case 'spiky':
      return (
        <>
          {[20, 55, 90, 110, 145, 180].map((angleDeg) => {
            const rad = (angleDeg * Math.PI) / 180;
            const baseX = 100 + Math.cos(rad) * 60;
            const baseY = 100 - Math.sin(rad) * 60;
            const tipX = 100 + Math.cos(rad) * 95;
            const tipY = 100 - Math.sin(rad) * 95;
            return (
              <polygon
                key={angleDeg}
                points={`${baseX - 10},${baseY} ${baseX + 10},${baseY} ${tipX},${tipY}`}
                fill={color}
              />
            );
          })}
        </>
      );
    case 'straight':
    default:
      return <ellipse cx="100" cy="95" rx="76" ry="72" fill={color} />;
  }
}

function HairFront({ style, color }: { style: AvatarOptions['hairStyle']; color: string }) {
  if (style === 'pigtails') {
    return <path d="M 34 90 Q 100 40 166 90 L 166 105 Q 100 65 34 105 Z" fill={color} />;
  }
  return <path d="M 30 100 Q 100 45 170 100 L 170 120 Q 100 78 30 120 Z" fill={color} />;
}

function Eyes({ style, color }: { style: AvatarOptions['eyeStyle']; color: string }) {
  const eyeXs = [75, 125];

  if (style === 'happy') {
    return (
      <>
        {eyeXs.map((x) => (
          <path
            key={x}
            d={`M ${x - 12} 138 Q ${x} 122 ${x + 12} 138`}
            stroke={color}
            strokeWidth={5}
            strokeLinecap="round"
            fill="none"
          />
        ))}
      </>
    );
  }

  if (style === 'wink') {
    return (
      <>
        <path d="M 63 138 Q 75 122 87 138" stroke={color} strokeWidth={5} strokeLinecap="round" fill="none" />
        <circle cx="125" cy="132" r="13" fill={color} />
        <circle cx="129" cy="127" r="4" fill="white" />
      </>
    );
  }

  return (
    <>
      {eyeXs.map((x) => (
        <g key={x}>
          <circle cx={x} cy="132" r="13" fill={color} />
          <circle cx={x + 4} cy="127" r="4" fill="white" />
          {style === 'sparkly' && <circle cx={x - 3} cy="136" r="2" fill="white" />}
        </g>
      ))}
    </>
  );
}

function Outfit({ style, color }: { style: AvatarOptions['outfitStyle']; color: string }) {
  return (
    <>
      {style === 'hoodie' && <polygon points="100,168 78,205 122,205" fill={color} opacity={0.85} />}
      <path d="M 60 175 Q 100 155 140 175 L 148 220 Q 100 235 52 220 Z" fill={color} />
      {style === 'dress' && <path d="M 45 210 Q 100 240 155 210 L 148 224 Q 100 238 52 224 Z" fill={color} opacity={0.9} />}
      {style === 'overalls' && (
        <>
          <rect x="70" y="172" width="10" height="30" fill="#4b5563" />
          <rect x="120" y="172" width="10" height="30" fill="#4b5563" />
        </>
      )}
    </>
  );
}

function AccessoryLayer({ accessory }: { accessory: AvatarOptions['accessory'] }) {
  switch (accessory) {
    case 'bow':
      return (
        <>
          <ellipse cx="82" cy="60" rx="14" ry="9" fill="#f472b6" transform="rotate(-20 82 60)" />
          <ellipse cx="112" cy="58" rx="14" ry="9" fill="#f472b6" transform="rotate(20 112 58)" />
        </>
      );
    case 'glasses':
      return (
        <g stroke="#3f3f46" strokeWidth={3} fill="none">
          <circle cx="75" cy="132" r="16" />
          <circle cx="125" cy="132" r="16" />
          <line x1="91" y1="132" x2="109" y2="132" />
        </g>
      );
    case 'hat':
      return <path d="M 40 70 Q 100 20 160 70 L 155 82 Q 100 45 45 82 Z" fill="#374151" />;
    case 'none':
    default:
      return null;
  }
}

export function AvatarPreview({ options }: { options: AvatarOptions }) {
  return (
    <svg viewBox="0 0 200 240" className="h-56 w-48" role="img" aria-label={`${options.name}'s avatar`}>
      <HairBack style={options.hairStyle} color={options.hairColor} />
      <Outfit style={options.outfitStyle} color={options.outfitColor} />
      <circle cx="100" cy="115" r="62" fill={SKIN} />
      <ellipse cx="68" cy="148" rx="10" ry="6" fill={CHEEK} />
      <ellipse cx="132" cy="148" rx="10" ry="6" fill={CHEEK} />
      <Eyes style={options.eyeStyle} color={options.eyeColor} />
      <path d="M 88 158 Q 100 168 112 158" stroke={MOUTH} strokeWidth={3} strokeLinecap="round" fill="none" />
      <HairFront style={options.hairStyle} color={options.hairColor} />
      <AccessoryLayer accessory={options.accessory} />
    </svg>
  );
}
