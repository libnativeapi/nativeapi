import { CheckIcon, LanguagesIcon } from 'lucide-react'
import { Button } from '@/components/ui/button'
import {
  DropdownMenu,
  DropdownMenuContent,
  DropdownMenuGroup,
  DropdownMenuItem,
  DropdownMenuTrigger,
} from '@/components/ui/dropdown-menu'
import {
  getMessages,
  localeStorageKey,
  localizedPath,
  locales,
  type Locale,
} from '@/lib/i18n'

export function LocaleSwitcher({
  locale,
  pagePath = '/',
}: {
  locale: Locale
  pagePath?: string
}) {
  const copy = getMessages(locale)

  return (
    <DropdownMenu>
      <DropdownMenuTrigger asChild>
        <Button
          variant="ghost"
          size="icon"
          aria-label={`${copy.header.language}: ${copy.languageName}`}
          title={`${copy.header.language}: ${copy.languageName}`}
        >
          <LanguagesIcon />
        </Button>
      </DropdownMenuTrigger>
      <DropdownMenuContent align="end">
        <DropdownMenuGroup>
          {locales.map(item => {
            const href = localizedPath(item, pagePath)
            return (
              <DropdownMenuItem key={item} asChild>
                <a
                  href={href}
                  hrefLang={item}
                  lang={item}
                  onClick={event => {
                    window.localStorage.setItem(localeStorageKey, item)
                    if (window.location.hash) {
                      event.preventDefault()
                      window.location.assign(`${href}${window.location.hash}`)
                    }
                  }}
                >
                  <span className="flex-1">{copy.languages[item]}</span>
                  {item === locale && <CheckIcon />}
                </a>
              </DropdownMenuItem>
            )
          })}
        </DropdownMenuGroup>
      </DropdownMenuContent>
    </DropdownMenu>
  )
}
