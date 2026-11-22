# open-field packs

`8 tracks`, `fx train`, `lfo` and `sidechain`: four packs for [open-field](https://github.com/LaP0573/open-field) on an OP-1 Field with OS 1.7.3. Each is here twice: as the finished file you put on your unit, and as everything that file is built from. The files for OS 1.7.0 stay in this repository's history, at commit `03adcba` (the install page for OS 1.7.0 names them there).

If you write packs, read them. They use the base the way your pack would, through a card, and they are a good deal bigger than open-field's examples.

**Unofficial.** This project is not made, endorsed or supported by Teenage Engineering, and it is not affiliated with them.

**At your own risk.** Installing open-field changes the firmware of your unit. That is a risky process: a unit can end up unable to start for good ("bricked"), and installing it will void the warranty of your device. Whether to do it is your decision alone.

**A pack is a program that can read and write the files on your unit: take packs only from people you trust.** Holding the help key while you switch the unit on starts it with no packs.

## The packs

| Pack | What it does | Category | Version | Needs | Sources | Size | SHA-256 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| [8 tracks.pack](8%20tracks.pack) | Eight tape tracks instead of four, on the tape and in the mixer. | tape | 10 | base.pack, level 7 | [`8 tracks/`](8%20tracks) | 106,548 bytes | `4779bda0d21fc61245aebd1e434011d0c341a91e3ba16f37c759e1264c2da666` |
| [fx train.pack](fx%20train.pack) | A chain of effects on every sound, with its presets kept as files. | effects | 17 | base.pack, level 8 | [`fx train/`](fx%20train) | 508,460 bytes | `d02ff1b4aba79ea3b5cbfd674c178138fa272d71f2dddf7aeffd5dd9656b147d` |
| [lfo.pack](lfo.pack) | Four LFOs on every sound, with their page and their presets. | modulation | 11 | base.pack, level 4 | [`lfo/`](lfo) | 430,318 bytes | `62d6464407751dd9556d4e59462e31d7c87acd8b77c6f91682a65f45035baee4` |
| [sidechain.pack](sidechain.pack) | One tape track ducks another, set on a page under shift and the mixer key. | mixer | 10 | base.pack, level 7 | [`sidechain/`](sidechain) | 107,850 bytes | `2268639259142f81e703c24d6446e3be74406a6d5ce708b21b964cd8232f723e` |

**`fx train` 17 has not run on a unit yet.** It was built and proven on a computer. `fx train` 16, which has run on one, is [the file at commit `2bcfe6a`](https://github.com/LaP0573/open-field-packs/raw/2bcfe6a7c946357002db0bbee3d2714466b458c3/fx%20train.pack): copy that over it to go back.

One case is known in `fx train` 17. If you turn the first wagon's knob and press another sound's key before the turn has taken effect (about 0.15 seconds), and that happens twice, first on a turn to none, the train can play a different effect than it shows and run past its limit. Turn that knob to an effect that fits, or hold it to switch the wagon off.

`base.pack`, the shared pack every other one needs, is not here: it lies beside the install page in the open-field repository (`builder/base.pack`), and the install page puts it on every unit it installs.

The example packs for people who write packs (`wink`, `yoyo`, `list`, `sketch`, `echo`, `dub`, `crush`, `ping`, `pluck`, `fold` and `trampolines`) are not here either: they live in the open-field repository, under `examples/`, and its build makes their files.

## Putting a pack on the unit

- **With the install page**: tick the pack in its step 5. The page downloads the file from this repository at the commit the open-field listing names, checks its SHA-256, and the helper copies it onto the unit with the rest.
- **Later, from the install page's packs section**: with open-field on the unit and the unit on USB, the page lists the packs the unit has and sends the ones you tick over USB MIDI, each file held to the same SHA-256. The unit asks you on its own screen before anything changes: T1 applies, T2 leaves everything as it was.
- **By hand**: download the file, put the unit into its disk mode, copy the file into the folder `packs`, leave the disk mode, and switch the unit off and on. The open-field guide, `docs/guide.md`, says more.

## What is where

| Path | What |
| --- | --- |
| `8 tracks/`, `fx train/`, `lfo/`, `sidechain/` | A pack each: its sources, `recipe.json` (what it is built from and what its card asks of the base), `profile.json` (the places of the firmware it changes, in words and checksums, no byte of the firmware), and in `pictures/` the instructions its screens are made by |
| `shared/` | What `fx train` and `lfo` both use |
| `open-field.json` | The commit of open-field these are built with, and the four headers taken from it |
| `build.py` | The build |
| `built.json` | The size and SHA-256 each finished file must have |
| `firmware-interfaces.md`, `firmware-interfaces.json` | The firmware's functions these sources call, and the words for them |

What to read for what, if you came to learn the base:

- `8 tracks/`: level 7. Values kept with each tape, and pictures that take the place of the unit's. It also shows a pack that has to stand deep in the firmware.
- `fx train/`: level 8. It runs other packs' effects on its wagons, keeps presets as files in folders of its own, and keeps a train with each sound.
- `lfo/`: level 4. The smallest set of asks of the four: a call every frame, pictures and folders, and it works with `fx train` when that is there.
- `sidechain/`: level 7. The shortest of the four to read: a page, values kept with each tape, and a partner pack it can do without.

In a comment, a path that begins with `module/` is a file of open-field, or an older form of one of these files that was never published.

## Building them

This works the way [A first pack](https://github.com/LaP0573/open-field/blob/main/docs/first-pack.md) builds a pack of your own, with open-field's tools and headers and nothing else of it. You need a checkout of open-field with its toolchain in `open-xy/` ([building](https://github.com/LaP0573/open-field/blob/main/docs/build.md) says how), and then:

```bash
python -I build.py --open-field ../open-field
```

It takes the tools and headers from the commit `open-field.json` names, whatever that checkout stands at, builds into `out/`, and says `the same` for each file that comes out byte for byte as kept. The listing of packs, `catalog/packs.json` in open-field, names each file here by commit and SHA-256, and the install page holds every download to it.

## Changing one

Change the source. Give the pack a higher `version` in its `recipe.json`. Then write the made files again: the table, the card, the directory of pictures and the page of the firmware's functions. The cards are held to your unit's own pictures, so this needs a copy of its folder `content/display`, kept outside this repository:

```bash
python -I build.py --open-field ../open-field --out out-make --make --display ../display
```

And keep what it builds:

```bash
python -I build.py --open-field ../open-field --out out-new --update
```

Put the new version, size and SHA-256 into the table at the top of this page too.

A pull request is welcome here. Say what you tried on a unit and what you did not. open-field's [rules for a change](https://github.com/LaP0573/open-field/blob/main/CONTRIBUTING.md) hold here too, the first above all: nothing of the firmware goes into this repository, not a file, not a log, not a few bytes in a comment.

A file here changes only with a new version of its pack, or when open-field moves to another firmware (the same version, built for the new firmware). It is tried on a unit first, or this page says that it was not.

## Credits

`fx train` 17's fix of the cost ceiling (a turn that goes on to the next effect that fits, and a hold that is held to the ceiling) is from [open-field's pull request 8](https://github.com/LaP0573/open-field/pull/8), by [hooligan333](https://github.com/hooligan333).

## Licence

MIT: [LICENSE](LICENSE).
