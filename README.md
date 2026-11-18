# open-field packs

The finished pack files of [open-field](https://github.com/LaP0573/open-field), ready to go onto an OP-1 Field that runs OS 1.7.0 with open-field installed. Each file here is built from the sources in the open-field repository; this repository holds only the finished files.

**Unofficial.** This project is not made, endorsed or supported by Teenage Engineering, and it is not affiliated with them.

**At your own risk.** Installing open-field changes the firmware of your unit. That is a risky process: a unit can end up unable to start for good ("bricked"), and installing it will void the warranty of your device. Whether to do it is your decision alone.

**A pack is a program that can read and write the files on your unit: take packs only from people you trust.** Holding the help key while you switch the unit on starts it with no packs.

## The packs

| Pack | What it does | Category | Version | Needs | Size | SHA-256 |
| --- | --- | --- | --- | --- | --- | --- |
| [8 tracks.pack](8%20tracks.pack) | Eight tape tracks instead of four, on the tape and in the mixer. | tape | 7 | base.pack | 99,476 bytes | `c80be48f3623e551fb6a65d518c59966212a35b955dbbf923c271b8ec8df346e` |
| [fx train.pack](fx%20train.pack) | A chain of effects on every sound, with its presets kept as files. | effects | 10 | base.pack | 426,790 bytes | `dd8666939dd3e939fa0065d9988ac07284676af09202010f349f36b3d8846429` |
| [lfo.pack](lfo.pack) | Four LFOs on every sound, with their page and their presets. | modulation | 8 | base.pack | 375,422 bytes | `6df6301e9e2e222bdb4dc331d71b6982e0b2529067ea686a9c3eada1f5e68b6c` |
| [sidechain.pack](sidechain.pack) | One tape track ducks another, set on a page under shift and the mixer key. | mixer | 7 | base.pack | 104,062 bytes | `217125222686ebb9d0aeaf2f0e8e5eb5fa3baf3bbbbe710febc6d4a1cd196a00` |
| [wink.pack](wink.pack) | The example of level 1: a red dot that blinks on the tempo screen. | examples | 1 | base.pack, level 1 | 2,130 bytes | `b8b685f2b13cf058c000382a643db2a2c270f1604bdee59df24f082e003784a0` |
| [yoyo.pack](yoyo.pack) | The example of level 2: shift and the metronome key open a page where the blue knob moves a yoyo. | examples | 1 | base.pack, level 2 | 4,381 bytes | `053ca1c45125c494e1679b284119099c86a106fd7256a701d79082e5dabd9f27` |
| [list.pack](list.pack) | The example of level 3: shift and the help key open a page that lists the files of its own folder. | examples | 1 | base.pack, level 3 | 7,281 bytes | `973413192503526b0f3e7d8b8201de84a512a1b716f8341598ce1c2478b071f8` |

`base.pack`, the shared pack every other one needs, is not here: it lies beside the install page in the open-field repository (`builder/base.pack`), and the install page puts it on every unit it installs.

## Putting a pack on the unit

- **With the install page**: tick the pack in its step 5. The page downloads the file from this repository at the commit the open-field listing names, checks its SHA-256, and the helper copies it onto the unit with the rest.
- **By hand**: download the file, put the unit into its disk mode, copy the file into the folder `packs`, leave the disk mode, and switch the unit off and on. The open-field guide, `docs/guide.md`, says more.

## Where the files come from

Every file is built byte for byte from the open-field repository's own sources: `python -I tools/build_all.py` there makes each one, and its `packs/built.json` keeps each file's size and SHA-256. The listing of packs, `catalog/packs.json` in open-field, names each file here by commit and SHA-256, and the install page holds every download to it.

A file here changes only with a new version of its pack, built and tried on a unit first.

## Licence

MIT: [LICENSE](LICENSE).
