# Faust's packs for open-field

These are my packs for [open-field](https://github.com/LaP0573/open-field), for an OP-1 Field on OS 1.7.3.

I want them to feel stock. Each one should look and play like something Teenage Engineering could have shipped: the same screens, the same keys, the same habits. They don't change what the Field is. They fill in the places where I kept wishing it did a little more.

## The packs

| Pack | What it does | |
| --- | --- | --- |
| 8 tracks | Eight tape tracks instead of four. | [manual](8%20tracks/README.md) |
| fx train | Up to four effects in a row on each sound, with presets. Version 17 hasn't been on a unit yet. | [manual](fx%20train/README.md) |
| lfo | Four LFOs on each synth sound, with presets. | [manual](lfo/README.md) |
| sidechain | One tape track ducks another, or ducks to the beat. | [manual](sidechain/README.md) |

Each one works on its own or with the others.

## Getting them

The easiest way is [the install page](https://lap0573.github.io/open-field/): tick the packs you want. It always offers the latest version from this repository.

By hand: download the `.pack` file, put the unit in disk mode, copy the file into the `packs` folder, leave disk mode and restart. To remove a pack, delete its file the same way.

## Before you install

- **Unofficial.** Not made, endorsed or supported by Teenage Engineering, and not affiliated with them.
- **At your own risk.** open-field changes your unit's firmware. That can leave a unit unable to start ("bricked"), and it voids the warranty.
- **A pack is a program that can read and write the files on your unit.** Only install packs from people you trust. If one acts up, hold the help key (the speech bubble) while you switch on: the unit starts without any packs, that one time.

## Credits

The fix of the cost ceiling in `fx train` 17 is from [open-field's pull request 8](https://github.com/LaP0573/open-field/pull/8), by [hooligan333](https://github.com/hooligan333).

Soloing tracks 5 to 8 in `8 tracks` 11 answers [open-field's issue 6](https://github.com/LaP0573/open-field/issues/6), found and worked out by [hasis](https://github.com/hasis).

## For developers

The sources of every pack are here. [BUILDING.md](BUILDING.md) says how to build them, change them and send a change.

MIT licence: [LICENSE](LICENSE).
