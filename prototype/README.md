# Prototype

Interface prototypes for the nativeapi example apps (`../examples/`), hosted
in Storybook and drawn with [DazzUI](https://github.com/dazzlabs/dazzui).
All data and behavior are simulated locally: nothing calls nativeapi, no
window is created and no input is injected.

```sh
pnpm install
pnpm dev
pnpm build
pnpm typecheck
```

Run these commands from `prototype/`. Open http://127.0.0.1:6026. The toolbar
switches the style (a DazzUI family, a desktop, or an Omarchy theme) and its
appearance (light, dark).

## DazzUI

The controls come from dazzlabs/dazzui's `packages/js/dazzui`, installed as a
git dependency (`@dazzlabs/dazzui` in `package.json`) pinned to a commit. pnpm
builds the package on install through its `prepare` script, which pnpm only
runs for the entry in `pnpm-workspace.yaml`'s `allowBuilds`; that entry names
the same commit. To move to a newer DazzUI, change the commit in both places
and run `pnpm install`.

## Architecture

See [docs/architecture.md](docs/architecture.md) for directory
responsibilities, naming, dependency boundaries, Storybook grouping and
validation rules. A window is DazzUI's own `WindowFrame`, drawn for the
Style toolbar's desktop through `windowPlatformOf` in `src/styles/themes.ts`.

- `src/examples/<example>/`: one example app's prototype — its view, internal
  components, example data and simulated behavior.
  Every example has one, named after it (`tray_icon` → `tray-icon/`); start
  at **Examples / <Name> / <Name> View**.
- `src/components/`: what every example is built from — the desktop stage,
  the example window, the event bar, panels and the getter read-back.
- `src/styles/`: global styles and the style / appearance switch.
- `docs/`: the prototype's architecture rules.
- `.storybook/`: Storybook configuration, the toolbars, and sidebar ordering.
- `stories/`: the Introduction.
