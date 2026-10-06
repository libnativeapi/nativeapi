import { useLayoutEffect, useRef, useState } from 'react'

/** An element's size, kept up to date as it resizes: what a panel lays itself out by. */
export function useElementSize<T extends HTMLElement = HTMLDivElement>() {
  const ref = useRef<T>(null)
  const [size, setSize] = useState({ width: 0, height: 0 })
  useLayoutEffect(() => {
    const element = ref.current
    if (!element) return
    const read = () => setSize({ width: Math.round(element.clientWidth), height: Math.round(element.clientHeight) })
    read()
    const observer = new ResizeObserver(read)
    observer.observe(element)
    return () => observer.disconnect()
  }, [])
  return [ref, size] as const
}
