# The firmware's functions our code uses

Made by open-field's `tools/interfaces.py` from the sources and from `firmware-interfaces.json`; change those, not this page.

This page lists the functions of the firmware that the packs of this repository call, for OS 1.7.3. For each it says where the function stands, what the caller hands over, what comes back and which thread our code calls it on. It says nothing of how a function works inside: that is not needed to call it, and it is not written down in this repository ([CONTRIBUTING.md](https://github.com/LaP0573/open-field/blob/main/CONTRIBUTING.md), "Writing about the firmware").

**Every address belongs to one firmware file**, OS 1.7.3. On another version each one has to be found again ([methods](https://github.com/LaP0573/open-field/blob/main/docs/methods.md), "Moving the work to another firmware version").

**How a source binds a function.** Two lines: the function's type, and a name for its address. The type is ours; nothing of the firmware is copied.

```cpp
typedef int (*FileSizeFn)(const char* path);
#define STOCK_FILE_SIZE ((FileSizeFn)0x8c051f9cu)
```

**How arguments travel.** The first three go in the registers R0, R1 and R2, the rest on the stack. A result comes back in R0. A function that gives back a structure gets the address for it in P0. The compiler does all of that when the type is right, so a wrong type is a crash: before you bind a function that is not on this page, read how the firmware itself calls it ([methods](https://github.com/LaP0573/open-field/blob/main/docs/methods.md), "Reading a caller before you bind a function").

**The threads.** *The main thread* runs the start-up. *The display thread* draws a frame some sixty times a second; the shared part calls our frame functions from it. *The key thread* hands key presses and knob turns to the mode that is shown. *The audio thread* makes the sound, a block of 64 samples at a time, and must never wait: no log line, no file, no memory asked for there. Where the last column says "ours", it names the threads our code calls the function on today; that it is safe on another thread has not been shown.

There are 127 functions bound in the sources. 38 are described below, by subject. The other 89 are listed at the end with their type and the files that bind them: their words are still to be written.

## The log

The unit writes a log into the folder `logs` of its disk. [Methods](https://github.com/LaP0573/open-field/blob/main/docs/methods.md) says how to copy a log off the unit and how to read one.

| Address | The call | What the caller hands over and gets back | Called from |
| --- | --- | --- | --- |
| `0x8c053a20` | `void log(int level, const char* file, int line, const char* format, ...)` | Writes one line into the unit's log. `level`: 3 for a line of information, 2 for a warning, 1 for an error. `file`: a text in the form of a source path; the log shows what stands between its last slash and its dot as the line's tag, so `op1f/mypack.cpp` gives the tag `mypack`. A text without a slash and a dot gives a line with no tag. `line`: a number shown in brackets behind the tag. `format` and the values behind it: as `printf` takes them. A line is cut at about 260 characters, the time and the tag counted. | Any thread but the audio thread: keep a count there and print it from a frame function. Ours: the main thread at the start, the display thread, the key thread, the tape's background thread. |

## Files on the unit's disk

A path is a C text, counted from the top of the unit's disk, with no slash in front: `user/pk/kit.pack`. There are two families of calls. Use one family for a file from its open to its close.

| Address | The call | What the caller hands over and gets back | Called from |
| --- | --- | --- | --- |
| `0x8c051f9c` | `int file_size(const char* path)` | The size of the file at a path, in bytes; -1 when there is no such file. It writes nothing into the log, so it is the way to ask whether a file is there. | Not the audio thread. Ours: the main thread at the start, the display thread, the disk mode's thread. |
| `0x8fc22180` | `i32 fopen(const char* path, const char* mode)` | The second family, which the tape's code uses. Opens a file with a mode as C has it (`rb`, `w+b`). Gives back a descriptor, which may be 0; a negative number when the open failed, after a line of its own in the log. To ask whether a file is there, use `file_size`, which writes no line. | Not the audio thread. |
| `0x8fc2214c` | `i32 fread(void* destination, i32 size, i32 count, i32 file)` | Reads `count` items of `size` bytes from a file opened with `fopen`. Gives back the number of items read. | As `fopen`. |
| `0x8fc220b8` | `i32 fwrite(const void* source, i32 size, i32 count, i32 file)` | Writes `count` items of `size` bytes to a file opened with `fopen`. Gives back the number of items written, and writes a line into the log itself when it fails. | As `fopen`. |
| `0x8fc220a0` | `void fclose(i32 file)` | Closes a file opened with `fopen`. | As `fopen`. |
| `0x8fc222f4` | `void rename(const char* old_path, const char* new_path)` | Gives a file another path, over a file that is there already. Our code writes a file under a spare name and renames it over the real one, so that a power cut leaves the old file or the new one, never half of one. | As `fopen`. |
| `0x8fcbc350` | `u32 file_mtime(const char* path)` | A number for when the file was last written; 0 when there is no such file. Our code only compares two such numbers: of two files, the one written later never has the smaller number. What the number counts in has not been established. | Not the audio thread. |

## JSON values and JSON files

The firmware keeps its settings as JSON, and so does our code. A JSON value is a small object of 12 bytes in memory the caller provides (our code keeps 16). It owns what it holds: make it with `json_ctor`, give it back with `json_dtor`. What `json_at`, `json_get` and `json_lookup` give back is a reference into another value, not a value of the caller's to destroy. A value has a kind: 0 nothing (null), 1 an integer, 2 an unsigned number, 3 a number with a fraction, 4 a text, 5 true or false, 6 an array, 7 an object. A number read from a file has the kind 2 or 3, never 1: do not test for 1, ask `json_int`.

| Address | The call | What the caller hands over and gets back | Called from |
| --- | --- | --- | --- |
| `0x8c085ba8` | `void json_ctor(void* json, int kind)` | Makes a value of a kind in the caller's 12 bytes. Kind 0 makes a null, which becomes an array at its first `json_push`. | Not the audio thread. Ours: the main thread at the start, the display thread, the key thread. |
| `0x8c0844e0` | `void json_dtor(void* json)` | Gives back what a value holds. Call it for every value made with `json_ctor`, and for none that is a reference. | As `json_ctor`. |
| `0x8c0534d0` | `int json_read_file(const char* path, void* json)` | Reads a JSON file and parses it into a value the caller has made. Not 0 when the file was read and parsed. | As `json_ctor`. |
| `0x8c053168` | `int json_write_file(void* path_string, void* json)` | Writes a value as a JSON file. The path is a text object of the firmware (`string_ctor` below), not a C text. The file is written under a spare name in the folder `tmp` and then renamed over the target. 0 comes back when that failed: look at the answer, and make the target's folder first. | As `json_ctor`. |
| `0x8c084840` | `int json_int(void* json)` | A number as an `int`. It takes all three kinds of number. For any other kind it gives back 0 and writes a line into the log. | As `json_ctor`. |
| `0x8c084b94` | `int json_bool(void* json)` | A value of the kind true-or-false as 1 or 0. | As `json_ctor`. |
| `0x8c084c22` | `int json_size(void* json)` | How many elements an array has. Ask it of an array only. | As `json_ctor`. |
| `0x8c084ee0` | `void* json_at(void* json, int index)` | An array's element by its number, from 0: a reference into the array. | As `json_ctor`. |
| `0x8c085068` | `int json_has(void* json, const char* key)` | Not 0 when an object has a member of that name. | As `json_ctor`. |
| `0x8c084f74` | `void* json_get(void* json, const char* key)` | An object's member by its name: a reference into the object. Our code asks `json_has` first and never asks for a member that is not there. | As `json_ctor`. |
| `0x8c085e84` | `void* json_lookup(void* json, const char* key)` | An object's member by its name, as `json_get` gives it; a member that is not there is made, as a null. The way to add a member: look it up, then assign to it. | As `json_ctor`. |
| `0x8c0845b8` | `void json_assign(void* target, void* value)` | Gives a value the content of another. The target may be a reference, such as a member got with `json_lookup`. | As `json_ctor`. |
| `0x8c085d54` | `void json_push(void* array, void* value)` | Puts a copy of a value at an array's end. A null becomes an array at its first push. | As `json_ctor`. |
| `0x8c083b1c` | `void json_from_int(void* json, int value)` | Makes a value an integer (the kind 1). | As `json_ctor`. |
| `0x8c083aec` | `void json_from_bool(void* json, int value)` | Makes a value true or false (the kind 5). | As `json_ctor`. |
| `0x8c084430` | `void json_from_cstr(void* json, const char* text)` | Makes a value a text (the kind 4), copied from a C text. | As `json_ctor`. |

## Texts and memory

Our code has no library of its own: no `malloc`, no `memcpy`, no division by a variable. What it needs of memory it keeps in its own data, or asks of the firmware with the calls below.

| Address | The call | What the caller hands over and gets back | Called from |
| --- | --- | --- | --- |
| `0x8c03e434` | `void string_ctor(void* out, const char* text)` | Makes a text object of the firmware from a C text, in memory the caller provides (our code keeps 48 zeroed bytes for it). A text of up to fifteen characters lives inside the object, and nothing has to be freed afterwards. Our code keeps its paths that short for this reason: `user/op1f.json` fits, `user/lfo/01.json` does not. | Not the audio thread. Ours: the main thread at the start, the display thread, the key thread, the disk mode's thread. |
| `0x8fcc5a68` | `void* heap_malloc(int heap_index, u32 size)` | A block of one of two large heaps that the firmware itself hardly uses, by the heap's number, 0 or 1. Our code takes large blocks there once and keeps them: the tape's cache for tracks 5 to 8 from heap 1, the effect train's private regions from heap 0. Gives back 0 when the heap has no room. | Not the audio thread. Ours: the tape's set-up at the start, and the display thread. |

## Screens, pictures and texts

A screen of the firmware is an object with a picture: an SVG text, parsed into a document whose parts can be found by their id, shown, hidden, moved and given a text. Our own screens are made the same way.

| Address | The call | What the caller hands over and gets back | Called from |
| --- | --- | --- | --- |
| `0x8fc25cc0` | `void* find_node(void* screen, const char* id, int optional)` | Finds a part of a screen's picture by its id. With `optional` not 0 it gives back 0 for a part that is not there, and our code always asks that way. The screen must have its document: look at the object's second word before you ask. | Ours: the display thread. |
| `0x8c03dbbe` | `void apply_matrix(void* node, const u32* matrix)` | Gives a part of a picture a transform: nine words, each a number with ten bits behind the point, `[a, b, 0, c, d, 0, e, f, 1024]`, for x' = a·x + c·y + e and y' = b·x + d·y + f. Keep the third, sixth and ninth word at 0, 0 and 1024. | Ours: the display thread. |
| `0x8fc25abc` | `void* label_from_node(void* screen, void* node, int kind)` | Turns a part of a picture into a label, an object whose text can be set while the unit runs. The part is a placeholder: it gives the label its place and its colour. The label takes the part's place in the document and comes back; keep it, since the id finds it no more. Our code passes 0 for `kind`: the text then starts at the placeholder's left edge. | Ours: the display thread, once for each label. |
| `0x8fc3aa3c` | `void label_text(void* label, const char* text, int max_width)` | Sets a label's text from a C text. A text wider than `max_width` pixels is shortened and gets a mark at its end. An empty text hides the label. Send small letters and digits: the firmware's font has the letters at the small letters' codes and its own signs at the capitals'. Each call asks the heap for memory, so set a text only when it changes; setting the text a label already has changes nothing. | Ours: the display thread. |
| `0x8fc3820c` | `LabelBox label_bounds(void* label)` | The box of a label's text as it was last laid out: left, right, top and bottom, each a number with ten bits behind the point. It is the box of the shapes without their stroke, so an upright stroke is 0 wide. The box comes back as a structure, by value: bind the function as one that returns the structure, and the compiler hands over the address for it in P0. | Ours: the display thread. |

## Keys and the screen that is shown

| Address | The call | What the caller hands over and gets back | Called from |
| --- | --- | --- | --- |
| `0x8c0b2554` | `void request_screen(void)` | Asks the firmware to show the screen the present mode wants now. The firmware does this itself after every key; our code calls it after a change that no key press follows, such as a timer running out. It takes no argument. From the display thread, call it with interrupts masked for the length of the call: the firmware stops when two threads are inside it at once. | Ours: the key thread, and the display thread with interrupts masked. |
| `0x8fc24ca4` | `int shift(void)` | Not 0 while the shift key is held. | Ours: the key thread, the display thread. |

## Sounds and their LFOs

The unit has eight synth sounds and eight drum sounds, in slots 0 to 7 each. Each slot has a record, an object of the firmware's at a fixed address, which holds the sound's document: what the unit saves with the sound. A sound's LFO has a kind (a row of the list under shift and T4), eight words of settings and a switch. Our code changes them through the calls below, the way the unit's own keys do, so that what the unit shows, plays and saves agrees.

| Address | The call | What the caller hands over and gets back | Called from |
| --- | --- | --- | --- |
| `0x8d4f3c84` | `void lfo_words_doc(void* record, const u32* words)` | Writes eight words of LFO settings into the document of the sound whose record is `record`. Follow it with the record's LFO apply (`lfo_apply_synth` or `lfo_apply_drum`), so that the sound plays them. | Ours: the key thread. |
| `0x8d4f3d0c` | `void lfo_active_doc(void* record, int value)` | Writes the LFO's switch, `on` 1 or 0, into the document of the sound whose record is `record`. Our code calls it right after `lfo_on_synth` or `lfo_on_drum`, so that the switch the sound plays with is the one it is saved with. | Ours: the display thread. |
| `0x8d504308` | `void lfo_on_synth(u32 slot, int value)` | Switches the LFO of synth slot `slot` (0 to 7) on (`on` 1) or off (0), as the unit's own key does. It changes the switch the sound plays with; `lfo_active_doc` writes it into the document. | Ours: the key thread, the display thread. |
| `0x8d5042d8` | `void lfo_on_drum(u32 slot, int value)` | The same for drum slot `slot` (0 to 7). | As `lfo_on_synth`. |
| `0x8d50441a` | `void reset_drum(u32 voice)` | Restarts the LFO of drum voice `voice` (0 to 7) for a new note, as the unit does at each drum hit while the slot's LFO is on. Our code calls it from its own function in its place at its two calls, and restarts its own LFOs for the same voice after it. | The audio thread, where the firmware calls it. |

## Bound in the sources, not described yet

Each of these is called by the files named. The type is what our code hands over and takes back; the name is the sources' own. Read the file's comments at the binding before you use one, and add its words to `firmware-interfaces.json` when you have.

| Address | The call | Bound in |
| --- | --- | --- |
| `0x040201c4` | `void* memcpy(void* destination, const void* source, u32 count)` | `build_b_storage.cpp` |
| `0x04020218` | `void* memmove(void* destination, const void* source, u32 count)` | `build_b_storage.cpp` |
| `0x04020fd8` | `i32 float_to_int(u32 a)` | `build_e_screens.cpp`, `build_r4b_duck.cpp` |
| `0x04021310` | `u32 rom_float_to_int(u32)` | `build_b_mix.cpp` |
| `0x04021854` | `u32 float_from_int(i32 value)` | `build_e_screens.cpp`, `build_k_clip.cpp`, `build_k_clips.cpp`, `build_r4b_mixer.cpp` |
| `0x040218a0` | `u32 rom_value_to_float(u32)` | `build_b_mix.cpp` |
| `0x040218e0` | `u32 rom_float_sub(u32, u32)` | `build_b_mix.cpp` |
| `0x040218e2` | `u32 rom_float_add(u32 a, u32 b)` | `build_b_mix.cpp`, `build_e_screens.cpp`, `build_r4b_duck.cpp` |
| `0x040219e4` | `u32 rom_float_div(u32 a, u32 b)` | `build_b_mix.cpp`, `build_e_screens.cpp`, `build_r4b_mixer.cpp` |
| `0x04021ac0` | `u32 rom_float_mul(u32 a, u32 b)` | `build_b_mix.cpp`, `build_e_screens.cpp`, `build_r4b_duck.cpp` |
| `0x04021bdc` | `u32 rom_half_to_float(u32)` | `build_b_mix.cpp` |
| `0x04022ea8` | `void* memset(void* destination, int value, u32 count)` | `build_b_edit.cpp`, `build_b_storage.cpp` |
| `0x8c052744` | `void string_clear(void* string)` | `build_d_clip.cpp` |
| `0x8c08437c` | `void json_from_float(void* json, u32 float_bits)` | `build_k_clip.cpp` |
| `0x8c08448c` | `void json_from_string(void* json, const void* string)` | `build_k_clip.cpp` |
| `0x8c084782` | `void json_to_string(void* out_string, void* json)` | `build_r6c_chain.cpp`, `build_r2_lfopresets.cpp` |
| `0x8c0bce20` | `void mode_switch(int mode)` | `build_r4b_duck.cpp` |
| `0x8c0c1dd0` | `void* tape_model(void)` | `build_b_mix.cpp`, `build_b_record.cpp`, `build_r4b_duck.cpp` |
| `0x8c0c2158` | `int tape_key_up(void* self, int key)` | `build_r6g_solo.cpp` |
| `0x8d4e4324` | `void* alloc_aligned(u32 alignment, u32 bytes)` | `build_k_copy.cpp` |
| `0x8d4e43a4` | `void free_aligned(void* block)` | `build_k_copy.cpp` |
| `0x8d4e63cc` | `void free(void* block)` | `build_k_clip.cpp`, `build_k_clips.cpp`, `build_r6c_chain.cpp`, `build_r2_lfopresets.cpp` and 1 more |
| `0x8d4e9724` | `void patch_apply(void* descriptor)` | `build_r6c_chain.cpp` |
| `0x8d4ed6e4` | `i32 clips_idle(void)` | `build_b_record.cpp`, `build_r4b_duck.cpp` |
| `0x8d4ed7e8` | `void clip_fill(i32* record, i32 track, i32 start, i32 stop, const i32* paddings)` | `build_k_clips.cpp` |
| `0x8d4ed944` | `void* clips(void)` | `build_d_clip.cpp` |
| `0x8d4eda26` | `void* clip_record(void* clips, i32 id, void* record)` | `build_d_clip.cpp` |
| `0x8d4edfa0` | `i32 clip_at(u32 track, i32 position)` | `build_d_clip.cpp` |
| `0x8d4ee6f8` | `void clips_relink(void* store)` | `build_k_clips.cpp` |
| `0x8d4ee98c` | `void clip_set(void* store, i32 id, const i32* record, i32 to_document)` | `build_k_clips.cpp` |
| `0x8d4eea84` | `void clips_load(void* store)` | `build_k_clips.cpp` |
| `0x8d4eef00` | `void clip_remove(void* store, i32 id)` | `build_k_clips.cpp` |
| `0x8d4f23f8` | `int db_clear(int db, const char* key)` | `build_k_clips.cpp` |
| `0x8d4f24dc` | `void db_set(int db, const char* key, void* value)` | `build_k_clips.cpp`, `build_l3_maps.cpp`, `build_r4b_mixer.cpp`, `build_r4b_duck.cpp` |
| `0x8d4f2b80` | `void* tape_doc(void)` | `build_k_clips.cpp`, `build_r4b_mixer.cpp`, `build_r4b_duck.cpp` |
| `0x8d4f398c` | `void pick(void* record, int id)` | `build_r6c_chain.cpp` |
| `0x8d4f3a54` | `void lfo_pick(void* record, int id)` | `build_i_lfo4.cpp` |
| `0x8d4f3b9c` | `void set_params_doc(void* record, const u32* words)` | `build_r6c_chain.cpp` |
| `0x8d4f3c24` | `void set_active_doc(void* record, int value)` | `build_r6c_chain.cpp` |
| `0x8d4f4e2c` | `void apply_drum(void* descriptor)` | `build_r6c_chain.cpp` |
| `0x8d4f635c` | `void apply_synth(void* descriptor)` | `build_r6c_chain.cpp` |
| `0x8d4fb908` | `void clone(int source, int destination, ProgressFn progress, u32 fourth)` | `build_k_copy.cpp` |
| `0x8d500438` | `void close_partition(i32 handle)` | `build_b_edit.cpp`, `build_k_copy.cpp` |
| `0x8d500688` | `i32 partition_read(i32 handle, u32 byte_offset, u32 length, void* buffer)` | `build_b_edit.cpp`, `build_b_storage.cpp`, `build_k_copy.cpp` |
| `0x8d500b88` | `i32 partition_write(i32 handle, u32 byte_offset, u32 length, void* buffer)` | `build_b_edit.cpp`, `build_b_storage.cpp`, `build_k_copy.cpp` |
| `0x8d500f94` | `void partition_sync(void)` | `build_b_edit.cpp` |
| `0x8d501008` | `i32 open_partition(const char* name)` | `build_b_edit.cpp`, `build_k_copy.cpp` |
| `0x8d504496` | `void reset_synth(u32 voice)` | `build_i_lfo4.cpp` |
| `0x8d504854` | `void page_copy(u32 slot, u32 type, int cls)` | `build_i_lfo4.cpp` |
| `0x8d505390` | `void build_producer(int index, int type)` | `build_r6c_chain.cpp` |
| `0x8d51233c` | `int random_param(int type, int index, int bias)` | `build_r6c_chain.cpp` |
| `0x8d5168f4` | `void* mixer_values(void)` | `build_r4b_mixer.cpp` |
| `0x8d516c90` | `void* vu_values(void)` | `build_e_screens.cpp`, `build_r4b_mixer.cpp`, `build_r4b_duck_alone.cpp` |
| `0x8d51750c` | `const char* param_name(int kind, int index)` | `build_r5_lfopage.cpp` |
| `0x8f0037be` | `void sample_finalize(void* sample)` | `build_b_edit.cpp` |
| `0x8f0061a8` | `u32 db_to_amp(u32 db)` | `build_e_screens.cpp`, `build_r4b_duck.cpp` |
| `0x8f0061d4` | `u32 vu_curve(u32 x)` | `build_e_screens.cpp`, `build_r4b_mixer.cpp` |
| `0x8f0099a4` | `void vector_assign(void* destination, const void* source)` | `build_d_clip.cpp`, `build_k_clip.cpp` |
| `0x8f009e94` | `void write_clipboard_json(void)` | `build_d_clip.cpp` |
| `0x8f00a62c` | `void lift_worker(u32 track, i32 start, i32 end)` | `build_d_clip.cpp` |
| `0x8f00a870` | `void drop_worker(const void* record, u32 source, u32 destination, i32 position, i32 end, i32 mix)` | `build_d_clip.cpp` |
| `0x8f00b1d0` | `void split(u32 track, i32 position)` | `build_d_clip.cpp` |
| `0x8f00c5e0` | `void clipboard_load(void)` | `build_k_clip.cpp` |
| `0x8f01e3ec` | `i32* track_output(int track)` | `build_b_record.cpp` |
| `0x8f01e850` | `void solo_writer(void)` | `build_r6g_solo.cpp` |
| `0x8f01ec34` | `int read_block(int speed, void* state, int rolling)` | `build_b_play.cpp`, `build_r4b_duck_alone.cpp` |
| `0x8f01f4e0` | `void mixer(int speed, void* state)` | `build_b_play.cpp` |
| `0x8f01ff94` | `void set_position(i32 position)` | `build_d_clip.cpp` |
| `0x8fc21280` | `Slot* slot_for_logical(int logical)` | `build_b_play.cpp`, `build_b_xfade.cpp`, `build_r4b_duck.cpp` |
| `0x8fc218f8` | `void invalidate_range(i32 start, i32 end)` | `build_b_edit.cpp` |
| `0x8fc21994` | `i32 tape_handle(void)` | `build_b_storage.cpp` |
| `0x8fc219a8` | `int tape_index(void)` | `build_b_storage.cpp`, `build_k_clips.cpp`, `build_k_sums.cpp`, `build_l3_maps.cpp` |
| `0x8fc21bac` | `i32 tape_io_read(u32 track, u32 byte_offset, u32 length, void* buffer)` | `build_b_edit.cpp`, `build_b_storage.cpp` |
| `0x8fc21d1c` | `i32 tape_io_write(u32 track, u32 byte_offset, u32 length, void* buffer)` | `build_b_edit.cpp`, `build_b_storage.cpp` |
| `0x8fc22064` | `void mark_map_dirty(void)` | `build_b_storage.cpp`, `build_k_sums.cpp` |
| `0x8fc22498` | `void set_current_snapshot(i32 index)` | `build_b_storage.cpp` |
| `0x8fc22804` | `i32 count_free(void)` | `build_b_storage.cpp` |
| `0x8fc22838` | `u32 alloc_chunk(void)` | `build_b_storage.cpp` |
| `0x8fc22a14` | `i32 drop_oldest(void)` | `build_b_storage.cpp` |
| `0x8fc22af0` | `i32 take_snapshot(void)` | `build_b_storage.cpp` |
| `0x8fc25ebc` | `u8* colour_pointer(void* screen, const char* id, int optional)` | `build_e_screens.cpp` |
| `0x8fc261fc` | `u32* screens(void)` | `build_r5_lfopage.cpp` |
| `0x8fc356a0` | `i32 meter_fixed(u32 amp)` | `build_e_screens.cpp`, `build_r4b_duck.cpp` |
| `0x8fc809b8` | `void flash_track(int track)` | `build_b_mix.cpp` |
| `0x8fc809cc` | `void clear_region(void)` | `build_d_clip.cpp` |
| `0x8fc809fc` | `void lift_region(i32 track, i32 start, i32 end, i32 flag)` | `build_d_clip.cpp` |
| `0x8fca1ed2` | `void notify(void* object)` | `build_b_mix.cpp`, `build_b_record.cpp`, `build_r4b_mixer.cpp`, `build_r4b_duck.cpp` |
| `0x8fca7b08` | `int fx_type_of(void* string)` | `build_r6c_chain.cpp` |
| `0x8fcc76bc` | `void array_ctor(void* storage, int count, int size, void* ctor, void* dtor)` | `build_r6c_chain.cpp` |
