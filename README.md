# open-field packs

The finished pack files of [open-field](https://github.com/LaP0573/open-field), ready to go onto an OP-1 Field that runs OS 1.7.3 with open-field installed. The files for OS 1.7.0 stay in this repository's history, at commit `03adcba` (the install page for OS 1.7.0 names them there). Each file here is built from the sources in the open-field repository; this repository holds only the finished files.

**Unofficial.** This project is not made, endorsed or supported by Teenage Engineering, and it is not affiliated with them.

**At your own risk.** Installing open-field changes the firmware of your unit. That is a risky process: a unit can end up unable to start for good ("bricked"), and installing it will void the warranty of your device. Whether to do it is your decision alone.

**A pack is a program that can read and write the files on your unit: take packs only from people you trust.** Holding the help key while you switch the unit on starts it with no packs.

## The packs

| Pack | What it does | Category | Version | Needs | Size | SHA-256 |
| --- | --- | --- | --- | --- | --- | --- |
| [8 tracks.pack](8%20tracks.pack) | Eight tape tracks instead of four, on the tape and in the mixer. | tape | 7 | base.pack | 99,484 bytes | `dffc324f1cb0a45fd42592418678904686f5cb240f4b135f4804d43f6cd8bf2f` |
| [fx train.pack](fx%20train.pack) | A chain of effects on every sound, with its presets kept as files. | effects | 10 | base.pack | 426,794 bytes | `58d07f9f5c69ba25e50b59b4920961da5a861e931dbf9a2e621775e49b5f68f8` |
| [lfo.pack](lfo.pack) | Four LFOs on every sound, with their page and their presets. | modulation | 8 | base.pack | 375,422 bytes | `22f7eafa2f002100ddb5d733a47b2de58b664e5cb2565090c11b72e6871e3363` |
| [sidechain.pack](sidechain.pack) | One tape track ducks another, set on a page under shift and the mixer key. | mixer | 7 | base.pack | 104,062 bytes | `505570137195cff863a7b794ec35c70b59ad239528ba93c46095cfa16cf689be` |

`base.pack`, the shared pack every other one needs, is not here: it lies beside the install page in the open-field repository (`builder/base.pack`), and the install page puts it on every unit it installs.

The example packs for people who write packs (`wink`, `yoyo` and `list`) are not here either: they live in the open-field repository, under `examples/`, and its build makes their files.

## Putting a pack on the unit

- **With the install page**: tick the pack in its step 5. The page downloads the file from this repository at the commit the open-field listing names, checks its SHA-256, and the helper copies it onto the unit with the rest.
- **By hand**: download the file, put the unit into its disk mode, copy the file into the folder `packs`, leave the disk mode, and switch the unit off and on. The open-field guide, `docs/guide.md`, says more.

## Where the files come from

Every file is built byte for byte from the open-field repository's own sources: `python -I tools/build_all.py` there makes each one, and its `packs/built.json` keeps each file's size and SHA-256. The listing of packs, `catalog/packs.json` in open-field, names each file here by commit and SHA-256, and the install page holds every download to it.

A file here changes only with a new version of its pack, or when open-field moves to another firmware (the same version, built for the new firmware), built and tried on a unit first.

## Licence

MIT: [LICENSE](LICENSE).
