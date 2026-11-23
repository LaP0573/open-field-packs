# fx train

Up to four effects in a row on every synth and drum sound. The train has four wagons, one for each knob colour. Each wagon carries an effect, and the coupled ones play in order after the engine.

## Opening it

- **T3** opens the train from any page of the sound.
- **T3** again opens the page of the wagon under the lamp. T3 once more brings you back.
- T1, T2, T4, or leaving the sound, closes it.

## Knobs on the train

- **Turn**: choose the wagon's effect. The sound follows a moment after you stop turning.
- **Tap**: open that wagon's effect page, with all its knobs. The wagon moves under the lamp. On an empty wagon, the effect list opens instead.
- **Hold**: uncouple the wagon (it goes quiet and costs nothing) or couple it back.
- **Shift + turn**: move the wagon one place along the train.
- **Shift + tap**: random settings for that effect.
- **Shift + hold**: back to its starting settings.

## Which effects

All the unit's own effects except the vocoder (fazer only if you've unlocked it), plus the effects other packs add. The same effect can sit on two wagons.

The train has a processing budget. Turning onto an effect that won't fit skips it and lands on the next one that does. If nothing fits, the screen shows FULL in red for a second.

If a wagon carries another pack's effect and you remove that pack, the wagon stays, silent and drawn empty, with its settings kept. Put the pack back and it plays again.

## Presets

**Shift + T3**: tap for the list of saved trains, hold to save the one you're on. A new save takes the first free slot, named "chain" and its number.

On the list:

- **Blue**: scroll. Tap blue or press T3 to load.
- **Tap ochre**: rename. Blue moves along, ochre changes the letter, grey or T3 keeps it, red cancels.
- **Tap grey**: save.
- **Hold red**: delete.

Saved trains are files, 64 slots, in the `fx` folder in disk mode. Copy them to your computer or swap them with friends.

## Saved with your sounds

Each sound keeps its own train. Save a synth or drum sound and its train goes into the sound file with it, and comes back when you load that sound. A stock sound, or one saved before, arrives with its single effect on the blue wagon. Changing the engine keeps the train.

## Without the pack

T3 is stock again, and the sound keeps one effect: the one that was under the lamp. Your saved trains stay on the disk.

## Version 17

Version 17 hasn't been on a unit yet: it was built and checked on a computer only. What's new in it is the budget skip above. Version 16 has run on a unit: [its file is at commit `2bcfe6a`](https://github.com/LaP0573/open-field-packs/raw/2bcfe6a7c946357002db0bbee3d2714466b458c3/fx%20train.pack), copy it over 17 to go back.

One known case, rare but real. On the wagon under the lamp:

1. Turn it to none and press another sound's key within about 0.15 s, before the turn has taken.
2. Later, turn the same wagon to an effect and again press another sound's key that quickly.

The train can then play a different effect than it shows, and go over its budget. To fix it, turn that wagon to an effect that fits, or hold it to uncouple it.

Not tried on a unit yet: a train saved into a drum sound, and a saved sound bringing back its LFOs from the lfo pack.
