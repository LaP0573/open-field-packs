# fx train

Up to four effects in a row on every synth and drum sound. It's one more effect in the sound's own list, called **train**. The train has four wagons, one for each knob colour. Each wagon carries an effect, and the coupled ones play in order after the engine.

## Turning it on

- **Shift + T3** opens the sound's list of effects, as always. **train** is in it by name, among the unit's own effects and the ones other packs add (on my unit, just under terminal). Pick it.
- The effect the sound had goes onto the wagon under the lamp: the blue one, unless you've moved the lamp with a tap.
- **T3** now shows the train: the four wagons. From any other page of the sound, T3 brings you back to it.
- On a sound that isn't on train, T3 is the stock effect page. Nothing changes there.
- Pick any other effect in the list and the sound goes back to one effect, and stays that way after a restart. Its wagons are kept: pick train again and they come back as you left them, with the effect you'd just picked on the wagon under the lamp.

It works the same on drum sounds.

The master isn't there yet. **train** shows in the master effect's list on the mixer too, but picking it there does nothing: the master keeps its own effect. The train on the master comes in a later version.

## On the train

- **T3**: the effect page of the wagon under the lamp, with all its knobs. T3 there brings you back to the train.
- **Hold T3** (about half a second): your saved trains (below).
- **Shift + T3**: the list of effects, as on any sound.
- T1, T2 and T4 leave the page.

The knobs:

- **Turn**: choose the wagon's effect. The sound follows a moment after you stop turning. A name in grey is an effect that doesn't fit the budget (below).
- **Tap**: open that wagon's effect page, with all its knobs. The lamp moves to that wagon, and T3 brings you back. On an empty wagon, or one carrying another pack's effect, nothing happens.
- **Hold**: uncouple the wagon (it goes quiet and costs nothing) or couple it back.
- **Shift + turn**: move the wagon one step along the train.
- **Shift + tap**: random settings for that effect.
- **Shift + hold**: back to its starting settings.

## Which effects

All the unit's own effects except the vocoder (fazer only if you've unlocked it), plus the effects other packs add. The same effect can sit on two wagons. On the wagon under the lamp, other packs' effects are skipped: they can sit on the other three.

The train has a processing budget. When you turn onto an effect that won't fit, the knob stops on it anyway:

- its name shows in grey under the wagon, FULL and the red needle show on the engine, and a line under the rails says what it needs and what the rest of the train leaves free, as a share of the budget: for example "needs 42%, 37% free";
- the wagon keeps the effect it had, and keeps playing it;
- turn on to reach one that fits: it's picked as usual. Another that won't fit stops the knob again.

A press of any knob, a turn of another knob, moving a wagon, the saved trains' list or leaving the page ends it. Coupling a wagon with a hold is held to the budget too: if it won't fit, the wagon stays uncoupled and FULL shows for a second.

If a wagon carries another pack's effect and you remove that pack, the wagon stays, silent and drawn empty, with its settings kept. Put the pack back and it plays again.

## Saved trains

**Hold T3** on the train for the list of saved trains.

On the list:

- **Blue**: scroll. Tap blue or press T3 to load.
- **Tap ochre**: rename. Blue moves along, ochre changes the letter, grey or T3 keeps it, red cancels.
- **Tap grey**: save the train you're on. A new save takes the first free slot, named "chain" and its number.
- **Hold red**: delete.

Saved trains are files, 64 slots, in the `fx` folder in disk mode. Copy them to your computer or swap them with friends.

## Your sounds

- Each sound keeps its own train, on every synth and drum slot.
- Save a sound that's on train and the train goes into the sound file with it, and comes back when you load that sound.
- Sounds you set up with an older fx train, with two or more wagons coupled, come back on train by themselves a moment after they load, everything as you saved it. A sound with one wagon loads as that plain effect.

## With lfo

- T4 from the train shows the sound's LFO page. With lfo 12 or later, that's the UFOs when the sound is on ufo, and T3 from the UFOs brings you back to the train. See lfo's manual.
- On the stock LFO page, the FX destination knob steps through the train's wagons only on a sound that's on train. On any other sound it's stock.

## Without the pack, or on an older open-field

A sound that was on train plays the effect that was under the lamp, as it was saved. Your saved trains stay on the disk.

fx train 19 needs open-field 0.17 or later. Older releases leave it out (the start screen counts it as left out), and nothing of it runs. The install page puts on 0.19, and the start screen then shows `1.7.3 - open-field 0.19`.

## Version 19

Version 19 has been tried in the emulator only, not yet on a unit. What it adds to 18, each with the label of the commit that brought it:

- **[E] A lighter picture.** The train page's picture still carried the UFO page of fx train 17, which this page no longer shows (lfo 12 shows its own UFOs, from its own picture). Version 19 leaves those parts out: the unit puts the picture together faster at the start and keeps about half a megabyte less of its own memory for it, for the whole session. The page looks and works as in version 18.
- **[D] Names whole, and the train page kept under fast turns.** On a wagon a name of another pack's effect may now be as wide as the wagon's place on the rails, where 18 cut beignet short. On the card after a turn, a name the card's letters can't hold is written in smaller ones, as the unit's own card writes terminal; a name too long for both shows no card, and the wagon's label names it. While the card is up it hides the last wagon's label whole. The number on the save screen shows both its digits. And the train's page no longer goes away under fast turns: turning several knobs fast could leave the sound's own effect page on the screen in place of the train, for good, with T3 there switching the effect off; version 19 notices it within a few frames and brings the train back, and T3 in that state brings it back too.
- **[B] The budget shown.** A turn onto an effect that won't fit stops on it, in grey, with FULL and what it needs, instead of skipping it. Versions 17 and 18 went past such an effect to the next one that fits, so effects seemed to be missing from the list.

## Version 18

Version 18 has run on a unit, beside lfo 12, and I've played it: old trains on a synth sound and a drum sound coming back on train by themselves, most of the keys above, saving a train, the list of effects, another effect and back, a restart, and train picked on the master.

Not tried on a unit yet: a sound saved and loaded again, on train and off it; loading a saved train from the list; moving a wagon (shift + turn); holding a knob to couple or uncouple it, shift + hold and shift + tap; renaming and deleting on the list of saved trains; a wagon carrying another pack's effect; MIDI changing the effect of a sound that's on train; the LFO page's FX destination on a sound that's on train; and turning onto an effect that won't fit (the budget skip, new in version 17).

To go back, copy fx train 16 over it, the last version before 18 that has run on a unit (17 never has): [its file is at commit `2bcfe6a`](https://github.com/LaP0573/open-field-packs/raw/2bcfe6a7c946357002db0bbee3d2714466b458c3/fx%20train.pack). Version 16 reads 18's sounds and saved trains as its own, and a sound you took off the train plays all its coupled wagons again there. T3 is then the train's on every sound, as it was.

One known case, rare but real. On the wagon under the lamp:

1. Turn it to none and press another sound's key within about 0.15 s, before the turn has taken.
2. Later, turn the same wagon to an effect and again press another sound's key that quickly.

The train can then play a different effect than it shows, and go over its budget. To fix it, turn that wagon to an effect that fits, or hold it to uncouple it.
