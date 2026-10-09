import { createServerFn } from '@tanstack/react-start'

export type GithubStats = {
  stars: number
  forks: number
}

export const githubStatsFallback: GithubStats = { stars: 100, forks: 10 }

export const getGithubStats = createServerFn({ method: 'GET' }).handler(
  async (): Promise<GithubStats> => {
    try {
      const response = await fetch(
        'https://api.github.com/repos/libnativeapi/nativeapi',
        {
          headers: { Accept: 'application/vnd.github+json' },
          // Cache at the Cloudflare edge so homepage traffic doesn't hammer
          // GitHub's unauthenticated rate limit (60 req/hour per IP).
          // @ts-expect-error `cf` is a Cloudflare Workers-only fetch option
          cf: { cacheTtl: 3600, cacheEverything: true },
        }
      )
      if (!response.ok) throw new Error('GitHub request failed')
      const data = (await response.json()) as {
        stargazers_count?: number
        forks_count?: number
      }
      if (
        typeof data.stargazers_count === 'number' &&
        typeof data.forks_count === 'number'
      ) {
        return { stars: data.stargazers_count, forks: data.forks_count }
      }
      return githubStatsFallback
    } catch {
      return githubStatsFallback
    }
  }
)
