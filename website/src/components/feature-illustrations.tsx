import type { BrandIcon } from './illustration-primitives'
import {
  accentAmber,
  accentBlue,
  accentGreen,
  accentOrange,
  accentPink,
  bg,
  border,
  BrandMark,
  CheckBadge,
  CircleShadow,
  Code,
  CodeGlyph,
  FileGlyph,
  Flow,
  fg,
  GearGlyph,
  IllustrationFrame,
  Label,
  muted,
  mutedFg,
  primary,
  primaryFg,
  round2,
  SoftShadow,
} from './illustration-primitives'

const trafficLights = ['#ff5f57', '#febc2e', '#28c840']

/* ------------------------------------------------------------------ */
/*  1. Windows — a window with a hidden title bar, its drag-to-move   */
/*  and drag-to-resize areas, and a floating vibrancy panel on top.   */
/* ------------------------------------------------------------------ */

export function WindowsIllustration({ className }: { className?: string }) {
  const win = { x: 70, y: 92, w: 430, h: 300 }
  const panel = { x: 430, y: 236, w: 214, h: 150 }
  return (
    <IllustrationFrame className={className}>
      {/* drag-to-resize outline around the main window */}
      <rect
        x={win.x - 12}
        y={win.y - 12}
        width={win.w + 24}
        height={win.h + 24}
        rx={20}
        fill="none"
        stroke={primary}
        strokeOpacity={0.45}
        strokeWidth={1.5}
        strokeDasharray="4 6"
      />
      <path
        d={`M${win.x + win.w + 2} ${win.y + win.h + 22} l20 20 m0 -12 v12 h-12`}
        fill="none"
        stroke={primary}
        strokeWidth={2.2}
        strokeLinecap="round"
        strokeLinejoin="round"
      />
      <Code x={win.x - 12} y={win.y + win.h + 40} size={10.5} fill={primary} opacity={0.9}>
        DragToResizeArea
      </Code>

      {/* main window */}
      <SoftShadow x={win.x} y={win.y} w={win.w} h={win.h} rx={14} />
      <rect x={win.x} y={win.y} width={win.w} height={win.h} rx={14} fill={bg} stroke={border} />
      {/* hidden title bar: traffic lights over the content, drag area */}
      <rect
        x={win.x + 76}
        y={win.y + 10}
        width={win.w - 88}
        height={30}
        rx={8}
        fill={primary}
        fillOpacity={0.07}
        stroke={primary}
        strokeOpacity={0.45}
        strokeDasharray="4 5"
      />
      <Code x={win.x + 92} y={win.y + 29} size={10.5} fill={primary} opacity={0.9}>
        DragToMoveArea
      </Code>
      {trafficLights.map((color, i) => (
        <circle key={color} cx={win.x + 22 + i * 18} cy={win.y + 25} r={6} fill={color} />
      ))}
      {/* body */}
      <Label x={win.x + 28} y={win.y + 86} size={20} weight={700} anchor="start" opacity={0.9}>
        My window
      </Label>
      <Code x={win.x + 28} y={win.y + 120} size={11} opacity={0.75}>
        <tspan fill={accentBlue}>window</tspan>
        <tspan>.setTitleBarStyle(</tspan>
        <tspan fill={accentOrange}>Hidden</tspan>
        <tspan>)</tspan>
      </Code>
      <Code x={win.x + 28} y={win.y + 142} size={11} opacity={0.75}>
        <tspan fill={accentBlue}>window</tspan>
        <tspan>.setMinimumSize(</tspan>
        <tspan fill={accentOrange}>400, 300</tspan>
        <tspan>)</tspan>
      </Code>
      <rect x={win.x + 28} y={win.y + 176} width={150} height={88} rx={10} fill={muted} />
      <rect x={win.x + 192} y={win.y + 176} width={150} height={88} rx={10} fill={muted} />

      {/* bounds readout */}
      <rect x={win.x} y={44} width={196} height={30} rx={15} fill={bg} stroke={border} />
      <circle cx={win.x + 18} cy={59} r={4.5} fill={accentGreen} />
      <Code x={win.x + 30} y={62.5} size={10} opacity={0.75}>
        bounds 120, 80 · 860×600
      </Code>

      {/* floating vibrancy panel */}
      <SoftShadow x={panel.x} y={panel.y} w={panel.w} h={panel.h} rx={14} />
      <rect
        x={panel.x}
        y={panel.y}
        width={panel.w}
        height={panel.h}
        rx={14}
        fill={bg}
        fillOpacity={0.82}
        stroke={border}
      />
      <rect
        x={panel.x}
        y={panel.y}
        width={panel.w}
        height={panel.h}
        rx={14}
        fill={primary}
        fillOpacity={0.08}
      />
      {trafficLights.map((color, i) => (
        <circle key={color} cx={panel.x + 18 + i * 14} cy={panel.y + 18} r={4.5} fill={color} />
      ))}
      <Label x={panel.x + 20} y={panel.y + 60} size={13} anchor="start" opacity={0.9}>
        Visual effect
      </Label>
      <Code x={panel.x + 20} y={panel.y + 80} size={10} fill={mutedFg} opacity={0.9}>
        vibrancy · always on top
      </Code>
      {[0, 1, 2, 3].map(i => (
        <rect
          key={i}
          x={panel.x + 20 + i * 46}
          y={panel.y + 100}
          width={36}
          height={30}
          rx={8}
          fill={bg}
          fillOpacity={0.7}
          stroke={border}
        />
      ))}

      {/* ambient accents */}
      <circle cx={640} cy={110} r={46} fill={primary} fillOpacity={0.06} />
      <circle cx={110} cy={470} r={40} fill={primary} fillOpacity={0.05} />
    </IllustrationFrame>
  )
}

/* ------------------------------------------------------------------ */
/*  2. Tray icons & menus — a menu bar with the app's status item     */
/*  and its native menu, plus where it lives on each platform.        */
/* ------------------------------------------------------------------ */

export function TrayMenusIllustration({ className }: { className?: string }) {
  const tray = { cx: 520, cy: 70 }
  const menu = { x: 410, y: 102, w: 230, rowH: 30 }
  const items: (
    | { type: 'item', label: string, shortcut?: string, check?: boolean, sub?: boolean, active?: boolean }
    | { type: 'separator' }
  )[] = [
    { type: 'item', label: 'Show window', shortcut: '⌘O' },
    { type: 'item', label: 'Preferences…', shortcut: '⌘,' },
    { type: 'separator' },
    { type: 'item', label: 'Launch at login', check: true },
    { type: 'item', label: 'Theme', sub: true, active: true },
    { type: 'separator' },
    { type: 'item', label: 'Quit', shortcut: '⌘Q' },
  ]
  let y = menu.y + 10
  const rows = items.map(item => {
    const top = y
    y += item.type === 'separator' ? 11 : menu.rowH
    return { item, top }
  })
  const menuH = y - menu.y + 10
  const themeRow = rows.find(row => row.item.type === 'item' && row.item.sub)!
  const submenu = { x: menu.x - 174, y: themeRow.top - 10, w: 180 }
  const platforms: { icon: BrandIcon, label: string, cx: number }[] = [
    { icon: 'apple', label: 'Menu bar', cx: 180 },
    { icon: 'windows', label: 'Notification area', cx: 360 },
    { icon: 'linux', label: 'StatusNotifier', cx: 540 },
  ]
  return (
    <IllustrationFrame className={className}>
      {/* menu bar */}
      <rect x={40} y={50} width={640} height={40} rx={10} fill={bg} stroke={border} />
      <BrandMark icon="apple" cx={66} cy={70} scale={0.7} fill={fg} opacity={0.75} />
      <Label x={88} y={74.5} size={12} weight={700} anchor="start" opacity={0.85}>
        MyApp
      </Label>
      {['File', 'Edit', 'View'].map((label, i) => (
        <Label key={label} x={146 + i * 46} y={74.5} size={12} weight={500} anchor="start" opacity={0.7}>
          {label}
        </Label>
      ))}
      {[0, 1].map(i => (
        <rect key={i} x={448 + i * 26} y={63} width={14} height={14} rx={4} fill={mutedFg} fillOpacity={0.35} />
      ))}
      {/* our tray icon, highlighted while its menu is open */}
      <rect x={tray.cx - 16} y={56} width={32} height={28} rx={7} fill={primary} fillOpacity={0.16} />
      <CodeGlyph cx={tray.cx} cy={tray.cy} scale={0.75} stroke={primary} strokeWidth={2.2} />
      <Label x={652} y={74.5} size={12} weight={600} anchor="end" opacity={0.8}>
        9:41
      </Label>

      {/* tray menu */}
      <SoftShadow x={menu.x} y={menu.y} w={menu.w} h={menuH} rx={12} />
      <rect x={menu.x} y={menu.y} width={menu.w} height={menuH} rx={12} fill={bg} stroke={border} />
      {rows.map(({ item, top }, i) => {
        if (item.type === 'separator') {
          return (
            <line key={i} x1={menu.x + 12} y1={top + 5.5} x2={menu.x + menu.w - 12} y2={top + 5.5} stroke={border} />
          )
        }
        const textY = top + menu.rowH / 2 + 4.5
        return (
          <g key={i}>
            {item.active
              ? <rect x={menu.x + 6} y={top} width={menu.w - 12} height={menu.rowH} rx={7} fill={primary} />
              : null}
            {item.check
              ? (
                  <path
                    d={`M${menu.x + 18} ${top + 15} l4 4 8 -8`}
                    fill="none"
                    stroke={fg}
                    strokeOpacity={0.8}
                    strokeWidth={2}
                    strokeLinecap="round"
                    strokeLinejoin="round"
                  />
                )
              : null}
            <Label
              x={menu.x + 38}
              y={textY}
              size={12}
              weight={500}
              anchor="start"
              fill={item.active ? primaryFg : fg}
              opacity={item.active ? 1 : 0.85}
            >
              {item.label}
            </Label>
            {item.shortcut
              ? (
                  <Label x={menu.x + menu.w - 18} y={textY} size={11.5} weight={500} anchor="end" fill={mutedFg}>
                    {item.shortcut}
                  </Label>
                )
              : null}
            {item.sub
              ? (
                  <path
                    d={`M${menu.x + menu.w - 24} ${top + 10} l5 5 -5 5`}
                    fill="none"
                    stroke={item.active ? primaryFg : mutedFg}
                    strokeWidth={2}
                    strokeLinecap="round"
                    strokeLinejoin="round"
                  />
                )
              : null}
          </g>
        )
      })}

      {/* submenu opened to the left */}
      <SoftShadow x={submenu.x} y={submenu.y} w={submenu.w} h={110} rx={12} />
      <rect x={submenu.x} y={submenu.y} width={submenu.w} height={110} rx={12} fill={bg} stroke={border} />
      {['Light', 'Dark', 'System'].map((label, i) => (
        <g key={label}>
          {i === 1 ? <circle cx={submenu.x + 22} cy={submenu.y + 26 + i * 30} r={4} fill={fg} fillOpacity={0.8} /> : null}
          <Label x={submenu.x + 38} y={submenu.y + 30.5 + i * 30} size={12} weight={500} anchor="start" opacity={0.85}>
            {label}
          </Label>
        </g>
      ))}

      {/* where the icon lives on each platform */}
      <line x1={80} y1={404} x2={640} y2={404} stroke={border} />
      {platforms.map(platform => (
        <g key={platform.label}>
          <CircleShadow cx={platform.cx} cy={448} r={22} />
          <circle cx={platform.cx} cy={448} r={22} fill={bg} stroke={border} />
          <BrandMark icon={platform.icon} cx={platform.cx} cy={448} scale={0.85} />
          <Label x={platform.cx} y={490} size={10.5} fill={mutedFg} weight={500}>
            {platform.label}
          </Label>
        </g>
      ))}
    </IllustrationFrame>
  )
}

/* ------------------------------------------------------------------ */
/*  3. Displays & input — two monitors with real geometry, a cursor   */
/*  crossing between them, and a global shortcut firing.              */
/* ------------------------------------------------------------------ */

export function DisplaysInputIllustration({ className }: { className?: string }) {
  const screens = [
    { x: 56, y: 70, w: 330, h: 196, name: 'DELL U2723QE', detail: '2560 × 1440 · 1×', primary: false },
    { x: 424, y: 118, w: 236, h: 148, name: 'Built-in Retina', detail: '1512 × 982 · 2×', primary: true },
  ]
  const keys = [
    { label: '⌘', w: 52 },
    { label: '⇧', w: 52 },
    { label: 'Space', w: 120 },
  ]
  let keyX = 110
  const keycaps = keys.map(key => {
    const x = keyX
    keyX += key.w + 14
    return { ...key, x }
  })
  return (
    <IllustrationFrame className={className}>
      {screens.map(screen => (
        <g key={screen.name}>
          {/* stand */}
          <path
            d={`M${screen.x + screen.w / 2 - 10} ${screen.y + screen.h} v26 M${screen.x + screen.w / 2 - 40} ${screen.y + screen.h + 28} h80`}
            stroke={mutedFg}
            strokeOpacity={0.45}
            strokeWidth={4}
            strokeLinecap="round"
          />
          <SoftShadow x={screen.x} y={screen.y} w={screen.w} h={screen.h} rx={12} />
          <rect x={screen.x} y={screen.y} width={screen.w} height={screen.h} rx={12} fill={bg} stroke={border} strokeWidth={1.5} />
          {/* menu bar + work area */}
          <rect x={screen.x + 8} y={screen.y + 8} width={screen.w - 16} height={12} rx={4} fill={muted} />
          <rect
            x={screen.x + 8}
            y={screen.y + 26}
            width={screen.w - 16}
            height={screen.h - 34}
            rx={6}
            fill={primary}
            fillOpacity={0.05}
            stroke={primary}
            strokeOpacity={0.35}
            strokeDasharray="4 5"
          />
          <Label x={screen.x + 22} y={screen.y + 56} size={13} anchor="start" opacity={0.9}>
            {screen.name}
          </Label>
          <Code x={screen.x + 22} y={screen.y + 76} size={10.5} fill={mutedFg} opacity={0.9}>
            {screen.detail}
          </Code>
          <Code x={screen.x + screen.w - 20} y={screen.y + screen.h - 18} size={9.5} fill={primary} opacity={0.8} anchor="end">
            work area
          </Code>
          {screen.primary
            ? (
                <g>
                  <rect x={screen.x + 22} y={screen.y + 90} width={64} height={22} rx={11} fill={primary} fillOpacity={0.12} />
                  <Label x={screen.x + 54} y={screen.y + 105} size={10} fill={primary} weight={600} opacity={1}>
                    primary
                  </Label>
                </g>
              )
            : null}
        </g>
      ))}

      {/* cursor crossing displays */}
      <Flow d="M300 200 C 380 150, 480 170, 572 214" />
      <path
        d="M576 212 l0 22 6 -6 4 9 4 -2 -4 -9 8 0 Z"
        fill={bg}
        stroke={fg}
        strokeOpacity={0.85}
        strokeWidth={1.6}
        strokeLinejoin="round"
      />

      {/* global shortcut */}
      {keycaps.map(key => (
        <g key={key.label}>
          <rect x={key.x} y={384} width={key.w} height={48} rx={10} fill={muted} />
          <rect x={key.x} y={380} width={key.w} height={46} rx={10} fill={bg} stroke={border} />
          <Label x={key.x + key.w / 2} y={409} size={key.label.length > 1 ? 13 : 17} weight={600} opacity={0.8}>
            {key.label}
          </Label>
        </g>
      ))}
      <Flow d={`M${keyX} 404 H 466`} />
      <rect x={470} y={386} width={178} height={36} rx={18} fill={bg} stroke={border} />
      <CheckBadge cx={490} cy={404} />
      <Code x={510} y={408} size={10.5} opacity={0.8}>
        shortcut fired
      </Code>
      <Label x={110} y={458} size={10.5} weight={500} fill={mutedFg} anchor="start">
        Global — works while the app is in the background
      </Label>
    </IllustrationFrame>
  )
}

/* ------------------------------------------------------------------ */
/*  4. System services — one hub radiating the OS services.           */
/* ------------------------------------------------------------------ */

export function SystemServicesIllustration({ className }: { className?: string }) {
  const hub = { cx: 360, cy: 262 }
  const colors = [accentBlue, accentGreen, accentOrange, accentPink, accentAmber]
  const services = [
    'Message dialog',
    'File dialog',
    'Preferences',
    'Secure storage',
    'Clipboard',
    'Launch at login',
    'Open URL',
    'Accessibility',
  ]
  const pills = services.map((label, i) => {
    const angle = (Math.PI * 2 * i) / services.length - Math.PI / 2
    return {
      label,
      color: colors[i % colors.length],
      cx: round2(hub.cx + Math.cos(angle) * 236),
      cy: round2(hub.cy + Math.sin(angle) * 176),
      w: 40 + label.length * 7,
    }
  })
  return (
    <IllustrationFrame className={className}>
      {/* spokes under everything */}
      <g
        fill="none"
        stroke={primary}
        strokeOpacity={0.35}
        strokeWidth={2}
        strokeDasharray="3 7"
        strokeLinecap="round"
      >
        {pills.map(pill => (
          <line key={pill.label} x1={hub.cx} y1={hub.cy} x2={pill.cx} y2={pill.cy} />
        ))}
      </g>

      {/* hub node */}
      <circle cx={hub.cx} cy={hub.cy} r={60} fill="none" stroke={primary} strokeOpacity={0.15} strokeWidth={1.5} />
      <CircleShadow cx={hub.cx} cy={hub.cy} r={52} />
      <circle cx={hub.cx} cy={hub.cy} r={52} fill={bg} stroke={border} />
      <GearGlyph cx={hub.cx} cy={hub.cy} scale={2} stroke={primary} strokeWidth={2.6} />

      {/* service pills */}
      {pills.map(pill => {
        const x = pill.cx - pill.w / 2
        return (
          <g key={pill.label}>
            <rect x={x} y={pill.cy - 15} width={pill.w} height={30} rx={15} fill={bg} stroke={border} />
            <circle cx={x + 17} cy={pill.cy} r={4.5} fill={pill.color} />
            <Label x={x + 29} y={pill.cy + 4} size={11.5} anchor="start" opacity={0.8}>
              {pill.label}
            </Label>
          </g>
        )
      })}

      {/* ambient accents */}
      <circle cx={132} cy={132} r={46} fill={primary} fillOpacity={0.06} />
      <circle cx={596} cy={402} r={58} fill={primary} fillOpacity={0.05} />
    </IllustrationFrame>
  )
}

/* ------------------------------------------------------------------ */
/*  5. Generated bindings — one header line through the generator to  */
/*  the same call, spelled the way each language spells it.           */
/* ------------------------------------------------------------------ */

export function GeneratedBindingsIllustration({ className }: { className?: string }) {
  const hub = { cx: 300, cy: 260, r: 28 }
  const outputs: { cy: number, ext: string, color: string, call: string }[] = [
    { cy: 90, ext: 'dart', color: '#0175C2', call: "window.title = 'Hi'" },
    { cy: 158, ext: 'rs', color: '#CE422B', call: 'window.set_title("Hi")' },
    { cy: 226, ext: 'cs', color: '#512BD4', call: 'window.SetTitle("Hi")' },
    { cy: 294, ext: 'ts', color: '#D9A400', call: 'window.setTitle("Hi")' },
    { cy: 362, ext: 'py', color: '#3776AB', call: 'window.title = "Hi"' },
    { cy: 430, ext: 'go', color: '#00ADD8', call: 'window.SetTitle("Hi")' },
  ]
  const chip = { x: 410, w: 250, h: 44 }
  return (
    <IllustrationFrame className={className}>
      {/* flows: header → generator → each binding */}
      <Flow d={`M206 260 H ${hub.cx - hub.r - 4}`} />
      {outputs.map(output => (
        <Flow
          key={output.ext}
          d={`M${hub.cx + hub.r - 4} ${hub.cy + (output.cy - hub.cy) * 0.12} C 370 ${hub.cy + (output.cy - hub.cy) * 0.4}, 370 ${output.cy}, ${chip.x - 2} ${output.cy}`}
        />
      ))}

      {/* header card */}
      <SoftShadow x={44} y={196} w={162} h={128} rx={14} />
      <rect x={44} y={196} width={162} height={128} rx={14} fill={bg} stroke={border} />
      <FileGlyph cx={66} cy={220} scale={0.75} />
      <Code x={80} y={224} size={10.5} opacity={0.85}>
        window.h
      </Code>
      <line x1={44} y1={240} x2={206} y2={240} stroke={border} />
      <Code x={60} y={266} size={10.5}>
        <tspan fill={accentBlue}>void</tspan>
        <tspan fill={fg} fillOpacity={0.7}> SetTitle(</tspan>
      </Code>
      <Code x={72} y={286} size={10.5}>
        <tspan fill={accentBlue}>std::string</tspan>
      </Code>
      <Code x={72} y={306} size={10.5} fill={fg} opacity={0.7}>
        title);
      </Code>

      {/* generator hub */}
      <circle cx={hub.cx} cy={hub.cy} r={hub.r + 8} fill="none" stroke={primary} strokeOpacity={0.15} strokeWidth={1.5} />
      <CircleShadow cx={hub.cx} cy={hub.cy} r={hub.r} />
      <circle cx={hub.cx} cy={hub.cy} r={hub.r} fill={bg} stroke={border} />
      <CodeGlyph cx={hub.cx} cy={hub.cy} scale={1.1} stroke={primary} strokeWidth={2.2} />
      <Label x={hub.cx} y={hub.cy + 54} size={11} fill={mutedFg} weight={500}>
        ./codegen
      </Label>

      {/* one chip per binding, in that language's idiom */}
      {outputs.map(output => (
        <g key={output.ext}>
          <SoftShadow x={chip.x} y={output.cy - chip.h / 2} w={chip.w} h={chip.h} rx={12} />
          <rect x={chip.x} y={output.cy - chip.h / 2} width={chip.w} height={chip.h} rx={12} fill={bg} stroke={border} />
          <rect x={chip.x + 10} y={output.cy - 11} width={40} height={22} rx={6} fill={output.color} fillOpacity={0.12} />
          <Code x={chip.x + 30} y={output.cy + 4} size={10} fill={output.color} opacity={1} anchor="middle">
            .{output.ext}
          </Code>
          <Code x={chip.x + 62} y={output.cy + 4} size={10.5} opacity={0.8}>
            {output.call}
          </Code>
        </g>
      ))}

      {/* ambient accents */}
      <circle cx={150} cy={100} r={44} fill={primary} fillOpacity={0.06} />
      <circle cx={140} cy={440} r={48} fill={primary} fillOpacity={0.05} />
    </IllustrationFrame>
  )
}

const illustrationsById = {
  'windows': WindowsIllustration,
  'tray-menus': TrayMenusIllustration,
  'displays-input': DisplaysInputIllustration,
  'system-services': SystemServicesIllustration,
  'generated-bindings': GeneratedBindingsIllustration,
} as const

export type FeatureIllustrationId = keyof typeof illustrationsById

export function FeatureIllustration({
  id,
  className,
}: {
  id: FeatureIllustrationId
  className?: string
}) {
  const Illustration = illustrationsById[id]
  return <Illustration className={className} />
}
