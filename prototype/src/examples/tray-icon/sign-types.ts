export type SignStyle = 'missing' | 'welcome' | 'travel' | 'guide'

export interface SignContent {
  primary: string
  secondary: string
  distance: string
}

export interface SignEntry {
  id: number
  style: SignStyle
  contents: Record<SignStyle, SignContent>
  green: boolean
  english: boolean
  right: boolean
}

export interface SignTemplate {
  value: SignStyle
  label: string
  fields: readonly (keyof SignContent)[]
  labels: readonly string[]
  defaults: SignContent
}
