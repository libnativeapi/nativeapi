import { useEffect, useState } from 'react'

/** Seconds since the page loaded. */
export const nowSeconds = () => performance.now() / 1000

/** The current time in seconds, re-rendering every `interval` ms while `active`. */
export function useNow(interval = 100, active = true) {
  const [now, setNow] = useState(nowSeconds)
  useEffect(() => {
    if (!active) return
    const id = window.setInterval(() => setNow(nowSeconds()), interval)
    return () => clearInterval(id)
  }, [interval, active])
  return now
}
