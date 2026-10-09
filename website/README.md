# nativeapi website

The project website: landing page, documentation and support, built with
TanStack Start and deployed as one Cloudflare Worker.

```bash
pnpm install
pnpm dev
```

The development server listens on <http://localhost:3000>.

## Documentation

`scripts/build-docs.mjs` compiles the Markdown into JSON under
`src/content/docs-gen/` (gitignored) before `dev`, `build` and `check-types`.
It reads two sources:

- `content/docs/<lang>/` — the hand-written pages (`en`, `zh-Hans`);
- each binding's `README.md` (and `README-ZH.md` where one exists) under
  `../bindings/`, published as the `bindings/<lang>` pages, so the binding
  docs have a single source.

Relative links that do not point at a compiled page are rewritten to the file
on GitHub.

## Deploying

```bash
pnpm exec wrangler login
pnpm run deploy
```
