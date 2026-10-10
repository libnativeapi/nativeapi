import type { ViewKind, ViewLayoutName, ViewNode, ViewTree } from './types'

/** The data attributes every simulated native view carries (`components/native-view.tsx`). */
export const VIEW_ATTR = 'data-view'

const nearestView = (el: Element | null) => el?.parentElement?.closest(`[${VIEW_ATTR}]`) ?? null

/**
 * Reads the workbench's views back the way the inspector's `describeTree()`
 * walks them: `subviews` in order, each one's `frame` relative to its parent
 * (what the layout pass computed, here measured off the page), and its
 * getters — layout and spacing, flex, `isVisible`, `isEnabled`,
 * `isFocused`, `text`. A hidden view has the empty frame core gives it.
 */
export function describeViewTree(rootEl: HTMLElement | null): ViewTree {
  if (!rootEl) return { root: null, count: 0, depth: 0 }
  let count = 0
  let depth = 0

  // parentRect: null for the root (its frame starts at 0,0), undefined under a view with no frame.
  const walk = (el: HTMLElement, level: number, parentRect: DOMRect | null | undefined): ViewNode => {
    count++
    depth = Math.max(depth, level)
    const rect = el.getBoundingClientRect()
    const hidden = el.dataset.hidden !== undefined
    // Under a hidden parent a view keeps its own isVisible but has no frame either.
    const laidOut = !hidden && parentRect !== undefined && (rect.width > 0 || rect.height > 0)
    const frame = !laidOut
      ? { x: 0, y: 0, width: 0, height: 0 }
      : parentRect
        ? { x: rect.left - parentRect.left, y: rect.top - parentRect.top, width: rect.width, height: rect.height }
        : { x: 0, y: 0, width: rect.width, height: rect.height }
    const kids = Array.from(el.querySelectorAll<HTMLElement>(`[${VIEW_ATTR}]`)).filter(child => nearestView(child) === el)
    const layout = el.dataset.layout as ViewLayoutName | undefined
    return {
      name: el.dataset.view!,
      kind: (el.dataset.kind as ViewKind) ?? 'View',
      frame,
      layout: kids.length > 0 && layout && layout !== 'absolute' ? layout : undefined,
      gap: el.dataset.gap ? Number(el.dataset.gap) : 0,
      flex: el.dataset.flex ? Number(el.dataset.flex) : 0,
      hidden,
      disabled: el.dataset.disabled !== undefined,
      focused: el.dataset.focused !== undefined,
      text: el.dataset.text,
      tooltip: el.dataset.tooltip,
      children: kids.map(child => walk(child, level + 1, laidOut ? rect : undefined)),
    }
  }

  const root = walk(rootEl, 0, null)
  return { root, count, depth }
}

/** Depth-first, the order the inspector prints the tree in. */
export function flattenTree(node: ViewNode | null, out: ViewNode[] = []): ViewNode[] {
  if (!node) return out
  out.push(node)
  node.children.forEach(child => flattenTree(child, out))
  return out
}

/** The first view whose name contains `query`, case-insensitively. */
export function findView(tree: ViewTree, query: string): ViewNode | null {
  const needle = query.trim().toLowerCase()
  if (!needle) return null
  return flattenTree(tree.root).find(node => node.name.toLowerCase().includes(needle)) ?? null
}

/** A view's text as the tree prints it: one line, clipped to 24 characters. */
export function clipText(text: string, limit = 24) {
  const line = text.replace(/\n/g, '⏎')
  return line.length > limit ? `${line.slice(0, limit - 1)}…` : line
}

/** The element a view name stands for, under the workbench's root. */
export function viewElement(rootEl: HTMLElement | null, name: string): HTMLElement | null {
  if (!rootEl) return null
  if (rootEl.dataset.view === name) return rootEl
  return rootEl.querySelector<HTMLElement>(`[${VIEW_ATTR}="${CSS.escape(name)}"]`)
}
