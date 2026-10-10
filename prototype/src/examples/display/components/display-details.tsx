import { Desktop20Regular, Laptop20Regular } from '@fluentui/react-icons'

import { Badge, Icon, Table, TableCell, TableHead, TableRow } from '@dazzlabs/dazzui'

import { Panel } from '../../../components/panel'
import { ReadBack } from '../../../components/read-back'
import { ORIENTATION_NAMES } from '../data'
import { marginsText, scaleText, sizeText } from '../use-displays'
import type { PlacedDisplay } from '../types'
import './display-details.css'

const at = (x: number, y: number) => `(${x}, ${y})`

/** The selected display's getters, by what they describe. */
export function DisplayDetails({ display: d }: { display: PlacedDisplay }) {
  return (
    <Panel>
      <div className="display-details__head">
        <Icon icon={d.name.startsWith('Built-in') ? Laptop20Regular : Desktop20Regular} size={28} />
        <div className="display-details__title">
          <strong>{d.name}</strong>
          <span>
            {d.primary && (
              <Badge size="small" variant="tinted" tint="success">
                Primary
              </Badge>
            )}
            <Badge size="small" variant="outlined">
              {sizeText(d)}
            </Badge>
            <Badge size="small" variant="outlined">
              @{scaleText(d.scale)}x
            </Badge>
          </span>
        </div>
      </div>
      <ReadBack
        label="Basic"
        columns={1}
        keyWidth="7rem"
        rows={[
          ['getId', String(d.id)],
          ['getName', `"${d.name}"`],
          ['isPrimary', String(d.primary)],
        ]}
      />
      <ReadBack
        label="Hardware"
        keyWidth="7rem"
        rows={[
          ['getScaleFactor', scaleText(d.scale)],
          ['getRefreshRate', `${d.refreshRate} Hz`],
          ['getBitDepth', `${d.bitDepth} bit`],
          ['getOrientation', ORIENTATION_NAMES[d.orientation]],
        ]}
      />
      <ReadBack
        label="Geometry"
        keyWidth="7rem"
        rows={[
          ['getPosition', at(d.x, d.y)],
          ['getSize', `${sizeText(d)} px`],
          ['workArea size', `${sizeText(d.workArea)} px`],
          ['workArea at', at(d.workArea.x, d.workArea.y)],
          ['margins', marginsText(d)],
          ['pixels', `${d.pixelWidth} × ${d.pixelHeight}`],
        ]}
      />
      <p className="example-panel__note">
        Positions are in desktop coordinates: the primary display's top-left is (0, 0). The margins are what the
        system's bars keep off the work area.
      </p>
    </Panel>
  )
}

/** Every display side by side, one row each; a row selects its display. */
export function DisplayTable({
  displays,
  selectedId,
  onSelect,
}: {
  displays: readonly PlacedDisplay[]
  selectedId: number
  onSelect: (id: number) => void
}) {
  return (
    <Table className="display-table">
      <TableHead>
        <TableCell head className="display-table__name">
          Display
        </TableCell>
        <TableCell head>Size</TableCell>
        <TableCell head className="display-table__wide">
          Position
        </TableCell>
        <TableCell head className="display-table__narrow">
          Scale
        </TableCell>
        <TableCell head className="display-table__narrow">
          Refresh
        </TableCell>
        <TableCell head>Orientation</TableCell>
      </TableHead>
      {displays.map(d => (
        <TableRow key={d.id} interactive active={d.id === selectedId} onClick={() => onSelect(d.id)}>
          <TableCell className="display-table__name">
            <span className="display-table__label">
              <span>{d.name}</span>
              {d.primary && (
                <Badge size="small" variant="tinted" tint="success">
                  Primary
                </Badge>
              )}
            </span>
          </TableCell>
          <TableCell className="display-table__mono">{sizeText(d)}</TableCell>
          <TableCell className="display-table__mono display-table__wide">{at(d.x, d.y)}</TableCell>
          <TableCell className="display-table__mono display-table__narrow">{scaleText(d.scale)}×</TableCell>
          <TableCell className="display-table__mono display-table__narrow">{d.refreshRate} Hz</TableCell>
          <TableCell className="display-table__mono">{ORIENTATION_NAMES[d.orientation]}</TableCell>
        </TableRow>
      ))}
    </Table>
  )
}
