import { cn } from '@/lib/utils'

/*
 * The nativeapi mark: a window frame (title bar + body) holding a pair of
 * code chevrons — code in, native window out. Shared by the icon, the
 * lockup and the favicon.
 */

export const brandGradientStops = [
  { offset: '0', color: '#1D4ED8' },
  { offset: '.5', color: '#2563EB' },
  { offset: '1', color: '#06B6D4' },
] as const

const titleBarPath =
  'M12 34a18 18 0 0 1 18-18h68a18 18 0 0 1 18 18v12H12Z'
const framePath =
  'M17 46v48a13 13 0 0 0 13 13h68a13 13 0 0 0 13-13V46'
const chevronsPath = 'M52 62 38 76l14 14M76 62l14 14-14 14'

export function BrandMarkPaths({ gradientId }: { gradientId: string }) {
  return (
    <>
      <path d={titleBarPath} fill={`url(#${gradientId})`} />
      <path
        d={framePath}
        fill="none"
        stroke={`url(#${gradientId})`}
        strokeWidth={10}
        strokeLinejoin="round"
      />
      <path
        d={chevronsPath}
        fill="none"
        stroke={`url(#${gradientId})`}
        strokeWidth={10}
        strokeLinecap="round"
        strokeLinejoin="round"
      />
    </>
  )
}

export function BrandGradient({ id }: { id: string }) {
  return (
    <linearGradient id={id} x1="0" y1="0" x2="1" y2="1">
      {brandGradientStops.map(stop => (
        <stop key={stop.offset} offset={stop.offset} stopColor={stop.color} />
      ))}
    </linearGradient>
  )
}

/** Standalone SVG source of the mark, for the favicon. */
export const brandMarkSvg = `<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 128 128"><defs><linearGradient id="g" x1="0" y1="0" x2="1" y2="1">${brandGradientStops
  .map(stop => `<stop offset="${stop.offset}" stop-color="${stop.color}"/>`)
  .join('')}</linearGradient></defs><path d="${titleBarPath}" fill="url(#g)"/><path d="${framePath}" fill="none" stroke="url(#g)" stroke-width="10" stroke-linejoin="round"/><path d="${chevronsPath}" fill="none" stroke="url(#g)" stroke-width="10" stroke-linecap="round" stroke-linejoin="round"/></svg>`

export function BrandIcon({ className }: { className?: string }) {
  return (
    <svg viewBox="0 0 128 128" className={cn(className)} aria-hidden="true">
      <defs>
        <BrandGradient id="na-gradient" />
      </defs>
      <BrandMarkPaths gradientId="na-gradient" />
    </svg>
  )
}
