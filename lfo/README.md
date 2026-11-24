# lfo

Four LFOs on one sound, all running at once, on synth and drum sounds. They sit in the sound's own LFO list as one more kind, called **ufo**. Each of the four is a stock LFO with its stock page, so you already know how they work.

## Turning it on

- **Shift + T4** opens the sound's LFO list, as always. **ufo** is in it, between tremolo and value. Pick it.
- **T4** now shows the four LFOs as four UFOs over a moon. The LFO the sound had becomes the one under the searchlight, and the other three run beside it.
- On a sound that isn't on ufo, T4 is the stock LFO page. Nothing changes there.
- Pick any other kind in the list and the sound goes back to one LFO. The other three are kept: pick ufo again and they come back as you left them. That holds over a restart too.

It works the same on drum sounds.

## On the UFO page

- **T4**: the stock page of the LFO under the searchlight. T4 there brings you back to the UFOs.
- **Hold T4** (about half a second): your saved sets (below).
- **Shift + T4**: the LFO list, as on any sound.
- T1 to T3 leave the page (T3 goes to the train, if you have fx train).

The knobs:

- **Turn**: change that LFO's kind: none, tremolo, value, random, element, midi or velocity. A new one starts on the stock defaults.
- **Tap**: that LFO takes the searchlight and its stock page opens. On an empty UFO, nothing happens.
- **Hold**: switch it off (the UFO lands) or back on.
- **Shift + turn**: move it one place. This only changes the order on screen: all four always run side by side.

## Each LFO's page

It's the stock page, knobs and all. On value, for example: blue sets the speed, ochre the amount, grey the destination, red the parameter.

With fx train on the unit and at least two wagons carrying an effect, the FX destination steps through the wagons, one per click. A small wagon in that knob's colour shows which one it moves, and the effect's parameter name shows over the knob.

## Saved sets

**Hold T4** on the UFO page for the list. Blue scrolls, tap blue or press T4 to load, tap ochre to rename, tap grey to save the four you're on, hold red to delete. A new save takes the first free slot, named "lfo" and its number. Each row shows the set's four kinds in their knob colours.

A set keeps all four LFOs: their kinds, which ones are on, their knobs, the wagon each one turns, and their order on screen. Loading one leaves the searchlight where it is.

Saved sets are files, 64 slots, in the `lfo` folder in disk mode. Copy them to your computer or swap them with friends.

## Your sounds

- Each sound keeps its own four, on every synth and drum slot.
- Sounds you set up with lfo 11 come back on ufo by themselves, all four as they were.
- With fx train on the unit, a saved synth sound carries its four LFOs in the sound file. Drum sounds keep their four on the unit, one set per drum slot. Without fx train, a sound file carries only the LFO under the searchlight.

## Without the pack

T4 is stock again. A sound that was on ufo plays the LFO under the searchlight, as it was saved. Your saved sets stay on the disk.

## What it needs

open-field 0.19 or later for drum sounds. On 0.17 and 0.18 it works on synth sounds only, and older releases leave it out (the start screen counts it as left out). The start screen shows the release: `1.7.3 - open-field 0.19`.

## Version 12

Version 12 has run on a unit, with fx train on it and without: a synth sound and a drum sound on ufo, the keys above, a save, another kind and back, a restart, and an lfo 11 sound coming back on ufo.

Not tried on a unit yet: the midi kind's choice of wagon (it needs a controller), a saved sound file bringing its four LFOs back when loaded (synth, with fx train), and a drum sound saved to a file and loaded again.
