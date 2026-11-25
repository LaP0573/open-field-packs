# maestro

A chord sequencer for synth sounds. One key plays a whole chord, in the key you pick. The left half of the keyboard holds twelve chords from a library of chord sets; the right half suggests where to go next, or plays a melody over the chord. The screen shows what each key plays.

## Turning it on

- On a synth sound, hold **shift** and press the **sequencer key**: the unit's list of sequencers. **MAESTRO** comes after the unit's own seven. Pick it.
- After every start the sequencer is off and the keys play plain notes. Press the sequencer key once and maestro's page comes up. Press it again for plain notes.

## The page

- The top row, in the knob colours: the key (C MAJ), the set, HUMANIZE, and FIELD or MELODY in the red pill. BASS or ARP shows in red under it.
- Left: wind chimes, one per key, a tube per note. Blue for a chord of the key, ochre for one from outside it. The chord you played last is lit.
- Middle: the maestro on his podium, and a board naming the chord that sounds.
- Keys, chords and the dolls' notes are spelled the way chord charts spell them, with flats where music uses them: Bb major rather than A# major, and Ab and Bb major chords in C major rather than G# and A#. Maestro draws the flat and sharp signs itself, beside the letter: the unit's font has no flat, and its sharp fills in to a dot on the small plates.
- Right, in field mode: twelve kites, one suggestion per key. The colour is the mood, dark indigo to bright yellow; the height is how far the move goes. White keys stay in the key, black keys step outside it.
- Right, in melody mode: the choir of dolls. The front row sings the chord's own notes from its root up, the back row the key's other notes.

## The keys

- **A left key** plays its chord. The kites and dolls follow the one you played last. A bare hook is an empty key.
- **A right key in field mode** plays its suggestion while you hold it. Nothing else moves.
- **Hold a right key and tap a left key**: the suggestion goes onto that key. The tapped key doesn't sound; its plate takes the new name.
- **A right key in melody mode** plays its note over your chord.
- **The arrows** are the unit's octave, as always. They move the melody only.
- **Shift + left or right arrow**: the chords' octave, -3 to +3, for everything the left keys play and the kites. It shows after the key, as MAJ +1.

## The knobs

- **Blue**: turn for the key, all 24 major and minor keys in one row. Press to put the last chord back as the set has it.
- **Ochre**: turn for the set: the library's sets, then your saves. Press for the set browser (below). Hold to save. Shift + press: the set's twelve chords back as loaded.
- **Grey**: turn for HUMANIZE, 0 to 100: each strike's timing and loudness loosen, more as it rises. Press to strike the chords you hold again.
- **Red**: turn for the play mode: chord, bass or arp. A quick press switches FIELD and MELODY. Shift + press mutes the left chords (the chimes go dim): they still steer the kites and dolls, which always sound. Record the chords to tape, mute them, play a melody over them.

Under shift, three knobs reshape the chord you played last, and its key keeps the change as you turn:

- **Shift + blue**: octave and inversion, seven steps (0 ROOT, -1 INV1, ...).
- **Shift + ochre**: maj, min, dim, aug, sus2, sus4.
- **Shift + grey**: triad, 6th, 7th, maj7, 9th, add9.
- **Shift + red**: HOLD (right) or FREE (left). With HOLD the last chord rings after you let go, until the next left key.

## The play modes

Hold the red knob down and the top row shows the play mode's settings. Turn any knob, the red one too, to set them; hold shift as well for more.

- **chord**: blue the strum, 0 to 250 ms. Ochre the order: low-high, high-low, alternate, outside-in, inside-out, random. HUMANIZE plays here.
- **bass**: each chord you hold plays a bass line instead. Blue the pattern: hold, pulse, root-fifth, octave, walk, 1-5-8-5, arpeggio, tresillo, offbeat. Ochre the rate: 1/4, 1/8, 1/8t, 1/16. Grey the swing, 0 to 100. Red the length, 10 to 100. With shift: blue the octave, -2 to 5; ochre the note it's built on: chord (its lowest), root, 3rd, 5th or 7th.
- **arp**: the newest chord you hold plays one arpeggio. Blue the pattern: up, down, up-down, converge, diverge, thumb, pinky, up in 3rds, random, shuffle, walk. Ochre the rate, grey the swing, red the gate, 5 to 100. With shift: blue the notes (chord, +9, power, shell, penta), ochre the octave (-2 to 2), grey the span (1 to 3 octaves), red carry or restart at a new chord.

Bass and arp start on the press and follow the unit's tempo.

## Your progressions

- Everything you set is kept over a restart.
- **Hold the ochre knob** (a line fills under the set's name) to save. It goes to the first free of 64 slots as "my" and its number; the set's place shows SAVED! and the number, or NOT SAVED when all are taken.
- A save keeps the twelve chords, the key, HUMANIZE, HOLD and the play mode. The play modes' settings and the chords' octave stay as you have them.
- Saves are files, `01.json` to `64.json`, in the `maestro` folder in disk mode. Copy them to your computer or swap them with friends. Delete one there to free its slot.

## The set browser

A quick press of the ochre knob opens it over the page. The keys keep playing.

- Left: MINE, the eight families and sixteen moods. Blue turns, a blue press ticks or unticks (a small red square).
- Right: the sets that match, your saves first. A red dot marks the loaded one. Ochre turns, an ochre press loads it and closes.
- Ticked families and MINE add up. Ticked moods narrow it down, and only the library's sets have moods. Nothing ticked lists everything.
- A grey or red press closes it.

## Without the pack

MAESTRO leaves the list. It changes nothing in the firmware. Your saves stay on the disk.

## What it needs

open-field 0.14 or later. From 0.19 melody mode redraws only what moved and runs at full frame rate. On 0.14 to 0.18 the page is slower but the same. The start screen shows the release: `1.7.3 - open-field 0.19`.

## Version 10

Version 10 spells keys, chords and the dolls' notes the way chord charts do, with drawn flat and sharp signs. Otherwise it plays and looks like version 8, the first one shared here.

How it plays has run on a unit since version 7: the keys, a suggestion stored on a key, the set reloaded, a save, the set browser, bass with two chords held at once, the arp and its settings, the span 1 to 3 among them, both octaves (shift + arrows, and the arrows alone), picking it from the list, the folder in disk mode, and a power cycle. Version 10 itself has run on a unit too: it started and drew its page in C major, E major and F# major, with the chords, the suggestions and the dolls' notes named.

Not tried on a unit yet: a synth sound saved and loaded again while maestro is chosen.
