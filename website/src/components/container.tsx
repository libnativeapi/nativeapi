import type { ElementType, ReactNode } from 'react'
import { cn } from '@/lib/utils'

export function Container({
  children,
  className,
  component: Component = 'div',
}: {
  component?: ElementType
  children: ReactNode
  className?: string
}) {
  return (
    <Component
      className={cn(
        'mx-auto w-full max-w-[86rem] px-4 sm:px-6 lg:px-8',
        className,
      )}
    >
      {children}
    </Component>
  )
}
