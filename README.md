# open-field packs

The finished pack files of [open-field](https://github.com/LaP0573/open-field), ready to go onto an OP-1 Field that runs OS 1.7.3 with open-field installed. The files for OS 1.7.0 stay in this repository's history, at commit `03adcba` (the install page for OS 1.7.0 names them there). Each file here is built from the sources in the open-field repository; this repository holds only the finished files.

**Unofficial.** This project is not made, endorsed or supported by Teenage Engineering, and it is not affiliated with them.

**At your own risk.** Installing open-field changes the firmware of your unit. That is a risky process: a unit can end up unable to start for good ("bricked"), and installing it will void the warranty of your device. Whether to do it is your decision alone.

**A pack is a program that can read and write the files on your unit: take packs only from people you trust.** Holding the help key while you switch the unit on starts it with no packs.

## The packs

| Pack | What it does | Category | Version | Needs | Size | SHA-256 |
| --- | --- | --- | --- | --- | --- | --- |
| [8 tracks.pack](8%20tracks.pack) | Eight tape tracks instead of four, on the tape and in the mixer. | tape | 10 | base.pack, level 7 | 106,548 bytes | `4779bda0d21fc61245aebd1e434011d0c341a91e3ba16f37c759e1264c2da666` |
| [fx train.pack](fx%20train.pack) | A chain of effects on every sound, with its presets kept as files. | effects | 16 | base.pack, level 8 | 505,416 bytes | `bc88433ed295383944bc62d84452db226ea7d35f19565652d3338b3dcd1126ee` |
| [lfo.pack](lfo.pack) | Four LFOs on every sound, with their page and their presets. | modulation | 11 | base.pack, level 4 | 430,318 bytes | `62d6464407751dd9556d4e59462e31d7c87acd8b77c6f91682a65f45035baee4` |
| [sidechain.pack](sidechain.pack) | One tape track ducks another, set on a page under shift and the mixer key. | mixer | 10 | base.pack, level 7 | 107,850 bytes | `2268639259142f81e703c24d6446e3be74406a6d5ce708b21b964cd8232f723e` |

`base.pack`, the shared pack every other one needs, is not here: it lies beside the install page in the open-field repository (`builder/base.pack`), and the install page puts it on every unit it installs.

The example packs for people who write packs (`wink`, `yoyo`, `list`, `sketch`, `echo`, `dub`, `crush`, `ping`, `pluck`, `fold` and `trampolines`) are not here either: they live in the open-field repository, under `examples/`, and its build makes their files.

## Putting a pack on the unit

- **With the install page**: tick the pack in its step 5. The page downloads the file from this repository at the commit the open-field listing names, checks its SHA-256, and the helper copies it onto the unit with the rest.
- **Later, from the install page's packs section**: with open-field on the unit and the unit on USB, the page lists the packs the unit has and sends the ones you tick over USB MIDI, each file held to the same SHA-256. The unit asks you on its own screen before anything changes: T1 applies, T2 leaves everything as it was.
- **By hand**: download the file, put the unit into its disk mode, copy the file into the folder `packs`, leave the disk mode, and switch the unit off and on. The open-field guide, `docs/guide.md`, says more.

## Where the files come from

Every file is built byte for byte from the open-field repository's own sources: `python -I tools/build_all.py` there makes each one, and its `packs/built.json` keeps each file's size and SHA-256. The listing of packs, `catalog/packs.json` in open-field, names each file here by commit and SHA-256, and the install page holds every download to it.

A file here changes only with a new version of its pack, or when open-field moves to another firmware (the same version, built for the new firmware), built and tried on a unit first.

## Licence

MIT: [LICENSE](LICENSE).
