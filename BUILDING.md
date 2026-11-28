# Building and changing the packs

This page is for people who write packs. If you just want to play them, the [README](README.md) and each pack's manual are all you need.

## The files

| Pack | What it does | Category | Version | Needs | Sources | Size | SHA-256 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| [8 tracks.pack](8%20tracks.pack) | Eight tape tracks instead of four, on the tape and in the mixer. | tape | 12 | base.pack, level 7 | [`8 tracks/`](8%20tracks) | 109,084 bytes | `b4048ba43aeff15a0a283a3fe845c0e731774cee66177644ef5875c6b9f866be` |
| [fx train.pack](fx%20train.pack) | A chain of effects on every sound, with its presets kept as files. | effects | 19 | base.pack, level 8 | [`fx train/`](fx%20train) | 511,600 bytes | `69869f1f6dd5cac150a30ac6548e9f28c0c87182acdcdfb07b15c54cab3d86ab` |
| [lfo.pack](lfo.pack) | Four LFOs on every sound, with their page and their presets. | modulation | 12 | base.pack, level 8 | [`lfo/`](lfo) | 429,278 bytes | `72d167e802ffd671d523c41f1324d01978e21df44c48549d21e5a1a33e9ed2c7` |
| [maestro.pack](maestro.pack) | Chords from one key, with the next chords or a melody on the right half of the keyboard. | sequencing | 10 | base.pack, level 5 | none: the file only | 337,672 bytes | `fb4d05964d1195d04b36e6b85a886da95008a11cbf2446cf93eff3f8b7fb56fb` |
| [sidechain.pack](sidechain.pack) | One tape track ducks another, set on a page under shift and the mixer key. | mixer | 10 | base.pack, level 7 | [`sidechain/`](sidechain) | 107,850 bytes | `2268639259142f81e703c24d6446e3be74406a6d5ce708b21b964cd8232f723e` |

`maestro` is here as its finished file only. Its sources are not published, so `build.py` does not build it.

`base.pack`, the shared pack every other one needs, is not here: it lies beside the install page in the open-field repository (`builder/base.pack`), and the install page puts it on every unit it installs.

The example packs for people who write packs (`wink`, `yoyo`, `list`, `sketch`, `echo`, `dub`, `crush`, `ping`, `pluck`, `fold` and `trampolines`) are not here either: they live in the open-field repository, under `examples/`, and its build makes their files.

The files for OS 1.7.0 stay in this repository's history, at commit `03adcba`.

## What is where

| Path | What |
| --- | --- |
| `8 tracks/`, `fx train/`, `lfo/`, `sidechain/` | A pack each: its manual (`README.md`), its sources, `recipe.json` (what it is built from and what its card asks of the base), `profile.json` (the places of the firmware it changes, in words and checksums, no byte of the firmware), and in `pictures/` the instructions its screens are made by |
| `maestro/` | Its manual (`README.md`) and its picture. No sources |
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

It takes the tools and headers from the commit `open-field.json` names, whatever that checkout stands at, builds into `out/`, and says `the same` for each file that comes out byte for byte as kept.

## Changing one

Change the source. Give the pack a higher `version` in its `recipe.json`. Then write the made files again: the table, the card, the directory of pictures and the page of the firmware's functions. The cards are held to your unit's own pictures, so this needs a copy of its folder `content/display`, kept outside this repository:

```bash
python -I build.py --open-field ../open-field --out out-make --make --display ../display
```

And keep what it builds:

```bash
python -I build.py --open-field ../open-field --out out-new --update
```

A pull request is welcome. Say what you tried on a unit and what you did not. open-field's [rules for a change](https://github.com/LaP0573/open-field/blob/main/CONTRIBUTING.md) hold here too, the first above all: nothing of the firmware goes into this repository, not a file, not a log, not a few bytes in a comment.

A file here changes only with a new version of its pack, or when open-field moves to another firmware (the same version, built for the new firmware). It is tried on a unit first, or the pack's manual says that it was not.

What is merged into `main` here reaches owners by itself: the open-field listing follows this repository's `main`, and the install page reads each file from there when it opens, a few minutes after the merge at most. So a new file goes into `main` only when it is ready.
