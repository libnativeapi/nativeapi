import { cn } from '@/lib/utils'
import {
  accentBlue,
  accentGreen,
  accentOrange,
  bg,
  border,
  BrandMark,
  Code,
  CodeGlyph,
  fg,
  Label,
  muted,
  mutedFg,
  primary,
  SoftShadow,
} from './illustration-primitives'

/* ------------------------------------------------------------------ */
/*  Hero — a desktop: an editor cycling the same four calls through    */
/*  all six bindings, and the native window those calls produce. The   */
/*  language tab, the code, the active chip and the window's caption   */
/*  switch together on one discrete SMIL clock; without SMIL the       */
/*  TypeScript frame stays put.                                        */
/* ------------------------------------------------------------------ */

type Token = [text: string, kind: 'kw' | 'type' | 'fn' | 'str' | 'p']

const languages: {
  ext: string
  name: string
  file: string
  color: string
  lines: Token[][]
}[] = [
  {
    ext: 'ts',
    name: 'TypeScript',
    file: 'main.ts',
    color: '#D9A400',
    lines: [
      [['const ', 'kw'], ['win = ', 'p'], ['Window', 'type'], ['.', 'p'], ['create', 'fn'], ['()!', 'p']],
      [['win.', 'p'], ['setTitle', 'fn'], ['(', 'p'], ['"Hello"', 'str'], [')', 'p']],
      [['win.', 'p'], ['center', 'fn'], ['()', 'p']],
      [['win.', 'p'], ['show', 'fn'], ['()', 'p']],
    ],
  },
  {
    ext: 'dart',
    name: 'Dart',
    file: 'main.dart',
    color: '#0175C2',
    lines: [
      [['final ', 'kw'], ['win = ', 'p'], ['Window', 'type'], ['.', 'p'], ['create', 'fn'], ['()!;', 'p']],
      [['win.title = ', 'p'], ["'Hello'", 'str'], [';', 'p']],
      [['win.', 'p'], ['center', 'fn'], ['();', 'p']],
      [['win.', 'p'], ['show', 'fn'], ['();', 'p']],
    ],
  },
  {
    ext: 'rs',
    name: 'Rust',
    file: 'main.rs',
    color: '#CE422B',
    lines: [
      [['let ', 'kw'], ['win = ', 'p'], ['Window', 'type'], ['::', 'p'], ['new', 'fn'], ['().unwrap();', 'p']],
      [['win.', 'p'], ['set_title', 'fn'], ['(', 'p'], ['"Hello"', 'str'], [');', 'p']],
      [['win.', 'p'], ['center', 'fn'], ['();', 'p']],
      [['win.', 'p'], ['show', 'fn'], ['();', 'p']],
    ],
  },
  {
    ext: 'cs',
    name: 'C#',
    file: 'Program.cs',
    color: '#512BD4',
    lines: [
      [['var ', 'kw'], ['win = ', 'p'], ['Window', 'type'], ['.', 'p'], ['Create', 'fn'], ['()!;', 'p']],
      [['win.', 'p'], ['SetTitle', 'fn'], ['(', 'p'], ['"Hello"', 'str'], [');', 'p']],
      [['win.', 'p'], ['Center', 'fn'], ['();', 'p']],
      [['win.', 'p'], ['Show', 'fn'], ['();', 'p']],
    ],
  },
  {
    ext: 'py',
    name: 'Python',
    file: 'main.py',
    color: '#3776AB',
    lines: [
      [['win = ', 'p'], ['Window', 'type'], ['()', 'p']],
      [['win.title = ', 'p'], ['"Hello"', 'str']],
      [['win.', 'p'], ['center', 'fn'], ['()', 'p']],
      [['win.', 'p'], ['show', 'fn'], ['()', 'p']],
    ],
  },
  {
    ext: 'go',
    name: 'Go',
    file: 'main.go',
    color: '#00ADD8',
    lines: [
      [['win, _ := ', 'p'], ['NewWindow', 'fn'], ['()', 'p']],
      [['win.', 'p'], ['SetTitle', 'fn'], ['(', 'p'], ['"Hello"', 'str'], [')', 'p']],
      [['win.', 'p'], ['Center', 'fn'], ['()', 'p']],
      [['win.', 'p'], ['Show', 'fn'], ['()', 'p']],
    ],
  },
]

const STEP_SECONDS = 2.6
const CYCLE = `${STEP_SECONDS * languages.length}s`
const KEY_TIMES = languages.map((_, i) => (i / languages.length).toFixed(4)).join(';')

/** Discrete on/off track: visible during step `index` of the cycle. */
function Cycle({ index, attribute = 'opacity' }: { index: number, attribute?: string }) {
  return (
    <animate
      attributeName={attribute}
      values={languages.map((_, i) => (i === index ? 1 : 0)).join(';')}
      keyTimes={KEY_TIMES}
      calcMode="discrete"
      dur={CYCLE}
      repeatCount="indefinite"
    />
  )
}

const tokenStyle: Record<Token[1], { fill: string, opacity: number }> = {
  kw: { fill: accentBlue, opacity: 0.95 },
  type: { fill: accentOrange, opacity: 0.95 },
  fn: { fill: primary, opacity: 1 },
  str: { fill: accentGreen, opacity: 0.95 },
  p: { fill: fg, opacity: 0.72 },
}

const trafficLights = ['#ff5f57', '#febc2e', '#28c840']

export function HeroIllustration({ className }: { className?: string }) {
  const desk = { x: 16, y: 48, w: 768, h: 396 }
  const editor = { x: 44, y: 116, w: 384, h: 206 }
  const win = { x: 474, y: 154, w: 282, h: 206 }
  const lineY = (i: number) => editor.y + 72 + i * 30
  const chipW = 56
  const chipGap = 9.6
  const titleY = win.y + 19
  const connector = `M${editor.x + editor.w} ${lineY(1) - 4} C ${editor.x + editor.w + 30} ${lineY(1) - 4}, ${win.x - 30} ${titleY}, ${win.x} ${titleY}`

  return (
    <div className={cn('relative h-full w-full select-none', className)}>
      <svg
        viewBox="0 0 800 500"
        className="size-full"
        fill="none"
        xmlns="http://www.w3.org/2000/svg"
        aria-hidden="true"
      >
        <defs>
          <linearGradient id="hero-wallpaper" x1="0" y1="0" x2="1" y2="1">
            <stop offset="0" stopColor={primary} stopOpacity={0.1} />
            <stop offset=".55" stopColor={primary} stopOpacity={0.03} />
            <stop offset="1" stopColor="#06B6D4" stopOpacity={0.1} />
          </linearGradient>
        </defs>

        {/* desktop */}
        <SoftShadow x={desk.x} y={desk.y} w={desk.w} h={desk.h} rx={20} />
        <rect x={desk.x} y={desk.y} width={desk.w} height={desk.h} rx={20} fill={bg} stroke={border} />
        <rect x={desk.x} y={desk.y} width={desk.w} height={desk.h} rx={20} fill="url(#hero-wallpaper)" />

        {/* menu bar with the app's tray icon */}
        <path
          d={`M${desk.x} ${desk.y + 20} a20 20 0 0 1 20 -20 h${desk.w - 40} a20 20 0 0 1 20 20 v12 h-${desk.w} Z`}
          fill={bg}
          fillOpacity={0.75}
        />
        <line x1={desk.x} y1={desk.y + 32} x2={desk.x + desk.w} y2={desk.y + 32} stroke={border} />
        <Label x={desk.x + 24} y={desk.y + 21} size={11.5} weight={700} anchor="start" opacity={0.85}>
          Hello
        </Label>
        {['File', 'Edit', 'Window'].map((item, i) => (
          <Label key={item} x={desk.x + 70 + i * 42} y={desk.y + 21} size={11.5} weight={500} anchor="start" opacity={0.6}>
            {item}
          </Label>
        ))}
        {[0, 1].map(i => (
          <rect key={i} x={desk.x + desk.w - 150 + i * 24} y={desk.y + 10} width={13} height={13} rx={4} fill={mutedFg} fillOpacity={0.3} />
        ))}
        <rect x={desk.x + desk.w - 100} y={desk.y + 5} width={28} height={22} rx={6} fill={primary} fillOpacity={0.16} />
        <CodeGlyph cx={desk.x + desk.w - 86} cy={desk.y + 16} scale={0.62} stroke={primary} strokeWidth={2.2} />
        <Label x={desk.x + desk.w - 22} y={desk.y + 21} size={11.5} weight={600} anchor="end" opacity={0.75}>
          9:41
        </Label>

        {/* connector: setTitle → the native window's title bar */}
        <path d={connector} stroke={primary} strokeOpacity={0.25} strokeWidth={2} strokeLinecap="round" />
        <path d={connector} stroke={primary} strokeWidth={2} strokeLinecap="round" strokeDasharray="3 8">
          <animate attributeName="stroke-dashoffset" values="22;0" dur="1.1s" repeatCount="indefinite" />
        </path>

        {/* editor */}
        <SoftShadow x={editor.x} y={editor.y} w={editor.w} h={editor.h} rx={12} />
        <rect x={editor.x} y={editor.y} width={editor.w} height={editor.h} rx={12} fill={bg} stroke={border} />
        <path
          d={`M${editor.x} ${editor.y + 12} a12 12 0 0 1 12 -12 h${editor.w - 24} a12 12 0 0 1 12 12 v24 h-${editor.w} Z`}
          fill={muted}
          fillOpacity={0.7}
        />
        <line x1={editor.x} y1={editor.y + 36} x2={editor.x + editor.w} y2={editor.y + 36} stroke={border} />
        {trafficLights.map((color, i) => (
          <circle key={color} cx={editor.x + 18 + i * 15} cy={editor.y + 18} r={4.5} fill={color} />
        ))}
        {/* active file tab */}
        <rect x={editor.x + 72} y={editor.y + 6} width={108} height={30} rx={7} fill={bg} />
        <rect x={editor.x + 72} y={editor.y + 30} width={108} height={8} fill={bg} />
        {languages.map((lang, index) => (
          <g key={lang.ext} opacity={index === 0 ? 1 : 0}>
            <Cycle index={index} />
            <circle cx={editor.x + 86} cy={editor.y + 22} r={4} fill={lang.color} />
            <Code x={editor.x + 96} y={editor.y + 25.5} size={10.5} opacity={0.85}>
              {lang.file}
            </Code>
          </g>
        ))}
        {/* gutter */}
        {[0, 1, 2, 3].map(i => (
          <Code key={i} x={editor.x + 26} y={lineY(i)} size={10.5} fill={mutedFg} opacity={0.6} anchor="end">
            {i + 1}
          </Code>
        ))}
        <rect x={editor.x + 36} y={lineY(1) - 15} width={editor.w - 44} height={22} rx={5} fill={primary} fillOpacity={0.07} />
        {/* code, one frame per language */}
        {languages.map((lang, index) => (
          <g key={lang.ext} opacity={index === 0 ? 1 : 0}>
            <Cycle index={index} />
            {lang.lines.map((tokens, i) => (
              <Code key={i} x={editor.x + 46} y={lineY(i)} size={12} opacity={1}>
                {tokens.map(([text, kind], j) => (
                  <tspan key={j} fill={tokenStyle[kind].fill} fillOpacity={tokenStyle[kind].opacity}>
                    {text}
                  </tspan>
                ))}
              </Code>
            ))}
          </g>
        ))}

        {/* binding chips */}
        {languages.map((lang, index) => {
          const x = editor.x + index * (chipW + chipGap)
          const y = editor.y + editor.h + 22
          return (
            <g key={lang.ext}>
              <rect x={x} y={y} width={chipW} height={28} rx={14} fill={bg} stroke={border} />
              <rect
                x={x}
                y={y}
                width={chipW}
                height={28}
                rx={14}
                fill={lang.color}
                fillOpacity={0.14}
                stroke={lang.color}
                strokeOpacity={0.7}
                opacity={index === 0 ? 1 : 0}
              >
                <Cycle index={index} />
              </rect>
              <Code x={x + chipW / 2} y={y + 18} size={11} fill={lang.color} opacity={1} anchor="middle">
                .{lang.ext}
              </Code>
            </g>
          )
        })}

        {/* the native window */}
        <SoftShadow x={win.x} y={win.y} w={win.w} h={win.h} rx={12} />
        <rect x={win.x} y={win.y} width={win.w} height={win.h} rx={12} fill={bg} stroke={border} />
        <path
          d={`M${win.x} ${win.y + 12} a12 12 0 0 1 12 -12 h${win.w - 24} a12 12 0 0 1 12 12 v26 h-${win.w} Z`}
          fill={muted}
          fillOpacity={0.7}
        />
        <line x1={win.x} y1={win.y + 38} x2={win.x + win.w} y2={win.y + 38} stroke={border} />
        {trafficLights.map((color, i) => (
          <circle key={color} cx={win.x + 18 + i * 15} cy={titleY} r={4.5} fill={color} />
        ))}
        <Label x={win.x + win.w / 2} y={titleY + 4.5} size={12.5} weight={600} opacity={0.85}>
          Hello
        </Label>
        {/* pulse on the title as each frame lands */}
        <rect x={win.x + win.w / 2 - 34} y={titleY - 11} width={68} height={22} rx={7} fill="none" stroke={primary} strokeWidth={1.5}>
          <animate attributeName="stroke-opacity" values="0.9;0;0" keyTimes="0;0.35;1" dur={`${STEP_SECONDS}s`} repeatCount="indefinite" />
        </rect>
        <Label x={win.x + 24} y={win.y + 82} size={11} weight={500} fill={mutedFg} anchor="start">
          Created from
        </Label>
        {languages.map((lang, index) => (
          <g key={lang.ext} opacity={index === 0 ? 1 : 0}>
            <Cycle index={index} />
            <Label x={win.x + 24} y={win.y + 110} size={22} weight={700} anchor="start" fill={lang.color} opacity={1}>
              {lang.name}
            </Label>
          </g>
        ))}
        <line x1={win.x + 24} y1={win.y + 134} x2={win.x + win.w - 24} y2={win.y + 134} stroke={border} />
        {(
          [
            { icon: 'apple', label: 'AppKit' },
            { icon: 'windows', label: 'Win32' },
            { icon: 'linux', label: 'GTK' },
          ] as const
        ).map((platform, i) => {
          const cx = win.x + 24 + i * 84
          return (
            <g key={platform.label}>
              <rect x={cx} y={win.y + 150} width={74} height={32} rx={9} fill={muted} fillOpacity={0.6} />
              <BrandMark icon={platform.icon} cx={cx + 17} cy={win.y + 166} scale={0.62} />
              <Label x={cx + 30} y={win.y + 170} size={10.5} weight={600} anchor="start" opacity={0.75}>
                {platform.label}
              </Label>
            </g>
          )
        })}
        <Code x={win.x + win.w / 2} y={win.y + win.h + 28} size={10.5} fill={mutedFg} opacity={0.85} anchor="middle">
          → native_window_set_title()
        </Code>

        {/* ambient accents */}
        <circle cx={714} cy={404} r={40} fill={primary} fillOpacity={0.06} />
        <circle cx={456} cy={410} r={22} fill={primary} fillOpacity={0.05} />
      </svg>
    </div>
  )
}
