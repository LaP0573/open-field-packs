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

- **Turn**: choose the wagon's effect. The sound follows a moment after you stop turning.
- **Tap**: open that wagon's effect page, with all its knobs, and T3 brings you back. On one of the unit's own effects, the lamp moves to that wagon. On another pack's effect you get that pack's own page for it instead: its knobs set that wagon only, and the lamp stays put. On an empty wagon nothing happens.
- **Hold**: uncouple the wagon (it goes quiet and costs nothing) or couple it back.
- **Shift + turn**: move the wagon one step along the train.
- **Shift + tap**: random settings for that effect.
- **Shift + hold**: back to its starting settings.

## Which effects

All the unit's own effects except the vocoder (fazer only if you've unlocked it), plus the effects other packs add. The same effect can sit on two wagons. On the wagon under the lamp, other packs' effects are skipped: they can sit on the other three.

The train has a processing budget. Turning onto an effect that won't fit skips it and lands on the next one that does. If nothing fits, the screen shows FULL in red for a second.

Some packs' effects have extra settings beyond their four knobs (often on shift). If the pack is made to keep them per wagon, which open-field 0.26 made possible, each wagon keeps its own, in the sound and in your saved trains. Otherwise they belong to the sound: two wagons with the same effect on one sound share them, and a saved train doesn't carry them.

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

fx train 19 needs open-field 0.26 (base 44) or later. Older releases leave it out (the start screen counts it as left out), and nothing of it runs. The install page puts on 0.26, and the start screen then shows `1.7.3 - open-field 0.26`.

## Version 19

New in 19: a tap on a wagon carrying another pack's effect opens that effect's own page (galfaroth noticed it did nothing in 18), and a wagon can keep its own extra settings for a pack's effect.

Version 19 has run on a unit, on open-field 0.26 and the test build of it just before: crush on a wagon, a tap opening crush's own page with that wagon's values, a turn moving the wagon's value, and T3 back to the train; galfaroth's veffbode 13 from the listing on a wagon, with its animated page, its knobs and its shift settings; and a test effect made to keep a setting per wagon, which kept it across a restart. No faults in the unit's log. I haven't played 19 by ear.

Not tried on a unit yet: a saved sound or a saved train bringing back a wagon's own settings; two wagons with the same pack's effect on one sound; a sound saved and loaded again, on train and off it; loading a saved train from the list; moving a wagon (shift + turn); holding a knob to couple or uncouple it, shift + hold and shift + tap; renaming and deleting on the list of saved trains; MIDI changing the effect of a sound that's on train; the LFO page's FX destination on a sound that's on train; and turning onto an effect that won't fit (the budget skip, new in version 17).

To go back, copy fx train 18 over it: [its file is at commit `270e04f`](https://github.com/LaP0573/open-field-packs/raw/270e04fd6d401bb93b3cb780f189f1cfea592fb4/fx%20train.pack). Version 18 reads 19's sounds and saved trains as its own and leaves out the settings 19 keeps for a pack's effect on a wagon. A tap on such a wagon then opens nothing again.

One known case, rare but real. On the wagon under the lamp:

1. Turn it to none and press another sound's key within about 0.15 s, before the turn has taken.
2. Later, turn the same wagon to an effect and again press another sound's key that quickly.

The train can then play a different effect than it shows, and go over its budget. To fix it, turn that wagon to an effect that fits, or hold it to uncouple it.
