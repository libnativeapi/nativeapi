import { cn } from '@/lib/utils'
import { BrandGradient, BrandMarkPaths } from './brand-icon'

interface BrandLockupProps {
  className?: string
}

export function BrandLockup({ className }: BrandLockupProps) {
  return (
    <svg
      viewBox="0 0 520 128"
      className={cn('text-foreground dark:text-slate-300', className)}
      role="img"
      aria-label="nativeapi"
    >
      <defs>
        <BrandGradient id="na-gradient-lockup" />
      </defs>
      <title>nativeapi</title>
      <BrandMarkPaths gradientId="na-gradient-lockup" />
      <text
        x="140"
        y="96"
        fontFamily="system-ui, -apple-system, sans-serif"
        fontWeight="800"
        fontSize="92"
        letterSpacing="-0.05em"
        fill="currentColor"
      >
        nativeapi
      </text>
    </svg>
  )
}
