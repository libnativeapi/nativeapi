import type { CSSProperties, ReactNode, Ref } from 'react'

import { BOX_SIZE, BOXES, MAX_TASKS, NOTES_BUDGET, STAGE_HEIGHT } from '../data'
import { frameText, simulateStackLayout } from '../stack-layout'
import type { Workbench } from '../use-workbench'
import { NButton, NField, NLabel, NView } from './native-view'
import './workbench-sections.css'

/** A titled block of the workbench: a heading over its body, on a faint tint of the accent. */
function Section({ name, title, flex, children }: { name: string; title: string; flex?: number; children: ReactNode }) {
  return (
    <NView name={name} gap={8} padding={12} flex={flex} className="workbench-section">
      <NLabel name={`${name}.heading`} text={title.toUpperCase()} tone="muted" size="small" className="workbench-section__heading" />
      <NView name={`${name}.body`} gap={8} flex={flex}>
        {children}
      </NView>
    </NView>
  )
}

/**
 * Sign in: validation as you type, Enter moves to the password and submits,
 * a masked field that can be revealed (`setSecure`), and a fake round trip
 * on a timer during which the whole form is disabled.
 */
export function SignInSection({ signIn }: { signIn: Workbench['signIn'] }) {
  const { state } = signIn
  const locked = state.pending || state.signedInAs !== null
  return (
    <Section name="signIn" title="Sign in">
      <NField
        name="signIn.username"
        text={state.username}
        placeholder="Username"
        disabled={locked}
        onChanged={signIn.setUsername}
        onSubmitted={signIn.usernameSubmitted}
      />
      <NView name="signIn.passwordRow" layout="row" gap={8}>
        <NField
          name="signIn.password"
          text={state.password}
          placeholder="Password (6+ characters)"
          secure={state.secure}
          flex={1}
          alignment="center"
          disabled={locked}
          onChanged={signIn.setPassword}
          onSubmitted={signIn.passwordSubmitted}
        />
        <NButton
          name="signIn.reveal"
          text={state.secure ? 'Show' : 'Hide'}
          tooltip="Show or mask the password"
          disabled={locked}
          onPress={signIn.toggleReveal}
        />
      </NView>
      <NView name="signIn.actions" layout="row" gap={8}>
        <NLabel name="signIn.status" text={state.status.text} tone={state.status.tone} flex={1} alignment="center" />
        <NButton name="signIn.clear" text="Clear" disabled={locked} onPress={signIn.clear} />
        <NButton
          name="signIn.submit"
          text={state.pending ? 'Wait…' : state.signedInAs ? 'Sign out' : 'Sign in'}
          disabled={!signIn.canSubmit}
          onPress={signIn.submit}
        />
      </NView>
    </Section>
  )
}

/**
 * Tasks: rows added, reordered (`removeSubview` + `insertSubview`) and
 * removed at run time, each with buttons of its own; the empty state and the
 * list hide and show (`setVisible`), and a full list disables the input.
 */
export function TasksSection({ tasks }: { tasks: Workbench['tasks'] }) {
  const done = tasks.list.filter(t => t.done).length
  const full = tasks.list.length >= MAX_TASKS
  const n = tasks.list.length
  return (
    <Section name="tasks" title="Tasks">
      <NView name="tasks.entry" layout="row" gap={8}>
        <NField
          name="tasks.input"
          text={tasks.input}
          placeholder={full ? `The list is full (${MAX_TASKS} tasks)` : 'What needs doing? (Enter adds it)'}
          flex={1}
          alignment="center"
          disabled={full}
          onChanged={tasks.setInput}
          onSubmitted={tasks.add}
        />
        <NButton name="tasks.add" text="Add" disabled={full || tasks.input.trim() === ''} onPress={tasks.add} />
      </NView>
      <NLabel
        name="tasks.empty"
        text="Nothing to do. Add a task above."
        tone="muted"
        textAlignment="center"
        hidden={n > 0}
        className="tasks-section__empty"
      />
      <NView name="tasks.list" gap={4} hidden={n === 0}>
        {tasks.list.map((task, index) => (
          <NView
            key={task.id}
            name={`tasks.item${task.id}`}
            layout="row"
            gap={6}
            padding={[2, 6]}
            className="tasks-section__row"
            style={{ '--task-wash': task.done ? '0%' : '9%' } as CSSProperties}
          >
            <NLabel
              name={`tasks.item${task.id}.title`}
              text={task.done ? `✓ ${task.title}` : task.title}
              tone={task.done ? 'muted' : 'default'}
              flex={1}
              alignment="center"
            />
            <NButton name={`tasks.item${task.id}.toggle`} text={task.done ? 'Undo' : 'Done'} onPress={() => tasks.toggle(task)} />
            <NButton
              name={`tasks.item${task.id}.up`}
              text="↑"
              tooltip="Move up"
              disabled={index === 0}
              onPress={() => tasks.moveUp(task)}
            />
            <NButton name={`tasks.item${task.id}.remove`} text="✕" tooltip="Remove" onPress={() => tasks.remove(task)} />
          </NView>
        ))}
      </NView>
      <NView name="tasks.footer" layout="row" gap={8}>
        <NLabel
          name="tasks.summary"
          text={n === 0 ? '' : `${n} task${n === 1 ? '' : 's'}, ${done} done`}
          tone="muted"
          flex={1}
          alignment="center"
        />
        <NButton name="tasks.clearDone" text="Clear done" disabled={done === 0} onPress={tasks.clearDone} />
      </NView>
    </Section>
  )
}

export interface PlaygroundSectionProps {
  playground: Workbench['playground']
  /** The stage's size: its width follows the column, its height what the column leaves it — both follow the window. */
  stageSize: { width: number; height: number }
  stageRef: Ref<HTMLDivElement>
}

/**
 * The layout engine on display: four boxes with flex 0, 1, 2 and 0 in a
 * stage whose layout, alignment, spacing and padding the buttons change. The
 * boxes stand where core's layout pass puts them, and the line underneath
 * reads back the frames it computed.
 */
export function PlaygroundSection({ playground, stageSize, stageRef }: PlaygroundSectionProps) {
  const { state } = playground
  const boxes = state.order.map(name => BOXES.find(b => b.name === name)!)
  const frames = simulateStackLayout(
    state.layout,
    stageSize,
    state.padding,
    state.spacing,
    boxes.map(box => ({
      visible: !(box.name === 'B' && state.hiddenB),
      preferred: BOX_SIZE,
      flex: box.flex,
      alignment: state.alignment,
    })),
  )
  const readOut = boxes
    .map((box, i) => (box.name === 'B' && state.hiddenB ? 'B hidden' : `${box.name} ${frameText(frames[i]!)}`))
    // A line breaks between boxes, never inside one's frame.
    .map(entry => entry.replace(/ /g, '\u00a0'))
    .join('   ')
  return (
    <Section name="playground" title="Layout playground" flex={1}>
      <NView name="playground.controls" layout="row" gap={6}>
        <NButton name="playground.layout" text={state.layout === 'row' ? 'Row' : 'Column'} onPress={playground.toggleLayout} />
        <NButton name="playground.align" text={`Align: ${state.alignment}`} onPress={playground.cycleAlignment} />
        <NView name="playground.controls.fill" flex={1} />
        <NButton
          name="playground.hideB"
          text={state.hiddenB ? 'Show B' : 'Hide B'}
          tooltip="A hidden view takes no space in a row or column"
          onPress={playground.toggleB}
        />
        <NButton name="playground.shuffle" text="Shuffle" onPress={playground.shuffle} />
      </NView>
      <NView name="playground.metrics" layout="row" gap={6}>
        <NButton name="playground.spacing" text={`Gap ${state.spacing}`} onPress={playground.cycleSpacing} />
        <NButton name="playground.padding" text={`Pad ${state.padding}`} onPress={playground.cyclePadding} />
      </NView>
      <NView
        ref={stageRef}
        name="playground.stage"
        layout={state.layout}
        gap={state.spacing}
        padding={state.padding}
        flex={1}
        className="playground-stage"
        style={{ minHeight: STAGE_HEIGHT }}
      >
        {boxes.map((box, i) => {
          const f = frames[i]!
          return (
            <NView
              key={box.name}
              name={`playground.${box.name}`}
              flex={box.flex}
              alignment={state.alignment}
              hidden={box.name === 'B' && state.hiddenB}
              padding={4}
              tooltip={`Box ${box.name}, flex ${box.flex}`}
              className="playground-stage__box"
              style={
                {
                  left: f.x,
                  top: f.y,
                  width: f.width,
                  height: f.height,
                  '--box-shade': `${box.shade}%`,
                } as CSSProperties
              }
            >
              <NLabel
                name={`playground.${box.name}.caption`}
                text={`${box.name}\nflex ${box.flex}`}
                tone="white"
                size="small"
                textAlignment="center"
              />
            </NView>
          )
        })}
      </NView>
      <NLabel name="playground.frames" text={readOut} tone="muted" size="small" mono />
    </Section>
  )
}

/**
 * Notes: a multi-line field with a live budget, a read-only toggle
 * (`setEditable`), and buttons that change the text from code — which,
 * unlike typing, emits no `TextFieldChangedEvent`.
 */
export function NotesSection({ notes }: { notes: Workbench['notes'] }) {
  const words = notes.text.split(/\s+/).filter(Boolean).length
  const left = NOTES_BUDGET - [...notes.text].length
  return (
    <Section name="notes" title="Notes">
      <NField
        name="notes.text"
        text={notes.text}
        placeholder="Anything on your mind? Enter starts a new line here."
        multiline
        rows={3}
        editable={notes.editable}
        onChanged={notes.setText}
      />
      <NView name="notes.footer" layout="row" gap={6}>
        <NButton
          name="notes.readOnly"
          text={notes.editable ? 'Lock' : 'Unlock'}
          tooltip="setEditable: a read-only field can still be selected"
          onPress={notes.toggleEditable}
        />
        <NButton name="notes.shout" text="UPPER" tooltip="Sets the text from code: no change event" onPress={notes.upper} />
        <NButton name="notes.stamp" text="Timestamp" onPress={notes.timestamp} />
        <NLabel
          name="notes.counter"
          text={`${words} word${words === 1 ? '' : 's'} · ${left >= 0 ? `${left} left` : `${-left} over`}`}
          tone={left < 0 ? 'danger' : left < 20 ? 'warning' : 'muted'}
          textAlignment="end"
          flex={1}
          alignment="center"
        />
      </NView>
    </Section>
  )
}
