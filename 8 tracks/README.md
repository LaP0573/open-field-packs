# 8 tracks

Eight tape tracks instead of four. Tracks 5 to 8 record, play, loop, lift, drop, clear and undo just like 1 to 4, and the tape, mixer and signal-flow screens show all eight.

## Controls

On the tape:

- **Hold tape + press T1 to T4**: select track 5 to 8. T1 to T4 on their own still pick 1 to 4.
- **Hold tape + hold T1 to T4**: solo track 5 to 8, the same way holding T1 to T4 solos 1 to 4. Hold several to solo them together.
- **Hold tape + shift + T1 to T4**: mute or unmute track 5 to 8. Shift + T1 to T4 still mutes 1 to 4.
- **Hold tape + blue / ochre**: volume and pan of the selected track, any of the eight.

In the mixer, on the levels page:

- **Hold the mixer key** to see tracks 5 to 8. While you hold it, the knobs set their levels, shift + a knob their pan (shift + a click puts it back in the centre), and a click of a knob or shift + T1 to T4 mutes or unmutes that track. Let go and you're back on 1 to 4.

## What's saved

Everything stays with the tape: the audio, the clips, the volume and pan of 5 to 8, and which track was selected. Mutes aren't saved, same as stock.

In disk mode, each tape folder shows `track_5.wav` to `track_8.wav` next to the usual four.

## Without the pack

The stock firmware still plays the tape: tracks 1 to 4 are stored exactly the way it stores them, and 5 to 8 sit beside them in files it leaves alone.

One thing to watch: if you record on that tape without the pack, the firmware may reuse tape that tracks 5 to 8 were using. When the pack is back, those parts of 5 to 8 play silence. Tracks 1 to 4 are never touched.

## Not tried on a unit yet

MIDI control of tracks 5 to 8 (channels 5 to 8 should set their level, pan and mute), importing a file onto `track_5.wav` to `track_8.wav` in disk mode, and the cassette and mini-disc tape styles with eight tracks.

## For other packs: tracks 5 to 8 through four names (version 14)

Version 14 offers four names to every pack, so a pack that rewrites a tape in place (speedlock, which locks the tape's varispeed in) can take tracks 5 to 8 with tracks 1 to 4. Nothing of 8 tracks calls them: with no pack that asks for them, version 14 does what version 13 does. The interface stays 2: nothing that was there changed its meaning.

| Name | What it is |
| --- | --- |
| `op1f_ext_api` | A word: 1. A later meaning of the names below takes a new number |
| `op1f_ext_map(record, track)` | The map of track 5 + `track` (0 to 3) in undo record `record` (0 to 7): 969 chunk numbers, `0xFFFF` for none. The live record is the firmware's own index. The pointer is good until the next snapshot or tape load moves the records: read it in the same pass of the main loop. 0 for an argument out of range |
| `op1f_ext_quiet()` | 1 when a change of these maps crosses nothing of 8 tracks: a tape is open, no take is being recorded, and the cache holds tracks 5 to 8 |
| `op1f_ext_put(track, chunk, physical)` | The live record's entry of track 5 + `track` at `chunk` made `physical` (`0xFFFF` for none), the way the firmware's map setter does it for tracks 1 to 4: the chunk that was there loses the live record's bit (unless it is none or the same), the new one gains it, its sum is unknown until its next whole read, and the map is marked for saving. 0, or -1 when an argument is out of range or it is not quiet (then nothing changed) |

How a pack uses them, the same way the firmware's own edits change tracks 1 to 4:

1. On the main loop (where the tape service and the firmware's edits run), while `op1f_ext_quiet()` answers 1, read the live record's maps.
2. Take free chunks on the firmware's reference bytes (the live record's bit, as its allocator does; 8 tracks takes its own from the top of the tape) and write the new audio there. Never write a chunk an undo record holds.
3. In one pass of the main loop: the firmware's `take_snapshot_ex` (8 tracks copies the maps of tracks 5 to 8 into the new record, so the firmware's undo takes them back), then `op1f_ext_put` for every entry that changes, then the firmware's map flush: 8 tracks writes the file of tracks 5 to 8 first, then the firmware's.

Undo and redo need nothing more: the maps of tracks 5 to 8 follow the firmware's undo index, as they always have.
