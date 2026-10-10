import type { SignEntry, SignStyle, SignTemplate } from './sign-types'

export const TEMPLATES: readonly SignTemplate[] = [
  { value: 'missing', label: 'Missing You Sign', fields: ['primary', 'secondary'], labels: ['Place', 'Pinyin · Separate syllables with spaces'], defaults: { primary: '上海', secondary: 'SHANG HAI', distance: '' } },
  { value: 'welcome', label: 'City Welcome Sign', fields: ['primary', 'secondary'], labels: ['City', 'English name'], defaults: { primary: '大理', secondary: 'DALI', distance: '' } },
  { value: 'travel', label: 'Travel Sign', fields: ['primary', 'secondary'], labels: ['Upper text', 'Lower text'], defaults: { primary: '下一站，山海', secondary: '去有风的地方', distance: '' } },
  { value: 'guide', label: 'Scenic Guide Sign', fields: ['primary', 'secondary', 'distance'], labels: ['Attraction', 'English name', 'Distance · e.g. 2 km / 300 m'], defaults: { primary: '观景台', secondary: 'VIEWPOINT', distance: '300 m' } },
]

export const templateOf = (style: SignStyle) => TEMPLATES.find(template => template.value === style)!
export const contentOf = (entry: SignEntry) => entry.contents[entry.style]
export const headingOf = (entry: SignEntry) => {
  const { primary } = contentOf(entry)
  return entry.style === 'missing' ? `我在${primary}很想你` : entry.style === 'welcome' ? `${primary}欢迎您` : primary
}

export function createSign(id: number, style: SignStyle = 'missing'): SignEntry {
  return {
    id, style, green: false, english: true, right: true,
    contents: Object.fromEntries(TEMPLATES.map(template => [template.value, { ...template.defaults }])) as SignEntry['contents'],
  }
}

export function validateContent(style: SignStyle, content: SignEntry['contents'][SignStyle]): string | null {
  const template = templateOf(style)
  for (const [index, key] of template.fields.entries()) {
    const limit = index === 1 ? 36 : 16
    if (!content[key] || [...content[key]].length > limit || /[\r\n\t]/.test(content[key])) {
      return `${template.labels[index]!.split(' · ')[0]} must contain 1–${limit} characters and no line breaks or tabs.`
    }
  }
  if (style === 'missing') {
    if ([...content.primary].length > 10) return 'Place must contain at most 10 characters.'
    if (content.secondary.length > 24 || !/^[a-zA-Z0-9 -]+$/.test(content.secondary)) {
      return 'Pinyin must contain 1–24 letters, digits, spaces or hyphens.'
    }
  }
  return null
}
