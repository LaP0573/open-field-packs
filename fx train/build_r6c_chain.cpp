// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// 2026-11-27: fx train 19 [B]: a turn that comes to an effect over the ceiling stops on it, where 17 and
// 18 went past it to the next that fits. What changes in this unit, and nothing else:
//
//   * The stand. pick() stops on the first effect that would put the train over CEILING_CYCLES: the wagon keeps the
//     effect it carries, and plays it; browse_knob says which knob stands, and on what (browse_item, browse_type),
//     what it needs and what the rest of the train leaves it (browse_need, browse_left). The refusal of 17 shows for
//     as long as the stand lasts (FULL, the red needle, the sag): op1f_chain_view's full, refused_knob and
//     refused_type follow it. The knob's next turn goes on from the effect it stands on: the next that fits is
//     picked as before, another that does not fit is stood on. So every effect of the list is reached and named, and
//     none seems to be missing.
//   * The end of a stand: a press of any knob, a turn of another knob, a move (shift + turn), the presets' list, another
//     slot, or the train page closing (frame_host). One knob a host.
//   * op1f_train_stand tells the page the knob, the effect's name and the words why ("needs 42%, 37% free": what it
//     needs rounded up, what the rest of the train leaves rounded down, in percent of the ceiling). A line in the log
//     says each stand once, with what every other wagon counts (wagon_cost, which chain_cost now adds up).
//   * A hold that would go over the ceiling is refused as in 17. At either end of the list a turn goes no further, and
//     a stand there stays.
//
// 2026-11-27: fx train 19 [F]: a wagon whose pack's effect the base will not make ready at the wagon's
// place (`op1f_rack_ready` answers neither 0 nor OP1F_NOT_NOW: no room for its working memory, or another refusal)
// plays nothing until a restart; the frame's rack step keeps that for each wagon of the slot on the screen
// (rk.no_room), and op1f_chain_racks tells the page such a wagon is away, so it is drawn silent as a wagon whose pack
// is away, where 18 drew it as if it played. The log line says when it is for want of room. Nothing else of [F] changes in
// this unit; the audio thread's code is 18's.
//
// 2026-11-24: the pack `fx train` version 18: the train is the effect layer "train"
// of base 37 (the card's ask 2), no longer a page that takes T3 over. What changes in this unit, and nothing else:
//
//   * The rides. op1f_riding(card, 2) once a frame into op1f_train_rides (bit n place n: 0 to 7 a synth slot, 8 to 15
//     a drum slot, 16 the master), for the key thread and the audio thread; a change marks the host's file to be
//     written, so that the flag below reaches the files at a ride's end.
//   * The sound. A wagon other than the first one (the mirror, the unit's own effect, which plays as always) plays only
//     while its place rides "train". The LFOs' wagon picks aim at those wagons only then; else as on the first wagon.
//     The first wagon's place riding "train" is no pack's effect (mirror_ride_id 0).
//   * The starts. A load of a place's integers (chain_from_values) whose store has another wagon with an effect,
//     coupled, no flag and no pack's effect on the first wagon makes a ride start due RIDE_DELAY_FRAMES frames on,
//     which the input unit's frame asks for (op1f_train_ride_due) and answers (op1f_train_ride_answer).
//   * The flag "kept, not riding": the 48th integer carries FLAG_NOT_RIDING more when the place does not ride, no start
//     is due and none was given up; read `& 0xf` for the parked mask as before, so fx train 16 and 17 ignore it.
//   * The keys come from the base's input (fx train/build_r14_train_input.cpp) through op1f_train_key and
//     op1f_train_turn, which take no controller; the controllers' key words and their handlers are gone, the encoder
//     word stays for the LFO page's destination walk, for a place that rides. On the train page T3 held opens the
//     presets, a tap wants the stock page of the first wagon's effect; a knob's click moves the first wagon and wants
//     its stock page (an empty wagon, or one with a pack's effect: nothing); no browser, no T3 made up for stock; a
//     turn of the first wagon passes over the packs' effects; shift + T3 is always the unit's list. The page wished
//     is taken by op1f_train_take_page; op1f_train_up says once a frame whose train page is up (train_open). The
//     packs' layers are no wagon's cargo.
//
// What fx train 17's unit said follows and holds, but where these words say otherwise.
//
// 2026-11-22: the pack `fx train` version 17: fx train 16's chain unit
// with the cost ceiling kept at a turn and at a hold, and nothing else. The two faults, the mend and what keeps the first
// wagon right at a coupling are from https://github.com/LaP0573/open-field/pull/8, by hooligan333 on GitHub. Built here is the plainer form that
// pull request offers beside the one it proposes, with the effects of other packs (level 8) counted as well.
//
//   * A turn goes on. A turn of a wagon's knob goes past every effect that would put the train over the ceiling, to the
//     next one that fits. The refusal (FULL, the red needle, the name in red) shows only where none fits up to the end
//     of the list, and it names the first one passed. A pick that lands ends that wagon's refusal at once. fx train 16
//     stopped at the first effect over the ceiling, so the effects behind it could not be reached. On the first wagon,
//     where the sound rides a pack's effect until the knob's rest, the effect of the unit's it stands on brings that
//     effect back: such a pick has to fit as both, by its own cost and by what the pack's effect declares, and a turn
//     that has passed an effect since its last rest goes by it, where it would else go no further.
//   * A hold is held to the ceiling. A coupling that would put the train over it is refused as a pick is: the wagon stays
//     uncoupled, nothing is written, and the refusal shows. A pack's effect counts what it declares, the first wagon's
//     ride too; one whose pack is away counts nothing. Uncoupling, and coupling an empty wagon, always go through.
//   * A coupling switches nothing on that was not counted. The first wagon's switch and document follow at once, but
//     while the unit's slot still holds another effect, or waits for a pack's: then the knob's rest brings the effect
//     and its switch together. An empty first wagon is an effect switched off to the unit, and the switch it is owed
//     comes with the effect it is given, or goes to the wagon a tap makes the first one, whose own coupling decides it.
//     Where the unit's own list gives that effect, its pick switches the slot on alone: the document follows it, however
//     many picks the list makes on the way. Another sound's key inside a rest is given what is owed before it is left.
//     Another wagon that still runs the effect of before its last pick falls silent
//     until the rest makes its own. As before, a wagon waiting for its rest runs its effect of before until then (the
//     first wagon's is counted for as long as it runs, also where another sound's key has dropped that pick), and a
//     train loaded from a file and a tap that makes another wagon the first one are not held to the ceiling.
//   * One case is known and left. A first wagon turned to none whose pick another sound's key drops counts nothing while
//     the effect of before is still in its place. Filled up beside it, then turned to an effect and dropped the same way,
//     the train runs over the ceiling (251,300 at punch, punch, mother, nitro) until that wagon lands a pick or is
//     switched off.
//
// What fx train 16's unit said follows and holds, but where these words say otherwise.
//
// Base 29's try on a unit, the mend of its crash (2026-11-21): the pack `fx train` version 16: fx train 15's chain unit
// (fx train 15's chain unit, which stays as it was) with two changes and nothing else.
//
//   * A write of the firmware's effect slot in the firmware's own order. The firmware's own effect apply (1.7.0
//     0x8d4f635c, 1.7.3 0x8d4f635c) sets the word of the instrument's effect object to 0 before it writes the slot's
//     effect type, its eight knob words and its on byte, and makes the object last, only where the slot is the one the
//     GUI shows and the audio mode is the instrument's; the audio thread runs no effect while that word is 0. fx train 15
//     (and every train since H6c) wrote the type and the words first, then the document and a line, and made the object
//     only after them: for that time the object of the sound before ran on the new sound's words. At base 29's try on a unit the
//     delay's own check of its first word refused a mother word on the audio thread, and the unit stopped. Now one
//     helper (slot_put) writes the slot for the three writers (mirror_push, mirror_silent, mirror_write): where the slot
//     written is the one whose object plays (the audio mode is this instrument's and its built slot is this slot), the
//     object's word is set to 0 first; then the type, the eight words and the on byte; then the object is made at once
//     where the firmware's apply makes it (the slot the GUI shows, in this instrument's audio mode, and the one built;
//     the firmware's apply asks the audio mode and the slot the GUI shows, not the GUI's mode, which fx train 15 asked
//     too), else the word is left 0 for the firmware's own change of slot or mode, which makes the object from the type
//     word when its fade ends, as its own apply leaves it. The document and the line come after. While the slot is
//     written the audio thread finds no object, as during the firmware's own apply.
//   * No write of the slot at a load's tell while the firmware's apply is still to come. Since base 29 the base tells
//     the packs of a sound loaded from the unit's own list (and since base 26 of the engine list's sampler sounds and of
//     a sound sent over MIDI) at the next frame, which for a sampler sound nearly always comes before the firmware's own
//     apply of it: the list's job loads the sample between its put and the apply. The chain takes the sound's document
//     into its model at the tell as before (the sound's own train, or the document's effect on the first wagon), but
//     where the document's effect is not the slot's type word yet it writes nothing to the slot (tell_wait, one line):
//     the firmware's apply that follows writes it from that document in its own order, and the chain's reconcile at
//     that apply finds the chain standing and writes nothing. A sound the firmware never applies (one sent over MIDI to
//     another slot) leaves that slot as the firmware has it until its apply comes, as without the pack.
//
// What fx train 15's unit said follows and holds, but where these words say otherwise.
//
// Base 29's mends (2026-11-21, after base 28's try on a unit: T3 and the wagons' order), the pack
// `fx train` version 15: fx train 14's chain unit (which stays as it was) with two changes.
//
//   * M1, the wagon's order. A turn of a wagon's knob walks one order, the unit's own list's: none first, then every
//     effect the unit's list shows (its own but the vocoder, which the list never shows, and fazer only while the
//     unit shows it: the byte the list's builder reads, set while the unit's fazer is unlocked) and every effect the
//     packs registered, all sorted by name in byte order, as the unit's list sorts its rows. fx train 14 walked the
//     factory catalog's order of the unit's eleven (its comment called it the browser's order; it never was) and then
//     the packs' effects past the end. The order is a small table made at the first frame the base's list is read and
//     made again only when that list or the fazer byte changes (the base's list is fixed from its first frame on); a
//     turn reads the table the frame made last (two tables, the frame writes the one not in use and then names it).
//     The ends stay ends; a pack's effect whose pack is away stands past the end, a type the order does not hold (the
//     vocoder, fazer while hidden) at its start, as before.
//   * M5, T3 after the sequencer: the train's half. T3 on the train opens the first wagon's page only when the train's
//     page is the one on the screen in the instrument's own mode at the last frame (the train unit's word,
//     op1f_train_seen); where it is not (the sequencer's view, another page over it), T3 shows the train: the train
//     unit asks its page again, as its own request (op1f_train_show_again), which also takes the page out of the place
//     the base set it aside in, and nothing goes to the firmware. And the record that the train is open ends where its
//     page has left the screen: when the unit is in the instrument's own mode and no page of this pack has been on the
//     screen for GONE_FRAMES frames in a row (another page over it, a page the base did not put back), as a change of
//     mode ends it (the GUI's word, as before). A visit to another view of the base's (the sequencer's, mode 7) keeps
//     it while away: the base keeps the train's page set aside for the instrument's mode and puts it back when the unit
//     returns; if it does not, the record ends then. So T3 from the sequencer always brings the train, the knobs never
//     act on a train that is not on the screen, and the instrument's key brings back the train where the base puts it
//     back.
//
// What fx train 14's unit said follows and holds, but where these words say otherwise.
//
// Step R5 (base 28), the pack `fx train` version 14: the chain unit of fx train 13 (which
// stays as it was for the packs before) with the effects of other packs on its wagons (level 8 of the base,
// module/build_r5_card.h), and nothing else. The requirement: any effect made by anyone shows both in the
// unit's own effect list and as a possible wagon of the train. The base keeps the list of every registered effect and
// offers any pack a way to run one inside its own sound work (op1f_rack_list, op1f_rack_at, op1f_rack_ready,
// op1f_rack_run); this unit uses those and nothing made for it.
//
//   * The wagon picker (a knob turned on the train) walks the eleven effects of the unit's own as before, then every
//     registered effect, in the list's order (R6b: one order, the unit's list's, M1 above). A wagon with a pack's effect keeps the effect of the unit's that it
//     stands on (`rides`) as its type: the knob rows, the knobs' travel, the LFOs aimed at it, the random key's values
//     are that effect's, as the base keeps them for a sound. Its name is kept beside it (rack_name).
//   * Such a wagon, when it is not the first one (the mirrored wagon, which the unit's own slot shows), plays the
//     pack's effect: from the frame, op1f_rack_ready makes the effect ready at the train's own place for that wagon,
//     `which` = 4 x host + knob (0 to 3 the synth's knobs, 4 to 7 the drum's: a wagon is its knob, so the effect's
//     memory and its tail go with the wagon when it moves); on the audio thread, in the chain's order at the wagon's
//     position, op1f_rack_run with the wagon's four words as the unit keeps them (OP1F_RACK_WORDS: the words the LFOs
//     turned this block, else the wagon's own), `on` as its coupling says, `fresh` when the chain was rebuilt. A run
//     the base refuses leaves the wagon silent: the sound passes it dry, never through the unit's own effect.
//   * The first wagon plays through the unit's own effect object, as always. When the sound's place rides a pack's
//     effect (op1f_rack_at: the base's ride, picked in the unit's list), it is named by that effect. The first wagon
//     is given a pack's effect the way the owner gives one in the unit's list: the unit's own pick of the effect's id
//     (0x8d4f398c, whose head is the base's), after the knob's rest on the train, and at a tap that makes a wagon with
//     a pack's effect the first one (its own four values put back over the effect's start values the next two frames).
//     When the first wagon leaves a wagon whose place rode a pack's effect for another wagon, that wagon keeps the
//     effect and plays it on the train; but where the new first wagon carries the very effect of the unit's that the
//     ride stands on, the place goes on riding the pack's effect, which the new first wagon then plays and is named by,
//     and the one that was first plays the unit's own effect (the contract's wrinkle, 4.5).
//   * A wagon whose pack is away stays on the train, silent, with its name and its words; nothing of it is reset or
//     rewritten; it plays again when the pack is back. On the first wagon such a wish holds the unit's effect off.
//   * Kept: in the chain file and in an instrument's integers a pack's wagon is the type 32 + the effect it stands on
//     (an older train reads that as an empty wagon) and its name rides in a tail of its own behind every other (the tag
//     99 and four names of four integers, four characters each); in a preset file the wagon's `fx` is the pack's
//     effect's name, never one of the unit's eleven, with `rides` beside it. A file with no pack's wagon is written
//     byte for byte as fx train 13 wrote it.
//
// What follows is the elder's description, which holds.
//
// Step R4c (base 27), the pack `fx train` version 13: the chain unit of step R2 (which stays
// as it was for the packs before) with one change and nothing else. Its save of user/op1f.json (save_file) tells the
// pack lfo, once the file is written, that the chain's file was written now: op1f_lfo_file_chain_saved, a name lfo
// offers from its version 11 on and this unit asks optionally. Until fx train 12 lfo's own file took a run of the
// synth's slots 0 to 7 collected in a row (op1f_lfo4_store_save) for that sign, which bound the order in which any unit
// of this pack asked the LFO unit for a slot, and which a save that left out a slot not known yet (at a start) did not
// give. The save gives the word whatever slots it holds. Where lfo is away the name is 0 and nothing is called; a lfo
// of before version 11 does not run beside this pack (both are of interface 3).
//
// What follows is the elder's description, which holds.
//
// Step R2, the pack `fx train`: the chain unit as the pack carries it from R2 on. A sibling of
// Build H's chain unit, which stays as it was for the packs built before. What changed, and nothing else:
//
//   * The effect presets lie in the pack's own folder, the first folder of its card (user/pf/fx/), no longer in
//     user/fx/. A slot's path is asked of the base (op1f_path) into a buffer of forty characters: user/pf/fx/NN.json
//     has eighteen, past the fifteen a stock string keeps in its own buffer, so the text the string puts on the heap
//     is freed after each write, as json_text frees a long text. The writer keeps nothing of the string.
//   * The folder is the base's to make: the scan makes no folder and no file. A slot whose file is not there is an
//     empty slot. A preset deleted on the unit (the red knob held) has its file taken off the disk through the base
//     (op1f_remove) at the pack's next frame call, so that the disk mode never shows a file the pack emptied; a slot
//     whose file is still to be taken off is not free for a save. When the base does not take it at once (the disk
//     mode is open) the delete is dropped, never tried later: the file stays, and the preset is in the list again.
//   * The presets of user/fx/ are copied into the pack's folder once, by shared/build_r2_move.cpp, from the pack's
//     frame call, before anything reads the store. Until that copy has ended well the store is shut: a scan is owed;
//     a save, a rename and a delete write nothing and go the way of a write that failed, and so until the first scan
//     since the copy has run, which tells the empty slots from the copied ones; the frame call says how many. That
//     scan runs in the pack's frame call right after the copy ends, asked or not (op1f_fx_presets_owed).
//     op1f_fx_holds_preset is this unit's own rule of a file that holds a preset, which the copy asks of each file.
//   * At the end of each of the four key handlers the train's page step runs (op1f_train_page_now, the pack's train
//     unit), so that the page the pack asks the base for follows the key at the next frame; not for the keys that play
//     notes (13 to 53), which may come on the audio thread and never change the page.
//
// The rest is the elder's, unchanged; its description follows and holds, but where it says the presets lie in
// user/fx/ and the scan makes the folder and the missing files.
//
// Build H images H1 to H3: the chain's engine, the train's knobs and keys and the persistence. H1 to H1g painted a
// provisional train page as text and
// boxes over the finished frame; since H2 the page is a screen of the GUI's own, drawn from the chain's view by
// Build H's train page unit (op1f_chain_view, below), and this unit paints nothing. H3 adds the bus peak the train runs on,
// the refused pick's type for the page, shift + T3 on the train as the whole-chain bypass (every wagon onto its siding,
// the couplings remembered in the file) and the file's tag 92 with the remembered couplings as a 48th integer (a tag-91
// file of 47 integers still loads). H3b arms the hold only on a clean press (no turn of the knob in the quarter second
// before it: a knob pressed while it is turned fired holds the owner did not mean) and logs every hold, every press
// that armed none and every follow of the mirror's fx_active byte, the three paths that move a wagon by themselves.
// H4b (section 20.6) times the hold and the clean window by the core's cycle counter as milliseconds (every timer
// had counted GUI frames, and the train page's frame rate is not 58 a second for sure: a try on the unit found the hold
// long), shortens the window to 120 ms and logs the frame rate (the `chain clock:` line) for the read-out.
// H5 (section 21) adds the chain presets: a list in the stock browser's shape under shift + T3 (the whole-chain
// bypass of H3 gave up the key), a hold of shift + T3 saving the chain as a preset, the list loading, renaming and
// deleting; the store travels in the module's file as a tag-93 element; the screen is Build H's presets unit's.
// H5b (section 21.8): every text in the font's lowercase (the OS font's table is indexed by the character code and
// holds the letters at the lowercase codes; H5's capitals drew icons and question marks), the default name's maker
// no longer compares the name with its own slot (H5 named every preset chain 99), and the save shows: stock's save
// screen redrawn at the id 38 with the host's box and the preset's number, "saving" with the clock while shift + T3
// is held (from 200 ms; the save at 700 ms), "saved!" with the check for a second after any save.
// H6c (section 22.10): the record's document follows the mirrored wagon whenever the module puts another effect on
// stock's slot (mirror_document: fx_type, fx_params, fx_active and the type the chain knows the document by; H6b
// left the document at the browser's last effect, so the slots file and the effect page showed that effect's
// parameters), and the frame's follow of the slot's block is counted and, soon after a document write, logged.
// H6 (section 22) makes the presets files: thirty-two slots user/fx/01.json .. 32.json, one JSON object each (the
// name, the order, the four wagons with the effect's name, its coupling and its eight parameters; {} for an empty
// slot), written by stock's json file writer and read by its reader, scanned at the first file load and at every
// list open (a file whose time or size changed is read again, a missing file is made), the disk mode showing them
// as fx/NN.json (Build H's disk unit); the H5 presets of op1f.json's tag-93 element are moved into the first free
// slots once, and the element is written empty from then on. H6b (section 22.7): the folder is made with its
// trailing slash (createFolderRecursive makes the components that end in a slash: H6's user/fx made user alone, and
// every write failed its rename into the missing folder), every write's result is checked and counted, the H5
// element stays in op1f.json until its presets have landed in files, and the slots are sixty-four.
// H4 lifts the one-wagon-per-type rule (section 20): every module-built instance gets a private state region (512 KB
// of C heap 0, eight regions for the eight wagon slots of the two hosts) and the fixed state the H0 census found is
// redirected to it around the instance's process call on the audio thread: the delay's and cwo's pointer-table
// entries (0x84ad0700[0], 0x84ad06f8[0], read inside their process), the spring's and mother's region bases through
// the module words op1f_spring_base / op1f_mother_base that the three CALL.a stubs of build_h_regions.s load in place
// of stock's immediates, and nitro's three static words at 0x080266e8 copied in and out; the L1 buffers 0x11904004 and
// 0x11904604 are scratch filled and consumed within one call and need nothing, and grid's and phone's "addresses" of
// the census were fixed-point constants. Stock's own instances (the mirrored wagon's producer, the master effect's at
// index 1) keep stock's regions. Without the pool (the allocation refused) the H1 rule stands as before.
//
// The model (study section 3): four wagons, each bound to one knob for good (the colour travels with the wagon); a
// wagon carries one effect or none, coupled (its effect runs) or uncoupled (skipped, costing nothing); the chain
// runs in position order after the engine, inside the instrument's second stage, where stock ran its one effect.
//
// What stock does, as read for H1 (study sections 6 and 9):
//   * Both sound controllers (synth, vtable 0x8401364c; drum, 0x8400e88c) share the key-down handler 0x8c0cfe7c and
//     the key-up 0x8c0cf364; their shifted key-down (0x8c0cf904 / 0x8c0b6e1e) and encoder (0x8c0d5c10 / 0x8c0b7634)
//     handlers differ. The host is the word at this + 0xc (0 synth, else drum), the page at this + 0x28 (0 engine,
//     1 envelope, 2 effect, 3 lfo). The dispatchers 0x8c0b1ae8 (key-down), 0x8c0b1b58 (shifted key-down) and
//     0x8c0b1b8c (key-up) call the current controller's virtual with (this, event*) or (this, key), the key the
//     first word of the event (the handlers read nothing else of it), and run a fallback when the handler returns
//     0. The keys: T1..T4 = 6..9 (T3 = 8 sets page 2, and on page 2 toggles fx_active through the controller's
//     virtual at vtable + 0x50); the encoder handler gets (this, index + 4 while shift is held, delta). A knob's
//     physical press (blue to orange) arrives at key-down as the key 61..64 (at the shifted key-down while shift is
//     held) and its release at key-up as the same key; then, when the knob was not turned while pressed, the key
//     handler 0x8c0b1468 synthesizes a click: the key 54..57 at key-down (shifted by shift at the release) and at
//     key-up (section 10.3; H1 had the press and the click confused). Shift + T3 calls the virtual at vtable + 0x3c
//     (this, slot): the effect browser.
//   * Stock's effect is one slot per host: the type word 0x11904794[slot] (synth) / 0x119047b4[slot] (drum), the
//     eight parameter words at 0x119036e0 + 0x20 slot / 0x119039e0 + 0x20 slot (copied every block into the ctx
//     0x11904754 the effect reads), the fx_active byte at 0x84a0154c + slot / 0x84a0155c + slot, the producer
//     object in the slot 0x84a03c84, shared by the synth and the drum: 0x8d505390(0, type) nulls the slot pointer,
//     builds the type's object through the factory 0x8f011068 into the fixed storage 0x11906918, calls its init
//     (vtable + 0xc)(this, 0) and publishes the pointer; the second stages test the pointer for null, then the
//     fx_active byte, then call process (vtable + 8)(this, L bus 0x11902ba0, R bus 0x11902ca0, ctx). The GUI's pick
//     path 0x8f007bf8(index, type) is that build and nothing else.
//   * A sound slot's settings are a document the firmware keeps in memory: a record of 52 bytes for each slot
//     (0x080235dc + 52 slot for the synth, 0x0802377c + 52 slot for the drum) with the slot's number in its second
//     word and the JSON object in its third. The firmware's setters write a key of the document and see to it that
//     the slot's file is written: 0x8d4f3b9c(record, eight words) for the effect's parameters, each stored as the
//     word shifted right by sixteen, and 0x8d4f3c24(record, value) for the effect's on-off. A record's document is
//     put onto the sound by the fifth function of the record's table (0x8d4f635c for the synth and 0x8d4f4e2c for
//     the drum, in the words 0x8401f894 and 0x8401f870, at which this unit's two wrappers stand): the effect's type
//     from its name, the parameter words, the on-off byte and, for the slot that is played, the effect's object
//     built anew. The effect browser and the loading of a preset go the same way. A key the firmware does not know
//     is not kept in such a document from one start to the next, which is why the chain has a file of its own
//     (below). The effects' names come from the firmware's table at 0x8490e288, in the order 0 delay, 1 grid,
//     2 punch, 3 phone, 4 spring, 5 mother, 6 nitro, 7 cwo, 8 vocoder, 9 terminal, 10 fazer.
//
// H1's design:
//   * The module owns the whole chain. The start stubs (build_h_fx.s) hand stock a null producer, so stock's own
//     call is skipped; the end stubs call op1f_fx_chain_run(host), which runs the coupled wagons in position order
//     on the two buses. Wagon instances are built by the module through each type's constructor and init into its
//     own memory, except the "mirrored" wagon: the one stock's slot shows and edits (its type word, parameters and
//     fx_active byte, its effect page, its browser), whose instance is stock's own producer object. Tapping a wagon
//     makes it the mirror (stock's slot takes its type and parameters, stock's producer is rebuilt as the browser
//     does it) and opens stock's effect page for it, so every effect keeps its real page and knobs. Until the
//     frame hook has put a slot's chain together, the end stubs run stock's producer as stock would.
//   * One wagon per effect type (study 6.3: eight of the types keep state in fixed memory), and the delay and nitro
//     exclude each other (they share the L1 state 0x11904004): the H1 rule, lifted in H4 (section 20; the shared L1
//     word turned out to be scratch). A pick that would push the chain over the ceiling
//     (the summed per-type maxima of H1a, 200,000 cycles) is refused and shown.
//   * The chain is saved in the module's own file, user/op1f.json (H1e): an array of the tag and the sixteen slots'
//     chains (the mirror, the document's effect type the chain was last reconciled with, the order, then per knob
//     the type, the coupling and the eight parameters as stock's integers), read at the first reconcile and written
//     half a second after the last change, or at once when the shutdown countdown begins. The preset documents
//     cannot carry it: the boot sets each record's document from the slots file whole and then rebuilds it from the
//     known keys (0x8d4f787c, run by 0x8d4f5544 after every set), which the H1d read-out showed as `key none` at
//     every apply while the file held the key. The chain is reconciled against the record's document (H1d): after
//     stock applied a document (the wrappers on the two records' apply vtable words, H1b), when the module first
//     sees a slot and when the slot's type word changes under the module; the document's own fx_type is compared
//     with the type the chain was last reconciled with: equal means a stale document re-applied, and the chain
//     stands, its mirrored wagon written back into the slot; different means the browser picked (or a preset was
//     loaded), and the mirrored wagon takes the document's effect. A slot without a chain of its own gives the
//     document's effect on the blue wagon, the others empty. Each reconcile logs one line (the first forty).
//   * The train: T3 opens it (from any page, when the train unit's screen exists: op1f_train_available), T3 on it
//     opens the mirrored wagon's effect page, T3 on the effect page opens it again; a knob's turn picks the wagon's effect (the label at once, the instance after a 150 ms
//     rest), shift + turn moves the wagon, a tap (the click stock synthesizes at a release without a turn) opens
//     its page (the browser for an empty wagon), a hold (400 ms from the press) couples or uncouples it, with the
//     click after it swallowed, shift + tap randomizes its parameters with stock's own random-preset value function
//     (H1c), shift + hold resets them; a turn while pressed is a pick and cancels the hold, and a press within
//     120 ms of a turn arms no hold (H3b: a quarter second; H4b: the window and the hold in milliseconds of the core
//     clock, not frames); shift + T3 on the train opens the presets list with a tap and saves the chain as a new
//     preset with a hold (H5; H3's whole-chain bypass under that key is gone). The train closes when another page
//     key is pressed or the mode leaves the host.
//
// GUI thread: the four handlers, op1f_chain_frame (per frame), op1f_fx_patch_apply, op1f_chain_view. Audio thread: op1f_fx_chain_run
// (no logging, no division, no floats). Shared state is written by the GUI thread in whole words; the audio thread
// reads it once per block and tolerates a block of staleness.

#include "build_h_chain.h"
#include "build_i_lfo4.h"
#include "build_i_patch.h"
#include "build_q_quiet.h"
#include "build_r6e_card.h"                                 // R5: level 8's list and runs beside levels 1 to 7's words; R14: base 35's layers (op1f_riding, the layer helpers)

typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef int i32;

typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
typedef int (*EncoderFn)(void* self, int index, int delta);
typedef void (*ApplyFn)(void* descriptor);
typedef void (*ProcessFn)(void* self, int* left, int* right, void* ctx);
typedef void (*InitFn)(void* self, int index);
typedef void (*CtorFn)(void* storage);
typedef void (*ArrayCtorFn)(void* storage, int count, int size, void* ctor, void* dtor);
typedef void (*BuildProducerFn)(int index, int type);
typedef void (*SetActiveDocFn)(void* record, int value);
typedef void (*SetParamsDocFn)(void* record, const u32* words);
typedef int (*RandomParamFn)(int type, int index, int bias);     // 0x8d51233c: stock's random value for an effect parameter, the JSON integer in the low half
typedef void (*JsonToStringFn)(void* out_string, void* json);     // 0x8c084782: the json's text into a std::string (the data pointer at +0)
typedef int (*FxTypeOfNameFn)(void* string);                       // 0x8fca7b08: the effect type of a name, as the apply uses it
typedef int (*JsonBoolFn)(void* json);                              // 0x8c084b94
typedef int (*JsonSizeFn)(void* json);                              // 0x8c084c22
typedef void* (*JsonLookupFn)(void* json, const char* key);          // 0x8c085e84: the member, inserted when missing
typedef int (*JsonHasFn)(void* json, const char* key);               // 0x8c085068
typedef void* (*JsonGetFn)(void* json, const char* key);             // 0x8c084f74
typedef void* (*JsonAtFn)(void* json, int index);                   // 0x8c084ee0
typedef int (*JsonIntFn)(void* json);                                // 0x8c084840
typedef void (*JsonCtorFn)(void* json, int kind);                    // 0x8c085ba8(json, kind): a fresh value of that kind; 0 is null, which the first push turns into an array
typedef void (*JsonFromIntFn)(void* json, int value);                // 0x8c083b1c
typedef void (*JsonPushFn)(void* array, void* value);                // 0x8c085d54
typedef void (*JsonAssignFn)(void* target, void* value);             // 0x8c0845b8
typedef void (*JsonDtorFn)(void* json);                              // 0x8c0844e0
typedef int (*JsonReadFileFn)(const char* path, void* json);         // 0x8c0534d0: the file parsed into the json, non-zero on success
typedef int (*JsonWriteFileFn)(void* path_string, void* json);       // 0x8c053168: the json written to the path, a stock string object
typedef void (*StringCtorFn)(void* out, const char* text);           // 0x8c03e434: a stock string object from text

#define STOCK_LOG ((LogFn)0x8c053a20u)
#define STOCK_BUILD_PRODUCER ((BuildProducerFn)0x8d505390u)
#define STOCK_SET_ACTIVE_DOC ((SetActiveDocFn)0x8d4f3c24u)
#define STOCK_SET_PARAMS_DOC ((SetParamsDocFn)0x8d4f3b9cu)
#define STOCK_RANDOM_PARAM ((RandomParamFn)0x8d51233cu)
#define STOCK_ARRAY_CTOR ((ArrayCtorFn)0x8fcc76bcu)
#define STOCK_PATCH_APPLY ((ApplyFn)0x8d4e9724u)
#define STOCK_APPLY_SYNTH ((ApplyFn)0x8d4f635cu)
#define STOCK_APPLY_DRUM ((ApplyFn)0x8d4f4e2cu)
#define STOCK_JSON_LOOKUP ((JsonLookupFn)0x8c085e84u)
#define STOCK_JSON_HAS ((JsonHasFn)0x8c085068u)
#define STOCK_JSON_GET ((JsonGetFn)0x8c084f74u)
#define STOCK_JSON_AT ((JsonAtFn)0x8c084ee0u)
#define STOCK_JSON_INT ((JsonIntFn)0x8c084840u)
#define STOCK_JSON_CTOR ((JsonCtorFn)0x8c085ba8u)
#define STOCK_JSON_FROM_INT ((JsonFromIntFn)0x8c083b1cu)
#define STOCK_JSON_PUSH ((JsonPushFn)0x8c085d54u)
#define STOCK_JSON_ASSIGN ((JsonAssignFn)0x8c0845b8u)
#define STOCK_JSON_DTOR ((JsonDtorFn)0x8c0844e0u)
#define STOCK_JSON_TO_STRING ((JsonToStringFn)0x8c084782u)
#define STOCK_FX_TYPE_OF ((FxTypeOfNameFn)0x8fca7b08u)
#define STOCK_JSON_BOOL ((JsonBoolFn)0x8c084b94u)
#define STOCK_JSON_SIZE ((JsonSizeFn)0x8c084c22u)
#define STOCK_JSON_READ_FILE ((JsonReadFileFn)0x8c0534d0u)
#define STOCK_JSON_WRITE_FILE ((JsonWriteFileFn)0x8c053168u)
#define STOCK_STRING_CTOR ((StringCtorFn)0x8c03e434u)
// R5: the instruments' pick of an entry of the unit's effect list for a slot's record, as the list makes it: R0 the
// record (0x080235dc + 52 x slot for the synth, 0x0802377c + 52 x slot for the drum), R1 the entry's id; its two callers
// 0x8d4f6e4a and 0x8d4f6ea4 (1.7.3 0x8d4f7f9a, 0x8d4f7ff4) hand it those, and the list's pick then switches the
// controller's effect on (+0x4c). Nothing is answered. From base 28 on its head is the base's (W2): an id of a pack's
// effect makes the place ride it and the firmware picks the effect it stands on.
typedef void (*PickFn)(void* record, int id);
#define STOCK_PICK ((PickFn)0x8d4f398cu)
#define RD32(a) (*(volatile u32*)(a))
#define RD8(a) (*(volatile u8*)(a))
#define WR32(a, v) (*(volatile u32*)(a) = (u32)(v))
#define WR8(a, v) (*(volatile u8*)(a) = (u8)(v))

static const char LOG_FILE[] = "../src/main/op1f_chain.cpp";
static const char FILE_PATH[] = "user/op1f.json";             // 14 characters: short enough to live inside the string object, nothing to free
static const char FX_TYPE_KEY[] = "fx_type";
static const char FX_PARAMS_KEY[] = "fx_params";
static const char FX_ACTIVE_KEY[] = "fx_active";

enum { HOSTS = 2, SLOTS = 8, WAGONS = 4, TYPES = 11, NONE = -1, PARAMS = 8, CHAIN_TAG = 91, CHAIN_TAG_V2 = 92 };
enum { CHAIN_INTS = 3 + WAGONS + WAGONS * (2 + PARAMS) };                  // 47: the H1e to H2 chain (the tag 91)
enum { CHAIN_INTS_V2 = CHAIN_INTS + 1 };                                   // 48: H3 adds the bypass's remembered couplings (the tag 92)
enum { BUS_SAMPLES = 64 };
enum { KEY_T1 = 6, KEY_T3 = 8, KEY_T4 = 9, KEY_CLICK0 = 54, KEY_PRESS0 = 61, PAGE_FX = 2, PAGE_LFO = 3 };
// H6h: the LFO's type words and parameter blocks beside the effect's, and the two descriptor
// tables the converter 0x8d510b64 reads as (min, max, step), six bytes per parameter, 48 per type; the static tables read as
// (0, 0, step) for every knob parameter, which the knobs' behaviour contradicts, so every knob turn on the effect and LFO pages
// is logged with the entry and the word before and after (H6h: the first twenty-four lines; H6i: a budget per knob and
// a range line when the knob rests, see stock_knob_turn).
static const u32 LFO_TYPE_ARRAY[HOSTS] = { 0x119035a0u, 0x119035c0u }, LFO_BLOCK[HOSTS] = { 0x119037e0u, 0x11903ae0u };
static const u32 FX_DESCRIPTORS = 0x840afe60u, LFO_DESCRIPTORS = 0x840af940u;
// The six LFO kinds' names are stock's own and are read where stock keeps them (packs L3; until then this unit carried
// them, in stock's order): six entries of eight bytes at 0x84d256c4, the name's address and the kind. That table is not in the image: one of the
// constructors the start code runs before main fills it (0x8fca6224, the word at 0x08002424 of the constructors' table;
// the walk is 0x8fcc4dd0, called at 0x11a00152 right before main). Stock's own lookup of a kind by its name, 0x8fca618c,
// walks the same six entries. This
// table says where each name's address stands. The effects' names come from stock's table 0x8490e288 (fx_name).
#define LFO_NAME_ENTRY(kind) ((const char* const*)(0x84d256c4u + 8u * (kind)))
static const char* const* const LFO_NAMES[6] = { LFO_NAME_ENTRY(0), LFO_NAME_ENTRY(1), LFO_NAME_ENTRY(2), LFO_NAME_ENTRY(3), LFO_NAME_ENTRY(4), LFO_NAME_ENTRY(5) };
#define RD16(a) (*(volatile u16*)(a))
enum { CEILING_CYCLES = 200000, BLOCK_CYCLES = 580500, REST_FRAMES = 9, SAVE_DELAY_FRAMES = 30, FLASH_FRAMES = 60, PRINT_FRAMES = 232 };
enum { HOLD_MS = 400, CLEAN_MS = 120, CYCLES_PER_MS = 400000 };
// H5 (section 21): the chain presets, a list at the screen id 37 (85 until H6c; H6d moved it below the power popup's 82) under shift + T3.
enum { PRESETS = 64, NAME_CHARS = OP1F_PRESET_NAME, PRESET_INTS = NAME_CHARS + CHAIN_INTS_V2, ROOT_TAG_V3 = 93, LIST_ROWS = OP1F_PRESET_ROWS, FLASH_MS = 1000 };   // 64 slots (H6b; H5 and H6 had 32); the file's root tag 93 carries the presets as the seventeenth element (91: none)
enum { FLASH_NONE = 0, FLASH_SAVED = 1, FLASH_DELETED = 2, FLASH_FULL = 3 };
enum { SAVE_SHOW_MS = 200, SAVE_HOLD_MS = 700, SAVED_MS = 1000 };                 // H5b: shift + T3 held: the save screen from 200 ms, the save at 700 ms, the confirmation for a second
enum { SAVE_NONE = OP1F_SAVE_NONE, SAVE_SAVING = OP1F_SAVE_SAVING, SAVE_SAVED = OP1F_SAVE_SAVED };   // the save screen's states (the header's, for the presets unit)
// H6 (section 22): the presets as files, one per slot; since R2 in the pack's own folder, the first of its card (user/pf/fx/NN.json has 18 characters: past a stock string's own buffer, whose text is freed after the write).
enum { PRESET_FORMAT = 1, JSON_OBJECT_KIND = 7, JSON_STRING_KIND = 4, JSON_BOOL_KIND = 5 };
static const char KEY_FORMAT[] = "op1f_fx_preset", KEY_NAME[] = "name", KEY_ORDER[] = "order", KEY_WAGONS[] = "wagons", KEY_FX[] = "fx", KEY_ON[] = "on", KEY_PARAMS[] = "params";
static const char KEY_RIDES[] = "rides";                                          // R5: a pack's effect's wagon: the unit's effect it stands on, behind the three
typedef int (*FileSizeFn)(const char* path);                                      // 0x8c051f9c: the file's size, -1 when it is not there
typedef u32 (*FileMtimeFn)(const char* path);                                     // 0x8fcbc350: the file's modification time, 0 when it is not there
typedef void (*JsonFromCstrFn)(void* json, const char* text);                     // 0x8c084430: the value becomes a string copied from the text (the kind 4)
typedef void (*JsonFromBoolFn)(void* json, int value);                            // 0x8c083aec: the value becomes a bool (the kind 5)
#define STOCK_FILE_SIZE ((FileSizeFn)0x8c051f9cu)
#define STOCK_FILE_MTIME ((FileMtimeFn)0x8fcbc350u)
#define STOCK_JSON_FROM_CSTR ((JsonFromCstrFn)0x8c084430u)
#define STOCK_JSON_FROM_BOOL ((JsonFromBoolFn)0x8c083aecu)
enum { KNOB_BLUE = 0, KNOB_OCHRE = 1, KNOB_GREY = 2, KNOB_RED = 3 };
static const char ALPHABET[] = " abcdefghijklmnopqrstuvwxyz0123456789-.";          // the rename editor's letters, in the font's lowercase (the OS font keeps the capitals' shapes at the lowercase codes; the uppercase codes are icons: H5b)
enum { ALPHABET_COUNT = 39 };
// R14: the screens come and go by the base's pages (the input unit's frame: op1f_layer_page, op1f_page_show), so no
// screen request of the firmware's is made here any more (fx train 17's STOCK_REQUEST_SCREEN, from the key thread and
// masked from the frame).
extern "C" int __attribute__((weak)) op1f_presets_available() asm("op1f_presets_available");   // Build H's presets unit (H5): the list screen exists
// R2: the pack's card (written by tools/pack_build.py from the recipe), whose first folder holds the presets; the copy
// of the presets of user/fx/ into it (build_r2_move.cpp), which says whether the store may be used. R14: fx train 17's
// page step (op1f_train_page_now, op1f_train_seen, op1f_train_show_again) is gone with its key handlers.
extern "C" const Op1fCard op1f_card_fx_train asm("op1f_card_fx_train");
extern "C" int op1f_r2_move_done(void) asm("op1f_r2_move_done");
enum { PRESETS_FOLDER = 0, PATH_ROOM = 40, SHUT_LINES = 16, REMOVE_LINES = 16, NOTE_KEY_FIRST = 13, NOTE_KEY_LAST = 53 };
// R14: the layer "train" is the card's ask 2; a place is 0 to 7 a synth slot, 8 + slot a
// drum slot, 16 the master. A ride start after a load: no sooner than RIDE_DELAY_FRAMES frames on, again at the next
// frame on OP1F_NOT_NOW up to RIDE_TRY_FRAMES frames after it was first due (lfo 12's rule). The flag "kept, not riding"
// rides on the 48th integer. The page a key wishes (op1f_train_take_page). The keys whose down this unit took.
enum { TRAIN_ASK = 2, PLACES = HOSTS * SLOTS, RIDE_DELAY_FRAMES = 5, RIDE_TRY_FRAMES = 120, RIDE_LINES = 40, FLAG_NOT_RIDING = 256 };
enum { TAKE_NONE = 0, TAKE_OWN = 1, TAKE_UNIT = 2, KEYS_KEPT = 96 };
extern "C" volatile unsigned op1f_train_rides asm("op1f_train_rides");
volatile unsigned op1f_train_rides;                                                  // the frame's op1f_riding(card, TRAIN_ASK): bit n for place n (16 the master)
static int place_rides(int host, u32 slot) { return (int)((op1f_train_rides >> ((u32)host * (u32)SLOTS + (slot & 7u))) & 1u); }
static struct {                                                                      // the ride starts a load makes due (the display thread's, but `due` read by the key thread's flag)
    u8 due[PLACES], no_flag[PLACES];                                                 // a start is due; one was given up (no flag until the place rides or loads again)
    u32 due_at[PLACES], first_due[PLACES];                                           // the frame it is due at; the frame it was first due at
    u32 next, lines, started, given_up, others;                                      // where the next walk starts; the lines; the starts, give-ups, places found on another effect
    int in_frame;                                                                    // op1f_chain_frame is running: a load now is a load of this frame
} rd;
static u8 ride_began[HOSTS];                                                         // bit s: the host's slot s began to ride "train" at this frame (rides_frame, for frame_host; the display thread's)
static volatile int take_page;                                                       // the page a key wished (TAKE_...), for op1f_train_take_page
static u32 down_taken[KEYS_KEPT / 32];                                               // the keys whose down op1f_train_key took: their up is this unit's
static struct {                                                                      // R2: the store's own words beside the elder's
    volatile u32 owed;                                                               // a scan asked while the store was shut: 1, or 2 for a forced one
    volatile u32 removing[2];                                                        // the slots whose file is to be taken off the disk, a bit each
    u32 dropped;                                               // removals dropped because the base answered not now (the disk mode open)
    volatile u32 shut_writes, shut_deletes;                                          // saves and renames, and deletes, that wrote nothing while the store was shut
    u32 said_writes, said_deletes, shut_lines;                                       // what the frame call has said of them
    u32 removed, remove_failures, remove_lines;
} r2;
static int removal_owed(int slot) { return (int)((r2.removing[slot >> 5] >> (slot & 31)) & 1u); }
                  // H4b: the hold and the clean window in milliseconds of the core clock (400 MHz, the measurement unit's time base; H1 to H4 counted 23 frames for the hold and H3b 15 for the window)
enum { SWALLOW_FRAMES = 10, INIT_WAIT_FRAMES = 300 };                                // the click after a hold; the wait for a record's document at boot
enum { OBJECT_BYTES = 512 };
// H4 (section 20): the fixed state outside a producer's object, per type, and the private region a module instance gets.
enum { REGION_BYTES = 512 * 1024, REGION_COUNT = HOSTS * WAGONS, REGION_ALIGN = 32, REGION_HEAP = 0 };   // 4 MiB of C heap 0 (32 MiB, 1.4 MiB in stock's use)
enum { STATE_NONE = 0, STATE_TABLE_DELAY = 1, STATE_TABLE_CWO = 2, STATE_BASE_SPRING = 3, STATE_BASE_MOTHER = 4, STATE_STATIC_NITRO = 5 };
static const u8 STATE_KIND[TYPES] = { STATE_TABLE_DELAY, STATE_NONE, STATE_NONE, STATE_NONE, STATE_BASE_SPRING, STATE_BASE_MOTHER, STATE_STATIC_NITRO, STATE_TABLE_CWO, STATE_NONE, STATE_NONE, STATE_NONE };
static const u32 REGION_USED[TYPES] = { 176400, 0, 0, 0, 58272, 0x42318, 0, 262144, 0, 0, 0 };   // the bytes of its region a type writes, cleared at the build: the delay's 44,100-word line, the spring's 58,272, the mother's 0x42318, cwo's 128K halfwords
static const u32 DELAY_TABLE = 0x84ad0700u, CWO_TABLE = 0x84ad06f8u;             // the pointer tables the two read inside process (entry 0; stock's entries point at its two 512 KB regions 0x84ad02c4 / 0x84b502c4)
static const u32 NITRO_STATIC = 0x080266e8u;                                      // the static struct nitro's inner process keeps in L2: the words 0..2 move (the envelope follower), 4 and 5 are constants set once
static const u32 MOTHER_REGION_CTOR = 0x8d507338u;                                // constructs a mother region as the boot constructs stock's two (two sub-blocks: the positions and a 32 KB clear each)
static const u32 SPRING_BASE_STOCK = 0x84ab3fb0u, MOTHER_BASE_STOCK = 0x8402960cu;
enum { NITRO_STATIC_WORDS = 3 };
typedef void* (*HeapMallocFn)(int heap_index, u32 size);                          // 0x8fcc5a68 (build_b_cache.cpp)
typedef void (*RegionCtorFn)(void* region);
#define STOCK_HEAP_MALLOC ((HeapMallocFn)0x8fcc5a68u)
typedef void (*FreeFn)(void* block);                                               // 0x8d4e63cc: the match of 0x8d4e63bc, the allocator stock's strings use
#define STOCK_FREE ((FreeFn)0x8d4e63ccu)
extern "C" volatile u32 op1f_spring_base asm("op1f_spring_base");                  // read by the stubs of build_h_regions.s in place of stock's immediates
extern "C" volatile u32 op1f_mother_base asm("op1f_mother_base");
volatile u32 op1f_spring_base = SPRING_BASE_STOCK;
volatile u32 op1f_mother_base = MOTHER_BASE_STOCK;
static const u32 MODE_GUI = 0x84d21088u, MODE_AUDIO = 0x11902da4u, FX_PRODUCER = 0x84a03c84u, STOCK_CTX = 0x11904754u;
static const u32 L_BUS = 0x11902ba0u, R_BUS = 0x11902ca0u, FX_NAME_TABLE = 0x8490e288u;
static const u32 POWER_POPUP_SHOWN = 0x84c9b3e4u;             // the power popup's byte: 1 while the countdown popup is on the screen (set by 0x8fc6144c, cleared by 0x8fc61438)

struct HostAddr {
    u32 slot_word, built_slot, type_array, param_block, fx_active, patch_table;
    u32 key_down, key_shift, key_up, encoder;                 // stock's handlers (R14: only the encoder is called, for the turns the module leaves alone)
};
static const HostAddr HOST[HOSTS] = {
    { 0x11904704u, 0x84a01524u, 0x11904794u, 0x119036e0u, 0x84a0154cu, 0x080235dcu, 0x8c0cfe7cu, 0x8c0cf904u, 0x8c0cf364u, 0x8c0d5c10u },
    { 0x11904708u, 0x84a01528u, 0x119047b4u, 0x119039e0u, 0x84a0155cu, 0x0802377cu, 0x8c0cfe7cu, 0x8c0b6e1eu, 0x8c0cf364u, 0x8c0b7634u },
};

// R6b (M1): the knob walks the unit's list's order, made at the frame (order_build, below); fx train 14 walked the
// factory catalog's fx_types order of the unit's eleven here (vocoder left out, then the packs' effects past the end),
// which its comment called the browser's order. The firmware's enumeration: 0 delay, 1 grid, 2 punch, 3 phone, 4 spring,
// 5 mother, 6 nitro, 7 cwo, 8 vocoder, 9 terminal, 10 fazer.
enum { TYPE_DELAY = 0, TYPE_NITRO = 6, TYPE_VOCODER = 8, TYPE_TERMINAL = 9, TYPE_FAZER = 10 };
static const u32 FAZER_SHOWN = 0x8401f0f4u;                // the byte the unit's list's builder reads: not 0 while fazer is unlocked (it is shown)
static const u32 CTOR[TYPES] = { 0x8d5061fcu, 0x8d508984u, 0x8d50f8f0u, 0x8d511ac8u, 0x8d517c84u, 0x11a071bau, 0x8d50822cu, 0x8d50256cu, 0, 0, 0x8d510616u };
static const u32 COST_MAX[TYPES] = { 22400, 53000, 83100, 47400, 49600, 44300, 40800, 44500, 0, 36300, 16800 };   // H1a, cycles per block
static const short DEFAULTS[TYPES][PARAMS] = {                                                                     // content/op1_factory.json, fx_types[].default_params
    { 8000, 8000, 8000, 8000, 0, 0, 0, 0 }, { 8000, 8000, 18000, 18000, 8000, 8000, 8000, 8000 }, { 6000, 15000, 20000, 28000, 8000, 8000, 8000, 8000 },
    { 8000, 8000, 8000, 16000, 8000, 8000, 8000, 8000 }, { 8000, 15000, 2000, 20000, 8000, 8000, 8000, 8000 }, { 17000, 0, 17000, 17000, 0, 0, 0, 0 },
    { 500, 0, 10500, 16000, 0, 0, 0, 0 }, { 5000, 8192, 16384, 32767, 0, 0, 0, 0 }, { 16384, 16384, 16384, 11253, 0, 0, 0, 0 },
    { 18203, 13235, 8091, 32767, 0, 0, 0, 0 }, { 7802, 20560, 9322, 21695, 0, 0, 0, 0 } };

struct Wagon { i32 type; i32 coupled; u32 params[PARAMS]; };
struct Stat { u32 sum, count, max; };
struct Live {                                                  // a module-built instance, one per host and knob
    void* volatile obj;                                        // the audio thread reads this once per block
    u32 ctx[PARAMS];                                           // the eight parameter words the instance reads
    i32 type;                                                  // the type built, or NONE
    u32 which;                                                 // the buffer in use
    u32 buf[2][OBJECT_BYTES / 4];
    u8* region;                                                // H4: the instance's private state region (REGION_BYTES), or 0 without the pool
    u32 nitro_state[NITRO_STATIC_WORDS];                       // H4: a nitro instance's copy of the static words, swapped in around its call
    volatile int kind;                                         // H4: the state kind of the built type (written before obj; the audio thread reads it after obj)
    // R5: a wagon with a pack's effect. Its `obj` is a word of zeros of this unit's (rack_token), so that the LFOs'
    // fx side turns its words as it turns any wagon's; nothing is ever called through it.
    volatile u32 rack;                                         // 0: no pack's effect (the object runs); RACK_SILENT: one that plays nothing now; else its id, ready at the wagon's place
    volatile u32 fresh;                                        // the next run tells the effect it starts anew (the chain rebuilt, the effect put on the wagon)
    volatile u32 tails;                                        // the effect rings out while its wagon is uncoupled (OP1F_EFFECT_TAILS)
    char rack_name[16];                                        // the pack's effect this Live was set up for, "" for none (the display thread's)
};
struct Json { u32 words[4]; };                                 // a stock json value is 12 bytes; 16 for alignment

// R5: the effects of other packs on the wagons (level 8). A wagon's type stays the unit's effect the pack's stands on;
// its name is kept here, "" for an effect of the unit's own. The type 32 + that effect stands for such a wagon in the
// chain file and an instrument's integers, with the names in a tail of their own (the tag 99, then four integers a
// knob, four characters each, the first in the low byte); a preset file names it, with `rides` beside.
enum { RACK_NAME = 16, RACK_TYPE = 32, RACK_TAG = 99, RACK_WORDS = 4, RACK_INTS = 1 + WAGONS * RACK_WORDS, RACK_SILENT = 1, RACK_LINES = 40,
       RESTORE_FRAMES = 2, HOLD_LINES = 8 };
static char rack_name[HOSTS][SLOTS][WAGONS][RACK_NAME];
static u32 rack_token[HOSTS][WAGONS];                          // the words of zeros the pack's wagons' Lives point at
static const Op1fRackEntry* volatile rack_entries;             // the base's list, taken at each frame (the same from the first frame to the end)
static volatile u32 rack_entry_count;
static volatile u32 mirror_ride_id[HOSTS], mirror_ride_slot[HOSTS];   // the frame's op1f_rack_at for the slot on the screen: the id the place rides (0 none), and the slot
static struct {                                                // the first wagon's own values put back after the unit's pick gave the effect its start values
    int due, slot, keep, coupled;
    u32 frame;
    u32 params[PARAMS];
} restore[HOSTS];
static struct {
    u32 builds, silent_lines, ride_lines, rides, hold_offs, hold_lines, converts;
    volatile u32 runs[HOSTS], refused[HOSTS];                  // the audio thread's counts: runs the base took, runs it did not
    char said[HOSTS][WAGONS][RACK_NAME];                       // the name each Live's state was last said for
    char refused_name[HOSTS][RACK_NAME];                       // the pack's effect a refused pick named, "" for one of the unit's
    int said_how[HOSTS][WAGONS];
    u8 no_room[HOSTS][WAGONS];                                 // 19: the base would not make this wagon's pack's effect ready: silent until a restart (1 for want of room, 2 another answer); the frame's
} rk;

static Wagon wagons[HOSTS][SLOTS][WAGONS];
static volatile u32 order_word[HOSTS][SLOTS];                  // byte p = the knob at position p; written whole
static volatile u8 mirror[HOSTS][SLOTS];                       // the knob stock's slot shows
static i32 doc_type[HOSTS][SLOTS];                             // the document's effect type the chain was last reconciled with
static u8 known[HOSTS][SLOTS];                                 // the slot's chain is the module's own (from the file, or made in this session)
static int file_loaded;
static int keep_h5_presets;                                    // H6b: the H5 presets element is written back as read while its presets could not become files
static Json h5_presets;
enum { JSON_ARRAY_KIND = 6 };                                  // the kind byte (json + 4) of an array: the list walk of the size getter's table (H1g; H1f had learnt 0, the null a fresh value starts as)
static u8 initialised[HOSTS][SLOTS];
static u8 bypass_mask[HOSTS][SLOTS];                           // H3: the couplings shift + T3 parked, 1 << knob; always 0 since H5 (a file's mask re-couples at the load)
static struct {                                                // H6: the presets store, one slot per file user/fx/NN.json, shared by both hosts
    u8 used[PRESETS];                                          // the slot's file holds a preset
    char name[PRESETS][NAME_CHARS + 1];                        // per slot, in the font's lowercase
    int values[PRESETS][CHAIN_INTS_V2];                        // per slot: a tag-92 chain (chain_values); the mirror and the document type are the slot's own at the load
    u32 mtime[PRESETS];                                        // the file's time at the last read
    int size[PRESETS];                                         // and its size (a changed file is read again at the next scan)
    int order[PRESETS];                                        // the used slots in slot order: the list's rows
    int count;                                                 // used slots
    int scanned;                                               // the folder was scanned once (the first file load)
    char rack[PRESETS][4][16];                                 // R5: per slot and knob, the pack's effect's name of a wagon whose type in `values` is 32 + the effect it stands on
} presets;
static struct {                                                // H5: the list's state
    int open[HOSTS], cursor[HOSTS], first[HOSTS], editing[HOSTS], caret[HOSTS];
    int loaded_slot[HOSTS];                                    // the slot the chain was loaded from or saved as, or -1 (H6: a slot, not a row)
    char buffer[HOSTS][NAME_CHARS + 1];                        // the rename editor's text
    int flash_kind, flash_slot, flash_until_ms;                // one row's word for a second: saved, deleted, list full (the row of that slot)
    int pending_delete_slot;                                   // the slot emptied when its deleted flash ends, or -1
    int t3_pressed[HOSTS], t3_ms[HOSTS], t3_acted[HOSTS];      // shift + T3 held: the save after HOLD_MS, else the tap at the release
    int pressed[HOSTS][WAGONS], press_ms[HOSTS][WAGONS], acted[HOSTS][WAGONS], swallow_until[HOSTS][WAGONS];   // the knobs on the list (the red hold deletes)
    int keep_loaded;                                           // mark_dirty leaves `loaded` alone while a preset is saved, loaded, renamed or deleted
    int save_state, save_host, save_slot, save_ms;             // H5b: the save screen: SAVING from 200 ms of the hold (save_ms the press), SAVED for a second after a save (save_ms the save); the slot shown as its number
    char row_text[LIST_ROWS][NAME_CHARS + 1];
    char wagon_text[WAGONS][NAME_CHARS + 6];
    char cell_text[NAME_CHARS][2];
    u32 opens, saves, loads, deletes, renames;
    u32 mirror_moves;                                          // H6g: loads that moved the mirror to a loaded effect (the mirrored knob's wagon was empty)
    u32 scans, rereads, writes, write_failures, migrated, folder_made;   // H6: the files (H6b: the failed writes counted)
} ls;
static volatile u32 bus_peak[HOSTS];                           // H3: the instrument bus's largest sample since the frame last read it (the audio thread writes, the frame clears)
static i32 stock_type_seen[HOSTS][SLOTS];                      // the slot's type word at the last look
static u32 memory[HOSTS][SLOTS][WAGONS][TYPES][PARAMS];         // each wagon's parameters per effect it carried
static u8 memory_set[HOSTS][SLOTS][WAGONS][TYPES];
static Live live[HOSTS][WAGONS];
static u8* region_pool;                                        // H4: the heap-0 block the regions are cut from (0 when refused)
static int regions_ready;
static volatile int chain_ready[HOSTS];
static volatile Stat chain_stat[HOSTS];
static volatile Stat wagon_stat[HOSTS][WAGONS];

static struct {
    u32 frames;
    int print_countdown;
    int train_open[HOSTS];
    int current_slot[HOSTS];                                   // the slot whose chain is live, or -1
    u32 ms, last_cycles, cycle_acc;                            // H4b: milliseconds of the core clock since the first frame, from the CYCLES deltas between frames
    u32 frames_at_clock, ms_at_clock;                          // H4b: the last `chain clock:` line's frame count and time
    int pressed[HOSTS][WAGONS], press_ms[HOSTS][WAGONS], press_shift[HOSTS][WAGONS], acted[HOSTS][WAGONS];   // H4b: the press timed in milliseconds
    int swallow_until[HOSTS][WAGONS];                          // the click that follows a hold is not a tap
    int hold_armed[HOSTS][WAGONS];                             // H3b: the press was clean (no turn of the knob in the quarter second before it)
    int last_turn_ms[HOSTS][WAGONS];                           // H3b: the knob's last encoder step on the train (H4b: in milliseconds)
    int pending_type[HOSTS][WAGONS], pending_since[HOSTS][WAGONS];   // a pick waiting for the knob's rest
    int push_pending[HOSTS];                                   // the mirror's pick waiting for the rest
    int dirty[HOSTS], dirty_since[HOSTS];
    int flash_until[HOSTS];
    int turn_knob[HOSTS], turn_frame[HOSTS];                   // the knob last turned on the train, for the page's card (H2)
    int turn_move[HOSTS];                                      // that turn was a move (H3: the page's hop)
    int refused_knob[HOSTS], refused_type[HOSTS];              // the pick that did not fit, shown during the flash (H3)
    u32 picks, moves, taps, holds, randoms, resets, refusals, saves, loads, adopts, builds, page_opens, train_opens, mirror_syncs, bypasses;
    u32 inits, swallowed, reconciles, restores;
    u32 turn_presses, follows;                                 // H3b: presses that armed no hold; the mirror's coupling following stock's fx_active byte
    u32 diag_lines;                                            // reconcile log lines printed (the first forty)
    u32 doc_writes, doc_lines, doc_ms;                         // H6c: the record's document rewritten for the mirror (the lines printed, the first twenty-four; the time of the last)
    u32 knob_turns, turn_lines[2], range_lines, row_mask[2];   // H6h, H6i: knob turns on the effect and LFO pages passed to stock; the per-turn lines printed (effect, LFO), the range lines, the descriptor rows printed (a bit per type)
    u8 probes_full;                                            // H6i: the table of knobs is full and its line is printed
    u32 clamps, ghosts, value_lines, rows_next;                // H6j: stored parameters held to their knob's range, ghost delays taken as empty, their lines (the first twenty-four), the next descriptor row of the boot's print
    u32 param_follows, early_follows, param_lines;             // H6c: the mirrored wagon's parameters following the slot's block; those within two seconds of a document write (the first twenty-four logged)
    u32 lfo4_carried;                                          // J5f: sounds whose LFOs the chain file held while the LFO unit was away, carried to the next save
    char line[200];
    QuietGate quiet;                                           // Q1: the gate of the periodic lines (build_q_quiet.h); the last field, so that no other field's place moved
} st;

extern "C" u32 op1f_fx_t0[HOSTS] asm("op1f_fx_t0");          // the start stubs' stamps (build_h_fx.cpp)
extern "C" u32 __attribute__((weak)) op1f_fx_voices asm("op1f_fx_voices");   // Q1 (build_h_fx.cpp): what the synth stage's own records counted in the last period, 0 while no synth voice sounds
extern "C" void __attribute__((weak)) op1f_lfopage_frame() asm("op1f_lfopage_frame");     // Build I's LFO page unit (I2, I3): the LFO pages' wagon marker, every frame
extern "C" void __attribute__((weak)) op1f_lfo4_block(int host) asm("op1f_lfo4_block");   // build_i_lfo4.cpp (I5, I6): the three other LFOs' fx-side calls, first in the chain's run
extern "C" void __attribute__((weak)) op1f_lfo4_frame() asm("op1f_lfo4_frame");           // build_i_lfo4.cpp: its frame work on the GUI thread (the instances built, the holds, the log)
// R14: the weak op1f_lfotrain_* key calls of I6 are gone (no pack offers them since lfo 12 takes its keys through the base).
extern "C" void __attribute__((weak)) op1f_lfo4_store_load(int slot, const int* values) asm("op1f_lfo4_store_load");   // a synth slot's three other LFOs and its lamp, from the chain file
extern "C" int __attribute__((weak)) op1f_lfo4_store_save(int slot, int* values) asm("op1f_lfo4_store_save");         // the same for the file; 0 while the slot has nothing but stock's one LFO on the first wagon
extern "C" void __attribute__((weak)) op1f_lfo_file_chain_saved() asm("op1f_lfo_file_chain_saved");   // R4c (lfo 11): the chain's file was written now, and lfo's own file follows it
// I8: the LFO presets (Build I's LFO presets unit), a library of its own beside this unit's: its folder and files at the first file load, its frame work
extern "C" void __attribute__((weak)) op1f_lfopresets_scan() asm("op1f_lfopresets_scan");
extern "C" void __attribute__((weak)) op1f_lfopresets_frame() asm("op1f_lfopresets_frame");
// I9: an instrument carries its train and its LFOs (Build I's patch unit): what a patch loaded from a file brought for its slot, and that unit's frame work
extern "C" int __attribute__((weak)) op1f_patch_pending(int host, int slot, int* out) asm("op1f_patch_pending");
extern "C" void __attribute__((weak)) op1f_patch_frame() asm("op1f_patch_frame");

static u32 cycles() {
    u32 c;
    asm volatile("%0 = CYCLES;" : "=d"(c));
    return c;
}

static u32 udiv(u32 n, u32 d) {
    if (d == 0) return 0;
    u32 q = 0, r = 0;
    for (int i = 31; i >= 0; --i) {
        r = (r << 1) | ((n >> i) & 1u);
        if (r >= d) { r -= d; q |= 1u << i; }
    }
    return q;
}

static char* put_text(char* p, const char* s) {
    while (*s) *p++ = *s++;
    return p;
}

static char* put_u32(char* p, u32 v) {
    char digits[10];
    int n = 0;
    do {
        u32 q = udiv(v, 10);
        digits[n++] = (char)('0' + (v - q * 10));
        v = q;
    } while (v);
    while (n) *p++ = digits[--n];
    return p;
}

static const char* fx_name(int type) {
    if (type < 0) return "none";
    if (type >= TYPES) return "?";
    return *(const char* const*)(FX_NAME_TABLE + 8u * (u32)type);
}

static int host_of(void* self) { return *(int*)((u8*)self + 0xc) ? 1 : 0; }
static int* page_of(void* self) { return (int*)((u8*)self + 0x28); }
static u32 gui_slot(int host) { return RD32(HOST[host].slot_word) & 7u; }
static u32 patch_record(int host, u32 slot) { return HOST[host].patch_table + 52u * slot; }
static int knob_at(int host, u32 slot, int pos) { return (int)((order_word[host][slot] >> (8 * pos)) & 3u); }
static u32 word_of(int value) { return (u32)value << 16; }
static int int_of(u32 word) { return (int)word >> 16; }

static void set_order(int host, u32 slot, const int* knobs) {
    u32 w = 0;
    for (int p = 0; p < WAGONS; ++p) w |= (u32)(knobs[p] & 3) << (8 * p);
    order_word[host][slot] = w;
}

static int conflicts(int a, int b) {                            // the H1 rule: kept for the fallback without regions only (H4)
    if (regions_ready) return 0;
    if (a < 0 || b < 0) return 0;
    if (a == b) return 1;
    return (a == TYPE_DELAY && b == TYPE_NITRO) || (a == TYPE_NITRO && b == TYPE_DELAY);
}

// ---------------------------------------------------------------- R5: the packs' effects (level 8)

static int name_same(const char* a, const char* b) {                      // two names of up to fifteen characters
    for (int i = 0; i < RACK_NAME; ++i) {
        if (a[i] != b[i]) return 0;
        if (a[i] == 0) return 1;
    }
    return 1;
}

static void name_put(char* to, const char* from) {                        // at most fifteen characters, zero padded
    int i = 0;
    for (; i < RACK_NAME - 1 && from && from[i]; ++i) to[i] = from[i];
    for (; i < RACK_NAME; ++i) to[i] = 0;
}

// The base's list, by an entry's place in it: entries of `bytes` each (20 at level 8; a later base may hand more).
static const Op1fRackEntry* rack_entry(u32 i) {
    const Op1fRackEntry* list = rack_entries;
    if (list == 0 || i >= rack_entry_count) return 0;
    u32 stride = list->bytes < (u32)sizeof(Op1fRackEntry) ? (u32)sizeof(Op1fRackEntry) : list->bytes;
    return (const Op1fRackEntry*)((const u8*)list + stride * i);
}

static int rideable(int type) {                                          // the eight effects of the unit's a pack's may stand on: delay, grid, punch, phone, spring, nitro, cwo, terminal
    return type == 0 || type == 1 || type == 2 || type == 3 || type == 4 || type == 6 || type == 7 || type == TYPE_TERMINAL;
}

// The registered effect of a name, or 0: none of that name (its pack away), or the list empty.
static const Op1fRackEntry* rack_find(const char* name) {
    if (name == 0 || name[0] == 0) return 0;
    u32 n = rack_entry_count;
    for (u32 i = 0; i < n; ++i) {
        const Op1fRackEntry* e = rack_entry(i);
        if (e && e->effect && name_same(e->effect->head.name, name)) return e;
    }
    return 0;
}

static int rack_index(const Op1fRackEntry* entry) {                      // its place in the list
    u32 n = rack_entry_count;
    for (u32 i = 0; i < n; ++i) if (rack_entry(i) == entry) return (int)i;
    return -1;
}

static int rack_rides(const Op1fRackEntry* e) { return (e && rideable((int)e->rides)) ? (int)e->rides : NONE; }

static const char* rack_name_of(const Op1fRackEntry* e) { return e && e->effect ? e->effect->head.name : ""; }

// The effect the first wagon of the slot on the screen rides through the base, while its type is the one the ride
// stands on: the frame's op1f_rack_at, kept for the threads that may not ask the base (the key thread).
static const Op1fRackEntry* mirror_entry(int host, u32 slot) {
    u32 id = mirror_ride_id[host];
    if (id == 0 || mirror_ride_slot[host] != slot) return 0;
    for (u32 i = 0; i < rack_entry_count; ++i) {
        const Op1fRackEntry* e = rack_entry(i);
        if (e && e->id == id) return rack_rides(e) == wagons[host][slot][mirror[host][slot]].type ? e : 0;
    }
    return 0;
}

// The pack's effect a wagon carries: its own name, or for the first wagon the effect its place rides.
static const char* cargo_name(int host, u32 slot, int knob) {
    if (rack_name[host][slot][knob][0]) return rack_name[host][slot][knob];
    if (knob == mirror[host][slot]) { const Op1fRackEntry* e = mirror_entry(host, slot); if (e) return rack_name_of(e); }
    return 0;
}

// The word a line says for a wagon: its pack's effect's name, else the unit's own effect's.
static const char* wagon_word(int host, u32 slot, int knob) {
    const char* name = cargo_name(host, slot, knob);
    return name ? name : fx_name(wagons[host][slot][knob].type);
}

// A pack's effect's start values as the four words of the effect it stands on, the knob's whole travel for the value's
// whole range, by the base's own arithmetic (base 28's effects unit, word_of and ratio: the knob's row, its
// lowest signed and its highest unsigned; the ratio's whole part and its fraction of sixteen bits rounded up), so that
// the words are the ones the base writes at a pick of the unit's list; the other four words, and a knob whose row has
// no span, the effect's defaults. No runtime: every product fits in 32 bits, and the divisions are the unit's own.
static void rack_start_params(const Op1fRackEntry* e, int type, u32* out) {
    for (int i = 0; i < PARAMS; ++i) out[i] = word_of(DEFAULTS[type][i]);
    const Op1fEffect* fx = e->effect;
    u32 count = fx->count > 4u ? 4u : fx->count;
    for (u32 i = 0; i < count; ++i) {
        u32 row = FX_DESCRIPTORS + 48u * (u32)type + 6u * i;
        int lo = (int)(short)RD16(row), hi = (int)RD16(row + 2u);
        const Op1fParam* p = &fx->params[i];
        if (hi <= lo || p->high <= p->low || (u32)(p->high - p->low) > 65535u) continue;
        u32 span = (u32)(hi - lo), range = (u32)(p->high - p->low);
        u32 q = udiv(span, range), f = udiv(((span - q * range) << 16) + range - 1u, range);
        int start = p->start < p->low ? p->low : (p->start > p->high ? p->high : p->start);
        u32 d = (u32)(start - p->low);
        int v = lo + (int)(d * q + ((d * f) >> 16));
        out[i] = word_of(v > hi ? hi : v);
    }
}

// ---------------------------------------------------------------- the instances

static void construct(void* storage, int type) {
    if (type == TYPE_TERMINAL) {                               // the factory's own sequence (producer_factory.cpp)
        u8* b = (u8*)storage;
        *(u32*)b = 0x840ae684u;
        STOCK_ARRAY_CTOR(b + 0x0c, 4, 8, (void*)0x8f011052u, 0);
        STOCK_ARRAY_CTOR(b + 0x2c, 4, 8, (void*)0x8f011052u, 0);
        STOCK_ARRAY_CTOR(b + 0x4c, 2, 8, (void*)0x8f011052u, 0);
        STOCK_ARRAY_CTOR(b + 0x64, 2, 16, (void*)0x8f011038u, 0);
        STOCK_ARRAY_CTOR(b + 0x84, 2, 16, (void*)0x8f011038u, 0);
        return;
    }
    ((CtorFn)CTOR[type])(storage);
}

static int buildable(int type) { return type >= 0 && type < TYPES && (CTOR[type] != 0 || type == TYPE_TERMINAL); }

// Builds the wagon's instance of `type` in the spare buffer and publishes it; anything else unpublishes. H4: an
// instance whose type keeps state outside its object gets the Live's private region first (cleared over the bytes the
// type writes; a mother region constructed as the boot constructs stock's two), and a Live that still holds an
// instance is only unpublished this frame: the build follows at the next frame's call, when the audio thread has left
// the old instance and its region may be cleared (the frame is 17 ms, the block 1.45 ms).
static void build_live(int host, int knob, int type) {
    Live* l = &live[host][knob];
    l->rack = 0;                                                   // R5: no pack's effect on this wagon from here (the audio thread reads it first)
    l->rack_name[0] = 0;
    if (!buildable(type)) {
        l->obj = 0;
        l->type = NONE;
        l->kind = STATE_NONE;
        return;
    }
    if (l->obj) {
        l->obj = 0;
        l->type = NONE;                                            // the pending type differs from NONE: the caller comes back next frame
        return;
    }
    int kind = regions_ready ? STATE_KIND[type] : STATE_NONE;
    if (kind != STATE_NONE) {
        u32* r = (u32*)l->region;
        u32 words = REGION_USED[type] / 4;
        for (u32 i = 0; i < words; ++i) r[i] = 0;
        if (kind == STATE_BASE_MOTHER) ((RegionCtorFn)MOTHER_REGION_CTOR)(l->region);
        for (int i = 0; i < NITRO_STATIC_WORDS; ++i) l->nitro_state[i] = 0;
    }
    u32 which = l->which ^ 1u;
    u32* buf = l->buf[which];
    for (int i = 0; i < OBJECT_BYTES / 4; ++i) buf[i] = 0;
    construct(buf, type);
    u32* vtable = *(u32**)buf;
    ((InitFn)vtable[3])(buf, 0);
    l->which = which;
    l->type = type;
    l->kind = kind;
    asm volatile("" ::: "memory");                                 // the kind and the region before the pointer the audio thread reads
    l->obj = buf;
    ++st.builds;
    STOCK_LOG(3, LOG_FILE, 10, "chain build: %s knob %d %s, state kind %d, region %08x", host ? "drum" : "synth", knob, fx_name(type), kind, (u32)l->region);
}

// R5: a wagon with a pack's effect: no object of the unit's is built for it. Its Live takes the type the effect stands
// on and a word of zeros as its object (the LFOs' fx side turns its words by that type); it plays nothing until the
// frame has made the effect ready at the wagon's place (rack_frame), and then tells the effect it starts anew. The
// Live is unpublished first, so that the audio thread meets either the wagon before or this one, never half of each.
static void build_rack(int host, int knob, int type, const char* name) {
    Live* l = &live[host][knob];
    l->obj = 0;
    l->rack = 0;
    asm volatile("" ::: "memory");
    l->type = type;
    l->kind = STATE_NONE;
    name_put(l->rack_name, name);
    l->fresh = 1;
    l->tails = 0;
    l->rack = RACK_SILENT;
    asm volatile("" ::: "memory");
    l->obj = (void*)&rack_token[host][knob];
    ++rk.builds;
    if (rk.builds <= (u32)RACK_LINES)
        STOCK_LOG(3, LOG_FILE, 40, "chain rack: %s knob %d carries '%s', standing on %s", host ? "drum" : "synth", knob, l->rack_name, fx_name(type));
}

// R5: the Live a wagon wants: its own effect's object, or its pack's effect.
static void build_wagon(int host, int knob, const Wagon* w, const char* name) {
    if (name && name[0] && w->type >= 0) build_rack(host, knob, w->type, name);
    else build_live(host, knob, w->type);
}

// ---------------------------------------------------------------- the audio thread

static void run_one(int host, int knob, void* obj, void* ctx) {
    u32* vtable = *(u32**)obj;
    if (vtable == 0) return;
    u32 t = cycles();
    ((ProcessFn)vtable[2])(obj, (int*)L_BUS, (int*)R_BUS, ctx);
    u32 dt = cycles() - t;
    volatile Stat* s = &wagon_stat[host][knob];
    s->sum += dt;
    s->count += 1;
    if (dt > s->max) s->max = dt;
}

// H4: a module instance's call with its fixed state pointed at its private region (section 20): the pointer-table
// entry the delay or cwo reads inside process, the base word the spring's or mother's stubs load, or nitro's three
// static words copied in and out; stock's values are put back after the call, so stock's own instances (the mirror,
// the master effect) never see the module's regions. The kind is read after obj (build_live publishes it before).
static void run_live(int host, int knob, void* obj, void* ctx) {         // I2: ctx is the instance's own words, or the copy stock's LFO turned this block
    Live* l = &live[host][knob];
    switch (l->kind) {
    case STATE_TABLE_DELAY: {
        volatile u32* entry = (volatile u32*)DELAY_TABLE;
        u32 saved = entry[0];
        entry[0] = (u32)l->region;
        run_one(host, knob, obj, ctx);
        entry[0] = saved;
        break;
    }
    case STATE_TABLE_CWO: {
        volatile u32* entry = (volatile u32*)CWO_TABLE;
        u32 saved = entry[0];
        entry[0] = (u32)l->region;
        run_one(host, knob, obj, ctx);
        entry[0] = saved;
        break;
    }
    case STATE_BASE_SPRING: {
        u32 saved = op1f_spring_base;
        op1f_spring_base = (u32)l->region;
        run_one(host, knob, obj, ctx);
        op1f_spring_base = saved;
        break;
    }
    case STATE_BASE_MOTHER: {
        u32 saved = op1f_mother_base;
        op1f_mother_base = (u32)l->region;
        run_one(host, knob, obj, ctx);
        op1f_mother_base = saved;
        break;
    }
    case STATE_STATIC_NITRO: {
        volatile u32* words = (volatile u32*)NITRO_STATIC;
        u32 saved[NITRO_STATIC_WORDS];
        for (int i = 0; i < NITRO_STATIC_WORDS; ++i) { saved[i] = words[i]; words[i] = l->nitro_state[i]; }
        run_one(host, knob, obj, ctx);
        for (int i = 0; i < NITRO_STATIC_WORDS; ++i) { l->nitro_state[i] = words[i]; words[i] = saved[i]; }
        break;
    }
    default:
        run_one(host, knob, obj, ctx);
        break;
    }
}

// ---------------------------------------------------------------- I2: the stock LFO's effect destination on any wagon
//
// The rule: the LFO is stock's own, with every target stock has;
// the one thing added is that its effect destination can turn the knobs of any effect wagon of the train, not
// only the wagon under the lamp (stock's slot effect).
//
// How stock reaches its slot effect (sections 5.3, 8.3, 10.1): once a block the second stage copies the slot's
// eight effect words into the scratch 0x11904754, then calls the fx-side LFO object's apply,
// object->vtable[2](object, the slot's LFO block, 2 for the synth or 5 for the drum), which adds its offset to the
// scratch through stock's converter with the descriptor row of the slot's effect type (the word at type_array +
// 4 x slot), then the effect runs on the scratch.
//
// What the module does, with stock's own code doing every part of the LFO: a stub in place of the 8-byte load
// before that call (0x11a05500 for the synth, 0x11a0404e for the drum; build_i_lfofx.s) hands stock a table whose
// third word is the wrapper below, so stock calls the wrapper with its own arguments. For a wagon other than the
// lamp's, the wrapper puts that wagon's eight words into the scratch and its effect type into the slot's type word,
// calls stock's apply, keeps what the scratch holds after it for the wagon's run of this block, and puts the lamp's
// words and type back. The audio thread runs a block to its end before the GUI thread runs again (one core), so
// nothing else sees the two swapped for the length of the call. The wagon's own words, which the knobs, the chain
// file and the preset files hold, are never written.
//
// Which wagon: one choice per sound slot, 0 for the wagon under the lamp (stock's own way, and what every sound has
// until the destination knob is turned inside the effect's range) or 1 to 4 for the wagon of a knob. A chosen wagon
// that carries no effect counts as no choice: the LFO turns the lamp's effect, as stock. The choice is kept in the
// chain file after a chain's 48 integers (the tag 96 and the choice), not in stock's document.
//
// Where the swap is made: only for the LFO types whose apply has been read through for what it reads for the effect
// (the slot from 0x8d503078 / 0x8d503064, the effect type from the slot's type word, that type's flag byte and
// descriptor row, the scratch), told by the class of the fx-side object, and only while the type's destination is
// the effect as its own apply decodes the destination word. I2: the value LFO (the apply 0x8d50c308, the vtable
// 0x840ae3bc, the word's top four bits 2). I3: the random, element and velocity LFOs (the applies 0x8d50bbc0,
// 0x8d50a8b0, 0x8d50cf14, the vtables 0x840ae258, 0x840ae0b0, 0x840ae550, the word's top five bits 2 through the
// decoders 0x8d50b8a6, 0x8d50a44c, 0x8d50cc2e; what else they read is their own: the random's generator and seed,
// the element's and the velocity's source buffers at 0x080268a4 and 0x08026aa8, none of it the scratch or the
// type word). Every other class and destination passes to stock's apply untouched.
//
// I4: the midi LFO (the apply 0x8d50a23c, the vtable 0x840adf4c) has four destinations of its own, the block's words
// 4 to 7 (the same four classes as the three above), each with its parameter in the word four before it and its
// value in the four words at 0x0802e3c4; the apply walks the four in one call and keeps nothing between calls. Each
// destination has a choice of wagon of its own, so the wrapper calls stock's apply once for the lamp's effect with
// the other wagons' destinations masked (the class 0, which this call's class does nothing with), then once per
// other wagon with only that wagon's destinations left, that wagon's words and type swapped in as above. With no
// destination on another wagon than the lamp's it is one call of stock's apply, untouched.
enum { LFO_TYPE_VALUE = 1, LFO_TYPE_RANDOM = 2, LFO_TYPE_ELEMENT = 3, LFO_TYPE_MIDI = 4, LFO_TYPE_VELOCITY = 5, LFO_DEST_KNOB = 2, LFO_DEST_WORD = 2, LFO_DEST_FX = 2, LFOFX_TAG = 96, LFOFX_LINES = 48, LFOFX_PERIOD_LINES = 64 };
// I4: the midi LFO's four destinations: the block's words 4 to 7, set by the shifted knobs (the handler's indexes 4 to 7); the tag of their choices in the chain file
enum { MIDI_DESTS = 4, LFO_MIDI_DEST_WORD = 4, LFO_MIDI_DEST_KNOB = 4, LFOMIDI_TAG = 97 };
// I6: a synth slot's three other LFOs and its lamp ride after the two pairs: the tag 98 and the LFO unit's integers (build_i_lfo4.h)
enum { LFO4_TAG = 98, LFO4_INTS = OP1F_LFO4_STORE_INTS, LFO4_MIN = OP1F_LFO4_STORE_MIN, LFO4_AT = CHAIN_INTS_V2 + 4 };   // I7: one more integer than I6 wrote (the order on the page); a file of I6's is read with it missing
// the value LFO's destination word, as the H6i read-out and the apply 0x8d50c308 give it: the value's top four bits
// the destination (2 the effect), bit 11 the form shared by all voices (the page's FREE); the knob runs from 1024 to 11264
enum { LFO_FX_LO = 8192, LFO_FX_SHARED = 10240, LFO_FX_HI = 11264, LFO_FX_PARK = 9216, LFO_FX_PARK_SHARED = 10752, LFO_UNDER_FX = 7168 };
// I3: the random, element and velocity LFOs' destination word, as their decoders read it with the class of the call:
// the value's top five bits, four classes of 2048 (0 the engine, 1 the envelope, 2 the effect, 3 the three direct
// ones), no shared form; the knob runs from 1024 to 7168 (the I2 read-out's rows). The parked values are the middles.
enum { LFO4_FX_LO = 4096, LFO4_FX_HI = 6144, LFO4_FX_PARK = 5120, LFO4_UNDER_FX = 3072, LFO4_OVER_FX = 6656 };
static const u32 VALUE_LFO_FX_VTABLE = 0x840ae3bcu;           // the value LFO's fx-side class (one object at 0x11906ef8 for both hosts)
static const u32 RANDOM_LFO_FX_VTABLE = 0x840ae258u, ELEMENT_LFO_FX_VTABLE = 0x840ae0b0u, VELOCITY_LFO_FX_VTABLE = 0x840ae550u;   // I3: the three's fx-side classes (the same object, built again by the type's init)
static const u32 MIDI_LFO_FX_VTABLE = 0x840adf4cu;            // I4: the midi LFO's
static const u32 LFO_FX_OBJECT[HOSTS] = { 0x84a01570u, 0x84a01574u };   // the hosts' fx-side LFO object pointers (the stages read them at 0x11a051be and 0x11a03d6e), for the log
typedef int (*LfoApplyFn)(void* object, u32* block, int cls);

static u8 lfo_fx_pick[HOSTS][SLOTS];                           // 0 the wagon under the lamp; 1..4 the wagon of knob 0..3
static u8 lfo_midi_pick[HOSTS][SLOTS][MIDI_DESTS];             // I4: the same choice for each of the midi LFO's four destinations

// J5f: the LFO unit's integers of each synth slot, carried while the LFO unit is away. The pack `lfo` is a file of its
// own since J5f, and with it away this unit would write the chain file, and an instrument's file, without the three
// other LFOs of each sound: the next save would drop them. So what the chain file
// or an instrument held is kept as it was read and written back as it was, and the LFOs are there again when the pack
// is. With the LFO unit there nothing here is used.
// lfo4-carry-begin
static int lfo4_kept[SLOTS][LFO4_INTS];
static u8 lfo4_kept_has[SLOTS];

static void lfo4_keep(u32 slot, const int* more) {
    for (int i = 0; i < LFO4_INTS; ++i) lfo4_kept[slot][i] = more[i];
    lfo4_kept_has[slot] = 1;
}

static void lfo4_drop(u32 slot) { lfo4_kept_has[slot] = 0; }

static int lfo4_carried(u32 slot, int* more) {
    if (!lfo4_kept_has[slot]) return 0;
    for (int i = 0; i < LFO4_INTS; ++i) more[i] = lfo4_kept[slot][i];
    return 1;
}
// lfo4-carry-end
struct FxMod {                                                 // the audio thread's: a wagon's words as stock's LFO left them this block
    u32 words[PARAMS];
    void* obj;                                                 // the instance they were made for: another instance on the knob runs on its own words
    int valid;
};
static FxMod fx_mod[HOSTS][WAGONS];                            // by the wagon's knob (I4: the midi LFO can turn several wagons in a block; I2 and I3 kept one set a host)
static volatile u32 lfofx_turned[HOSTS], lfofx_idle[HOSTS], lfofx_stock[HOSTS];   // blocks: a wagon's words turned; the LFO kept off the lamp's effect with its wagon parked or not built; stock's own way
extern "C" volatile u32 op1f_lfofx_calls[HOSTS] asm("op1f_lfofx_calls");
volatile u32 op1f_lfofx_calls[HOSTS];                           // I5: stock's fx-side LFO applies the wrapper has seen, by host (build_i_lfo4.cpp calls its twin once for each)
static u32 lfofx_lines, lfofx_period_lines;                    // the knob's lines and the periodic lines printed

// The wagon the LFO's effect destination turns: the chosen one while it carries an effect, else the one under the lamp.
static int lfo_fx_knob_of(int host, u32 slot) {
    int pick = lfo_fx_pick[host][slot];
    return (pick && wagons[host][slot][pick - 1].type >= 0) ? pick - 1 : mirror[host][slot];
}

// I4: the wagon the midi LFO's destination i turns, by the same rule.
static int lfo_midi_knob_of(int host, u32 slot, int i) {
    int pick = lfo_midi_pick[host][slot][i];
    return (pick && wagons[host][slot][pick - 1].type >= 0) ? pick - 1 : mirror[host][slot];
}

// Is the effect this class's destination, as its own apply decodes the block's destination word. 0 for a class
// whose apply has not been read through: stock's way.
static int lfo_class_to_fx(u32 vtable, const u32* block) {
    i32 word = (i32)block[LFO_DEST_WORD];
    if (vtable == VALUE_LFO_FX_VTABLE) return (word >> 28) == (i32)LFO_DEST_FX;
    if (vtable == RANDOM_LFO_FX_VTABLE || vtable == ELEMENT_LFO_FX_VTABLE || vtable == VELOCITY_LFO_FX_VTABLE) return (word >> 27) == (i32)LFO_DEST_FX;
    return 0;
}

// I4: the midi LFO. target[i] is the knob of the wagon destination i turns, or -1 when it is not the effect.
static int lfofx_apply_midi(int host, void* object, u32* block, int cls, LfoApplyFn real, u32 slot) {
    int target[MIDI_DESTS];
    int lamp = mirror[host][slot];
    int other = 0;
    for (int i = 0; i < MIDI_DESTS; ++i) {
        target[i] = ((i32)block[LFO_MIDI_DEST_WORD + i] >> 27) == (i32)LFO_DEST_FX ? lfo_midi_knob_of(host, slot, i) : -1;
        if (target[i] >= 0 && target[i] != lamp) other = 1;
    }
    if (!other || !chain_ready[host]) {
        lfofx_stock[host] = lfofx_stock[host] + 1;             // every effect destination on the lamp's wagon, or none: stock's own way, untouched
        return real(object, block, cls);
    }
    u32 part[PARAMS];
    for (int i = 0; i < PARAMS; ++i) part[i] = block[i];
    for (int i = 0; i < MIDI_DESTS; ++i) if (target[i] >= 0 && target[i] != lamp) part[LFO_MIDI_DEST_WORD + i] = 0;
    int result = real(object, part, cls);                      // the lamp's effect, by the destinations that are its own
    volatile u32* ctx = (volatile u32*)STOCK_CTX;
    volatile u32* type_word = (volatile u32*)(HOST[host].type_array + 4u * slot);
    u32 kept[PARAMS];
    for (int i = 0; i < PARAMS; ++i) kept[i] = ctx[i];         // the scratch as that call left it: the lamp's effect runs on it
    u32 kept_type = *type_word;
    for (int knob = 0; knob < WAGONS; ++knob) {
        if (knob == lamp) continue;
        int mine = 0;
        for (int i = 0; i < MIDI_DESTS; ++i) if (target[i] == knob) mine = 1;
        if (!mine) continue;
        Live* l = &live[host][knob];
        const Wagon* w = &wagons[host][slot][knob];
        void* obj = l->obj;
        i32 type = l->type;
        if (obj == 0 || type != w->type || type < 0 || type >= TYPES || !w->coupled) {
            lfofx_idle[host] = lfofx_idle[host] + 1;           // parked or not built this block: nothing to turn, and the apply keeps no state to run on
            continue;
        }
        for (int i = 0; i < PARAMS; ++i) part[i] = block[i];
        for (int i = 0; i < MIDI_DESTS; ++i) if (target[i] != knob) part[LFO_MIDI_DEST_WORD + i] = 0;
        for (int i = 0; i < PARAMS; ++i) ctx[i] = l->ctx[i];
        *type_word = (u32)type;
        real(object, part, cls);
        FxMod* m = &fx_mod[host][knob];
        for (int i = 0; i < PARAMS; ++i) { m->words[i] = ctx[i]; ctx[i] = kept[i]; }
        *type_word = kept_type;
        m->obj = obj;
        m->valid = 1;
        lfofx_turned[host] = lfofx_turned[host] + 1;
    }
    return result;                                             // stock's answer for the lamp's effect: its page copy as stock makes it
}

static int lfofx_apply(int host, void* object, u32* block, int cls) {
    op1f_lfofx_calls[host] = op1f_lfofx_calls[host] + 1;       // I5
    LfoApplyFn real = (LfoApplyFn)(*(u32**)object)[2];
    u32 slot = RD32(HOST[host].built_slot) & 7u;               // the slot stock's apply reads the effect type of (0x8d503078 / 0x8d503064)
    if (!place_rides(host, slot)) {                            // R14: the other wagons play only while the place rides "train": the pick is the first wagon's, stock's own way
        lfofx_stock[host] = lfofx_stock[host] + 1;
        return real(object, block, cls);
    }
    if (*(u32*)object == MIDI_LFO_FX_VTABLE) return lfofx_apply_midi(host, object, block, cls, real, slot);   // I4
    int knob = lfo_fx_knob_of(host, slot);
    if (knob == mirror[host][slot] || !chain_ready[host] || !lfo_class_to_fx(*(u32*)object, block)) {
        lfofx_stock[host] = lfofx_stock[host] + 1;             // the wagon under the lamp, another LFO type, another destination: stock's own way, untouched
        return real(object, block, cls);
    }
    volatile u32* ctx = (volatile u32*)STOCK_CTX;
    u32 kept[PARAMS];
    for (int i = 0; i < PARAMS; ++i) kept[i] = ctx[i];
    Live* l = &live[host][knob];
    const Wagon* w = &wagons[host][slot][knob];
    void* obj = l->obj;
    i32 type = l->type;
    if (obj == 0 || type != w->type || type < 0 || type >= TYPES || !w->coupled) {
        real(object, block, cls);                               // the LFO runs on (its phase); its wagon is parked or not built this block, and the lamp's effect is not its target
        for (int i = 0; i < PARAMS; ++i) ctx[i] = kept[i];
        lfofx_idle[host] = lfofx_idle[host] + 1;
        return 0;
    }
    volatile u32* type_word = (volatile u32*)(HOST[host].type_array + 4u * slot);
    u32 kept_type = *type_word;
    for (int i = 0; i < PARAMS; ++i) ctx[i] = l->ctx[i];
    *type_word = (u32)type;                                    // the descriptor row stock's converter holds the result to is this wagon's
    real(object, block, cls);
    FxMod* m = &fx_mod[host][knob];
    for (int i = 0; i < PARAMS; ++i) { m->words[i] = ctx[i]; ctx[i] = kept[i]; }
    *type_word = kept_type;
    m->obj = obj;
    m->valid = 1;
    lfofx_turned[host] = lfofx_turned[host] + 1;
    return 0;                                                  // the lamp's effect did not move: no copy for its page (0x8d504854)
}

extern "C" int op1f_lfofx_apply_synth(void* object, u32* block, int cls) asm("op1f_lfofx_apply_synth");
extern "C" int op1f_lfofx_apply_synth(void* object, u32* block, int cls) { return lfofx_apply(0, object, block, cls); }
extern "C" int op1f_lfofx_apply_drum(void* object, u32* block, int cls) asm("op1f_lfofx_apply_drum");
extern "C" int op1f_lfofx_apply_drum(void* object, u32* block, int cls) { return lfofx_apply(1, object, block, cls); }
// the tables the stubs hand stock in place of the object's vtable: stock reads the word at +8 and calls it
extern "C" const LfoApplyFn op1f_lfofx_table_synth[3] asm("op1f_lfofx_table_synth");
extern "C" const LfoApplyFn op1f_lfofx_table_synth[3] = { 0, 0, op1f_lfofx_apply_synth };
extern "C" const LfoApplyFn op1f_lfofx_table_drum[3] asm("op1f_lfofx_table_drum");
extern "C" const LfoApplyFn op1f_lfofx_table_drum[3] = { 0, 0, op1f_lfofx_apply_drum };

// ---------------------------------------------------------------- I6: the fx side of the three other LFOs
//
// Four LFOs per sound: the one under the LFO train's lamp is stock's slot
// LFO, served by the wrapper above; the three others are objects of stock's LFO classes that build_i_lfo4.cpp keeps
// and calls. Their fx-side apply comes here, from that unit, first thing in the chain's run: after stock's own LFO
// has had its call and before any wagon reads its words. Each of them has an effect wagon of its own choice, as
// stock's LFO has since I2 (0 the wagon under the effect train's lamp, 1 to 4 the wagon of a knob), and the midi
// kind one for each of its four destinations.
//
// What differs from the wrapper above. The apply is called once a block whatever the destination, as stock calls
// its own (the kinds advance their state in it; the value kind's fx-side call advances the oscillator its voices
// read in the FREE form). A wagon's words start from what stock's LFO or an earlier one of the three left them at
// this block (fx_mod), so several LFOs on one wagon add up as their offsets add up on the scratch for the lamp's
// effect. On the lamp's effect the call is stock's apply on the scratch as it stands, and its answer goes back to
// the caller, which sends stock's page its copy.
static volatile u32 lfox_turned[HOSTS], lfox_idle[HOSTS], lfox_stock[HOSTS];   // blocks, as lfofx_turned, lfofx_idle and lfofx_stock, for the three other LFOs

// The wagon a choice turns: the chosen one while it carries an effect, else the one under the lamp.
static int extra_knob_of(int host, u32 slot, int pick) {
    return (pick > 0 && pick <= WAGONS && wagons[host][slot][pick - 1].type >= 0) ? pick - 1 : mirror[host][slot];
}

// One wagon that is not the lamp's, turned: its words into the scratch and its type into the slot's type word, the
// apply, the result kept for the wagon's run of this block, the scratch and the type word put back. A wagon that is
// parked or not built gets nothing; the kinds that keep a phase still have their call (call_parked), on a scratch
// that is put back.
static void extra_turn(int host, u32 slot, int knob, void* object, u32* block, int cls, LfoApplyFn real, int call_parked) {
    volatile u32* ctx = (volatile u32*)STOCK_CTX;
    u32 kept[PARAMS];
    for (int i = 0; i < PARAMS; ++i) kept[i] = ctx[i];
    Live* l = &live[host][knob];
    const Wagon* w = &wagons[host][slot][knob];
    void* obj = l->obj;
    i32 type = l->type;
    if (obj == 0 || type != w->type || type < 0 || type >= TYPES || !w->coupled) {
        if (call_parked) {
            real(object, block, cls);
            for (int i = 0; i < PARAMS; ++i) ctx[i] = kept[i];
        }
        lfox_idle[host] = lfox_idle[host] + 1;
        return;
    }
    FxMod* m = &fx_mod[host][knob];
    const u32* from = (m->valid && m->obj == obj) ? m->words : l->ctx;   // as an LFO before this one left them at this block, else the wagon's own
    volatile u32* type_word = (volatile u32*)(HOST[host].type_array + 4u * slot);
    u32 kept_type = *type_word;
    for (int i = 0; i < PARAMS; ++i) ctx[i] = from[i];
    *type_word = (u32)type;
    real(object, block, cls);
    for (int i = 0; i < PARAMS; ++i) { m->words[i] = ctx[i]; ctx[i] = kept[i]; }
    *type_word = kept_type;
    m->obj = obj;
    m->valid = 1;
    lfox_turned[host] = lfox_turned[host] + 1;
}

// Audio thread. Non-zero when the lamp's effect was moved on the scratch (stock's answer).
extern "C" int op1f_lfofx_extra(int host, void* object, u32* block, int pick, const u8* midi_pick) asm("op1f_lfofx_extra");
extern "C" int op1f_lfofx_extra(int host, void* object, u32* block, int pick, const u8* midi_pick) {
    LfoApplyFn real = (LfoApplyFn)(*(u32**)object)[2];
    int cls = host ? 5 : 2;
    if (!chain_ready[host]) {
        lfox_stock[host] = lfox_stock[host] + 1;
        return real(object, block, cls);
    }
    u32 slot = RD32(HOST[host].built_slot) & 7u;
    if (!place_rides(host, slot)) {                            // R14: no ride of "train": every pick is the first wagon's (the lamp's effect on the scratch)
        lfox_stock[host] = lfox_stock[host] + 1;
        return real(object, block, cls);
    }
    u32 vtable = *(u32*)object;
    int lamp = mirror[host][slot];
    if (vtable == MIDI_LFO_FX_VTABLE) {                        // four destinations, each with its wagon: a call for the lamp's effect, one for each other wagon (as lfofx_apply_midi)
        int target[MIDI_DESTS];
        int other = 0;
        for (int i = 0; i < MIDI_DESTS; ++i) {
            target[i] = ((i32)block[LFO_MIDI_DEST_WORD + i] >> 27) == (i32)LFO_DEST_FX ? extra_knob_of(host, slot, midi_pick[i]) : -1;
            if (target[i] >= 0 && target[i] != lamp) other = 1;
        }
        if (!other) {
            lfox_stock[host] = lfox_stock[host] + 1;
            return real(object, block, cls);
        }
        u32 part[PARAMS];
        for (int i = 0; i < PARAMS; ++i) part[i] = block[i];
        for (int i = 0; i < MIDI_DESTS; ++i) if (target[i] >= 0 && target[i] != lamp) part[LFO_MIDI_DEST_WORD + i] = 0;
        int result = real(object, part, cls);
        for (int knob = 0; knob < WAGONS; ++knob) {
            if (knob == lamp) continue;
            int mine = 0;
            for (int i = 0; i < MIDI_DESTS; ++i) if (target[i] == knob) mine = 1;
            if (!mine) continue;
            for (int i = 0; i < PARAMS; ++i) part[i] = block[i];
            for (int i = 0; i < MIDI_DESTS; ++i) if (target[i] != knob) part[LFO_MIDI_DEST_WORD + i] = 0;
            extra_turn(host, slot, knob, object, part, cls, real, 0);
        }
        return result;
    }
    int knob = extra_knob_of(host, slot, pick);
    if (knob == lamp || !lfo_class_to_fx(vtable, block)) {     // the lamp's effect, another destination, a class not read through: stock's apply on the scratch as it stands
        lfox_stock[host] = lfox_stock[host] + 1;
        return real(object, block, cls);
    }
    extra_turn(host, slot, knob, object, block, cls, real, 1);
    return 0;
}

// What the LFO unit's log line says of the fx side: the three counts since the last call.
extern "C" void op1f_lfofx_extra_counts(u32* out) asm("op1f_lfofx_extra_counts");
extern "C" void op1f_lfofx_extra_counts(u32* out) {
    out[0] = lfox_turned[0]; out[1] = lfox_idle[0]; out[2] = lfox_stock[0];
    lfox_turned[0] = 0; lfox_idle[0] = 0; lfox_stock[0] = 0;
}

// R5: a wagon's pack's effect run once, through the base, at the wagon's place in the chain, on the instrument's two
// buses: the wagon's four words as the unit keeps them (those the LFOs turned this block, else the wagon's own), on as
// its coupling says (an effect with tails rings out while the wagon is uncoupled; the base calls nothing for one
// without), fresh until a run has reached the effect. Timed into the wagon's own measure. A run the base refuses leaves
// the buses as they were: the wagon is silent. No line, no division, nothing of the firmware's.
static void run_rack(int host, int knob, const Wagon* w, Live* l, u32 id) {
    void* obj = l->obj;
    FxMod* m = &fx_mod[host][knob];
    Op1fRackRun run;
    run.bytes = (u32)sizeof(Op1fRackRun);
    run.id = id;
    run.form = OP1F_RACK_WORDS;
    run.frames = BUS_SAMPLES;
    run.left = (int*)L_BUS;
    run.right = (int*)R_BUS;
    run.values = (const int*)((m->valid && m->obj == obj) ? m->words : l->ctx);
    run.on = w->coupled ? 1u : 0u;
    run.fresh = l->fresh ? 1u : 0u;
    u32 t = cycles();
    int answer = op1f_rack_run(&op1f_card_fx_train, (u32)(host * WAGONS + knob), &run);
    u32 dt = cycles() - t;
    if (answer != 0) {
        rk.refused[host] = rk.refused[host] + 1;
        return;
    }
    rk.runs[host] = rk.runs[host] + 1;
    if (run.on || l->tails) l->fresh = 0;                     // the effect was called: it has been told
    volatile Stat* s = &wagon_stat[host][knob];
    s->sum += dt;
    s->count += 1;
    if (dt > s->max) s->max = dt;
}

extern "C" void op1f_fx_chain_run(int host) asm("op1f_fx_chain_run");
extern "C" void op1f_fx_chain_run(int host) {
    if (&op1f_lfo4_block) op1f_lfo4_block(host);               // I5: the second LFO's fx-side call, before any wagon reads the scratch
    u32 slot = RD32(HOST[host].built_slot) & 7u;
    if (chain_ready[host]) {
        Wagon* ws = wagons[host][slot];
        u32 ord = order_word[host][slot];
        int mir = mirror[host][slot];
        u32 rides = op1f_train_rides;                             // R14: read once a call
        int extras = (int)((rides >> ((u32)host * (u32)SLOTS + slot)) & 1u);   // the other wagons play only while the place rides "train"
        for (int pos = 0; pos < WAGONS; ++pos) {
            int knob = (int)((ord >> (8 * pos)) & 3u);
            Wagon* w = &ws[knob];
            if (w->type < 0) continue;
            if (knob == mir) {
                if (!RD8(HOST[host].fx_active + slot)) continue;
                void* obj = (void*)RD32(FX_PRODUCER);
                if (obj) run_one(host, knob, obj, (void*)STOCK_CTX);
            } else {
                if (!extras) continue;
                u32 rack = live[host][knob].rack;                 // R5: a pack's effect on this wagon: its run, or silence
                if (rack) {
                    if (rack != (u32)RACK_SILENT) run_rack(host, knob, w, &live[host][knob], rack);
                    continue;
                }
                if (!w->coupled) continue;
                void* obj = live[host][knob].obj;
                if (obj) run_live(host, knob, obj, (fx_mod[host][knob].valid && fx_mod[host][knob].obj == obj) ? (void*)fx_mod[host][knob].words : (void*)live[host][knob].ctx);   // H4: the instance's fixed state on its own region; I2: the words stock's LFO turned this block, when it aims at this wagon
            }
        }
    } else {                                                   // as stock: the slot's producer while fx_active
        void* obj = (void*)RD32(FX_PRODUCER);
        if (obj && RD8(HOST[host].fx_active + slot)) run_one(host, mirror[host][slot], obj, (void*)STOCK_CTX);
    }
    for (int k = 0; k < WAGONS; ++k) fx_mod[host][k].valid = 0;   // I2: the turned words were this block's
    {                                                          // H3: the bus's largest sample, for the page (the train runs while the instrument sounds)
        const int* bus = (const int*)L_BUS;
        int peak = 0;
        for (int i = 0; i < BUS_SAMPLES; ++i) { int v = bus[i]; if (v < 0) v = -v; if (v > peak) peak = v; }
        if ((u32)peak > bus_peak[host]) bus_peak[host] = (u32)peak;
    }
    u32 total = cycles() - op1f_fx_t0[host];
    volatile Stat* c = &chain_stat[host];
    c->sum += total;
    c->count += 1;
    if (total > c->max) c->max = total;
}

// ---------------------------------------------------------------- the wagons' values

static void params_default(int type, u32* out) {
    for (int i = 0; i < PARAMS; ++i) out[i] = type >= 0 ? word_of(DEFAULTS[type][i]) : 0;
}

static void remember(int host, u32 slot, int knob, const Wagon* w) {
    if (w->type < 0) return;
    if (rack_name[host][slot][knob][0]) return;                    // R5: a pack's effect's words are not its type's own
    for (int i = 0; i < PARAMS; ++i) memory[host][slot][knob][w->type][i] = w->params[i];
    memory_set[host][slot][knob][w->type] = 1;
}

static void slot_write_params(int host, u32 slot, const u32* params) {
    for (int i = 0; i < PARAMS; ++i) WR32(HOST[host].param_block + 0x20u * slot + 4u * i, params[i]);
}

static void slot_read_params(int host, u32 slot, u32* params) {
    for (int i = 0; i < PARAMS; ++i) params[i] = RD32(HOST[host].param_block + 0x20u * slot + 4u * i);
}

static int slot_type(int host, u32 slot) {
    int type = (int)RD32(HOST[host].type_array + 4u * slot);
    return (type >= 0 && type < TYPES) ? type : NONE;
}

// R6c (fx train 16): the slot's object is the one the audio thread runs: the audio mode is this instrument's and its
// built slot (the slot whose words the audio thread copies for the effect) is this slot.
static int slot_plays(int host, u32 slot) {
    return (int)RD32(MODE_AUDIO) == host && (RD32(HOST[host].built_slot) & 7u) == slot;
}

// R6c: the firmware's effect slot written in the firmware's own order (its effect apply, 1.7.0 0x8d4f635c, 1.7.3
// 0x8d4f635c): where the slot plays, the object's word set to 0 first, so that the audio thread runs no effect while
// the type and the words change; then the type word, the eight words, the on byte; then the object made where the
// firmware's apply makes it (the slot plays and is the one the GUI shows), else the word left 0 for the firmware's
// change of slot or mode, which makes the object from the type word at the end of its fade. The caller writes the
// document and its line after. Answers 1 where it made the object.
static int slot_put(int host, u32 slot, int type, const u32* params, int on) {
    int plays = slot_plays(host, slot);
    if (plays) WR32(FX_PRODUCER, 0);
    WR32(HOST[host].type_array + 4u * slot, type);
    slot_write_params(host, slot, params);
    WR8(HOST[host].fx_active + slot, on ? 1 : 0);
    if (!plays || gui_slot(host) != slot) return 0;
    STOCK_BUILD_PRODUCER(0, type);
    return 1;
}

// Clears every other wagon whose type conflicts with `type` (the one-wagon-per-type rule after an adoption; nothing
// with the regions of H4, where conflicts() is 0).
static void dedupe(int host, u32 slot, int keep_knob, int type) {
    for (int k = 0; k < WAGONS; ++k) {
        if (k == keep_knob || rack_name[host][slot][k][0]) continue;   // R5: a pack's effect builds nothing of its type
        Wagon* w = &wagons[host][slot][k];
        if (conflicts(w->type, type)) {
            remember(host, slot, k, w);
            w->type = NONE;
            for (int i = 0; i < PARAMS; ++i) w->params[i] = 0;
            if (st.current_slot[host] == (int)slot) { st.pending_type[host][k] = NONE; st.pending_since[host][k] = (int)st.frames - REST_FRAMES; }
        }
    }
}

static void set_text(void* obj, const char* key, const char* text);       // the json helpers, below
static int doc_fx_active(void* doc);

// H6c: the record's document follows the effect the module puts on stock's slot. Stock keeps the slot's effect in
// the document as well (fx_type, fx_params, fx_active): the slots file carries it, the boot and a slot switch
// apply it, and the effect page's widgets read it, so a document left at the browser's last effect while the
// module had put another on the slot (a pick on the mirror knob, a preset load) showed that effect's parameters
// and saved them to the slots file: the H6b finding (synth_slots.json said delay with fazer's defaults while the
// slot ran nitro, then fazer). The type word, the block and the byte were right; the document is written here
// with them, and the type the chain knows the document by follows, so the next apply finds the chain standing.
// fx_type has no stock setter (its seven references are reads): the name goes in through the json string setter
// on the key's entry; the fx_params setter queues the record's write job (the changed virtual), so the slots
// file follows as after a knob turn on the page. Every write is logged (the first twenty-four).
static void mirror_document(int host, u32 slot, const Wagon* w, const char* why) {
    void* record = (void*)patch_record(host, slot);
    void* doc = (void*)(patch_record(host, slot) + 8);
    int had = doc_type[host][slot];
    set_text(doc, FX_TYPE_KEY, fx_name(w->type));
    STOCK_SET_PARAMS_DOC(record, w->params);
    STOCK_SET_ACTIVE_DOC(record, w->coupled ? 1 : 0);
    doc_type[host][slot] = w->type;
    ++st.doc_writes;
    st.doc_ms = st.ms;
    if (st.doc_lines < 24) {
        ++st.doc_lines;
        STOCK_LOG(3, LOG_FILE, 22, "chain document: %s s%u (%s): %s onto the slot and the document (the document had %s), %s, params %d %d %d %d; %u documents written in all",
                  host ? "drum" : "synth", slot, why, fx_name(w->type), fx_name(had), w->coupled ? "coupled" : "uncoupled",
                  int_of(w->params[0]), int_of(w->params[1]), int_of(w->params[2]), int_of(w->params[3]), st.doc_writes);
    }
}

// Stock's slot and the mirrored wagon agree. When the slot already runs the wagon's effect, stock's values are the
// authority (its document carries the knob turns made on the effect page, its fx_active byte the toggles), and the
// wagon takes them (H1f; H1e had written the wagon's values over them, zeros for every slot it had just met). When the
// slot runs another effect, it takes the wagon's type, parameters and coupling, and the producer is rebuilt as the
// browser does it, in the host's own mode only (the producer storage is shared by the synth and the drum; stock
// rebuilds from the type word otherwise).
static u8 switch_owed[HOSTS][SLOTS];                           // 17: 1 + the knob of the first wagon coupled while its switch was left to the rest (empty, or the slot another effect's); 0 none
static u8 document_owed[HOSTS][SLOTS];                         // 17: 1 + the first wagon's knob where an effect came over a switch owed with the document saying off; 0 none
// 17: the switch owed, given: the slot's and its document's.
static void switch_give(int host, u32 slot) {
    switch_owed[host][slot] = 0;
    document_owed[host][slot] = 0;
    WR8(HOST[host].fx_active + slot, 1);
    STOCK_SET_ACTIVE_DOC((void*)patch_record(host, slot), 1);
}

// 17: the document owed, given once the slot's switch is found on (the unit's list switches the slot on and not its document).
static void document_give(int host, u32 slot, int knob) {
    if (document_owed[host][slot] != knob + 1 || !RD8(HOST[host].fx_active + slot)) return;
    document_owed[host][slot] = 0;
    STOCK_SET_ACTIVE_DOC((void*)patch_record(host, slot), 1);
}

// 17: the sound slot left at a change of slot, which drops a push that waits: what is owed is given first.
static void switch_leave(int host, u32 slot) {
    int knob = mirror[host][slot];
    const Wagon* w = &wagons[host][slot][knob];
    if (w->type < 0 || rack_name[host][slot][knob][0] || slot_type(host, slot) != w->type) return;
    if (switch_owed[host][slot] == knob + 1 && w->coupled && st.push_pending[host]) switch_give(host, slot);
    document_give(host, slot, knob);
}

static void mirror_silent(int host, u32 slot);
static void mirror_push(int host, u32 slot) {
    int knob = mirror[host][slot];
    Wagon* w = &wagons[host][slot][knob];
    if (rack_name[host][slot][knob][0] && w->type >= 0) {                     // R5: a pack's effect wished on the first wagon: the frame rides it
        mirror_silent(host, slot);
        return;
    }
    if (w->type >= 0) {
        if (slot_type(host, slot) == w->type) {
            slot_read_params(host, slot, w->params);
            if (switch_owed[host][slot] == knob + 1 && w->coupled) switch_give(host, slot);   // 17: the switch a hold left to this rest, not read back as off
            w->coupled = RD8(HOST[host].fx_active + slot) ? 1 : 0;
        } else {
            slot_put(host, slot, w->type, w->params, w->coupled);                   // R6c: the firmware's order, the object made before the document
            mirror_document(host, slot, w, "push");                                 // H6c: the document too
        }
        switch_owed[host][slot] = 0;
    } else {
        WR8(HOST[host].fx_active + slot, 0);
        if (doc_fx_active((void*)(patch_record(host, slot) + 8))) STOCK_SET_ACTIVE_DOC((void*)patch_record(host, slot), 0);   // H6c: an empty mirror wagon is an inactive effect in the document as well
    }
    stock_type_seen[host][slot] = slot_type(host, slot);
}

// R5: the first wagon with a pack's effect it has not yet been given (a wish: a pick waiting for the frame, or one
// whose pack is away): the slot holds the effect it stands on with the wagon's words, switched off in the slot and in
// its document, so the sound passes it dry and a file says no effect is on.
static void mirror_silent(int host, u32 slot) {
    int knob = mirror[host][slot];
    Wagon* w = &wagons[host][slot][knob];
    void* record = (void*)patch_record(host, slot);
    void* doc = (void*)(patch_record(host, slot) + 8);
    WR8(HOST[host].fx_active + slot, 0);
    if (slot_type(host, slot) != w->type) {
        slot_put(host, slot, w->type, w->params, 0);                               // R6c: the firmware's order, switched off
        set_text(doc, FX_TYPE_KEY, fx_name(w->type));
        STOCK_SET_PARAMS_DOC(record, w->params);
        doc_type[host][slot] = w->type;
    }
    if (doc_fx_active(doc)) STOCK_SET_ACTIVE_DOC(record, 0);
    stock_type_seen[host][slot] = slot_type(host, slot);
}

static void mark_dirty(int host);
static u8 wish_from_pick[HOSTS][SLOTS];                        // R5: the first wagon's wish came from a pick on the train: the effect's start values, not the wagon's words

// R5: the first wagon given the pack's effect it wishes, the way the owner gives a sound one in the unit's list: the
// unit's own pick of the effect's id, which the base takes (W2): the place rides the effect, the firmware picks the
// effect it stands on, applies the record and asks for the slots file (the apply's reconcile takes that effect onto
// the first wagon, whose wish is spent first). The wagon keeps its coupling (the list's pick switches the effect on;
// the train's keeps the wagon's), and, where `keep`, its own four words, which the base writes over with the effect's
// start values at its next frame: put back now and at the frames that follow (restore). Nothing while the pack is away.
static void mirror_ride(int host, u32 slot, int keep) {
    int knob = mirror[host][slot];
    const Op1fRackEntry* e = rack_find(rack_name[host][slot][knob]);
    if (e == 0) return;
    Wagon* w = &wagons[host][slot][knob];
    u32 saved[PARAMS];
    for (int i = 0; i < PARAMS; ++i) saved[i] = w->params[i];
    int coupled = w->coupled;
    rack_name[host][slot][knob][0] = 0;
    wish_from_pick[host][slot] = 0;
    switch_owed[host][slot] = 0;                               // 17: the ride brings the wagon's own switch
    document_owed[host][slot] = 0;
    void* record = (void*)patch_record(host, slot);
    STOCK_PICK(record, (int)e->id);
    w = &wagons[host][slot][mirror[host][slot]];
    w->coupled = coupled;
    WR8(HOST[host].fx_active + slot, coupled ? 1 : 0);
    if (keep) {
        for (int i = 0; i < PARAMS; ++i) w->params[i] = saved[i];
        slot_write_params(host, slot, saved);
        STOCK_SET_PARAMS_DOC(record, saved);
    }
    restore[host].due = 1;
    restore[host].slot = (int)slot;
    restore[host].keep = keep;
    restore[host].coupled = coupled;
    restore[host].frame = st.frames;
    for (int i = 0; i < PARAMS; ++i) restore[host].params[i] = saved[i];
    stock_type_seen[host][slot] = slot_type(host, slot);
    mirror_ride_id[host] = e->id;                                          // the frame asks the base again; meanwhile the train names it
    mirror_ride_slot[host] = slot;
    ++rk.rides;
    if (rk.ride_lines < (u32)RACK_LINES) {
        ++rk.ride_lines;
        STOCK_LOG(3, LOG_FILE, 41, "chain ride: %s s%u knob %d: the first wagon given '%s' by the unit's pick of entry %u (on %s), %s, %s", host ? "drum" : "synth", slot, knob,
                  rack_name_of(e), e->id, fx_name(rack_rides(e)), coupled ? "coupled" : "uncoupled", keep ? "its own values kept" : "the effect's start values");
    }
    mark_dirty(host);
}

// R5: the frames after the unit's pick put the first wagon's coupling, and its own four words where they are kept,
// back over what the base set at its frame (the effect's start values, the switch it keeps for a ride).
static void restore_frame(int host, u32 slot) {
    if (!restore[host].due || restore[host].slot != (int)slot) return;
    u32 age = st.frames - restore[host].frame;
    if (age == 0) return;
    if (age > (u32)RESTORE_FRAMES) { restore[host].due = 0; return; }
    Wagon* w = &wagons[host][slot][mirror[host][slot]];
    if (slot_type(host, slot) != w->type) { restore[host].due = 0; return; }
    if (restore[host].keep) {
        u32 now[PARAMS];
        slot_read_params(host, slot, now);
        int differs = 0;
        for (int i = 0; i < PARAMS; ++i) if (now[i] != restore[host].params[i]) differs = 1;
        if (differs) {
            slot_write_params(host, slot, restore[host].params);
            STOCK_SET_PARAMS_DOC((void*)patch_record(host, slot), restore[host].params);
        }
        for (int i = 0; i < PARAMS; ++i) w->params[i] = restore[host].params[i];
    }
    if ((RD8(HOST[host].fx_active + slot) ? 1 : 0) != (restore[host].coupled ? 1 : 0)) WR8(HOST[host].fx_active + slot, restore[host].coupled ? 1 : 0);
    w->coupled = restore[host].coupled;
}

// R5: the first wagon's wish, at each frame for the slot on the screen: given where its pack is here (the pick's start
// values, or the wagon's own words); held silent where it is away, in the slot and in its document (the unit's own key
// on the effect's page switches the effect the wish stands on, which plays nothing of the pack's: the file must not say
// it is on), the wagon's own coupling kept.
static void wish_frame(int host, u32 slot) {
    int knob = mirror[host][slot];
    if (!rack_name[host][slot][knob][0] || wagons[host][slot][knob].type < 0) return;
    if (rack_find(rack_name[host][slot][knob])) { mirror_ride(host, slot, wish_from_pick[host][slot] ? 0 : 1); return; }
    if (RD8(HOST[host].fx_active + slot)) {
        WR8(HOST[host].fx_active + slot, 0);
        if (doc_fx_active((void*)(patch_record(host, slot) + 8))) STOCK_SET_ACTIVE_DOC((void*)patch_record(host, slot), 0);
        ++rk.hold_offs;
        if (rk.hold_lines < (u32)HOLD_LINES) {
            ++rk.hold_lines;
            STOCK_LOG(2, LOG_FILE, 42, "chain rack: %s s%u: the first wagon's '%s' has its pack away: the unit's %s is held off", host ? "drum" : "synth", slot,
                      rack_name[host][slot][knob], fx_name(wagons[host][slot][knob].type));
        }
    }
}

static void mark_dirty(int host) {
    st.dirty[host] = 1;
    st.dirty_since[host] = (int)st.frames;
    if (!ls.keep_loaded) ls.loaded_slot[host] = -1;            // H5: the chain changed, the lamp goes out
}

// R5: a wagon's cost: a pack's effect's is what it declares (nothing while its pack is away), the first wagon's riding
// one's likewise; the unit's own as before. `with_type` with `with_cycles` stands for a pick on `except_knob`: a pack's
// effect's declared cost, or -1 for the type's own.
static u32 cargo_cost(int type, const char* name, int coupled) {
    if (type < 0 || type >= TYPES || !coupled) return 0;
    if (name && name[0]) { const Op1fRackEntry* e = rack_find(name); return e ? e->effect->cycles : 0u; }
    return COST_MAX[type];
}

// 17: what runs in the first wagon's place now: the slot's producer, which is the effect of before until a pick's rest
// and for as long as a dropped pick leaves it there. A ride counts what it declares.
static const Op1fRackEntry* mirror_riding(int host, u32 slot);
static u32 mirror_runs(int host, u32 slot) {
    if (wagons[host][slot][mirror[host][slot]].type < 0 || !RD8(HOST[host].fx_active + slot)) return 0;
    int type = slot_type(host, slot);
    if (type < 0 || type >= TYPES) return 0;
    const Op1fRackEntry* riding = mirror_riding(host, slot);
    return riding && rack_rides(riding) == type ? riding->effect->cycles : COST_MAX[type];
}

// 17: at a turn or a hold (`at_turn`) the first wagon counts no less than what runs there; the needle says what the
// wagons show. 19: a wagon's share alone, for the line of a turn that stands.
static u32 wagon_cost(int host, u32 slot, int k, int at_turn) {
    const Wagon* w = &wagons[host][slot][k];
    u32 cost = cargo_cost(w->type, cargo_name(host, slot, k), w->coupled);
    if (at_turn && k == mirror[host][slot]) {                                // 17
        u32 runs = mirror_runs(host, slot);
        if (runs > cost) cost = runs;
    }
    return cost;
}

static u32 chain_cost(int host, u32 slot, int except_knob, int with_type, int with_cycles) {
    // 17: at a turn or a hold a knob is left out (`except_knob`), the needle leaves none out
    u32 sum = 0;
    for (int k = 0; k < WAGONS; ++k) {
        if (k == except_knob) {
            if (with_type >= 0 && with_type < TYPES) sum += with_cycles >= 0 ? (u32)with_cycles : COST_MAX[with_type];
            continue;
        }
        sum += wagon_cost(host, slot, k, except_knob >= 0);
    }
    return sum;
}

static int type_in_use(int host, u32 slot, int type, int except_knob) {
    for (int k = 0; k < WAGONS; ++k)
        if (k != except_knob && !rack_name[host][slot][k][0] && conflicts(wagons[host][slot][k].type, type)) return 1;   // R5: not a pack's effect's wagon
    return 0;
}

// R6b (M1): the picker's order, one table: none first, then the unit's own effects its list shows and every
// registered effect, sorted by name in byte order. An item is NONE, a type of the unit's (0 to 10), or ORDER_RACK plus
// an entry of the base's list. Two tables: the frame writes the one not in use and then names it in order_now, so that
// a turn on the key's thread reads a whole table.
enum { ORDER_RACK = 64, ORDER_MOST = 1 + TYPES + OP1F_EFFECTS_MOST };
static signed char order_items[2][ORDER_MOST];
static int order_count[2];
static volatile u32 order_now;                             // the table in use, 0 or 1
static u32 order_made, order_fazer, order_entries;         // made for: the fazer byte and the base's list's count
static const void* order_list;                             // and its first entry

static const char* order_name(int item) {
    if (item == NONE) return "";
    if (item >= ORDER_RACK) {
        const Op1fRackEntry* e = rack_entry((u32)(item - ORDER_RACK));
        return e ? rack_name_of(e) : "";
    }
    return fx_name(item);
}

static int name_before(const char* a, const char* b) {     // byte order, as the unit's list sorts its rows
    for (u32 i = 0;; ++i) {
        u32 x = (u8)a[i], y = (u8)b[i];
        if (x != y) return x < y;
        if (x == 0) return 0;
    }
}

// From the frame (the display thread), after the base's list is read: the table made again only when what it is made
// from changed.
static void order_build() {
    u32 fazer = RD8(FAZER_SHOWN) ? 1u : 0u;
    if (order_made && order_fazer == fazer && order_entries == rack_entry_count && order_list == (const void*)rack_entries) return;
    u32 into = order_now ^ 1u;
    signed char* items = order_items[into];
    int n = 0;
    items[n++] = NONE;
    for (int t = 0; t < TYPES; ++t)
        if (t != TYPE_VOCODER && (t != TYPE_FAZER || fazer)) items[n++] = (signed char)t;
    for (u32 i = 0; i < rack_entry_count && n < ORDER_MOST; ++i) {
        const Op1fRackEntry* e = rack_entry(i);
        if (e && e->effect && (e->effect->head.flags & (u32)OP1F_EFFECT_LAYER)) continue;   // R14: a layer ("train" itself among them) is no wagon's cargo: the base runs none at a pack's place
        items[n++] = (signed char)(ORDER_RACK + (int)i);
    }
    for (int i = 2; i < n; ++i) {                             // none stays first; the rest by name
        signed char item = items[i];
        const char* name = order_name(item);
        int j = i;
        while (j > 1 && name_before(name, order_name(items[j - 1]))) {
            items[j] = items[j - 1];
            --j;
        }
        items[j] = item;
    }
    order_count[into] = n;
    order_now = into;
    order_made = 1;
    order_fazer = fazer;
    order_entries = rack_entry_count;
    order_list = (const void*)rack_entries;
}

// R5, R6b: where a wagon's cargo stands in the picker's order (`items`, `count`); a pack's effect whose pack is away
// stands past the end, a type the order does not hold at its start.
static int cargo_index(int host, u32 slot, int knob, const signed char* items, int count) {
    const char* name = cargo_name(host, slot, knob);
    int want = wagons[host][slot][knob].type;
    if (name) {
        const Op1fRackEntry* e = rack_find(name);
        int i = e ? rack_index(e) : -1;
        if (i < 0) return count;
        want = ORDER_RACK + i;
    }
    for (int p = 0; p < count; ++p)
        if (items[p] == want) return p;
    return name ? count : 0;
}

// 17: the pack's effect the first wagon's place rides as the base has it (the frame's op1f_rack_at), whatever a turn
// not yet at rest has made of the wagon's type. A pick of the effect it stands on brings it back until that rest.
static const Op1fRackEntry* mirror_riding(int host, u32 slot) {
    u32 id = mirror_ride_id[host];
    if (id == 0 || mirror_ride_slot[host] != slot) return 0;
    for (u32 i = 0; i < rack_entry_count; ++i) {
        const Op1fRackEntry* e = rack_entry(i);
        if (e && e->id == id) return e;
    }
    return 0;
}

static u8 turn_passed[HOSTS][WAGONS];                          // 17: the knob's turn has passed an effect since its last rest

// 19: a turn that stands on an effect over the ceiling (pick, below; the header). Written by the key thread (the turns
// and the presses), each a whole word, the knob last; read by the frame, which also ends a stand whose page is gone.
static volatile int browse_knob[HOSTS] = { -1, -1 };          // the knob whose turn stands on such an effect, or -1
static volatile int browse_item[HOSTS], browse_type[HOSTS];   // the effect, an item of the picker's order; the type it stands on
static volatile u32 browse_slot[HOSTS], browse_need[HOSTS], browse_left[HOSTS];   // the slot; its cost, and what the ceiling leaves it (cycles a block)
static volatile u32 browse_stands;                            // stands since the start, for the log

static void browse_end(int host) { browse_knob[host] = -1; }

// A refused pick or coupling: the page names the effect in red for a second, with FULL and the red needle (H3).
static void refuse(int host, int knob, int type, const char* name) {
    ++st.refusals;
    st.flash_until[host] = (int)st.frames + FLASH_FRAMES;
    st.refused_knob[host] = knob;
    st.refused_type[host] = type;
    name_put(rk.refused_name[host], name);
}

// A turn of the wagon's knob: the next or previous effect in the browser's order (19: one over the ceiling is stood on,
// not picked; 17 went on past it to the next that fits), and past, without
// the regions of H4, the types another wagon carries; the label changes at once, the instance after
// the knob's rest. R5: past the unit's own eleven, every registered effect, in the list's order: the wagon takes the
// effect it stands on as its type and the pack's start values as its four words (a stock effect's remembered words are
// for that effect alone); on the first wagon the pack's effect is a wish the frame turns into a ride after the rest.
static void pick(int host, int knob, int delta) {
    u32 slot = gui_slot(host);
    Wagon* w = &wagons[host][slot][knob];
    u32 now = order_now;                                                     // R6b: the table the frame made last
    const signed char* items = order_items[now];
    int end = order_count[now];
    int index = cargo_index(host, slot, knob, items, end);
    if (browse_knob[host] == knob && browse_slot[host] == slot)              // 19: on from the effect the turn stands on
        for (int p = 0; p < end; ++p) if (items[p] == browse_item[host]) { index = p; break; }
    int step = delta > 0 ? 1 : -1;
    const Op1fRackEntry* riding = knob == mirror[host][slot] ? mirror_riding(host, slot) : 0;
    if ((int)st.frames - st.pending_since[host][knob] >= REST_FRAMES) turn_passed[host][knob] = 0;
    for (int tries = 0; tries < end + 1; ++tries) {
        index += step;
        if (index < 0 || index >= end) break;                                // the ends of the list
        const Op1fRackEntry* e = 0;
        int type = items[index];
        if (type >= ORDER_RACK && knob == mirror[host][slot]) continue;    // R14: no pack's effect on the first wagon: its pick would make the place ride it and end the train
        if (type >= ORDER_RACK) {
            e = rack_entry((u32)(type - ORDER_RACK));
            if (e == 0) return;
            type = rack_rides(e);
            if (type < 0) continue;                                          // not one a pack's effect may stand on (the base refuses such)
        }
        if (!e && type >= 0 && type_in_use(host, slot, type, knob)) continue;
        int back = !e && type >= 0 && riding && type == rack_rides(riding);  // 17: such a pick brings the ride back
        const Op1fRackEntry* over_as = e;
        int over = type >= 0 && chain_cost(host, slot, knob, type, e ? (int)e->effect->cycles : -1) > CEILING_CYCLES;
        if (!over && back && chain_cost(host, slot, knob, type, (int)riding->effect->cycles) > CEILING_CYCLES) {
            over = 1;                                                        // 17: and it has to fit as that too
            over_as = riding;
        }
        if (over) {
            // 19: the turn stands on it (17 went on to the next that fits): the wagon as it was, the page told what it
            // needs and what the rest of the train leaves it, FULL as 17 showed it
            turn_passed[host][knob] = 1;
            u32 others = chain_cost(host, slot, knob, NONE, -1);
            browse_item[host] = items[index];
            browse_type[host] = type;
            browse_slot[host] = slot;
            browse_need[host] = over_as ? over_as->effect->cycles : COST_MAX[type];
            browse_left[host] = others >= (u32)CEILING_CYCLES ? 0u : (u32)CEILING_CYCLES - others;
            browse_knob[host] = knob;
            ++browse_stands;
            refuse(host, knob, type, rack_name_of(over_as));
            return;
        }
        if (back && turn_passed[host][knob]) continue;                       // 17: no pick of it ends the ride, and the turn would stop there
        if (st.refused_knob[host] == knob) st.flash_until[host] = (int)st.frames;     // 17: this wagon's refusal is over
        browse_end(host);                                                    // 19: and its stand
        remember(host, slot, knob, w);
        w->type = type;
        if (e) {
            rack_start_params(e, type, w->params);                           // R5: the pack's start values
        } else if (type >= 0) {
            if (memory_set[host][slot][knob][type]) for (int i = 0; i < PARAMS; ++i) w->params[i] = memory[host][slot][knob][type][i];
            else params_default(type, w->params);
        } else {
            for (int i = 0; i < PARAMS; ++i) w->params[i] = 0;
        }
        name_put(rack_name[host][slot][knob], rack_name_of(e));
        if (knob == mirror[host][slot]) wish_from_pick[host][slot] = e ? 1 : 0;
        st.pending_type[host][knob] = type;
        st.pending_since[host][knob] = (int)st.frames;
        if (knob == mirror[host][slot]) st.push_pending[host] = 1;
        ++st.picks;
        mark_dirty(host);
        return;
    }
}

static void move_wagon(int host, int knob, int delta) {
    u32 slot = gui_slot(host);
    int knobs[WAGONS];
    int pos = 0;
    for (int p = 0; p < WAGONS; ++p) {
        knobs[p] = knob_at(host, slot, p);
        if (knobs[p] == knob) pos = p;
    }
    int to = pos + (delta > 0 ? 1 : -1);
    if (to < 0 || to >= WAGONS) return;
    knobs[pos] = knobs[to];
    knobs[to] = knob;
    set_order(host, slot, knobs);
    ++st.moves;
    mark_dirty(host);
}

// 17: a wagon switched on or off. The first wagon's switch and document follow at once; but a coupling while the slot
// still holds another effect, or a pack's effect waits to be given, is left to the push and the ride at the knob's rest,
// which bring the effect and its switch together. Another wagon coupled while its Live is still the effect of before
// its last pick is unpublished until the rest builds its own: no effect runs that the ceiling did not count.
static void couple(int host, u32 slot, int knob, int coupled) {
    Wagon* w = &wagons[host][slot][knob];
    const char* wish = rack_name[host][slot][knob];
    w->coupled = coupled ? 1 : 0;
    if (knob == mirror[host][slot]) {
        int owed = coupled && (w->type < 0 || slot_type(host, slot) != w->type || wish[0]);
        if (!coupled || w->type < 0 || (slot_type(host, slot) == w->type && !wish[0])) {
            int active = w->type >= 0 && w->coupled;                         // an empty first wagon is an effect switched off to the unit
            WR8(HOST[host].fx_active + slot, active);
            STOCK_SET_ACTIVE_DOC((void*)patch_record(host, slot), active);
        }
        switch_owed[host][slot] = owed ? (u8)(knob + 1) : 0;                 // what is left to the rest, the empty wagon's switch too
        document_owed[host][slot] = 0;
    } else if (coupled) {
        Live* l = &live[host][knob];
        if (l->type != w->type || !name_same(l->rack_name, wish)) {
            l->obj = 0;
            asm volatile("" ::: "memory");
            l->rack = 0;
            l->rack_name[0] = 0;
            l->type = NONE;
        }
    }
}

// A hold of the wagon's knob couples or uncouples it. 17: a coupling is held to the ceiling as a pick is, a pack's
// effect by what it declares: over it the wagon stays uncoupled, nothing is written and the refusal shows. Uncoupling
// and coupling an empty wagon always go through.
static void set_coupled(int host, int knob, int coupled) {
    u32 slot = gui_slot(host);
    Wagon* w = &wagons[host][slot][knob];
    if (coupled && !w->coupled && w->type >= 0 && w->type < TYPES) {
        const char* name = cargo_name(host, slot, knob);
        if (chain_cost(host, slot, knob, w->type, (int)cargo_cost(w->type, name, 1)) > CEILING_CYCLES) {
            refuse(host, knob, w->type, name);
            return;
        }
    }
    couple(host, slot, knob, coupled);
    mark_dirty(host);
}

// H3's whole-chain bypass under shift + T3 (toggle_bypass) is gone since H5: the key opens the presets list, and a
// chain whose file remembers parked wagons (the tag-92 mask) gets them back on the line at the load.

static void apply_params(int host, int knob) {
    u32 slot = gui_slot(host);
    Wagon* w = &wagons[host][slot][knob];
    if (knob == mirror[host][slot]) {
        slot_write_params(host, slot, w->params);
        STOCK_SET_PARAMS_DOC((void*)patch_record(host, slot), w->params);   // stock's own key follows its slot
    } else {
        for (int i = 0; i < PARAMS; ++i) live[host][knob].ctx[i] = w->params[i];
    }
    mark_dirty(host);
}

// Shift + tap: the four knob parameters from stock's own random-preset value function 0x8d51233c(type, index, bias),
// the one its "random preset" job (0x8d4e9724, at 0x8d4e9076) calls for each effect parameter with the descriptor table
// 0x840afe60 behind it, so every value is one stock would write itself (H1b drew 8 to 85 percent of a fixed 0..32767
// scale, which is outside the delay's own ranges: the delay crashed on it); the other four stay at the defaults.
static void randomize(int host, int knob) {
    u32 slot = gui_slot(host);
    Wagon* w = &wagons[host][slot][knob];
    if (w->type < 0) return;
    if (rack_name[host][slot][knob][0] && !rack_find(rack_name[host][slot][knob])) return;   // R5: a pack's effect whose pack is away: nothing rewritten
    for (int i = 0; i < PARAMS; ++i) {
        int value = i < 4 ? (int)(short)STOCK_RANDOM_PARAM(w->type, i, 0) : DEFAULTS[w->type][i];
        w->params[i] = word_of(value);
    }
    ++st.randoms;
    apply_params(host, knob);
}

static void reset_params(int host, int knob) {
    u32 slot = gui_slot(host);
    Wagon* w = &wagons[host][slot][knob];
    if (w->type < 0) return;
    const char* name = cargo_name(host, slot, knob);                       // R5: a pack's effect's own start values; nothing while its pack is away
    const Op1fRackEntry* e = rack_find(name);
    if (name && !e) return;
    if (e) rack_start_params(e, w->type, w->params);
    else params_default(w->type, w->params);
    ++st.resets;
    apply_params(host, knob);
}

// The tap: the wagon becomes the mirror (its effect pushed into the unit's slot) and the stock page of its effect is
// wished (R14: the input unit's frame shows it, the page choice UNIT; fx train 17 made up a T3 for stock here, and
// opened the unit's list for an empty wagon, which is now nothing: the list would end the ride).
static void open_wagon_page(int host, int knob) {
    u32 slot = gui_slot(host);
    int old = mirror[host][slot];
    if (old != knob) {
        // R5: the wagon that was first, where its place rode a pack's effect, keeps that effect and plays it on the
        // train; but not where the new first wagon carries the very effect of the unit's the ride stands on: then the
        // place goes on riding, the new first wagon plays and is named by the pack's effect, and the old one plays the
        // unit's own (the contract's wrinkle, 4.5: no pick of the unit's list ends a ride but one of another effect)
        const Op1fRackEntry* riding = mirror_entry(host, slot);
        const Wagon* nw = &wagons[host][slot][knob];
        if (riding && !(rack_name[host][slot][knob][0] == 0 && nw->type == rack_rides(riding))) {
            name_put(rack_name[host][slot][old], rack_name_of(riding));
            ++rk.converts;
        }
        mirror[host][slot] = (u8)knob;
        if (switch_owed[host][slot]) switch_owed[host][slot] = (u8)(knob + 1);   // 17: the slot's "off" was the empty wagon's: the one tapped brings its own switch
        live[host][knob].obj = 0;                                            // the mirror runs through stock's producer
        live[host][knob].rack = 0;
        live[host][knob].rack_name[0] = 0;
        live[host][knob].type = NONE;
        st.pending_type[host][knob] = NONE;
        st.push_pending[host] = 0;
        if (rack_name[host][slot][knob][0] && rack_find(rack_name[host][slot][knob])) mirror_ride(host, slot, 1);   // R5: a pack's effect with its own words, ridden now, before the page's key asks for its screen
        else mirror_push(host, slot);
        st.pending_type[host][old] = wagons[host][slot][old].type;          // the former mirror needs a module instance now
        st.pending_since[host][old] = (int)st.frames - REST_FRAMES;
        mark_dirty(host);
    }
    ++st.page_opens;
    take_page = TAKE_UNIT;
}

// ---------------------------------------------------------------- the keys (R14: from the base's input)

// H5: the presets list's handlers (defined after the persistence, which they use)
static void t3_press(int host);
static void t3_release(int host);
static void list_release(int host, int knob);
static void list_turn(int host, int knob, int delta);
static int list_key(int host, int key, int shifted);
static void presets_leave(int host);
static void list_frame(int host);

// A knob's physical press reaches the controller as the key 61 + knob at key-down (through the shifted key-down
// while shift is held), its release as the same key at key-up; then, when the knob was not turned while pressed,
// the key handler 0x8c0b1468 synthesizes a click, the key 54 + knob at key-down (shifted by shift at the release)
// and at key-up. On the train the press starts the hold timer (frame_host fires it), the release ends it, and the
// click is the tap unless a hold fired just before it.
static void knob_press(int host, int knob, int shifted) {
    int gap = (int)st.ms - st.last_turn_ms[host][knob];
    browse_end(host);                                                        // 19: a press ends a turn's stand
    st.pressed[host][knob] = 1;
    st.press_ms[host][knob] = (int)st.ms;
    st.press_shift[host][knob] = shifted;
    st.acted[host][knob] = 0;
    st.hold_armed[host][knob] = gap >= CLEAN_MS;                             // H3b: a press right after a turn arms no hold (the click after it is still a tap); H4b: 120 ms
    if (!st.hold_armed[host][knob]) {
        ++st.turn_presses;
        STOCK_LOG(3, LOG_FILE, 7, "chain press: knob %d turned %d ms before the press, no hold armed", knob, gap);
    }
}

static void knob_release(int host, int knob) {
    if (st.pressed[host][knob] && st.acted[host][knob]) st.swallow_until[host][knob] = (int)st.frames + SWALLOW_FRAMES;
    st.pressed[host][knob] = 0;
}

// The click: shift + click randomises the wagon's values; a click moves the mirror to the wagon and wishes its stock
// page. R14: an empty wagon, or one that carries a pack's effect (its page would be the base's page of a ride, and the
// place rides "train"), does nothing.
static int knob_click(int host, int knob, int shifted) {
    if ((int)st.frames < st.swallow_until[host][knob]) {                   // the click after a hold
        st.swallow_until[host][knob] = 0;
        ++st.swallowed;
        return 1;
    }
    u32 slot = gui_slot(host);
    if (shifted) randomize(host, knob);
    else if (wagons[host][slot][knob].type >= 0 && !cargo_name(host, slot, knob)) { ++st.taps; open_wagon_page(host, knob); }
    return 1;
}

static int host_of_place(int place) { return place >= (int)OP1F_PLACE_DRUM ? 1 : 0; }
static int valid_place(int place) { return place >= 0 && place < (int)PLACES; }

// The place on the screen: the synth's slot in the synth mode (MODE_GUI 0), 8 + the drum's slot in the drum mode (1),
// -1 in another mode.
extern "C" int op1f_train_place() asm("op1f_train_place");
extern "C" int op1f_train_place() {
    u32 mode = RD32(MODE_GUI);
    if (mode == 0u) return (int)gui_slot(0);
    if (mode == 1u) return (int)OP1F_PLACE_DRUM + (int)gui_slot(1);
    return -1;
}

// The train page's keys, the presets' list not showing: the knobs' press and click, T3 held for
// the presets (a tap wishes the stock page of the first wagon's effect); T1, T2, T4 and every other key are stock's.
static int page_key(int host, int key, int shifted) {
    if (key >= KEY_PRESS0 && key < KEY_PRESS0 + WAGONS) { knob_press(host, key - KEY_PRESS0, shifted); return 1; }
    if (key >= KEY_CLICK0 && key < KEY_CLICK0 + WAGONS) return knob_click(host, key - KEY_CLICK0, shifted);
    if (key == KEY_T3 && !shifted) { t3_press(host); return 1; }             // held HOLD_MS: the presets' list (list_frame); let go before: the tap (t3_release)
    if (key >= KEY_T1 && key <= KEY_T4) presets_leave(host);                 // another page: stock leaves the train's
    return 0;
}

// A key for a place (what: OP1F_DOWN, OP1F_DOWN_SHIFT, OP1F_UP), on the key thread: 1 taken, 0 stock's. The input
// unit calls it while the train page, or the presets over it, is up and the place rides "train"; the up of a down it
// took comes here whatever shows then. The keys 13 to 53 (the keyboard, shift) are never the page's: they may come on
// the audio thread. Shift + T3: 0 always (the unit's list of effects), the presets' list closed first.
extern "C" int op1f_train_key(int place, int key, int what) asm("op1f_train_key");
extern "C" int op1f_train_key(int place, int key, int what) {
    if (!valid_place(place) || key < 0 || key >= KEYS_KEPT || (key >= NOTE_KEY_FIRST && key <= NOTE_KEY_LAST)) return 0;
    int host = host_of_place(place);
    u32 bit = 1u << (key & 31);
    u32* taken = &down_taken[key >> 5];
    if (what == OP1F_UP) {
        int mine = (*taken & bit) != 0;
        *taken &= ~bit;
        if (key >= KEY_PRESS0 && key < KEY_PRESS0 + WAGONS) {             // the knob let go: the list's red hold or the wagon's hold ends (both: the list may have opened while it was down)
            list_release(host, key - KEY_PRESS0);
            knob_release(host, key - KEY_PRESS0);
        }
        if (key == KEY_T3 && ls.t3_pressed[host]) t3_release(host);         // the tap (the hold has opened the list already)
        return mine;
    }
    if ((what != OP1F_DOWN && what != OP1F_DOWN_SHIFT) || !place_rides(host, (u32)place & 7u)) return 0;
    int shifted = what == OP1F_DOWN_SHIFT;
    int answer;
    if (key == KEY_T3 && shifted) { presets_leave(host); answer = 0; }
    else if (ls.open[host]) answer = list_key(host, key, shifted);           // H5: the list over the train takes its keys first
    else answer = page_key(host, key, shifted);
    if (answer) *taken |= bit;
    return answer;
}

// A knob turned (knob 0 to 3; shifted: shift held, the wagon moves), on the key thread: the list's scroll, caret and
// letter while it shows, else the wagon's effect before or after. 1.
extern "C" int op1f_train_turn(int place, int knob, int steps, int shifted) asm("op1f_train_turn");
extern "C" int op1f_train_turn(int place, int knob, int steps, int shifted) {
    if (!valid_place(place) || knob < 0 || knob >= WAGONS) return 0;
    int host = host_of_place(place);
    if (steps == 0) return 1;
    if (ls.open[host]) { list_turn(host, knob, steps); return 1; }            // H5
    if (st.pressed[host][knob]) st.acted[host][knob] = 1;                   // a turn while pressed: no hold (stock sends no click either)
    st.last_turn_ms[host][knob] = (int)st.ms;                               // H3b: a press in the next 120 ms arms no hold (H4b)
    if (shifted || browse_knob[host] != knob) browse_end(host);              // 19: a move, or a turn of another knob, ends a stand
    if (shifted) move_wagon(host, knob, steps);
    else pick(host, knob, steps);
    st.turn_knob[host] = knob;                                              // the page's card (H2)
    st.turn_frame[host] = (int)st.frames;
    st.turn_move[host] = shifted ? 1 : 0;                                   // the page's hop (H3)
    return 1;
}

// The page a key wished since the last read: TAKE_NONE, TAKE_OWN (the train page) or TAKE_UNIT (the stock page of the
// first wagon's effect). Cleared by the read; the display thread (the input unit's frame).
extern "C" int op1f_train_take_page() asm("op1f_train_take_page");
extern "C" int op1f_train_take_page() {
    unsigned was = op1f_mask();                                // the key thread writes the wish: read and cleared as one
    int w = take_page;
    take_page = TAKE_NONE;
    op1f_unmask(was);
    return w;
}

// The train page left the screen for a host: the presets' list and its hold go, and the knobs held on it let go.
static void train_leave(int host) {
    presets_leave(host);
    for (int k = 0; k < WAGONS; ++k) st.pressed[host][k] = 0;
}

// Once a frame, from the input unit's frame: the place whose train page, or the presets over it, is on the screen,
// -1 for none. Its host's train is open (the view, the holds and the presets read it), the other host's is not.
extern "C" void op1f_train_up(int place) asm("op1f_train_up");
extern "C" void op1f_train_up(int place) {
    int up = valid_place(place) ? host_of_place(place) : -1;
    for (int h = 0; h < HOSTS; ++h) {
        int open = h == up;
        if (st.train_open[h] && !open) train_leave(h);
        if (open && !st.train_open[h]) ++st.train_opens;
        st.train_open[h] = open;
    }
}

// H6h: a knob turn stock handles on the effect page or the LFO page, with the descriptor entry the converter uses for it
// (the type word's row, the knob's column; the shift layer is the same column with the knob's value divided by 16) and
// the slot's parameter words before and after, for the read-out: the static tables read as (0, 0, step), and the
// device shows whether the clamp fields are filled at run time or the converter reads otherwise (the LFO study, 5.5).
// H6i: H6h printed one line a turn and twenty-four in all, which the first page's knobs alone would have used up (every
// detent is a turn). Each knob now has an entry of its own (the host, the page, the type, the index with its shift
// bit): the slot's whole eight-word block is read before and after stock's handler, the first two turns and the first
// turns that changed the block print a line each (eight per effect knob, sixteen per LFO knob, whose destination steps
// are worth following one by one), and a knob left alone for a second and a half prints one line with what it reached:
// the lowest and the highest value of its own word (the clamps, once the knob was turned to both ends), the smallest
// and the largest step, the deltas, the words it changed. A type's descriptor row (eight entries of three half-words)
// is printed once, when its first knob is turned. The budgets are counted against the checklist that spends them
// (the PC's tests hold them): twenty-four effect knobs, sixteen LFO knobs.
// H6j: the probe did its work (its read-out: the rows, the law of a turn, the LFO's words) and is
// switched off: left on, it would write up to some 860 lines a boot on the two pages the owner plays most.
// KNOB_PROBE = 1 brings it back as it ran in H6i.
enum { KNOB_PROBE = 0 };
enum { PROBES = 64, PROBE_IDLE_MS = 1500, BLOCK_WORDS = 8, FX_TURN_LINES = 8, LFO_TURN_LINES = 16, FX_TURN_LINES_ALL = 240, LFO_TURN_LINES_ALL = 320, RANGE_LINES_ALL = 300 };
struct KnobProbe {
    u32 key;                                                   // 0 free; else 1 | host << 1 | lfo << 2 | type << 4 | index << 8
    u32 desc;                                                  // the descriptor entry the converter reads for this knob
    u32 slot;                                                  // the sound slot of the last turn
    u32 turns, moved;                                          // the turns passed to stock; those that changed a word of the block
    i32 delta_lo, delta_hi;                                    // the encoder deltas seen
    i32 lo, hi;                                                // the knob's own word, the lowest and the highest seen, before or after a turn
    u32 step_lo, step_hi;                                      // the smallest and the largest change of the own word in one turn
    u32 changed;                                               // bit n: a turn changed word n of the block
    u32 lines;                                                 // the per-turn lines printed for this knob
    u32 dirty_ms;                                              // the time of the last turn
    u8 dirty;                                                  // turned since its last range line
};
static KnobProbe probes[PROBES];

static const char* probe_name(int lfo, u32 type) { return lfo ? (type < 6u ? *LFO_NAMES[type] : "?") : fx_name((int)type); }

static KnobProbe* probe_for(int host, int lfo, u32 type, int index) {
    u32 key = 1u | ((u32)host << 1) | ((u32)lfo << 2) | (type << 4) | ((u32)(index & 7) << 8);
    for (int i = 0; i < PROBES; ++i) {
        if (probes[i].key == key) return &probes[i];
        if (probes[i].key == 0) { probes[i].key = key; return &probes[i]; }   // the entries fill in order; a fresh one is all zeros
    }
    if (!st.probes_full) {
        st.probes_full = 1;
        STOCK_LOG(2, LOG_FILE, 27, "knob probe: the table of %d knobs is full: no line for a further knob", (int)PROBES);
    }
    return 0;
}

static void print_row(int lfo, u32 type) {
    if (st.row_mask[lfo] & (1u << type)) return;
    st.row_mask[lfo] |= 1u << type;
    u32 row = (lfo ? LFO_DESCRIPTORS : FX_DESCRIPTORS) + 48u * type;
    for (u32 half = 0; half < 2u; ++half) {
        u32 e = row + 24u * half;
        STOCK_LOG(3, LOG_FILE, 25, "knob row: %s %s (type %u) at %08x, entries %u to %u, three half-words each: %u %u %u, %u %u %u, %u %u %u, %u %u %u",
                  lfo ? "lfo" : "fx", probe_name(lfo, type), type, row, half * 4u, half * 4u + 3u,
                  (u32)RD16(e), (u32)RD16(e + 2u), (u32)RD16(e + 4u), (u32)RD16(e + 6u), (u32)RD16(e + 8u), (u32)RD16(e + 10u),
                  (u32)RD16(e + 12u), (u32)RD16(e + 14u), (u32)RD16(e + 16u), (u32)RD16(e + 18u), (u32)RD16(e + 20u), (u32)RD16(e + 22u));
    }
}

static int stock_knob_turn(void* self, int host, int index, int delta) {
    int page = *page_of(self);
    int lfo = page == PAGE_LFO;
    if (!KNOB_PROBE || (page != PAGE_FX && !lfo)) return ((EncoderFn)HOST[host].encoder)(self, index, delta);
    u32 slot = gui_slot(host);
    int own = index & 3;                                                     // the knob's own word of the block (the shifted knob adjusts the same word)
    u32 type_word = RD32((lfo ? LFO_TYPE_ARRAY[host] : HOST[host].type_array) + 4u * slot);
    u32 block = (lfo ? LFO_BLOCK[host] : HOST[host].param_block) + 0x20u * slot;
    u32 before[BLOCK_WORDS], after[BLOCK_WORDS];
    for (int w = 0; w < BLOCK_WORDS; ++w) before[w] = RD32(block + 4u * (u32)w);
    int result = ((EncoderFn)HOST[host].encoder)(self, index, delta);
    u32 mask = 0;
    for (int w = 0; w < BLOCK_WORDS; ++w) {
        after[w] = RD32(block + 4u * (u32)w);
        if (after[w] != before[w]) mask |= 1u << w;
    }
    ++st.knob_turns;
    if (type_word >= 16u) return result;                                     // no descriptor row for such a word
    KnobProbe* p = probe_for(host, lfo, type_word, index);
    if (p == 0) return result;
    u32 desc = (lfo ? LFO_DESCRIPTORS : FX_DESCRIPTORS) + 48u * type_word + 6u * (u32)own;
    i32 b = (i32)before[own], a = (i32)after[own];
    if (p->turns == 0) { p->lo = b; p->hi = b; p->delta_lo = delta; p->delta_hi = delta; p->step_lo = 0xffffffffu; p->step_hi = 0; }
    ++p->turns;
    if (delta < p->delta_lo) p->delta_lo = delta;
    if (delta > p->delta_hi) p->delta_hi = delta;
    if (b < p->lo) p->lo = b;
    if (a < p->lo) p->lo = a;
    if (b > p->hi) p->hi = b;
    if (a > p->hi) p->hi = a;
    if (a != b) {
        u32 step = a > b ? (u32)a - (u32)b : (u32)b - (u32)a;
        if (step < p->step_lo) p->step_lo = step;
        if (step > p->step_hi) p->step_hi = step;
    }
    if (mask) ++p->moved;
    p->changed |= mask;
    p->slot = slot;
    p->desc = desc;
    p->dirty = 1;
    p->dirty_ms = st.ms;
    print_row(lfo, type_word);
    u32 per_knob = lfo ? (u32)LFO_TURN_LINES : (u32)FX_TURN_LINES, all = lfo ? (u32)LFO_TURN_LINES_ALL : (u32)FX_TURN_LINES_ALL;
    if ((mask || p->turns <= 2u) && p->lines < per_knob && st.turn_lines[lfo] < all) {   // the first two turns whatever they did, then the turns that changed the block
        ++p->lines;
        ++st.turn_lines[lfo];
        int shown = own;                                                     // the knob's own word, or the lowest word that changed when the own one did not
        if (mask && !(mask & (1u << own))) for (shown = 0; !(mask & (1u << shown)); ++shown) {}
        STOCK_LOG(3, LOG_FILE, 24, "knob turn: %s s%u %s %s knob %d (index %d, delta %d): word %d %08x -> %08x (%d -> %d), changed mask %u; descriptor %08x (%u %u %u); turn %u",
                  host ? "drum" : "synth", slot, lfo ? "lfo" : "fx", probe_name(lfo, type_word), own, index, delta, shown, before[shown], after[shown],
                  int_of(before[shown]), int_of(after[shown]), mask, desc, (u32)RD16(desc), (u32)RD16(desc + 2u), (u32)RD16(desc + 4u), p->turns);
    }
    return result;
}

// H6j: every type's descriptor row printed once, some ten seconds after the first frame, a type a frame (two lines):
// the eleven effects, then the six LFOs, so one read-out holds the whole of both tables as the boot's initialiser
// left them (H6i printed the rows of the types whose knobs were turned: the delay, the terminal, the value LFO).
enum { ROWS_AT_FRAME = 600, LFO_TYPES = 6, ROW_TYPES = TYPES + LFO_TYPES };
static void rows_frame() {
    if (st.rows_next >= (u32)ROW_TYPES || st.frames < (u32)ROWS_AT_FRAME) return;
    u32 n = st.rows_next++;
    if (n < (u32)TYPES) print_row(0, n); else print_row(1, n - (u32)TYPES);
}

// H6i: a knob that rested for a second and a half says what it reached, two lines a frame at most.
static void probes_frame() {
    if (!KNOB_PROBE) return;
    int printed = 0;
    for (int i = 0; i < PROBES && printed < 2; ++i) {
        KnobProbe* p = &probes[i];
        if (p->key == 0) break;                                              // the entries fill in order
        if (!p->dirty || st.ms - p->dirty_ms < (u32)PROBE_IDLE_MS) continue;
        p->dirty = 0;
        if (st.range_lines >= (u32)RANGE_LINES_ALL) continue;
        ++st.range_lines;
        ++printed;
        int host = (int)((p->key >> 1) & 1u), lfo = (int)((p->key >> 2) & 1u), index = (int)((p->key >> 8) & 7u);
        u32 type = (p->key >> 4) & 15u;
        STOCK_LOG(3, LOG_FILE, 26, "knob range: %s s%u %s %s knob %d (index %d): %u turns, %u moved, delta %d..%d; word %08x..%08x (%d..%d), step %08x..%08x; changed mask %u; descriptor %08x (%u %u %u)",
                  host ? "drum" : "synth", p->slot, lfo ? "lfo" : "fx", probe_name(lfo, type), index & 3, index, p->turns, p->moved, p->delta_lo, p->delta_hi,
                  (u32)p->lo, (u32)p->hi, int_of((u32)p->lo), int_of((u32)p->hi), p->step_hi ? p->step_lo : 0u, p->step_hi, p->changed,
                  p->desc, (u32)RD16(p->desc), (u32)RD16(p->desc + 2u), (u32)RD16(p->desc + 4u));
    }
}

// I2: the value LFO's destination knob (index 2, no shift; stock's handler 0x8c0cec20 moves the block's word 2 by 352
// a detent between 1024 and 11264). Below the effect's range the knob is stock's. Inside it one detent is one effect
// wagon: the wagons that carry an effect, in knob order, first in the form of its own phase, then in the form shared
// by all voices (the page's FREE); the word is parked in the middle of the form's range meanwhile and changes only
// when the form changes or the knob leaves the range downwards. With no effect on any wagon the knob is stock's.
//
// Stock's handler is still called for every turn the module takes, with a turn of nothing (a delta of 0 moves no
// value: the converter adds (speed x delta) >> 15): it writes the slot's LFO block into the record's document
// (0x8d4f3c84, which queues the record's write job) and does what stock does after any turn on this page (the
// controller's encoder handler 0x8c0d5c10 / 0x8c0b7634: its hint when the LFO is off, its own bookkeeping).
static u32 lfo_type_word(int host, u32 slot) { return RD32(LFO_TYPE_ARRAY[host] + 4u * slot); }
static u32* lfo_block_of(int host, u32 slot) { return (u32*)(LFO_BLOCK[host] + 0x20u * slot); }

static int next_fx_knob(int host, u32 slot, int from, int dir) {         // the next wagon with an effect after `from` in the direction, or -1
    for (int k = from + dir; k >= 0 && k < WAGONS; k += dir) if (wagons[host][slot][k].type >= 0) return k;
    return -1;
}

static void lfofx_log_at(int host, u32 slot, const char* what, int value, int knob, int dest) {   // dest: 0 the LFO's one destination; 1 to 4 the midi LFO's (I4)
    if (lfofx_lines >= (u32)LFOFX_LINES) return;
    ++lfofx_lines;
    STOCK_LOG(3, LOG_FILE, 31, "lfo fx: %s s%u %s: the destination word %d of lfo type %u (destination %d), the effect wagon of knob %d (%s)%s",
              host ? "drum" : "synth", slot, what, value, lfo_type_word(host, slot), dest, knob, fx_name(wagons[host][slot][knob].type), knob == mirror[host][slot] ? ", the one under the lamp" : "");
}
static void lfofx_log(int host, u32 slot, const char* what, int value) { lfofx_log_at(host, slot, what, value, lfo_fx_knob_of(host, slot), 0); }

// I3: the random, element and velocity LFOs, whose destination word has four classes and no shared form
static int lfo_type_four(u32 type) { return type == (u32)LFO_TYPE_RANDOM || type == (u32)LFO_TYPE_ELEMENT || type == (u32)LFO_TYPE_VELOCITY; }

// Their destination knob: outside the effect's range stock's own step; a step that lands inside starts at the near
// end of the wagons (the first from below, the last from above) with the word parked in the range's middle; inside,
// a detent is a wagon, and a detent past the last or before the first leaves the range for the middle of the class
// next door, the choice kept.
// I4: the word and the choice are the caller's: the block's word 2 and the slot's one choice for the three LFOs, the
// word 4 + n and the destination's own choice for the midi LFO's destination n (dest n + 1 on the log's line).
static int lfo_dest_turn_four(void* self, int host, u32 slot, int index, int delta, int* result, int first, int last, int word_index, u8* pick, int dest) {
    EncoderFn stock = (EncoderFn)HOST[host].encoder;
    u32* word = &lfo_block_of(host, slot)[word_index];
    int value = int_of(*word);
    if (value < LFO4_FX_LO || value >= LFO4_FX_HI) {
        *result = stock(self, index, delta);
        int landed = int_of(*word);
        if (landed >= LFO4_FX_LO && landed < LFO4_FX_HI) {
            *pick = (u8)((value < LFO4_FX_LO ? first : last) + 1);
            *word = word_of(LFO4_FX_PARK);
            *result = stock(self, index, 0);
            mark_dirty(host);
            lfofx_log_at(host, slot, value < LFO4_FX_LO ? "the knob entered the effect's range from below" : "the knob entered the effect's range from above", LFO4_FX_PARK, (int)*pick - 1, dest);
        }
        return 1;
    }
    int dir = delta > 0 ? 1 : -1;
    int at = (*pick && wagons[host][slot][*pick - 1].type >= 0) ? *pick - 1 : mirror[host][slot];   // the wagon it turns now (lfo_fx_knob_of's rule)
    int next = next_fx_knob(host, slot, at, dir);
    const char* what;
    int now = value;
    if (next >= 0) {                                                         // the next wagon, the word where it is
        *pick = (u8)(next + 1);
        at = next;
        what = dir > 0 ? "a detent up" : "a detent down";
    } else if (dir > 0) {                                                    // past the last wagon: out upwards
        now = LFO4_OVER_FX;
        what = "up out of the effect's range";
    } else {                                                                 // before the first: out downwards
        now = LFO4_UNDER_FX;
        what = "down out of the effect's range";
    }
    if (now != value) *word = word_of(now);
    *result = stock(self, index, 0);
    mark_dirty(host);
    lfofx_log_at(host, slot, what, now, at, dest);
    return 1;
}

static int lfo_dest_turn(void* self, int host, int index, int delta, int* result) {   // 1: the module took the turn (*result is stock's answer)
    if (*page_of(self) != PAGE_LFO || delta == 0) return 0;
    u32 slot = gui_slot(host);
    u32 kind = lfo_type_word(host, slot);
    if (!initialised[host][slot]) return 0;
    int midi = kind == (u32)LFO_TYPE_MIDI && index >= LFO_MIDI_DEST_KNOB && index < LFO_MIDI_DEST_KNOB + MIDI_DESTS;   // I4: a shifted knob on the midi LFO's page, one of its four destinations
    if (!midi && (index != LFO_DEST_KNOB || (kind != (u32)LFO_TYPE_VALUE && !lfo_type_four(kind)))) return 0;
    int first = next_fx_knob(host, slot, -1, 1), last = next_fx_knob(host, slot, WAGONS, -1);
    if (first < 0 || first == last) return 0;                                // I3: fewer than two wagons with an effect, nothing to choose: stock's knob
    if (midi) return lfo_dest_turn_four(self, host, slot, index, delta, result, first, last, index, &lfo_midi_pick[host][slot][index - LFO_MIDI_DEST_KNOB], index - LFO_MIDI_DEST_KNOB + 1);
    if (kind != (u32)LFO_TYPE_VALUE) return lfo_dest_turn_four(self, host, slot, index, delta, result, first, last, LFO_DEST_WORD, &lfo_fx_pick[host][slot], 0);
    EncoderFn stock = (EncoderFn)HOST[host].encoder;
    u32* word = &lfo_block_of(host, slot)[LFO_DEST_WORD];
    int value = int_of(*word);
    if (value < LFO_FX_LO) {
        *result = stock(self, index, delta);                                // stock's own step; one that lands in the effect's range starts at the first wagon
        if (int_of(*word) >= LFO_FX_LO) {
            lfo_fx_pick[host][slot] = (u8)(first + 1);
            *word = word_of(LFO_FX_PARK);
            *result = stock(self, index, 0);
            mark_dirty(host);
            lfofx_log(host, slot, "the knob entered the effect's range", LFO_FX_PARK);
        }
        return 1;
    }
    int dir = delta > 0 ? 1 : -1;
    int shared = value >= LFO_FX_SHARED;
    int next = next_fx_knob(host, slot, lfo_fx_knob_of(host, slot), dir);
    const char* what = 0;
    int now = value;
    if (next >= 0) {                                                         // the next wagon, the word where it is
        lfo_fx_pick[host][slot] = (u8)(next + 1);
        what = dir > 0 ? "a detent up" : "a detent down";
    } else if (dir > 0 && !shared) {                                         // past the last wagon: the shared form, at the first
        lfo_fx_pick[host][slot] = (u8)(first + 1);
        now = LFO_FX_PARK_SHARED;
        what = "up into the shared form";
    } else if (dir < 0 && shared) {                                          // before the first wagon: the form of its own phase, at the last
        lfo_fx_pick[host][slot] = (u8)(last + 1);
        now = LFO_FX_PARK;
        what = "down into the form of its own phase";
    } else if (dir < 0) {                                                    // below the first wagon: out of the effect's range, the choice kept
        now = LFO_UNDER_FX;
        what = "down out of the effect's range";
    }                                                                        // else the last wagon in the shared form: the end of the knob's travel
    if (now != value) *word = word_of(now);
    *result = stock(self, index, 0);
    if (what) {
        mark_dirty(host);
        lfofx_log(host, slot, what, now);
    }
    return 1;
}

// what the LFO pages are told (Build I's LFO page unit). R14: while the place does not ride "train" the LFO acts on the
// first wagon (the gates of lfofx_apply and op1f_lfofx_extra), so the pages are told the first wagon; the stored pick
// is kept untouched for the next ride.
extern "C" void op1f_lfofx_view(int host, Op1fLfoFxView* out) asm("op1f_lfofx_view");
extern "C" void op1f_lfofx_view(int host, Op1fLfoFxView* out) {
    u32 slot = gui_slot(host);
    int knob = place_rides(host, slot) ? lfo_fx_knob_of(host, slot) : mirror[host][slot];
    u32 kind = lfo_type_word(host, slot);
    int value = int_of(lfo_block_of(host, slot)[LFO_DEST_WORD]);
    out->known = initialised[host][slot];
    out->lfo_type = (int)kind;
    out->steps = kind == (u32)LFO_TYPE_VALUE || lfo_type_four(kind);
    out->to_fx = kind == (u32)LFO_TYPE_VALUE ? value >= LFO_FX_LO : (lfo_type_four(kind) && value >= LFO4_FX_LO && value < LFO4_FX_HI);
    out->knob = knob;
    out->type = wagons[host][slot][knob].type;
    out->mirror = mirror[host][slot];
}

// I4: what the midi LFO's page is told: for each of its four destinations, whether it is the effect and which wagon it turns
extern "C" void op1f_lfofx_midi_view(int host, Op1fLfoMidiView* out) asm("op1f_lfofx_midi_view");
extern "C" void op1f_lfofx_midi_view(int host, Op1fLfoMidiView* out) {
    u32 slot = gui_slot(host);
    out->known = initialised[host][slot];
    out->midi = lfo_type_word(host, slot) == (u32)LFO_TYPE_MIDI;
    int rides = place_rides(host, slot);                                     // R14: not riding, every destination the first wagon's (as op1f_lfofx_view)
    for (int i = 0; i < MIDI_DESTS; ++i) {
        int value = int_of(lfo_block_of(host, slot)[LFO_MIDI_DEST_WORD + i]);
        int knob = rides ? lfo_midi_knob_of(host, slot, i) : mirror[host][slot];
        out->to_fx[i] = value >= LFO4_FX_LO && value < LFO4_FX_HI;
        out->knob[i] = knob;
        out->type[i] = wagons[host][slot][knob].type;
    }
}

extern "C" int op1f_fx_encoder(void* self, int index, int delta) asm("op1f_fx_encoder");
// R14: the controllers' encoder word, for the turns the base's input left to stock (the train page's own come through
// op1f_train_turn): on the LFO page the destination knob walks the effect wagons only while the place on the screen
// rides "train"; every other turn is stock's.
extern "C" int op1f_fx_encoder(void* self, int index, int delta) {
    int host = host_of(self);
    int result = 0;
    if (place_rides(host, gui_slot(host)) && lfo_dest_turn(self, host, index, delta, &result)) return result;   // I2, I3: the LFO's destination knob, a wagon a detent inside the effect's range
    return stock_knob_turn(self, host, index, delta);                                                         // H6h: logged on the effect and LFO pages (off since H6j)
}

// ---------------------------------------------------------------- the persistence: the module's own file

// user/op1f.json: an array of the tag 91 and the sixteen slots' chains (synth 0 to 7, then drum 0 to 7), each the 47
// integers of the chain (the tag, the mirror's knob, the document's effect type the chain was last reconciled with,
// the order as four knobs by position, then per knob the type, the coupling and the eight parameters as stock's
// integers, word >>> 16) or an empty array for a slot the module has not seen. Written by stock's json file writer
// as the slots files are, read by its reader at the first reconcile (the boot's first apply, 1.8 s in, on the main
// thread); a missing or damaged file leaves every slot to its document. A value's kind is its byte at +4: 0 null (what
// 0x8c085ba8(json, 0) makes; the first push turns it into an array), 1 an integer as 0x8c083b1c makes it, 2 unsigned,
// 3 float, 6 an array, 7 an object. An array is checked for by its kind before the size read (the size getter's
// table walks the list for 6 and returns the kind byte for the rest); the integer getter 0x8c084840 needs no check:
// it converts the kinds 1, 2 and 3 (a parsed number is a 2 or a 3, so H1e's check against a fresh integer's kind lost
// every number read) and returns 0 with a log line for anything else. H1f had taken the array's kind from a fresh
// value, which is the null 0, so no parsed array passed and the file loaded nothing.

static int is_array(void* j) { return j != 0 && *((u8*)j + 4) == JSON_ARRAY_KIND; }
static void name_trim(char* name);                                           // H5, below
static char lower_of(char c);                                                // H5b, below
static void fx_scan(int force);                                              // H6, below
static int free_slot();
static int fx_write_slot(int slot);
static void rebuild_order();
static void default_name(char* out, int slot);
static int int_at(void* array, int i) { void* e = STOCK_JSON_AT(array, i); return e ? STOCK_JSON_INT(e) : 0; }

// R5: the names of a slot's wagons' packs' effects, "" for a wagon of the unit's own; with `riding`, the first wagon's
// ride counts as its name (a preset keeps it; the chain file and an instrument leave it to the base, which keeps the
// ride with the sound). 1 when any wagon has one.
static int chain_names(int host, u32 slot, char (*names)[RACK_NAME], int riding) {
    int any = 0;
    for (int k = 0; k < WAGONS; ++k) {
        const char* name = rack_name[host][slot][k];
        if (!name[0] && riding && k == mirror[host][slot]) name = rack_name_of(mirror_entry(host, slot));
        name_put(names[k], wagons[host][slot][k].type >= 0 ? name : "");
        if (names[k][0]) any = 1;
    }
    return any;
}

// R5: the names' tail of a slot's integers: the tag, then four integers a knob, four characters each, the first in
// the low byte (RACK_INTS integers).
static void names_tail(char (*names)[RACK_NAME], int* out) {
    out[0] = RACK_TAG;
    for (int k = 0; k < WAGONS; ++k)
        for (int w = 0; w < RACK_WORDS; ++w) {
            u32 v = 0;
            for (int b = 0; b < 4; ++b) v |= (u32)(u8)names[k][4 * w + b] << (8 * b);
            out[1 + RACK_WORDS * k + w] = (int)v;
        }
}

// R5: the names read back from a tail (`in` behind its tag); a name with a character that is not a plain one is none.
static void names_from_tail(const int* in, char (*names)[RACK_NAME]) {
    for (int k = 0; k < WAGONS; ++k) {
        for (int w = 0; w < RACK_WORDS; ++w)
            for (int b = 0; b < 4; ++b) names[k][4 * w + b] = (char)(((u32)in[RACK_WORDS * k + w] >> (8 * b)) & 0xffu);
        names[k][RACK_NAME - 1] = 0;
        for (int c = 0; names[k][c]; ++c) if (names[k][c] < 32 || names[k][c] > 126) { names[k][0] = 0; break; }
    }
}

// R5: the most integers a slot holds (the chain's, the two pairs, the LFO unit's block, the names' tail), and a place to
// read them into, the file's reader's alone (one thread at a time: the start's, or the display thread).
enum { SLOT_INTS_MOST = CHAIN_INTS_V2 + 4 + 1 + LFO4_INTS + RACK_INTS };
static int slot_ints[SLOT_INTS_MOST];

// R5: where the names' tail stands in a slot's integers, behind every block an older reader reads (the LFOs' wagons'
// pairs, the LFO unit's integers): the index of its tag, or -1 when there is none.
static int names_at(const int* v, int n, int host) {
    int at = CHAIN_INTS_V2;
    if (n >= at + 2 && v[at] == LFOFX_TAG) at += 2; else return -1;
    if (n >= at + 2 && v[at] == LFOMIDI_TAG) at += 2; else return -1;
    if (host == 0 && n >= at + 1 + LFO4_MIN && v[at] == LFO4_TAG) at += 1 + LFO4_INTS;
    return (n >= at + RACK_INTS && v[at] == RACK_TAG) ? at : -1;
}

static void chain_values(int host, u32 slot, int* values, int riding) {     // CHAIN_INTS_V2 integers (R5: a pack's wagon as RACK_TYPE + the effect it stands on)
    char names[WAGONS][RACK_NAME];
    chain_names(host, slot, names, riding);
    int n = 0;
    values[n++] = CHAIN_TAG_V2;
    values[n++] = mirror[host][slot];
    values[n++] = doc_type[host][slot];
    for (int p = 0; p < WAGONS; ++p) values[n++] = knob_at(host, slot, p);
    for (int k = 0; k < WAGONS; ++k) {
        const Wagon* w = &wagons[host][slot][k];
        values[n++] = names[k][0] ? RACK_TYPE + w->type : w->type;
        values[n++] = w->coupled;
        for (int i = 0; i < PARAMS; ++i) values[n++] = int_of(w->params[i]);
    }
    int place = host * SLOTS + (int)slot;                                      // R14: the flag "kept, not riding": not riding, no start due, none given up
    int flag = !place_rides(host, slot) && !rd.due[place] && !rd.no_flag[place];
    values[n++] = bypass_mask[host][slot] + (flag ? (int)FLAG_NOT_RIDING : 0);  // H3: the 48th integer (read & 0xf for the parked mask)
}

// H6j: the wagon H6 to H6i made of a file's "none" (json_fx_type): an uncoupled delay with eight zero parameters. No knob
// reaches that state (the delay's first parameter starts at 1024, its second at 3276), so it is read as empty, here
// for the chains the unit saved while it ran those images and in the file reader for the presets saved from them.
static int ghost_delay(int type, int coupled, const int* params) {
    if (type != TYPE_DELAY || coupled) return 0;
    for (int i = 0; i < PARAMS; ++i) if (params[i]) return 0;
    return 1;
}

// H6j: a stored knob parameter held to the knob's own range, the type's descriptor row as it stands in memory: the
// minimum a signed half-word, the maximum the next (the H6i read-out: the rows are filled at boot by a static
// initialiser, and a knob turn clamps to them; a delay time above 11264 crashed the delay in H1b). The four knob
// parameters only: the other four words are not a knob's. An entry not filled (the image has 0, 0 there) holds nothing.
static int clamp_param(int host, u32 slot, int knob, int type, int i, int value) {
    if (i >= 4) return value;
    u32 e = FX_DESCRIPTORS + 48u * (u32)type + 6u * (u32)i;
    int lo = (int)(short)RD16(e), hi = (int)(short)RD16(e + 2u);
    if (hi <= lo || (value >= lo && value <= hi)) return value;
    int held = value < lo ? lo : hi;
    ++st.clamps;
    if (st.value_lines < 24) {
        ++st.value_lines;
        STOCK_LOG(3, LOG_FILE, 28, "chain values: %s s%u knob %d (%s) parameter %d: %d held to %d (the knob's range %d..%d)", host ? "drum" : "synth", slot, knob, fx_name(type), i, value, held, lo, hi);
    }
    return held;
}

static void chain_from_values(int host, u32 slot, const int* values, char (*names)[RACK_NAME]) {   // CHAIN_INTS_V2 integers (a tag-91 chain padded with 0); R5: the names, or 0
    int n = 1;
    mirror[host][slot] = (u8)(values[n++] & 3);
    doc_type[host][slot] = values[n++];
    wish_from_pick[host][slot] = 0;
    switch_owed[host][slot] = 0;                               // 17
    document_owed[host][slot] = 0;
    int knobs[WAGONS];
    for (int p = 0; p < WAGONS; ++p) knobs[p] = values[n++] & 3;
    for (int k = 0; k < WAGONS; ++k) {
        Wagon* w = &wagons[host][slot][k];
        int type = values[n++];
        int coupled = values[n++] ? 1 : 0;
        const int* stored = values + n;
        n += PARAMS;
        rack_name[host][slot][k][0] = 0;                                    // R5: a pack's effect, by the type it stands on and its name
        if (type >= RACK_TYPE && type < RACK_TYPE + TYPES && names && names[k][0] && rideable(type - RACK_TYPE)) {
            type -= RACK_TYPE;
            name_put(rack_name[host][slot][k], names[k]);
        }
        type = (type >= 0 && type < TYPES) ? type : NONE;
        if (!rack_name[host][slot][k][0] && ghost_delay(type, coupled, stored)) {   // H6j (R5: never a pack's effect)
            type = NONE;
            coupled = 1;
            ++st.ghosts;
            if (st.value_lines < 24) {
                ++st.value_lines;
                STOCK_LOG(3, LOG_FILE, 29, "chain values: %s s%u knob %d: an uncoupled delay with eight zero parameters (a file's none as H6 to H6i read it) taken as empty", host ? "drum" : "synth", slot, k);
            }
        }
        w->type = type;
        w->coupled = (u8)coupled;
        for (int i = 0; i < PARAMS; ++i) w->params[i] = type >= 0 ? word_of(clamp_param(host, slot, k, type, i, stored[i])) : 0;
    }
    int seen = 0;
    for (int p = 0; p < WAGONS; ++p) seen |= 1 << knobs[p];
    if (seen != 0xf) for (int p = 0; p < WAGONS; ++p) knobs[p] = p;
    set_order(host, slot, knobs);
    u32 parked = (u32)values[CHAIN_INTS] & 0xfu;                            // H3's bypass parked these: back on the line (H5 has no bypass key)
    if (parked) for (int k = 0; k < WAGONS; ++k) if ((parked & (1u << k)) && wagons[host][slot][k].type >= 0) wagons[host][slot][k].coupled = 1;
    bypass_mask[host][slot] = 0;
    // R14: a store with another wagon that carries an effect and is coupled (its parked bit too,
    // coupled just above), no flag and no pack's effect on the first wagon makes a ride start due RIDE_DELAY_FRAMES
    // frames on (a load inside the chain's frame counts from this frame, one outside it from the next).
    int place = host * SLOTS + (int)slot;
    rd.due[place] = 0;
    rd.no_flag[place] = 0;
    int m = mirror[host][slot], extra = 0;
    for (int k = 0; k < WAGONS; ++k) if (k != m && wagons[host][slot][k].type >= 0 && wagons[host][slot][k].coupled) extra = 1;
    int flagged = ((u32)values[CHAIN_INTS] & (u32)FLAG_NOT_RIDING) != 0;
    if (extra && !flagged && !rack_name[host][slot][m][0]) {
        rd.due_at[place] = rd.first_due[place] = st.frames + (u32)RIDE_DELAY_FRAMES + (rd.in_frame ? 0u : 1u);
        rd.due[place] = 1;
    }
}

// R14: a place's chain made anew from its document (a sound with none of this unit's integers): nothing due, the flag
// free to follow its ride again.
static void ride_forget(int host, u32 slot) {
    int place = host * SLOTS + (int)slot;
    rd.due[place] = 0;
    rd.no_flag[place] = 0;
}

static void load_file() {
    file_loaded = 1;
    Json root;
    STOCK_JSON_CTOR(&root, 0);
    int read = STOCK_JSON_READ_FILE(FILE_PATH, &root);
    int tag = int_at(&root, 0);
    int shape = read && is_array(&root) && STOCK_JSON_SIZE(&root) >= 1 + HOSTS * SLOTS && (tag == CHAIN_TAG || tag == ROOT_TAG_V3);   // H5: the root tag 93 adds the presets
    if (shape) {
        for (int h = 0; h < HOSTS; ++h) {
            for (u32 sl = 0; sl < SLOTS; ++sl) {
                void* e = STOCK_JSON_AT(&root, 1 + h * SLOTS + (int)sl);
                if (!is_array(e)) continue;
                int tag = int_at(e, 0);
                int count = tag == CHAIN_TAG_V2 ? CHAIN_INTS_V2 : (tag == CHAIN_TAG ? CHAIN_INTS : 0);   // the H3 chain, or an H1e to H2 one
                if (count == 0 || STOCK_JSON_SIZE(e) < count) continue;
                int values[CHAIN_INTS_V2];
                for (int i = 0; i < CHAIN_INTS_V2; ++i) values[i] = i < count ? int_at(e, i) : 0;
                char names[WAGONS][RACK_NAME];                                // R5: the names' tail, behind every other block
                int with_names = 0;
                if (count == CHAIN_INTS_V2) {
                    int size = STOCK_JSON_SIZE(e);
                    int seen = size < (int)SLOT_INTS_MOST ? size : (int)SLOT_INTS_MOST;
                    for (int i = 0; i < seen; ++i) slot_ints[i] = int_at(e, i);
                    int at = names_at(slot_ints, seen, h);
                    if (at >= 0) {
                        names_from_tail(slot_ints + at + 1, names);
                        with_names = 1;
                    }
                }
                chain_from_values(h, sl, values, with_names ? names : 0);
                if (count == CHAIN_INTS_V2 && STOCK_JSON_SIZE(e) >= CHAIN_INTS_V2 + 2 && int_at(e, CHAIN_INTS_V2) == LFOFX_TAG) {   // I2: the LFO's effect wagon
                    int pick = int_at(e, CHAIN_INTS_V2 + 1);
                    lfo_fx_pick[h][sl] = (u8)(pick >= 0 && pick <= WAGONS ? pick : 0);
                    if (STOCK_JSON_SIZE(e) >= CHAIN_INTS_V2 + 4 && int_at(e, CHAIN_INTS_V2 + 2) == LFOMIDI_TAG) {   // I4: the midi LFO's four, four bits each
                        int packed = int_at(e, CHAIN_INTS_V2 + 3);
                        for (int i = 0; i < MIDI_DESTS; ++i) {
                            int one = (packed >> (4 * i)) & 15;
                            lfo_midi_pick[h][sl][i] = (u8)(one <= WAGONS ? one : 0);
                        }
                        if (h == 0 && STOCK_JSON_SIZE(e) >= LFO4_AT + 1 + LFO4_MIN && int_at(e, LFO4_AT) == LFO4_TAG) {   // I6: the three other LFOs and the lamp's place
                            int more[LFO4_INTS];
                            int have = STOCK_JSON_SIZE(e) - (LFO4_AT + 1);
                            for (int i = 0; i < LFO4_INTS; ++i) more[i] = i < have ? int_at(e, LFO4_AT + 1 + i) : -1;   // I7: what a file of I6's does not have reads as -1
                            if (&op1f_lfo4_store_load) op1f_lfo4_store_load((int)sl, more);
                            else { lfo4_keep(sl, more); ++st.lfo4_carried; }           // J5f: the LFO unit is away: carried to the next save
                        }
                    }
                }
                known[h][sl] = 1;
                ++st.loads;
            }
        }
    }
    fx_scan(1);                                                              // H6: the presets' folder and files (made when missing)
    if (&op1f_lfopresets_scan) op1f_lfopresets_scan();                       // I8: the LFO presets' folder and files, at the same moment
    int migrated = 0, failed = 0;
    if (shape && tag == ROOT_TAG_V3 && STOCK_JSON_SIZE(&root) >= 2 + HOSTS * SLOTS) {    // the H5 presets of the seventeenth element, moved into the first free slots
        void* pa = STOCK_JSON_AT(&root, 1 + HOSTS * SLOTS);
        if (is_array(pa)) {
            int n = STOCK_JSON_SIZE(pa);
            for (int j = 0; j < n; ++j) {
                void* e = STOCK_JSON_AT(pa, j);
                if (!is_array(e) || STOCK_JSON_SIZE(e) < PRESET_INTS) continue;
                int s = free_slot();
                if (s < 0) break;
                for (int c = 0; c < NAME_CHARS; ++c) {
                    int code = int_at(e, c);
                    presets.name[s][c] = (code >= 32 && code < 127) ? lower_of((char)code) : ' ';
                }
                presets.name[s][NAME_CHARS] = 0;
                name_trim(presets.name[s]);
                if (!presets.name[s][0]) default_name(presets.name[s], s);
                for (int v = 0; v < CHAIN_INTS_V2; ++v) presets.values[s][v] = int_at(e, NAME_CHARS + v);
                presets.used[s] = 1;
                if (fx_write_slot(s)) ++migrated;                                   // H6b: counted only when the file landed
                else { presets.used[s] = 0; ++failed; }
            }
            if (migrated) { rebuild_order(); ls.migrated += (u32)migrated; }
            if (migrated && !failed) { st.dirty[0] = 1; st.dirty_since[0] = (int)st.frames; }   // the element is written empty at the next save, once every preset is a file (H6b: H6 dropped it while the writes failed)
            else if (failed) keep_h5_presets = 1;                                 // the element is written back as it was
        }
    }
    if (keep_h5_presets && shape && tag == ROOT_TAG_V3 && STOCK_JSON_SIZE(&root) >= 2 + HOSTS * SLOTS) {   // H6b: the element kept whole while a file could not be written
        STOCK_JSON_CTOR(&h5_presets, 0);
        STOCK_JSON_ASSIGN(&h5_presets, STOCK_JSON_AT(&root, 1 + HOSTS * SLOTS));
    }
    STOCK_LOG(3, LOG_FILE, 8, "chain file: %s, kind %u, %s, %u chains, tag %d, %d presets migrated into the files, %d not (kept in the file), %d slots used", read ? "read" : "not read", *((u8*)&root + 4), shape ? "shape ok" : "no shape", st.loads, tag, migrated, failed, presets.count);
    if (st.lfo4_carried) STOCK_LOG(2, LOG_FILE, 34, "chain file: the LFO pack is away: the LFOs of %u sound(s) are carried as the file has them and written back untouched", st.lfo4_carried);
    STOCK_JSON_DTOR(&root);
}

static void save_file() {
    Json root, chain, item;
    STOCK_JSON_CTOR(&root, 0);
    STOCK_JSON_FROM_INT(&item, ROOT_TAG_V3);                                  // H5: the root carries the presets after the chains
    STOCK_JSON_PUSH(&root, &item);
    STOCK_JSON_DTOR(&item);
    for (int h = 0; h < HOSTS; ++h) {
        for (u32 sl = 0; sl < SLOTS; ++sl) {
            STOCK_JSON_CTOR(&chain, 0);
            if (known[h][sl]) {
                int values[CHAIN_INTS_V2];
                chain_values(h, sl, values, 0);
                int tail[4] = { (int)LFOFX_TAG, (int)lfo_fx_pick[h][sl], (int)LFOMIDI_TAG, 0 };
                for (int i = 0; i < MIDI_DESTS; ++i) tail[3] |= (int)lfo_midi_pick[h][sl][i] << (4 * i);
                int more[LFO4_INTS];
                int have = h != 0 ? 0 : (&op1f_lfo4_store_save ? op1f_lfo4_store_save((int)sl, more) : lfo4_carried(sl, more));   // I6: nothing for a sound that has only stock's LFO, on the first wagon; J5f: what was read, while the LFO unit is away
                char names[WAGONS][RACK_NAME];
                int named = chain_names(h, sl, names, 0);                     // R5: a pack's wagon: its names' tail, behind every block an older image reads
                int extra = (tail[3] || have || named) ? 4 : 2;               // I4: the midi LFO's four choices ride after I2's pair only when one is made: a sound that never chose keeps I2's fifty integers; I6: both pairs stand before the tag 98
                for (int i = 0; i < CHAIN_INTS_V2 + extra; ++i) {              // I2: after the 48 integers the tag 96 and the LFO's effect wagon (0 the lamp's, 1 to 4 a knob's); an older image reads the first 48
                    STOCK_JSON_FROM_INT(&item, i < CHAIN_INTS_V2 ? values[i] : tail[i - CHAIN_INTS_V2]);
                    STOCK_JSON_PUSH(&chain, &item);
                    STOCK_JSON_DTOR(&item);
                }
                for (int i = 0; have && i < 1 + LFO4_INTS; ++i) {              // I6: the tag 98 and the LFO unit's integers; an older image reads what stands before them
                    STOCK_JSON_FROM_INT(&item, i == 0 ? (int)LFO4_TAG : more[i - 1]);
                    STOCK_JSON_PUSH(&chain, &item);
                    STOCK_JSON_DTOR(&item);
                }
                if (named) {                                                   // R5: the tag 99 and the names
                    int ints[RACK_INTS];
                    names_tail(names, ints);
                    for (int i = 0; i < RACK_INTS; ++i) {
                        STOCK_JSON_FROM_INT(&item, ints[i]);
                        STOCK_JSON_PUSH(&chain, &item);
                        STOCK_JSON_DTOR(&item);
                    }
                }
            }
            STOCK_JSON_PUSH(&root, &chain);
            STOCK_JSON_DTOR(&chain);
        }
    }
    Json list;                                                               // H6: the seventeenth element stays, empty: the presets live in user/fx/ (an H5 reader finds no preset here); H6b: the H5 presets written back while their files could not be made
    STOCK_JSON_CTOR(&list, 0);
    if (keep_h5_presets) STOCK_JSON_ASSIGN(&list, &h5_presets);
    STOCK_JSON_PUSH(&root, &list);
    STOCK_JSON_DTOR(&list);
    u32 path[12];
    for (int i = 0; i < 12; ++i) path[i] = 0;
    STOCK_STRING_CTOR(path, FILE_PATH);
    STOCK_JSON_WRITE_FILE(path, &root);
    STOCK_JSON_DTOR(&root);
    ++st.saves;
    if (&op1f_lfo_file_chain_saved) op1f_lfo_file_chain_saved();             // R4c: the file is written: lfo's own file follows it, in this frame
}

static void save_pending() {
    if (!st.dirty[0] && !st.dirty[1]) return;
    st.dirty[0] = st.dirty[1] = 0;
    save_file();
}

// ---------------------------------------------------------------- the presets (H5, section 21; the files H6, section 22)

// The store is thirty-two slots, one file each in the module's folder user/fx/ (01.json .. 32.json), shared by both
// hosts (a chain fits either). A used slot's file is a JSON object: op1f_fx_preset (the format, 1), name (the font's
// lowercase, twelve characters at most), order (the knob at each position), wagons (four objects: fx, stock's name
// of the effect or "none", on, params, the eight integers as stock keeps them); an empty slot's file is {}. The
// files are written by stock's json file writer and read by its reader; the folder is scanned at the first file
// load and at every list open (a file whose time or size changed since the last read is read again, a missing
// file is made), so a file the host wrote through the disk mode (Build H's disk unit shows the folder as fx/) is a
// preset at the next list open. The list is a screen of the GUI's own (Build H's presets unit, the id 37 since H6d) dressed
// from op1f_presets_view; the keys below route to it while it is open.
//   * R14: T3 held HOLD_MS on the train page opens the list (fx train 17: shift + T3's tap, and its hold of 700 ms
//     saved); shift + T3 is the unit's list of effects, which closes the list first.
//   * On the list: the blue knob scrolls, its tap (or T3) loads the highlighted preset into the current slot (the
//     types, the order, the couplings and the parameters; the mirror stays on its knob and takes its new effect,
//     the other instances rebuild) and closes the list; the ochre knob's tap opens the rename editor; the grey
//     knob's tap saves the current chain into the lowest free slot as chain NN (the slot's number), saved! for a
//     second; the red knob held HOLD_MS empties the highlighted preset's slot after a deleted flash of a second (its
//     file becomes {}); T1, T2 and T4 close the list and are stock's.
//   * The editor: the edited preset is scrolled to the window's first row and its name shows in twelve cells; the
//     blue knob moves the caret, the ochre knob steps the letter (space, a to z, 0 to 9, dash, dot), the grey knob's
//     tap or T3 keeps the name (all spaces: chain NN), the red knob's tap cancels.
//   * The lamp marks the slot the chain was loaded from or saved as until the chain changes (any mark_dirty of the
//     chain clears it; the presets' own writes keep it).
// Every save, load, rename, delete, scan and migration logs a line (presets:); the counts go on the presets: line.

static int alphabet_index(char c) {
    for (int i = 0; i < ALPHABET_COUNT; ++i) if (ALPHABET[i] == c) return i;
    return 0;
}

static void name_copy(char* dst, const char* src) {                          // at most NAME_CHARS characters, NUL-terminated
    int i = 0;
    for (; i < NAME_CHARS && src[i]; ++i) dst[i] = src[i];
    dst[i] = 0;
}

static void name_trim(char* name) {                                          // the trailing spaces off
    int n = 0;
    while (name[n]) ++n;
    while (n > 0 && name[n - 1] == ' ') name[--n] = 0;
}

static char lower_of(char c) { return (c >= 'A' && c <= 'Z') ? (char)(c + 32) : c; }   // H5b: the font's letters are the lowercase codes

static void default_name(char* out, int slot) {                             // chain NN, the slot's number (the file's)
    int n = slot + 1, tens = 0;
    while (n >= 10) { n -= 10; ++tens; }
    char* p = put_text(out, "chain ");
    *p++ = (char)('0' + tens);
    *p++ = (char)('0' + n);
    *p = 0;
}

static void slot_name(int slot, char* out) {                               // NN.json, the slot's number (R2)
    int n = slot + 1, tens = 0;
    while (n >= 10) { n -= 10; ++tens; }
    out[0] = (char)('0' + tens);
    out[1] = (char)('0' + n);
    *put_text(out + 2, ".json") = 0;
}

// R2: user/pf/fx/NN.json, asked of the base: the path's characters, or 0 and an empty path where the base has no
// folder for the pack.
static int fx_path(int slot, char* out) {
    char name[8];
    slot_name(slot, name);
    int got = op1f_path(&op1f_card_fx_train, PRESETS_FOLDER, name, out, PATH_ROOM);
    if (got <= 0) out[0] = 0;
    return got > 0 ? got : 0;
}

static int free_slot() {                                                    // R2: a slot whose file is still to be taken off is not free
    for (int s = 0; s < PRESETS; ++s) if (!presets.used[s] && !removal_owed(s)) return s;
    return -1;
}

static void rebuild_order() {                                               // the used slots in slot order
    int n = 0;
    for (int s = 0; s < PRESETS; ++s) if (presets.used[s]) presets.order[n++] = s;
    presets.count = n;
}

static int row_of_slot(int slot) {                                          // the list row of a slot, or -1
    for (int i = 0; i < presets.count; ++i) if (presets.order[i] == slot) return i;
    return -1;
}

// a json string's characters: a stock string object has its data inline at +4 up to fifteen characters, else a
// pointer there; the length at +0x14 (0x8c03e434)
static void json_text(void* entry, char* out, int max) {
    u32 str[8];
    for (int i = 0; i < 8; ++i) str[i] = 0;
    STOCK_JSON_TO_STRING(str, entry);
    u32 len = str[5], cap = str[6];
    const char* data = cap >= 16 ? (const char*)str[1] : (const char*)&str[1];
    int n = 0;
    if (data) for (; n < max && (u32)n < len && data[n]; ++n) out[n] = data[n];
    out[n] = 0;
    if (cap >= 16 && str[1]) STOCK_FREE((void*)str[1]);                      // a long text lives on the heap
}

static int text_equal(const char* a, const char* b) {
    while (*a && *a == *b) { ++a; ++b; }
    return *a == *b;
}

// "delay" .. "fazer" by stock's own names; "none" and anything else none. H6 to H6i asked stock's lookup 0x8fca7b08,
// which answers a name it does not know with type 0, the delay, and an error line (`unknown fx name: 'none'`): every
// empty wagon of a file came back as an uncoupled delay with zero parameters (the H6i read-out: the fifteen library
// files' twenty-one empty wagons). The name is compared here, and stock's lookup is left to stock's own documents.
static int json_fx_type(void* entry) {
    if (!entry || *((u8*)entry + 4) != JSON_STRING_KIND) return NONE;
    char name[16];
    json_text(entry, name, 15);
    for (int type = 0; type < TYPES; ++type) if (text_equal(name, fx_name(type))) return type;
    return NONE;
}

// R5: a wagon's `fx` that names no effect of the unit's: a pack's effect, when its name is a plain one of one to fifteen
// characters and the effect it stands on is known (the wagon's `rides`, else the registered effect's own): its type, with
// the name in `name`; else none (an older train reads every such wagon as empty).
static int json_fx_rack(void* wagon, char* name) {
    name[0] = 0;
    void* entry = STOCK_JSON_GET(wagon, KEY_FX);
    if (!entry || *((u8*)entry + 4) != JSON_STRING_KIND) return NONE;
    char text[RACK_NAME + 1];
    json_text(entry, text, RACK_NAME);
    int n = 0;
    for (; text[n]; ++n) if (text[n] < 32 || text[n] > 126) return NONE;
    if (n == 0 || n > RACK_NAME - 1 || text_equal(text, "none")) return NONE;
    int type = NONE;
    if (STOCK_JSON_HAS(wagon, KEY_RIDES)) type = json_fx_type(STOCK_JSON_GET(wagon, KEY_RIDES));
    if (type == NONE) type = rack_rides(rack_find(text));
    if (!rideable(type)) return NONE;
    name_put(name, text);
    return type;
}

static int json_truth(void* entry) {                                        // true/false, or an integer
    if (!entry) return 0;
    if (*((u8*)entry + 4) == JSON_BOOL_KIND) return STOCK_JSON_BOOL(entry) ? 1 : 0;
    return STOCK_JSON_INT(entry) != 0;
}

static void set_int(void* obj, const char* key, int value) {
    Json item;
    STOCK_JSON_FROM_INT(&item, value);
    STOCK_JSON_ASSIGN(STOCK_JSON_LOOKUP(obj, key), &item);
    STOCK_JSON_DTOR(&item);
}

static void set_text(void* obj, const char* key, const char* text) {
    Json item;
    STOCK_JSON_CTOR(&item, 0);
    STOCK_JSON_FROM_CSTR(&item, text);
    STOCK_JSON_ASSIGN(STOCK_JSON_LOOKUP(obj, key), &item);
    STOCK_JSON_DTOR(&item);
}

static void set_bool(void* obj, const char* key, int value) {
    Json item;
    STOCK_JSON_CTOR(&item, 0);
    STOCK_JSON_FROM_BOOL(&item, value ? 1 : 0);
    STOCK_JSON_ASSIGN(STOCK_JSON_LOOKUP(obj, key), &item);
    STOCK_JSON_DTOR(&item);
}

static void set_ints(void* obj, const char* key, const int* values, int count) {
    Json arr, item;
    STOCK_JSON_CTOR(&arr, 0);
    for (int i = 0; i < count; ++i) {
        STOCK_JSON_FROM_INT(&item, values[i]);
        STOCK_JSON_PUSH(&arr, &item);
        STOCK_JSON_DTOR(&item);
    }
    STOCK_JSON_ASSIGN(STOCK_JSON_LOOKUP(obj, key), &arr);
    STOCK_JSON_DTOR(&arr);
}

// The slot's file: {} for an empty slot, else the object above. The time and size are noted so the next scan does
// not read the module's own write back. 1 when the file landed (H6b: the writer's result and the file's size checked).
static int fx_write_slot(int slot) {
    if (!op1f_r2_move_done() || !presets.scanned) {                            // R2: the store is shut until the presets are copied and read once since: nothing is written, as when a write fails
        ++r2.shut_writes;
        return 0;
    }
    Json root;
    STOCK_JSON_CTOR(&root, JSON_OBJECT_KIND);
    if (presets.used[slot]) {
        const int* v = presets.values[slot];
        set_int(&root, KEY_FORMAT, PRESET_FORMAT);
        set_text(&root, KEY_NAME, presets.name[slot]);
        int order[WAGONS];
        for (int p = 0; p < WAGONS; ++p) order[p] = v[3 + p] & 3;
        set_ints(&root, KEY_ORDER, order, WAGONS);
        Json wagons, w;
        STOCK_JSON_CTOR(&wagons, 0);
        for (int k = 0; k < WAGONS; ++k) {
            const int* wv = v + 3 + WAGONS + k * (2 + PARAMS);
            STOCK_JSON_CTOR(&w, JSON_OBJECT_KIND);
            const char* pack = (wv[0] >= RACK_TYPE && wv[0] < RACK_TYPE + TYPES) ? presets.rack[slot][k] : "";   // R5: a pack's effect's name, never one of the unit's eleven
            if (pack[0]) set_text(&w, KEY_FX, pack);
            else set_text(&w, KEY_FX, wv[0] >= 0 && wv[0] < TYPES ? fx_name(wv[0]) : "none");
            set_bool(&w, KEY_ON, wv[1]);
            set_ints(&w, KEY_PARAMS, wv + 2, PARAMS);
            if (pack[0]) set_text(&w, KEY_RIDES, fx_name(wv[0] - RACK_TYPE));
            STOCK_JSON_PUSH(&wagons, &w);
            STOCK_JSON_DTOR(&w);
        }
        STOCK_JSON_ASSIGN(STOCK_JSON_LOOKUP(&root, KEY_WAGONS), &wagons);
        STOCK_JSON_DTOR(&wagons);
    }
    char path[PATH_ROOM];
    int written = 0;
    if (fx_path(slot, path)) {
        u32 str[12];
        for (int i = 0; i < 12; ++i) str[i] = 0;
        STOCK_STRING_CTOR(str, path);
        written = STOCK_JSON_WRITE_FILE(str, &root);                             // the writer writes tmp/NN.json and renames it over the slot's file; 0 when that fails (H6b)
        if (str[6] >= 16 && str[1]) STOCK_FREE((void*)str[1]);                  // R2: the path's text lives on the heap; the writer keeps nothing of it
    }
    STOCK_JSON_DTOR(&root);
    presets.mtime[slot] = path[0] ? STOCK_FILE_MTIME(path) : 0;
    presets.size[slot] = path[0] ? STOCK_FILE_SIZE(path) : -1;
    ++ls.writes;
    if (!written || presets.size[slot] < 0) {
        ++ls.write_failures;
        if (ls.write_failures <= 4) STOCK_LOG(1, LOG_FILE, 20, "presets: the write of %s failed (writer %d, size %d): the folder is missing or the disk is full", path, written, presets.size[slot]);
        return 0;
    }
    return 1;
}

// The slot's file read: a preset when it carries four wagons, else an empty slot (a host's file with an effect
// name stock does not know gets an empty wagon there; a missing name gets chain NN).
static void fx_read_slot(int slot) {
    char path[PATH_ROOM];
    if (!fx_path(slot, path)) { presets.used[slot] = 0; return; }             // R2: the base has no folder for the pack
    Json root;
    STOCK_JSON_CTOR(&root, 0);
    int read = STOCK_JSON_READ_FILE(path, &root);
    presets.used[slot] = 0;
    if (read && *((u8*)&root + 4) == JSON_OBJECT_KIND && STOCK_JSON_HAS(&root, KEY_WAGONS)) {
        void* wagons = STOCK_JSON_GET(&root, KEY_WAGONS);
        if (is_array(wagons) && STOCK_JSON_SIZE(wagons) >= WAGONS) {
            int* v = presets.values[slot];
            for (int i = 0; i < CHAIN_INTS_V2; ++i) v[i] = 0;
            v[0] = CHAIN_TAG_V2;
            for (int p = 0; p < WAGONS; ++p) v[3 + p] = p;
            if (STOCK_JSON_HAS(&root, KEY_ORDER)) {
                void* o = STOCK_JSON_GET(&root, KEY_ORDER);
                if (is_array(o) && STOCK_JSON_SIZE(o) >= WAGONS) for (int p = 0; p < WAGONS; ++p) v[3 + p] = int_at(o, p) & 3;
            }
            int seen = 0;
            for (int p = 0; p < WAGONS; ++p) seen |= 1 << v[3 + p];
            if (seen != 0xf) for (int p = 0; p < WAGONS; ++p) v[3 + p] = p;
            for (int k = 0; k < WAGONS; ++k) {
                int* wv = v + 3 + WAGONS + k * (2 + PARAMS);
                void* w = STOCK_JSON_AT(wagons, k);
                int type = NONE, on = 1;
                presets.rack[slot][k][0] = 0;
                if (w && *((u8*)w + 4) == JSON_OBJECT_KIND) {
                    if (STOCK_JSON_HAS(w, KEY_FX)) type = json_fx_type(STOCK_JSON_GET(w, KEY_FX));
                    if (type == NONE && STOCK_JSON_HAS(w, KEY_FX)) type = json_fx_rack(w, presets.rack[slot][k]);   // R5: a pack's effect
                    if (STOCK_JSON_HAS(w, KEY_ON)) on = json_truth(STOCK_JSON_GET(w, KEY_ON));
                }
                wv[0] = presets.rack[slot][k][0] ? RACK_TYPE + type : type;
                wv[1] = type >= 0 ? (on ? 1 : 0) : 1;                          // H6j: an empty wagon is coupled, as a fresh chain's: a later pick sounds at once
                if (type >= 0) {
                    for (int i = 0; i < PARAMS; ++i) wv[2 + i] = DEFAULTS[type][i];
                    if (STOCK_JSON_HAS(w, KEY_PARAMS)) {
                        void* params = STOCK_JSON_GET(w, KEY_PARAMS);
                        if (is_array(params)) {
                            int n = STOCK_JSON_SIZE(params);
                            for (int i = 0; i < PARAMS && i < n; ++i) wv[2 + i] = int_at(params, i);
                        }
                    }
                }
                if (ghost_delay(wv[0], wv[1], wv + 2)) { wv[0] = NONE; wv[1] = 1; }   // H6j: a file saved from a chain that carried H6's ghost
            }
            char name[NAME_CHARS + 1];
            name[0] = 0;
            if (STOCK_JSON_HAS(&root, KEY_NAME)) {
                void* e = STOCK_JSON_GET(&root, KEY_NAME);
                if (e && *((u8*)e + 4) == JSON_STRING_KIND) json_text(e, name, NAME_CHARS);
            }
            for (int c = 0; name[c]; ++c) name[c] = (name[c] >= 32 && name[c] < 127) ? lower_of(name[c]) : ' ';
            name_trim(name);
            if (!name[0]) default_name(name, slot);
            name_copy(presets.name[slot], name);
            presets.used[slot] = 1;
        }
    }
    STOCK_JSON_DTOR(&root);
}

// The folder's sixty-four files. R2: the folder is the base's to make, and the scan makes no file: a slot whose file
// is not there, or whose file is still to be taken off the disk, is an empty slot. A file whose time or size changed
// since the last read (or every file when forced) is read again. While the presets of user/fx/ are being copied into
// the folder the store is shut: the scan is owed, and runs when the copy has ended (op1f_fx_presets_owed).
static void fx_scan(int force) {
    if (!op1f_r2_move_done()) {
        u32 want = force ? 2u : 1u;
        if (r2.owed < want) r2.owed = want;
        return;
    }
    char folder[PATH_ROOM];
    if (op1f_path(&op1f_card_fx_train, PRESETS_FOLDER, 0, folder, PATH_ROOM) <= 0) folder[0] = 0;
    int reread = 0, made = 0;
    for (int s = 0; s < PRESETS; ++s) {
        char path[PATH_ROOM];
        int size = fx_path(s, path) ? STOCK_FILE_SIZE(path) : -1;
        if (size < 0 || removal_owed(s)) {                                         // no file, or one to be taken off: an empty slot, and no file made
            presets.used[s] = 0;
            presets.mtime[s] = 0;
            presets.size[s] = -1;
            continue;
        }
        u32 mtime = STOCK_FILE_MTIME(path);
        if (force || !presets.scanned || mtime != presets.mtime[s] || size != presets.size[s]) {
            fx_read_slot(s);
            presets.mtime[s] = mtime;
            presets.size[s] = size;
            ++reread;
        }
    }
    presets.scanned = 1;
    ++ls.scans;
    ls.rereads += (u32)reread;
    rebuild_order();
    STOCK_LOG(3, LOG_FILE, 19, "presets: scan %u of %s: %d files read, %d made, %d slots used, %u writes failed in all", ls.scans, folder, reread, made, presets.count, ls.write_failures);
}

// R2: this unit's own rule of a file that holds a preset, the one fx_read_slot reads by: an object whose wagons are an
// array of four or more. The copy of the presets of user/fx/ asks it of each file (build_r2_move.cpp).
extern "C" int op1f_fx_holds_preset(void* json) asm("op1f_fx_holds_preset");
extern "C" int op1f_fx_holds_preset(void* json) {
    if (!json || *((u8*)json + 4) != JSON_OBJECT_KIND || !STOCK_JSON_HAS(json, KEY_WAGONS)) return 0;
    void* wagons = STOCK_JSON_GET(json, KEY_WAGONS);
    return is_array(wagons) && STOCK_JSON_SIZE(wagons) >= WAGONS;
}

// R2: from the pack's frame call, once the presets are copied: the files of the presets deleted on the unit taken off
// the disk through the base, then the scan asked while the store was shut, and the first scan in any case: from the
// frame call the copy ends (or the first, where its mark is found) the store has been read, so that a save, a rename
// and a delete are never refused for want of a scan. The base takes a file off only in the pack's own frame call and
// not while the disk mode is open. When it answers not now, the computer has the folder: every removal still owed is
// dropped, never tried later (the file under that name may by then be one the computer brought), the files stay, and
// a forced scan puts the presets back in the list: the deletes did not happen.
extern "C" void op1f_fx_presets_owed(void) asm("op1f_fx_presets_owed");
extern "C" void op1f_fx_presets_owed(void) {
    if (r2.removing[0] | r2.removing[1]) {
        for (int s = 0; s < PRESETS; ++s) {
            if (!removal_owed(s)) continue;
            char name[8];
            slot_name(s, name);
            int answer = op1f_remove(&op1f_card_fx_train, PRESETS_FOLDER, name);
            if (answer == OP1F_NOT_NOW) {
                u32 dropped = 0;
                for (int d = 0; d < PRESETS; ++d) if (removal_owed(d)) ++dropped;
                r2.removing[0] = r2.removing[1] = 0;
                r2.owed = 2u;
                r2.dropped += dropped;
                if (r2.remove_lines < (u32)REMOVE_LINES) {
                    ++r2.remove_lines;
                    STOCK_LOG(2, LOG_FILE, 38, "%s: the disk mode was open: %u delete(s) did not happen, their files stay and the presets are in the list again", "presets", dropped);
                }
                break;
            }
            r2.removing[s >> 5] &= ~(1u << (s & 31));
            if (answer == 0 || answer == OP1F_NO_FILE) { ++r2.removed; continue; }
            ++r2.remove_failures;
            if (r2.remove_lines < (u32)REMOVE_LINES) {
                ++r2.remove_lines;
                STOCK_LOG(1, LOG_FILE, 37, "presets: the file of deleted slot %d could NOT be taken off the disk (%d): the preset is back at the next start", s + 1, answer);
            }
        }
    }
    u32 owed = r2.owed;
    if (owed || !presets.scanned) {                                            // the store read as soon as the copy has ended well, asked or not
        r2.owed = 0;
        fx_scan(owed == 2u);
    }
}

// R2: what a shut store did not write, said from the frame call (a save or a rename may come on the key thread).
static void r2_lines() {
    u32 writes = r2.shut_writes, deletes = r2.shut_deletes;
    if ((writes != r2.said_writes || deletes != r2.said_deletes) && r2.shut_lines < (u32)SHUT_LINES) {
        ++r2.shut_lines;
        STOCK_LOG(2, LOG_FILE, 36, "presets: the store is shut until the presets are copied into the pack's folder: %u saves or renames and %u deletes wrote nothing", writes, deletes);
    }
    r2.said_writes = writes;
    r2.said_deletes = deletes;
}

static void flash(int kind, int slot) {
    ls.flash_kind = kind;
    ls.flash_slot = slot;
    ls.flash_until_ms = (int)st.ms + FLASH_MS;
}

static void list_clamp(int host) {                                          // the cursor inside the list, the window around the cursor
    int n = presets.count;
    if (ls.cursor[host] >= n) ls.cursor[host] = n - 1;
    if (ls.cursor[host] < 0) ls.cursor[host] = 0;
    if (ls.editing[host]) { ls.first[host] = ls.cursor[host]; return; }   // the edited preset on the first row
    int max_first = n > LIST_ROWS ? n - LIST_ROWS : 0;
    if (ls.first[host] > max_first) ls.first[host] = max_first;
    if (ls.first[host] < 0) ls.first[host] = 0;
    if (ls.cursor[host] < ls.first[host]) ls.first[host] = ls.cursor[host];
    if (ls.cursor[host] >= ls.first[host] + LIST_ROWS) ls.first[host] = ls.cursor[host] - LIST_ROWS + 1;
}

static void list_open(int host) {
    if (&op1f_presets_available == 0 || !op1f_presets_available()) return;  // no list screen: shift + T3 does nothing
    fx_scan(0);                                                              // H6: a file the host changed through the disk mode is a preset now
    ls.open[host] = 1;
    ls.editing[host] = 0;
    int row = row_of_slot(ls.loaded_slot[host]);
    if (row >= 0) ls.cursor[host] = row;
    list_clamp(host);
    for (int k = 0; k < WAGONS; ++k) { ls.pressed[host][k] = 0; ls.acted[host][k] = 0; ls.swallow_until[host][k] = 0; }
    ++ls.opens;
}

static void list_close(int host) {
    ls.open[host] = 0;
    ls.editing[host] = 0;
}

// The mirrored wagon onto stock's slot with the wagon authoritative (a preset load): mirror_push reads the slot back
// into the wagon when the types agree, which is right for the knobs on the effect page and wrong here.
static void mirror_write(int host, u32 slot) {
    int knob = mirror[host][slot];
    Wagon* w = &wagons[host][slot][knob];
    void* record = (void*)patch_record(host, slot);
    if (rack_name[host][slot][knob][0] && w->type >= 0) {                     // R5: a pack's effect on the first wagon: ridden with the preset's words, or held off while its pack is away
        if (rack_find(rack_name[host][slot][knob])) mirror_ride(host, slot, 1);
        else mirror_silent(host, slot);
        return;
    }
    if (w->type >= 0) {
        if (slot_type(host, slot) != w->type) {
            slot_put(host, slot, w->type, w->params, w->coupled);                   // R6c: the firmware's order, the object made before the document
            mirror_document(host, slot, w, "load");                                 // H6c: the document too (H6b left it at the browser's last effect)
        } else {
            slot_write_params(host, slot, w->params);
            STOCK_SET_PARAMS_DOC(record, w->params);
            WR8(HOST[host].fx_active + slot, w->coupled ? 1 : 0);
            STOCK_SET_ACTIVE_DOC(record, w->coupled);
        }
    } else {
        WR8(HOST[host].fx_active + slot, 0);
    }
    stock_type_seen[host][slot] = slot_type(host, slot);
}

static void preset_save(int host) {
    u32 slot = gui_slot(host);
    if (!initialised[host][slot]) return;
    int s = free_slot();
    if (s < 0) { flash(FLASH_FULL, presets.count ? presets.order[ls.cursor[host]] : -1); return; }
    default_name(presets.name[s], s);
    chain_values(host, slot, presets.values[s], 1);                          // R5: the first wagon's ride kept by its name
    chain_names(host, slot, presets.rack[s], 1);
    presets.used[s] = 1;
    fx_write_slot(s);
    rebuild_order();
    ls.cursor[host] = row_of_slot(s);
    ls.editing[host] = 0;
    list_clamp(host);
    ls.keep_loaded = 1;
    ls.loaded_slot[host] = s;
    ls.keep_loaded = 0;
    flash(FLASH_SAVED, s);
    ls.save_state = SAVE_SAVED;                                                // H5b: "saved!" with the check for a second (the save screen), then the list
    ls.save_host = host;
    ls.save_slot = s;
    ls.save_ms = (int)st.ms;
    ++ls.saves;
    STOCK_LOG(3, LOG_FILE, 14, "presets: saved slot %d as %s from %s s%u (%s, %s, %s, %s)", s + 1, presets.name[s], host ? "drum" : "synth", slot,
              wagon_word(host, slot, 0), wagon_word(host, slot, 1), wagon_word(host, slot, 2), wagon_word(host, slot, 3));
}

static void preset_load(int host, int row) {
    u32 slot = gui_slot(host);
    if (row < 0 || row >= presets.count || !initialised[host][slot]) return;
    int s = presets.order[row];
    int values[CHAIN_INTS_V2];
    for (int n = 0; n < CHAIN_INTS_V2; ++n) values[n] = presets.values[s][n];
    values[1] = mirror[host][slot];                                            // the mirror stays on its knob
    values[2] = doc_type[host][slot];                                          // the document has not changed: the next reconcile finds the chain standing
    chain_from_values(host, slot, values, presets.rack[s]);                   // R5: a pack's effect by its name
    int moved_from = -1;
    int riding = place_rides(host, slot), m = mirror[host][slot];
    char* first = rack_name[host][slot][m];
    // R14: riding "train", the mirror takes no pack's effect (its pick would end the train): it goes to the first wagon in signal order with one of the unit's own, else to the first empty one, else (four packs' effects) its wagon is loaded empty.
    if (wagons[host][slot][m].type < 0 || (riding && first[0])) {               // H6g: a file whose wagons leave the mirrored knob empty (a two- or three-effect chain made on the PC fills the
        int to = -1, empty = -1;                                                  // blue, ochre and grey knobs): the mirror moves to the first effect in signal order, so stock's page shows an
        for (int p = 0; p < WAGONS && to < 0; ++p) {                              // effect of the chain, on, instead of the previous effect switched off (as first heard on the unit)
            int k = knob_at(host, slot, p);
            if (wagons[host][slot][k].type < 0) { if (empty < 0) empty = k; }
            else if (!riding || !rack_name[host][slot][k][0]) to = k;
        }
        if (to < 0 && first[0]) to = empty;
        if (to < 0 && first[0]) {
            wagons[host][slot][m].type = NONE;
            for (int i = 0; i < PARAMS; ++i) wagons[host][slot][m].params[i] = 0;
            first[0] = 0;
        }
        if (to >= 0 && to != m) { moved_from = m; mirror[host][slot] = (u8)to; ++ls.mirror_moves; }
    }
    st.push_pending[host] = 0;
    mirror_write(host, slot);                                                  // stock's slot takes the mirrored wagon's effect, parameters and coupling
    st.current_slot[host] = -1;                                                // the live instances follow at the next frame (as after a reconcile)
    ls.keep_loaded = 1;
    mark_dirty(host);                                                          // the chain's own file
    ls.keep_loaded = 0;
    ls.loaded_slot[host] = s;
    ++ls.loads;
    u32 cost = chain_cost(host, slot, -1, NONE, -1);                              // H6f: a file the host wrote is not held to the ceiling the picks are; said in the log
    STOCK_LOG(3, LOG_FILE, 15, "presets: loaded slot %d (%s) onto %s s%u: %s, %s, %s, %s; %u of %u cycles a block%s; mirror knob %d%s", s + 1, presets.name[s], host ? "drum" : "synth", slot,
              wagon_word(host, slot, 0), wagon_word(host, slot, 1), wagon_word(host, slot, 2), wagon_word(host, slot, 3),
              cost, (u32)CEILING_CYCLES, cost > (u32)CEILING_CYCLES ? " (OVER THE BUDGET: the block may overrun)" : "",
              mirror[host][slot], moved_from < 0 ? "" : rack_name[host][slot][moved_from][0] ? " (moved from a pack's effect)" : " (moved from an empty wagon)");
}

static void preset_delete_now(int s) {
    if (s < 0 || s >= PRESETS || !presets.used[s]) return;
    STOCK_LOG(3, LOG_FILE, 16, "presets: deleted slot %d (%s)", s + 1, presets.name[s]);
    presets.used[s] = 0;
    if (op1f_r2_move_done() && presets.scanned) r2.removing[s >> 5] |= 1u << (s & 31);   // R2: the file is taken off the disk at the pack's next frame call (op1f_fx_presets_owed)
    else ++r2.shut_deletes;                                                    // the store is shut: nothing is written
    rebuild_order();
    for (int h = 0; h < HOSTS; ++h) {
        if (ls.loaded_slot[h] == s) ls.loaded_slot[h] = -1;
        list_clamp(h);
    }
    ++ls.deletes;
}

static void delete_begin(int host) {
    if (!presets.count || ls.pending_delete_slot >= 0 || ls.editing[host]) return;
    int s = presets.order[ls.cursor[host]];
    ls.pending_delete_slot = s;
    flash(FLASH_DELETED, s);                                                   // the row reads deleted for a second, then the list closes up
}

static void rename_begin(int host) {
    if (!presets.count) return;
    ls.editing[host] = 1;
    ls.caret[host] = 0;
    const char* name = presets.name[presets.order[ls.cursor[host]]];
    int c = 0;
    for (; c < NAME_CHARS && name[c]; ++c) ls.buffer[host][c] = name[c];
    for (; c < NAME_CHARS; ++c) ls.buffer[host][c] = ' ';
    ls.buffer[host][NAME_CHARS] = 0;
    list_clamp(host);
}

static void rename_keep(int host) {
    if (!ls.editing[host]) return;
    ls.editing[host] = 0;
    if (ls.cursor[host] < 0 || ls.cursor[host] >= presets.count) { list_clamp(host); return; }
    int s = presets.order[ls.cursor[host]];
    char name[NAME_CHARS + 1];
    name_copy(name, ls.buffer[host]);
    name_trim(name);
    if (name[0] == 0) default_name(name, s);                                   // all spaces: the slot's name
    name_copy(presets.name[s], name);
    fx_write_slot(s);
    list_clamp(host);
    ++ls.renames;
    STOCK_LOG(3, LOG_FILE, 17, "presets: renamed slot %d to %s", s + 1, presets.name[s]);
}

static void rename_cancel(int host) {
    ls.editing[host] = 0;
    list_clamp(host);
}

static void t3_press(int host) {
    ls.t3_pressed[host] = 1;
    ls.t3_ms[host] = (int)st.ms;
    ls.t3_acted[host] = 0;
}

// R14: T3 let go on the train page before the hold opened the list: the tap, the stock page of the first wagon's
// effect wished (fx train 17: shift + T3's tap opened or closed the list, its hold saved).
static void t3_release(int host) {
    ls.t3_pressed[host] = 0;
    if (ls.t3_acted[host] || ls.open[host]) return;
    take_page = TAKE_UNIT;
}

// R14: the list, T3's hold and the save screen go (shift + T3, T1, T2, T4, the train page left).
static void presets_leave(int host) {
    list_close(host);
    ls.t3_pressed[host] = 0;
    if (ls.save_host == host) ls.save_state = SAVE_NONE;
}

static void list_press(int host, int knob) {
    ls.pressed[host][knob] = 1;
    ls.press_ms[host][knob] = (int)st.ms;
    ls.acted[host][knob] = 0;
}

static void list_release(int host, int knob) {
    if (ls.pressed[host][knob] && ls.acted[host][knob]) ls.swallow_until[host][knob] = (int)st.frames + SWALLOW_FRAMES;
    ls.pressed[host][knob] = 0;
}

static void list_click(int host, int knob, int shifted) {
    if ((int)st.frames < ls.swallow_until[host][knob]) {                    // the click after the red hold
        ls.swallow_until[host][knob] = 0;
        return;
    }
    if (shifted) return;
    if (ls.editing[host]) {
        if (knob == KNOB_GREY) rename_keep(host);
        else if (knob == KNOB_RED) rename_cancel(host);
        return;
    }
    if (knob == KNOB_BLUE) { preset_load(host, ls.cursor[host]); list_close(host); }
    else if (knob == KNOB_OCHRE) rename_begin(host);
    else if (knob == KNOB_GREY) preset_save(host);
}

static void list_turn(int host, int knob, int delta) {
    int step = delta > 0 ? 1 : -1;
    if (ls.editing[host]) {
        if (knob == KNOB_BLUE) {
            int c = ls.caret[host] + step;
            if (c >= 0 && c < NAME_CHARS) ls.caret[host] = c;
        } else if (knob == KNOB_OCHRE) {
            char* ch = &ls.buffer[host][ls.caret[host]];
            int i = alphabet_index(*ch) + step;
            if (i < 0) i = ALPHABET_COUNT - 1;
            if (i >= ALPHABET_COUNT) i = 0;
            *ch = ALPHABET[i];
        }
        return;
    }
    if (knob == KNOB_BLUE && presets.count) {
        ls.cursor[host] += step;
        list_clamp(host);
    }
}

// A key-down while the list shows over the train page (shift + T3 is the caller's): 1 taken, 0 stock's. T3 keeps the
// rename or loads, and closes the list; T1, T2 and T4 close it and are stock's; shift + another key does nothing.
static int list_key(int host, int key, int shifted) {
    if (key >= KEY_PRESS0 && key < KEY_PRESS0 + WAGONS) { list_press(host, key - KEY_PRESS0); return 1; }
    if (key >= KEY_CLICK0 && key < KEY_CLICK0 + WAGONS) { list_click(host, key - KEY_CLICK0, shifted); return 1; }
    if (key == KEY_T3 && !shifted) {
        if (ls.editing[host]) rename_keep(host); else preset_load(host, ls.cursor[host]);
        list_close(host);                                                       // back to the train page
        return 1;
    }
    if (key >= KEY_T1 && key <= KEY_T4) { presets_leave(host); return 0; }    // another page: the list closes, stock leaves the train's
    return shifted ? 1 : 0;                                                     // shift + another key on the list: nothing; any other key is stock's
}

static void list_frame(int host) {
    int now = (int)st.ms;
    if (ls.t3_pressed[host] && !ls.t3_acted[host] && now - ls.t3_ms[host] >= HOLD_MS) {   // R14: T3 held on the train page: the list (the input unit's frame shows it over the page)
        ls.t3_acted[host] = 1;
        if (st.train_open[host]) list_open(host);
    }
    if (ls.save_state == SAVE_SAVED && ls.save_host == host && now - ls.save_ms >= SAVED_MS) ls.save_state = SAVE_NONE;   // H5b: the confirmation ends, the list is back
    if (ls.open[host] && ls.pressed[host][KNOB_RED] && !ls.acted[host][KNOB_RED] && now - ls.press_ms[host][KNOB_RED] >= HOLD_MS) {
        ls.acted[host][KNOB_RED] = 1;
        delete_begin(host);
    }
    if (ls.flash_kind != FLASH_NONE && now - ls.flash_until_ms >= 0) {
        if (ls.flash_kind == FLASH_DELETED && ls.pending_delete_slot >= 0) preset_delete_now(ls.pending_delete_slot);
        ls.pending_delete_slot = -1;
        ls.flash_kind = FLASH_NONE;
    }
}

extern "C" int op1f_chain_list_open(int host) {
    return (host == 0 || host == 1) ? ls.open[host] : 0;
}

extern "C" int op1f_chain_screen_wanted(int host) {                          // H5b: the save screen over everything, the list over the train
    if (host != 0 && host != 1) return 0;
    if (ls.save_state != SAVE_NONE && ls.save_host == host) return OP1F_SAVE_SCREEN_ID;
    if (ls.open[host]) return OP1F_PRESETS_SCREEN_ID;
    return 0;
}

// The list's view (Build H's presets unit dresses the screens from it every frame).
extern "C" void op1f_presets_view(int host, Op1fPresetsView* out) {
    out->open = ls.open[host];
    out->count = presets.count;
    out->first = ls.first[host];
    out->cursor = ls.cursor[host];
    out->loaded = row_of_slot(ls.loaded_slot[host]);
    out->editing = ls.editing[host];
    out->caret = ls.caret[host];
    for (int r = 0; r < LIST_ROWS; ++r) {
        int i = ls.first[host] + r;
        int s = i < presets.count ? presets.order[i] : -1;
        const char* text = s >= 0 ? presets.name[s] : "";
        if (ls.flash_kind != FLASH_NONE && s >= 0 && ls.flash_slot == s) text = ls.flash_kind == FLASH_SAVED ? "saved" : (ls.flash_kind == FLASH_DELETED ? "deleted" : "list full");
        if (ls.flash_kind == FLASH_FULL && ls.flash_slot < 0 && r == 0) text = "list full";
        if (presets.count == 0 && r == 0 && ls.flash_kind == FLASH_NONE) text = "no presets";
        if (ls.editing[host] && r == 0) text = "";                             // the cells show the name
        name_copy(ls.row_text[r], text);
        out->row[r] = ls.row_text[r];
    }
    const int* v = presets.count ? presets.values[presets.order[ls.cursor[host] < presets.count ? ls.cursor[host] : 0]] : 0;
    for (int k = 0; k < WAGONS; ++k) {
        int type = v ? v[3 + WAGONS + k * (2 + PARAMS)] : NONE;
        int coupled = v ? v[3 + WAGONS + k * (2 + PARAMS) + 1] : 1;
        char* p = ls.wagon_text[k];
        const char* pack = (v && type >= RACK_TYPE && type < RACK_TYPE + TYPES) ? presets.rack[presets.order[ls.cursor[host] < presets.count ? ls.cursor[host] : 0]][k] : "";
        if (pack[0]) {                                                         // R5: a pack's effect by its name (thirteen characters at most, " off" behind: the row's room)
            for (int c = 0; c < NAME_CHARS + 1 && pack[c]; ++c) *p++ = pack[c];
            if (!coupled) p = put_text(p, " off");
        } else if (type >= 0 && type < TYPES) {
            p = put_text(p, fx_name(type));                                    // stock's own lowercase names
            if (!coupled) p = put_text(p, " off");
        } else {
            *p++ = '-';
        }
        *p = 0;
        out->wagon[k] = ls.wagon_text[k];
    }
    for (int c = 0; c < NAME_CHARS; ++c) {
        ls.cell_text[c][0] = ls.buffer[host][c] ? ls.buffer[host][c] : ' ';
        ls.cell_text[c][1] = 0;
        out->cell[c] = ls.cell_text[c];
    }
    out->save_state = ls.save_state;                                           // H5b
    out->save_host = ls.save_host;
    out->save_number = ls.save_slot + 1;
    int held = (int)st.ms - ls.save_ms;
    int spokes = ls.save_state == SAVE_SAVING ? (int)udiv((u32)(held < 0 ? 0 : held) * (u32)OP1F_SAVE_SPOKES, (u32)SAVE_HOLD_MS) : OP1F_SAVE_SPOKES;
    out->save_progress = spokes > OP1F_SAVE_SPOKES ? OP1F_SAVE_SPOKES : spokes;
}

// ---------------------------------------------------------------- the document

// The document's own effect: fx_type's name through stock's own lookup (the apply does the same).
static int doc_fx_type(void* doc) {
    if (!STOCK_JSON_HAS(doc, FX_TYPE_KEY)) return NONE;
    void* entry = STOCK_JSON_GET(doc, FX_TYPE_KEY);
    if (entry == 0) return NONE;
    u32 str[8];
    for (int i = 0; i < 8; ++i) str[i] = 0;
    STOCK_JSON_TO_STRING(str, entry);                              // a short name stays inside the object: nothing to free
    int type = STOCK_FX_TYPE_OF(str);
    return (type >= 0 && type < TYPES) ? type : NONE;
}

static void doc_fx_params(void* doc, u32* out) {
    for (int i = 0; i < PARAMS; ++i) out[i] = 0;
    if (!STOCK_JSON_HAS(doc, FX_PARAMS_KEY)) return;
    void* array = STOCK_JSON_GET(doc, FX_PARAMS_KEY);
    if (!is_array(array)) return;
    int n = STOCK_JSON_SIZE(array);
    for (int i = 0; i < PARAMS && i < n; ++i) out[i] = word_of(int_at(array, i));
}

static int doc_fx_active(void* doc) {
    if (!STOCK_JSON_HAS(doc, FX_ACTIVE_KEY)) return 0;
    void* entry = STOCK_JSON_GET(doc, FX_ACTIVE_KEY);
    return entry && STOCK_JSON_BOOL(entry) ? 1 : 0;
}

// The document's effect becomes the blue wagon; the others empty.
static void chain_defaults(int host, u32 slot, void* doc, int dt) {
    for (int k = 0; k < WAGONS; ++k) {
        rack_name[host][slot][k][0] = 0;                                // R5: no pack's effect
        Wagon* w = &wagons[host][slot][k];
        w->type = NONE;
        w->coupled = 1;
        for (int i = 0; i < PARAMS; ++i) w->params[i] = 0;
    }
    int knobs[WAGONS] = { 0, 1, 2, 3 };
    set_order(host, slot, knobs);
    mirror[host][slot] = 0;
    switch_owed[host][slot] = 0;                               // 17
    document_owed[host][slot] = 0;
    Wagon* first = &wagons[host][slot][0];
    first->type = dt;
    first->coupled = doc_fx_active(doc);
    if (dt >= 0) doc_fx_params(doc, first->params);
    doc_type[host][slot] = dt;
    ride_forget(host, slot);                                   // R14: one effect: no start due
}

// I9: a new instrument was loaded from a file onto the slot (Build I's patch unit
// saw stock's job put it on the record). Its file carried the slot's integers as this unit's own file holds them
// for a slot, and they are read as load_file reads them; or it carried none, an instrument as stock alone knows
// it, and the slot is forgotten: the reconcile that follows then makes the chain of a slot seen for the first time
// (chain_defaults: the document's effect on the blue wagon, the three others empty). Either way the LFOs' effect
// wagons and the LFO unit's three other LFOs are the instrument's, never the sound key's of before.
static int instrument_lines;

static void instrument_event(int host, u32 slot) {
    if (!&op1f_patch_pending) return;
    static int v[OP1F_PATCH_INTS];                                      // the GUI thread only
    int n = op1f_patch_pending(host, (int)slot, v);
    if (n < 0) return;
    lfo_fx_pick[host][slot] = 0;
    for (int i = 0; i < MIDI_DESTS; ++i) lfo_midi_pick[host][slot][i] = 0;
    int own = n >= CHAIN_INTS_V2 && v[0] == CHAIN_TAG_V2, lfos = 0;
    if (own) {
        char names[WAGONS][RACK_NAME];                                  // R5: the names' tail, behind every other block
        int at = names_at(v, n, host);
        if (at >= 0) names_from_tail(v + at + 1, names);
        chain_from_values(host, slot, v, at >= 0 ? names : 0);
        if (n >= CHAIN_INTS_V2 + 2 && v[CHAIN_INTS_V2] == LFOFX_TAG) {
            int pick = v[CHAIN_INTS_V2 + 1];
            lfo_fx_pick[host][slot] = (u8)(pick >= 0 && pick <= WAGONS ? pick : 0);
            if (n >= CHAIN_INTS_V2 + 4 && v[CHAIN_INTS_V2 + 2] == LFOMIDI_TAG) {
                int packed = v[CHAIN_INTS_V2 + 3];
                for (int i = 0; i < MIDI_DESTS; ++i) {
                    int one = (packed >> (4 * i)) & 15;
                    lfo_midi_pick[host][slot][i] = (u8)(one <= WAGONS ? one : 0);
                }
                if (host == 0 && n >= LFO4_AT + 1 + LFO4_MIN && v[LFO4_AT] == LFO4_TAG) {
                    int more[LFO4_INTS];
                    int have = n - (LFO4_AT + 1);
                    for (int i = 0; i < LFO4_INTS; ++i) more[i] = i < have ? v[LFO4_AT + 1 + i] : -1;
                    if (&op1f_lfo4_store_load) op1f_lfo4_store_load((int)slot, more);
                    else lfo4_keep(slot, more);                         // J5f: the LFO unit is away: the instrument's four are carried for the slot
                    lfos = 1;
                }
            }
        }
        known[host][slot] = 1;
    } else {
        known[host][slot] = 0;                                          // the reconcile below: chain_defaults
    }
    if (host == 0 && !lfos && !&op1f_lfo4_store_load) lfo4_drop(slot);  // J5f: the LFO unit is away and the instrument brings no LFOs of ours: nothing is carried for the slot
    if (host == 0 && !lfos && &op1f_lfo4_store_load) {                  // the LFO page as stock alone has it: the sound's own LFO on the blue knob, the three others empty, the knobs' own order
        int plain[LFO4_INTS];
        for (int i = 0; i < LFO4_INTS; ++i) plain[i] = 0;
        plain[1] = 1;                                                   // the first wagon is not emptied (under the lamp the kind's sign alone counts)
        plain[LFO4_MIN] = -1;
        op1f_lfo4_store_load((int)slot, plain);
    }
    mark_dirty(host);
    if (instrument_lines < 60) {
        ++instrument_lines;
        STOCK_LOG(3, LOG_FILE, 31, "chain instrument: %s s%u: %s (%d integers); its LFOs: %s", host ? "drum" : "synth", slot,
                  own ? "the file's own train" : "none in the file, the instrument's one effect on the blue wagon", n,
                  host ? "stock's" : (lfos ? "the file's four" : "the instrument's one, on the blue knob"));
    }
}

// R6c: a load's tell whose document's effect the slot does not hold yet: the firmware's apply of that document is
// still to come (a sampler sound's sample is loaded between the list's put and its apply), and it writes the slot in
// its own order. The chain has taken the document into its model and writes nothing to the slot. Said, the first 24.
static u32 tell_waits, tell_lines;
static void tell_wait(int host, u32 slot, int dt, int applied) {
    ++tell_waits;
    if (tell_lines >= 24) return;
    ++tell_lines;
    STOCK_LOG(3, LOG_FILE, 46, "chain instrument: %s s%u: the document's %s is not the slot's effect yet (%s): the slot is left to the firmware's apply; %u in all",
              host ? "drum" : "synth", slot, fx_name(dt), fx_name(applied), tell_waits);
}

// The record's document against the chain. Run after stock applied a document to the slot (the hooks), when the
// module first sees a slot and when the slot's type word changes under the module. The type the chain was last
// reconciled with tells a stale document re-applied (the chain stands, its mirror written back) from a browser
// pick or a preset loaded (the mirror takes the document's effect). R6c: `told` at a load's tell
// (op1f_chain_instrument), where the slot is not written while the firmware's apply is to come (tell_wait).
static void reconcile(int host, u32 slot, const char* why, int told = 0) {
    if (!file_loaded) load_file();
    instrument_event(host, slot);                                    // I9: a new instrument on this sound key brings its own chain, or none
    void* doc = (void*)(patch_record(host, slot) + 8);
    int dt = doc_fx_type(doc);
    int applied = slot_type(host, slot);
    const char* action;
    if (!known[host][slot]) {
        chain_defaults(host, slot, doc, dt);
        known[host][slot] = 1;
        action = "defaults";
    } else if (dt == doc_type[host][slot]) {                         // the document the chain knows: the chain stands
        action = "stands";
        ++st.restores;
    } else {                                                         // another effect in the document: the mirror takes it
        int m = mirror[host][slot];
        Wagon* mw = &wagons[host][slot][m];
        remember(host, slot, m, mw);
        rack_name[host][slot][m][0] = 0;                             // R5: the document brought an effect: a pack's wished on the first wagon is let go
        mw->type = dt;
        mw->coupled = doc_fx_active(doc);
        document_owed[host][slot] = (switch_owed[host][slot] || document_owed[host][slot]) && !mw->coupled ? (u8)(m + 1) : 0;   // 17: the empty wagon's "off" still stands in it, from pick to pick
        switch_owed[host][slot] = 0;                             // 17: the document's switch
        if (dt >= 0) doc_fx_params(doc, mw->params); else for (int i = 0; i < PARAMS; ++i) mw->params[i] = 0;
        dedupe(host, slot, m, dt);
        doc_type[host][slot] = dt;
        mark_dirty(host);
        action = "adopts";
        ++st.adopts;
    }
    initialised[host][slot] = 1;
    if (told && dt != applied) tell_wait(host, slot, dt, applied);  // R6c: the firmware's apply writes the slot from this document
    else mirror_push(host, slot);                                    // the slot holds the mirrored wagon
    if (st.current_slot[host] == (int)slot) st.current_slot[host] = -1;    // the live instances follow at the next frame
    ++st.reconciles;
    if (st.diag_lines < 40) {
        ++st.diag_lines;
        STOCK_LOG(3, LOG_FILE, 6, "chain reconcile: %s s%u (%s): doc %s, applied %s -> %s; mirror %d order %d%d%d%d: %s, %s, %s, %s",
                  host ? "drum" : "synth", slot, why, fx_name(dt), fx_name(applied), action,
                  mirror[host][slot], knob_at(host, slot, 0), knob_at(host, slot, 1), knob_at(host, slot, 2), knob_at(host, slot, 3),
                  fx_name(wagons[host][slot][0].type), fx_name(wagons[host][slot][1].type), fx_name(wagons[host][slot][2].type), fx_name(wagons[host][slot][3].type));
    }
}

// The vtable word 0x8401e90c (H1): a preset job's apply (load a preset by step); it changes the record and applies
// it through the record's own virtual below, so this wrapper only reconciles once more. Kept as H1 left it.
extern "C" void op1f_fx_patch_apply(void* descriptor) asm("op1f_fx_patch_apply");
extern "C" void op1f_fx_patch_apply(void* descriptor) {
    u32 slot = *(u32*)((u8*)descriptor + 4) & 7u;
    int host = *((u8*)descriptor + 8) ? 1 : 0;
    STOCK_PATCH_APPLY(descriptor);
    reconcile(host, slot, "job");
}

// The records' apply virtual (vtable + 0x10; the words 0x8401f894 and 0x8401f870, H1b): every application of a
// record's document to its slot passes here: the boot's applies of all sixteen records (1.8 s in, after the slots
// files are loaded and the documents rebuilt), the slot switch, the browser's pick, the preset jobs.
static void record_applied(int host, void* record) {
    u32 slot = *(u32*)((u8*)record + 4) & 7u;
    ((ApplyFn)(host ? STOCK_APPLY_DRUM : STOCK_APPLY_SYNTH))(record);
    reconcile(host, slot, "apply");
}

extern "C" void op1f_fx_apply_synth(void* record) asm("op1f_fx_apply_synth");
extern "C" void op1f_fx_apply_synth(void* record) { record_applied(0, record); }

extern "C" void op1f_fx_apply_drum(void* record) asm("op1f_fx_apply_drum");
extern "C" void op1f_fx_apply_drum(void* record) { record_applied(1, record); }

// A slot the module has not seen: once its record holds a document, or after five seconds, the chain is reconciled
// with it.
static void init_slot(int host, u32 slot) {
    void* doc = (void*)(patch_record(host, slot) + 8);
    if (STOCK_JSON_HAS(doc, FX_TYPE_KEY) || st.frames > (u32)INIT_WAIT_FRAMES) {
        reconcile(host, slot, "init");
        ++st.inits;
    }
}

// I9: the slot's integers for an instrument's file, as save_file writes them for the slot (the chain's 48, the two
// pairs of the LFOs' effect wagons, then the tag 98 and the LFO unit's integers when the sound has more than stock's
// one LFO). 0 for a slot the module does not know and for one that is as stock alone has it: the lamp on the blue
// knob, the knobs' own order, nothing on the three other wagons, no LFO aimed at a wagon of its choice, no added LFO.
extern "C" int op1f_chain_slot_values(int host, int slot, int* out) {
    if ((host != 0 && host != 1) || slot < 0 || slot >= SLOTS || !known[host][slot]) return 0;
    int values[CHAIN_INTS_V2];
    chain_values(host, (u32)slot, values, 0);
    char names[WAGONS][RACK_NAME];
    int named = chain_names(host, (u32)slot, names, 0);                 // R5: a pack's wagon (its ride is the base's, kept with the sound)
    int tail[4] = { (int)LFOFX_TAG, (int)lfo_fx_pick[host][slot], (int)LFOMIDI_TAG, 0 };
    for (int i = 0; i < MIDI_DESTS; ++i) tail[3] |= (int)lfo_midi_pick[host][slot][i] << (4 * i);
    int more[LFO4_INTS];
    int have = host != 0 ? 0 : (&op1f_lfo4_store_save ? op1f_lfo4_store_save(slot, more) : lfo4_carried((u32)slot, more));   // J5f: what was read, while the LFO unit is away
    int plain = !have && tail[1] == 0 && tail[3] == 0 && mirror[host][slot] == 0;
    for (int p = 0; p < WAGONS; ++p) if (knob_at(host, (u32)slot, p) != p) plain = 0;
    for (int k = 1; k < WAGONS; ++k) if (wagons[host][slot][k].type >= 0) plain = 0;
    if (named) plain = 0;
    if (plain) return 0;
    int n = 0;
    for (int i = 0; i < CHAIN_INTS_V2; ++i) out[n++] = values[i];
    for (int i = 0; i < 4; ++i) out[n++] = tail[i];
    if (have) {
        out[n++] = (int)LFO4_TAG;
        for (int i = 0; i < LFO4_INTS; ++i) out[n++] = more[i];
    }
    if (named) {                                                        // R5: the tag 99 and the names, behind every other block
        names_tail(names, out + n);
        n += RACK_INTS;
    }
    return n;
}

// I9: the slot's record against its chain once more, asked by Build I's patch unit for an instrument stock did not apply.
extern "C" void op1f_chain_instrument(int host, int slot) {
    if ((host != 0 && host != 1) || slot < 0 || slot >= SLOTS) return;
    reconcile(host, (u32)slot, "instrument", 1);                     // R6c: a load's tell: the slot is the firmware's apply's to write
}

// ---------------------------------------------------------------- the frame

// The page's view of the chain (H2): Build H's train page unit dresses the train screen from it every frame.
extern "C" void op1f_chain_view(int host, Op1fChainView* out) {
    u32 slot = gui_slot(host);
    out->open = st.train_open[host];
    out->known = initialised[host][slot];
    out->slot = (int)slot;
    for (int p = 0; p < WAGONS; ++p) out->order[p] = knob_at(host, slot, p);
    for (int k = 0; k < WAGONS; ++k) {
        const Wagon* w = &wagons[host][slot][k];
        out->type[k] = w->type;
        out->coupled[k] = w->coupled;
    }
    out->mirror = mirror[host][slot];
    int stand = browse_knob[host];                                              // 19: a turn's stand shows as the refusal, as long as it lasts
    if (stand >= 0 && browse_slot[host] != slot) stand = -1;
    out->full = (int)st.frames < st.flash_until[host] || stand >= 0;
    out->refused_knob = stand >= 0 ? stand : out->full ? st.refused_knob[host] : -1;
    out->refused_type = stand >= 0 ? browse_type[host] : out->full ? st.refused_type[host] : -1;
    out->cost_percent = (int)udiv(chain_cost(host, slot, -1, NONE, -1) * 100u, (u32)CEILING_CYCLES);
    out->turn_knob = st.turn_frame[host] ? st.turn_knob[host] : -1;
    out->turn_age = st.turn_frame[host] ? (int)st.frames - st.turn_frame[host] : 0x7fffffff;
    out->turn_move = st.turn_move[host];
    out->bypassed = bypass_mask[host][slot] != 0;
    out->peak = bus_peak[host];                                                // the audio thread's maximum since the last frame
    bus_peak[host] = 0;
}

// 19: what the page shows of a turn's stand (browse_knob, above): the knob, or -1; the effect's name; the words that say
// why, in percent of the ceiling (what it needs rounded up, what the rest of the train leaves it rounded down, so the
// first is always the larger). From the frame function.
struct Op1fTrainStand { int knob; const char* name; char why[32]; };
extern "C" void op1f_train_stand(int host, Op1fTrainStand* out) asm("op1f_train_stand");
extern "C" void op1f_train_stand(int host, Op1fTrainStand* out) {
    out->knob = -1;
    out->name = 0;
    out->why[0] = 0;
    if (host != 0 && host != 1) return;
    int knob = browse_knob[host];
    if (knob < 0 || knob >= WAGONS || browse_slot[host] != gui_slot(host)) return;
    u32 ceiling = (u32)CEILING_CYCLES;
    char* p = put_text(out->why, "needs ");
    p = put_u32(p, udiv(browse_need[host] * 100u + ceiling - 1u, ceiling));
    p = put_text(p, "%, ");
    p = put_u32(p, udiv(browse_left[host] * 100u, ceiling));
    p = put_text(p, "% free");
    *p = 0;
    out->name = order_name(browse_item[host]);
    out->knob = knob;
}

extern "C" int op1f_chain_train_open(int host) {
    return (host == 0 || host == 1) ? st.train_open[host] : 0;
}

// R5: what the train's page is told of the packs' effects on the slot on the screen (fx train 14's train unit, which
// holds the same structure), beside op1f_chain_view: per knob the name of the pack's effect its wagon carries (the
// first wagon's ride included), or 0 for one of the unit's own; whether that effect's pack is away (the wagon is
// silent); the pack's effect a refused pick named, or 0. From the frame function (the display thread).
struct Op1fChainRacks { const char* name[4]; int away[4]; const char* refused; };
extern "C" void op1f_chain_racks(int host, Op1fChainRacks* out) asm("op1f_chain_racks");
extern "C" void op1f_chain_racks(int host, Op1fChainRacks* out) {
    for (int k = 0; k < WAGONS; ++k) { out->name[k] = 0; out->away[k] = 0; }
    out->refused = 0;
    if (host != 0 && host != 1) return;
    u32 slot = gui_slot(host);
    for (int k = 0; k < WAGONS; ++k) {
        if (wagons[host][slot][k].type < 0) continue;
        const char* name = cargo_name(host, slot, k);
        out->name[k] = name;
        out->away[k] = name && !rack_find(name);
        if (name && !out->away[k] && rk.no_room[host][k]) out->away[k] = 1;   // 19: refused by the base: silent, as a pack away is
    }
    if ((int)st.frames < st.flash_until[host] && rk.refused_name[host][0]) out->refused = rk.refused_name[host];
}

// R5: the name of value `index` (0 to 3) of the pack's effect a wagon of the slot on the screen carries, for the LFO
// page's name over the knob: "" past the effect's values; 0 for a wagon of the unit's own, the first wagon (the base
// names a ride's values), or a pack away. The display thread.
extern "C" const char* op1f_chain_rack_value(int host, int knob, int index) asm("op1f_chain_rack_value");
extern "C" const char* op1f_chain_rack_value(int host, int knob, int index) {
    if ((host != 0 && host != 1) || knob < 0 || knob >= WAGONS || index < 0) return 0;
    u32 slot = gui_slot(host);
    if (knob == mirror[host][slot] || wagons[host][slot][knob].type < 0) return 0;
    const Op1fRackEntry* e = rack_find(rack_name[host][slot][knob]);
    if (e == 0) return 0;
    u32 i = (u32)index & 3u;
    return i < e->effect->count ? e->effect->params[i].name : "";
}

// R5: once a frame for each instrument's slot on the screen: the first wagon's ride as the base has it (op1f_rack_at,
// kept for the key thread), and each other wagon's pack's effect made ready at its place (op1f_rack_ready, a few
// comparisons when nothing changed): its id for the audio thread, or silence while its pack is away, while its memory
// is made ready, or where the base refuses it; each said once, at its change.
static void rack_frame(int host, u32 slot) {
    u32 id = op1f_rack_at(&op1f_card_fx_train, host ? (u32)OP1F_PLACE_DRUM + slot : slot);
    mirror_ride_id[host] = place_rides(host, slot) ? 0u : id;              // R14: the base answers the train's own layer while the place rides it: no pack's effect
    mirror_ride_slot[host] = slot;
    for (int k = 0; k < WAGONS; ++k) {
        Live* l = &live[host][k];
        rk.no_room[host][k] = 0;                                            // 19
        if (k == mirror[host][slot] || !l->rack_name[0]) continue;
        const Op1fRackEntry* e = rack_find(l->rack_name);
        u32 want = RACK_SILENT;
        int how = 1;                                                        // 1 its pack away, 0 ready, else the base's answer
        if (e) {
            how = op1f_rack_ready(&op1f_card_fx_train, (u32)(host * WAGONS + k), e->id);
            if (how == 0) want = e->id;
            l->tails = (e->effect->head.flags & (u32)OP1F_EFFECT_TAILS) ? 1u : 0u;
        }
        // 19: the base will not make it ready (its blocks of working memory for the session used up, or its runs inside
        // packs; or another answer): the wagon plays nothing until a restart, and the page draws it so (op1f_chain_racks)
        if (e && how != 0 && how != OP1F_NOT_NOW) rk.no_room[host][k] = (how == OP1F_FAILED || how == OP1F_TOO_LARGE) ? 1 : 2;
        if (l->rack != want) l->rack = want;
        if (how == OP1F_NOT_NOW) continue;                                  // its memory is being made ready: said by the base
        if (rk.said_how[host][k] == how && name_same(rk.said[host][k], l->rack_name)) continue;
        rk.said_how[host][k] = how;
        name_put(rk.said[host][k], l->rack_name);
        if (rk.silent_lines >= (u32)RACK_LINES) continue;
        ++rk.silent_lines;
        if (how == 0) STOCK_LOG(3, LOG_FILE, 43, "chain rack: %s s%u knob %d plays '%s' at the train's place %d, entry %u", host ? "drum" : "synth", slot, k, l->rack_name, host * WAGONS + k, want);
        else if (how == 1) STOCK_LOG(2, LOG_FILE, 44, "chain rack: %s s%u knob %d: the pack of '%s' is not here: the wagon is silent, kept as it is", host ? "drum" : "synth", slot, k, l->rack_name);
        else STOCK_LOG(2, LOG_FILE, 45, "chain rack: %s s%u knob %d: the base did not make '%s' ready (answer %d%s): the wagon is silent, and drawn so", host ? "drum" : "synth", slot, k, l->rack_name, how,
                       how == OP1F_FAILED || how == OP1F_TOO_LARGE ? ", no room: a restart frees the base's blocks" : "");
    }
}

// The slot's wagons' instances made anew, at a change of slot (a push that waits dropped) and, R14, when a ride of
// "train" begins at the slot on the screen (its extra wagons were kept as they were while it did not ride: no old tail
// plays, a pack's effect is told fresh). No block runs them meanwhile (chain_ready).
static void make_live(int host, u32 slot, int drop_push) {
    chain_ready[host] = 0;
    for (int k = 0; k < WAGONS; ++k) {
        Wagon* w = &wagons[host][slot][k];
        if (k == mirror[host][slot]) { live[host][k].obj = 0; live[host][k].rack = 0; live[host][k].rack_name[0] = 0; live[host][k].type = NONE; st.pending_type[host][k] = NONE; continue; }
        build_wagon(host, k, w, rack_name[host][slot][k]);                   // R5: a pack's effect, or the wagon's own object
        for (int i = 0; i < PARAMS; ++i) live[host][k].ctx[i] = w->params[i];
        st.pending_type[host][k] = w->type;
    }
    if (drop_push) st.push_pending[host] = 0;
    chain_ready[host] = 1;
}

// 19: a turn's stand on an effect over the ceiling, said once in the log: what it needs, and what each other wagon
// counts against the ceiling (the first wagon no less than what runs there), so that a unit's log shows the sum.
static void stand_log(int host, u32 slot) {
    static u32 said, lines;
    int knob = browse_knob[host];
    if (knob < 0 || browse_stands == said) return;
    said = browse_stands;
    if (lines >= 32u) return;
    ++lines;
    u32 c[WAGONS];
    for (int k = 0; k < WAGONS; ++k) c[k] = k == knob ? 0u : wagon_cost(host, slot, k, 1);
    STOCK_LOG(3, LOG_FILE, 48, "chain stand: %s s%u knob %d stands on '%s' (%u cycles a block), not put on: the other wagons count %u, %u, %u, %u (%s, %s, %s, %s; the first wagon knob %d) and leave it %u of %u",
              host ? "drum" : "synth", slot, knob, order_name(browse_item[host]), browse_need[host], c[0], c[1], c[2], c[3],
              knob == 0 ? "-" : wagon_word(host, slot, 0), knob == 1 ? "-" : wagon_word(host, slot, 1), knob == 2 ? "-" : wagon_word(host, slot, 2), knob == 3 ? "-" : wagon_word(host, slot, 3),
              mirror[host][slot], browse_left[host], (u32)CEILING_CYCLES);
}

static void frame_host(int host) {
    u32 slot = gui_slot(host);
    u32 began = ride_began[host];                                           // R14: the slots whose ride of "train" began at this frame (rides_frame)
    ride_began[host] = 0;
    if (!initialised[host][slot]) init_slot(host, slot);
    if (!initialised[host][slot]) return;                                   // the record not loaded yet: stock's producer runs as stock's (chain_ready stays 0)
    if (browse_knob[host] >= 0 && (!st.train_open[host] || browse_slot[host] != slot || ls.open[host])) browse_end(host);   // 19: the stand's page is gone
    stand_log(host, slot);
    // a new current slot: its chain goes live
    if (st.current_slot[host] != (int)slot) {
        if (st.current_slot[host] >= 0) switch_leave(host, (u32)st.current_slot[host]);   // 17
        st.current_slot[host] = (int)slot;
        make_live(host, slot, 1);
    } else if ((began >> slot) & 1u) {
        make_live(host, slot, 0);                                           // R14: a ride begun at the slot on the screen: its extra wagons start clean
    }
    int m = mirror[host][slot];
    Wagon* mw = &wagons[host][slot][m];
    // the mirror's own pick, after the knob's rest
    if (st.push_pending[host] && (int)st.frames - st.pending_since[host][m] >= REST_FRAMES) {
        st.push_pending[host] = 0;
        mirror_push(host, slot);
    }
    if (!st.push_pending[host]) {                                           // R5: the first wagon's wish given or held; what the unit's pick set put back
        wish_frame(host, slot);
        restore_frame(host, slot);
        m = mirror[host][slot];
        mw = &wagons[host][slot][m];
    }
    // the mirrored wagon follows stock's slot (the knobs on the effect page, a toggle); another effect in the slot
    // word means a document applied behind the module's back: reconcile
    if (!st.push_pending[host]) {
        int stock_type = slot_type(host, slot);
        if (stock_type != stock_type_seen[host][slot]) {
            stock_type_seen[host][slot] = stock_type;
            if (stock_type != mw->type) {
                ++st.mirror_syncs;
                reconcile(host, slot, "type word");
            }
        }
        m = mirror[host][slot];
        mw = &wagons[host][slot][m];
        if (mw->type >= 0 && slot_type(host, slot) == mw->type) {
            u32 current[PARAMS];
            slot_read_params(host, slot, current);
            int changed = 0;
            for (int i = 0; i < PARAMS; ++i) if (current[i] != mw->params[i]) changed = 1;
            if (changed) {                                                              // H6c: counted; one soon after a document write is something rewriting the block under the module
                ++st.param_follows;
                int early = st.doc_writes && st.ms - st.doc_ms < 2000u;
                if (early) ++st.early_follows;
                if (early && st.param_lines < 24) {
                    ++st.param_lines;
                    STOCK_LOG(3, LOG_FILE, 23, "chain params: %s s%u knob %d %s: the slot's block changed %u ms after a document write: %d %d %d %d -> %d %d %d %d",
                              host ? "drum" : "synth", slot, m, fx_name(mw->type), st.ms - st.doc_ms,
                              int_of(mw->params[0]), int_of(mw->params[1]), int_of(mw->params[2]), int_of(mw->params[3]),
                              int_of(current[0]), int_of(current[1]), int_of(current[2]), int_of(current[3]));
                }
                for (int i = 0; i < PARAMS; ++i) mw->params[i] = current[i];
                mark_dirty(host);
            }
            int active = RD8(HOST[host].fx_active + slot) ? 1 : 0;
            if (active != mw->coupled && !rack_name[host][slot][m][0]) {            // H3b: logged, the one path besides the hold and the bypass that moves a wagon (R5: not a wish held off: its coupling is kept)
                ++st.follows;
                STOCK_LOG(3, LOG_FILE, 9, "chain follow: %s s%u knob %d fx_active %d under the module, the wagon %s", host ? "drum" : "synth", slot, m, active, mw->coupled ? "coupled" : "uncoupled");
                mw->coupled = active;
                mark_dirty(host);
            }
            document_give(host, slot, m);                                       // 17: whenever the switch is found on
        }
    }
    // the other wagons' picks after the knob's rest (R5: a pack's effect put on or taken off a wagon, or another one)
    for (int k = 0; k < WAGONS; ++k) {
        if (k == m) continue;
        int type = st.pending_type[host][k];
        const char* name = rack_name[host][slot][k];
        if ((type != live[host][k].type || !name_same(name, live[host][k].rack_name)) && (int)st.frames - st.pending_since[host][k] >= REST_FRAMES) {
            if (name[0] && type >= 0) build_rack(host, k, type, name);
            else build_live(host, k, type);
            for (int i = 0; i < PARAMS; ++i) live[host][k].ctx[i] = wagons[host][slot][k].params[i];
        }
    }
    rack_frame(host, slot);                                                 // R5: the packs' effects made ready at the wagons' places
    // the holds (R14: while the train page is up)
    for (int k = 0; k < WAGONS; ++k) {
        if (st.train_open[host] && st.pressed[host][k] && st.hold_armed[host][k] && !st.acted[host][k] && (int)st.ms - st.press_ms[host][k] >= HOLD_MS) {
            st.acted[host][k] = 1;
            if (st.press_shift[host][k]) reset_params(host, k);
            else {
                ++st.holds;
                int wanted = !wagons[host][slot][k].coupled;
                set_coupled(host, k, wanted);
                int coupled = wagons[host][slot][k].coupled;                       // 17: as it stands: a coupling over the ceiling is refused
                int gap = st.press_ms[host][k] - st.last_turn_ms[host][k];         // H3b: every hold logged with the turn before it, for the read-out (H4b: in ms)
                STOCK_LOG(3, LOG_FILE, 8, "chain hold: knob %d %s, the last turn %d ms before the press", k,
                          coupled ? "coupled" : wanted ? "refused, over the ceiling" : "uncoupled", gap > 99999 ? 99999 : gap);
            }
        }
    }
    // the save, a moment after the last change (the whole file)
    if (st.dirty[host] && (int)st.frames - st.dirty_since[host] >= SAVE_DELAY_FRAMES) save_pending();
}

// Q1: what the periodic lines state, as one number for the gate (build_q_quiet.h): the wagons, their coupling and
// what is built for them, the counts of events (the knob turns on the effect and LFO pages among them, which this
// unit has counted since H6h), the presets' list. Not what the lines measure (cycles a block, the time of a period:
// those differ at every period by themselves), but for the frame rate's eighth, which moves when a page becomes
// heavy (58 frames a second at rest, 26 to 29 on the UFOs' page while playing).
static u32 lines_signature(u32 rate_whole) {
    u32 s = quiet_start();
    for (int host = 0; host < HOSTS; ++host) {
        u32 slot = gui_slot(host);
        s = quiet_mix(s, slot);
        s = quiet_mix(s, (u32)mirror[host][slot]);
        for (int i = 0; i < WAGONS; ++i) s = quiet_mix(s, (u32)knob_at(host, slot, i));
        for (int k = 0; k < WAGONS; ++k) {
            const Wagon* w = &wagons[host][slot][k];
            s = quiet_mix(s, (u32)w->type);
            s = quiet_mix(s, w->coupled ? 1u : 0u);
            s = quiet_mix(s, live[host][k].obj != 0 ? 1u : 0u);
            const char* name = cargo_name(host, slot, k);                       // R5: a pack's effect, and whether it plays
            for (int c = 0; name && c < RACK_NAME && name[c]; ++c) s = quiet_mix(s, (u32)(u8)name[c]);
            s = quiet_mix(s, live[host][k].rack);
        }
        s = quiet_mix(s, (u32)chain_ready[host]);
        s = quiet_mix(s, (u32)doc_type[host][slot]);
        s = quiet_mix(s, ls.open[host] ? 1u : 0u);
        s = quiet_mix(s, (u32)ls.cursor[host]);
        s = quiet_mix(s, (u32)ls.loaded_slot[host]);
    }
    s = quiet_mix(s, st.picks); s = quiet_mix(s, st.moves); s = quiet_mix(s, st.taps); s = quiet_mix(s, st.holds); s = quiet_mix(s, st.randoms);
    s = quiet_mix(s, st.resets); s = quiet_mix(s, st.refusals); s = quiet_mix(s, st.saves); s = quiet_mix(s, st.loads); s = quiet_mix(s, st.adopts);
    s = quiet_mix(s, st.restores); s = quiet_mix(s, st.reconciles); s = quiet_mix(s, st.builds); s = quiet_mix(s, st.page_opens); s = quiet_mix(s, st.train_opens);
    s = quiet_mix(s, st.mirror_syncs); s = quiet_mix(s, st.inits); s = quiet_mix(s, st.swallowed); s = quiet_mix(s, st.bypasses); s = quiet_mix(s, st.turn_presses);
    s = quiet_mix(s, st.follows);
    s = quiet_mix(s, (u32)presets.count); s = quiet_mix(s, ls.opens); s = quiet_mix(s, ls.saves); s = quiet_mix(s, ls.loads); s = quiet_mix(s, ls.deletes);
    s = quiet_mix(s, ls.renames); s = quiet_mix(s, ls.scans); s = quiet_mix(s, ls.rereads); s = quiet_mix(s, ls.writes); s = quiet_mix(s, ls.migrated);
    s = quiet_mix(s, st.ghosts); s = quiet_mix(s, st.clamps); s = quiet_mix(s, ls.mirror_moves);
    s = quiet_mix(s, st.doc_writes); s = quiet_mix(s, st.param_follows); s = quiet_mix(s, st.early_follows);
    s = quiet_mix(s, st.knob_turns);
    s = quiet_mix(s, rk.builds); s = quiet_mix(s, rk.rides); s = quiet_mix(s, rk.converts); s = quiet_mix(s, rk.hold_offs);   // R5
    s = quiet_mix(s, rate_whole >> 3);
    return s;
}

// The periodic lines. Q1: the wagons' and the section's statistics are read and cleared at every call as before;
// the lines go to the log when the gate says so (`why` is the power popup's reason when the frame call asks for the
// lines out of turn, else 0 and the gate is asked: a change in what the lines state, synth voices sounding, the two
// periods after either, the first three of the start, or five minutes without a line).
static void print_lines(int why) {
    u32 clock_frames = st.frames - st.frames_at_clock, clock_ms = st.ms - st.ms_at_clock;      // H4b: the frame rate over the period
    u32 rate10 = udiv(clock_frames * 10000u, clock_ms), rate_whole = udiv(rate10, 10u);
    st.frames_at_clock = st.frames;
    st.ms_at_clock = st.ms;
    if (!why) why = quiet_period(&st.quiet, lines_signature(rate_whole), &op1f_fx_voices && op1f_fx_voices != 0);
    int speak = why != 0;
    for (int host = 0; host < HOSTS; ++host) {
        u32 slot = gui_slot(host);
        int aimed = lfo_fx_knob_of(host, slot);
        if (lfo_type_word(host, slot) == (u32)LFO_TYPE_MIDI) {                 // I4: the midi LFO's four destinations this period, while one of them turned another wagon than the lamp's
            if ((lfofx_turned[host] || lfofx_idle[host]) && lfofx_period_lines < (u32)LFOFX_PERIOD_LINES) {
                ++lfofx_period_lines;
                u32 turned = lfofx_turned[host], idle = lfofx_idle[host], stock = lfofx_stock[host];
                lfofx_turned[host] = 0;
                lfofx_idle[host] = 0;
                u32 object = RD32(LFO_FX_OBJECT[host]);
                const u32* b = lfo_block_of(host, slot);
                STOCK_LOG(3, LOG_FILE, 33, "lfo fx: %s s%u midi: destinations %d %d %d %d on knobs %d %d %d %d, lamp on knob %d: %u wagon turns, %u parked or not built, %u blocks stock's way; object class %08x",
                          host ? "drum" : "synth", slot, int_of(b[4]), int_of(b[5]), int_of(b[6]), int_of(b[7]),
                          lfo_midi_knob_of(host, slot, 0), lfo_midi_knob_of(host, slot, 1), lfo_midi_knob_of(host, slot, 2), lfo_midi_knob_of(host, slot, 3),
                          (int)mirror[host][slot], turned, idle, stock, object ? RD32(object) : 0u);
            }
        } else if ((lfofx_turned[host] || lfofx_idle[host] || (aimed != mirror[host][slot] && lfofx_stock[host])) && lfofx_period_lines < (u32)LFOFX_PERIOD_LINES) {   // I2: the stock LFO's effect destination this period, while another wagon than the lamp's is chosen (the first sixty-four lines)
            ++lfofx_period_lines;
            u32 turned = lfofx_turned[host], idle = lfofx_idle[host], stock = lfofx_stock[host];
            lfofx_turned[host] = 0;
            lfofx_idle[host] = 0;
            u32 object = RD32(LFO_FX_OBJECT[host]);
            STOCK_LOG(3, LOG_FILE, 32, "lfo fx: %s s%u aims at knob %d (%s), choice %d, lamp on knob %d: %u blocks turned, %u parked or not built, %u stock's way; lfo type %u, destination %d, object class %08x",
                      host ? "drum" : "synth", slot, aimed, fx_name(wagons[host][slot][aimed].type), (int)lfo_fx_pick[host][slot], (int)mirror[host][slot], turned, idle, stock,
                      lfo_type_word(host, slot), int_of(lfo_block_of(host, slot)[LFO_DEST_WORD]), object ? RD32(object) : 0u);
        }
        lfofx_stock[host] = 0;
        char* p = put_text(st.line, host ? "chain drum: s" : "chain synth: s");
        p = put_u32(p, slot);
        p = put_text(p, " m");
        p = put_u32(p, mirror[host][slot]);
        p = put_text(p, " order ");
        for (int i = 0; i < WAGONS; ++i) p = put_u32(p, (u32)knob_at(host, slot, i));
        for (int k = 0; k < WAGONS; ++k) {
            const Wagon* w = &wagons[host][slot][k];
            p = put_text(p, " | k");
            p = put_u32(p, (u32)k);
            *p++ = ' ';
            p = put_text(p, wagon_word(host, slot, k));                     // R5: a pack's effect by its name
            p = put_text(p, w->coupled ? " on" : " off");
            volatile Stat* s = &wagon_stat[host][k];
            u32 count = s->count, sum = s->sum, max = s->max;
            s->sum = 0; s->count = 0; s->max = 0;
            if (count) {
                *p++ = ' ';
                p = put_u32(p, udiv(sum, count));
                *p++ = '/';
                p = put_u32(p, max);
            }
        }
        *p = 0;
        if (speak) STOCK_LOG(3, LOG_FILE, 1 + host, "%s", st.line);
        volatile Stat* c = &chain_stat[host];
        u32 count = c->count, sum = c->sum, max = c->max;
        c->sum = 0; c->count = 0; c->max = 0;
        u32 runs = rk.runs[host], refused = rk.refused[host];               // R5: the packs' effects' runs this period, the audio thread's counts
        rk.runs[host] = 0;
        rk.refused[host] = 0;
        if (count && speak)
            STOCK_LOG(3, LOG_FILE, 3 + host, "chain %s section: %u/%u cycles x%u, ready %d, live %d%d%d%d, doc type %d; packs' effects %u run(s), %u not taken",
                      host ? "drum" : "synth", udiv(sum, count), max, count, chain_ready[host],
                      live[host][0].obj != 0, live[host][1].obj != 0, live[host][2].obj != 0, live[host][3].obj != 0, doc_type[host][slot], runs, refused);
    }
    if (!speak) return;
    STOCK_LOG(3, LOG_FILE, 5, "chain: %u picks, %u moves, %u taps, %u holds, %u randoms, %u resets, %u refused, %u saves, %u loads, %u adopts, %u stands, %u reconciles, %u builds, %u pages, %u trains, %u syncs, %u inits, %u swallowed, %u bypasses, %u turn presses, %u follows",
              st.picks, st.moves, st.taps, st.holds, st.randoms, st.resets, st.refusals, st.saves, st.loads, st.adopts, st.restores, st.reconciles, st.builds, st.page_opens, st.train_opens, st.mirror_syncs, st.inits, st.swallowed, st.bypasses, st.turn_presses, st.follows);
    STOCK_LOG(3, LOG_FILE, 18, "presets: %d slots used, %u opens, %u saves, %u loads, %u deletes, %u renames, %u scans, %u files read, %u written, %u migrated; synth list %s cursor %d loaded slot %d; drum list %s cursor %d loaded slot %d",
              presets.count, ls.opens, ls.saves, ls.loads, ls.deletes, ls.renames, ls.scans, ls.rereads, ls.writes, ls.migrated, ls.open[0] ? "open" : "closed", ls.cursor[0], ls.loaded_slot[0], ls.open[1] ? "open" : "closed", ls.cursor[1], ls.loaded_slot[1]);
    // H6j: a line of their own, which the logger would cut off the line above at its 260 characters: the values' counts,
    // the ghosts taken as empty, the parameters held, the mirror's moves (R2: the disk mode's folders are the base's
    // business, and their counts no longer stand here)
    STOCK_LOG(3, LOG_FILE, 30, "presets values: %u ghost delays taken as empty, %u parameters held to their range, %u mirror moves",
              st.ghosts, st.clamps, ls.mirror_moves);
    // H4b: the frame rate over the period, for the read-out (the hold no longer depends on it)
    STOCK_LOG(3, LOG_FILE, 13, "chain clock: %u frames in %u ms, %u.%u a second; the hold %d ms, the clean window %d ms; %u documents written, %u param follows (%u within 2 s of a write)",
              clock_frames, clock_ms, rate_whole, rate10 - rate_whole * 10u, (int)HOLD_MS, (int)CLEAN_MS, st.doc_writes, st.param_follows, st.early_follows);
    if (st.quiet.silent_before)                                    // Q1: how long the lines were held back, and what brought them out
        STOCK_LOG(3, LOG_FILE, 35, "chain quiet: these lines after %u periods without (%s); %u periods printed and %u not since the start",
                  st.quiet.silent_before, quiet_why(why), st.quiet.spoken, st.quiet.skipped);
}

// H4: the eight private regions, one block of C heap 0 at the first frame (B0a's allocator; heap 0 is stock's own
// heap, 1.4 MiB of its 32 MiB in use); refused, the H1 rule stays and a level-1 line says so.
static void allocate_regions() {
    u32 bytes = (u32)REGION_BYTES * (u32)REGION_COUNT + (u32)REGION_ALIGN;
    region_pool = (u8*)STOCK_HEAP_MALLOC(REGION_HEAP, bytes);
    if (region_pool == 0) {
        STOCK_LOG(1, LOG_FILE, 11, "chain regions: heap %d refused %u bytes: one wagon per effect type, as before H4", (int)REGION_HEAP, bytes);
        return;
    }
    u32 base = ((u32)region_pool + REGION_ALIGN - 1u) & ~(u32)(REGION_ALIGN - 1);
    for (int h = 0; h < HOSTS; ++h)
        for (int k = 0; k < WAGONS; ++k) live[h][k].region = (u8*)(base + (u32)REGION_BYTES * (u32)(h * WAGONS + k));
    regions_ready = 1;
    STOCK_LOG(3, LOG_FILE, 12, "chain regions: %d x %u bytes at %08x (heap %d block %08x): the same effect may ride on several wagons", (int)REGION_COUNT, (u32)REGION_BYTES, base, (int)REGION_HEAP, (u32)region_pool);
}

// ---------------------------------------------------------------- R14: the rides and the starts

// The places that ride "train", once a frame from op1f_riding (it walks the cards), for the key thread and the audio
// thread. A ride begun or ended marks its host's file to be written, so that the flag reaches the file at a ride's end
// (lfo 12's PC test found that fault); a place that begins to ride lets its flag follow its ride again.
static void rides_frame() {
    u32 rides = op1f_riding(&op1f_card_fx_train, (unsigned)TRAIN_ASK);
    u32 turned = (rides ^ op1f_train_rides) & ((1u << PLACES) - 1u);
    op1f_train_rides = rides;
    for (int p = 0; p < PLACES; ++p) {
        if (!((turned >> p) & 1u)) continue;
        int host = p >> 3;
        if ((rides >> p) & 1u) {
            rd.no_flag[p] = 0;
            ride_began[host] = (u8)(ride_began[host] | (1u << (p & 7)));   // frame_host makes the extra wagons anew where it is the slot on the screen
        }
        if (!file_loaded || !known[host][p & 7]) continue;         // the base's rides at the start, before the file is read: nothing to write (a write now would put empty chains over the file)
        st.dirty[host] = 1;                                        // not mark_dirty: the chain did not change, the presets' lamp stays
        st.dirty_since[host] = (int)st.frames;
    }
}

static void ride_line(int place, const char* what, int answer) {
    if (rd.lines >= (u32)RIDE_LINES) return;
    ++rd.lines;
    STOCK_LOG(3, LOG_FILE, 47, "chain train: %s s%u: the ride a load asked for %s (answer %d, %u frames after it was due; %u started, %u given up, %u on another effect)",
              place >= (int)OP1F_PLACE_DRUM ? "drum" : "synth", (u32)place & 7u, what, answer, st.frames - rd.first_due[place], rd.started, rd.given_up, rd.others);
}

// A place whose ride the input unit's frame should start now (op1f_layer_start), or -1: due, its frame come, not
// riding yet (a riding place's due is dropped: the base's member may have started it). The walk starts after the
// place it answered last, so that one place answered OP1F_NOT_NOW holds no other back. A place that rides another
// pack's effect (op1f_rack_at) is not started: it plays that effect as saved, and its flag is written.
extern "C" int op1f_train_ride_due() asm("op1f_train_ride_due");
extern "C" int op1f_train_ride_due() {
    for (int i = 0; i < PLACES; ++i) {
        int p = (int)((rd.next + (u32)i) & (u32)(PLACES - 1));
        if (!rd.due[p]) continue;
        int host = p >> 3;
        if (place_rides(host, (u32)p & 7u)) { rd.due[p] = 0; continue; }
        if ((i32)(st.frames - rd.due_at[p]) < 0) continue;
        if (op1f_rack_at(&op1f_card_fx_train, (u32)p) != 0) {
            rd.due[p] = 0;
            ++rd.others;
            ride_line(p, "not made: the place rides another pack's effect", 0);
            st.dirty[host] = 1;                                    // its flag reaches the file
            st.dirty_since[host] = (int)st.frames;
            continue;
        }
        rd.next = (u32)(p + 1) & (u32)(PLACES - 1);
        return p;
    }
    return -1;
}

// op1f_layer_start's answer for that place: 0 it rides; OP1F_NOT_NOW due again at the next frame, until
// RIDE_TRY_FRAMES frames after it was first due; anything else given up (no flag until the place rides or loads again).
extern "C" void op1f_train_ride_answer(int place, int answer) asm("op1f_train_ride_answer");
extern "C" void op1f_train_ride_answer(int place, int answer) {
    if (!valid_place(place) || !rd.due[place]) return;
    if (answer == 0) {
        rd.due[place] = 0;
        ++rd.started;
        ride_line(place, "started", answer);
        return;
    }
    if (answer == OP1F_NOT_NOW && st.frames - rd.first_due[place] < (u32)RIDE_TRY_FRAMES) {
        rd.due_at[place] = st.frames;                              // due at the next walk, the next frame's (whether the input unit's frame asks before or after the chain's frame counts)
        return;
    }
    rd.due[place] = 0;
    rd.no_flag[place] = 1;
    ++rd.given_up;
    ride_line(place, "given up", answer);
}

extern "C" void op1f_chain_frame(u16* fb) asm("op1f_chain_frame");
extern "C" void op1f_chain_frame(u16* fb) {
    rides_frame();                                                 // R14: the places that ride "train", first
    rd.in_frame = 1;
    if (st.frames == 0) {
        allocate_regions();                                        // H4: before any instance is built
        for (int h = 0; h < HOSTS; ++h) {
            st.current_slot[h] = -1;
            for (int k = 0; k < WAGONS; ++k) st.last_turn_ms[h][k] = -100000;     // H3b: no turn yet
            ls.loaded_slot[h] = -1;                                 // H5
        }
        ls.pending_delete_slot = -1;
        ls.flash_slot = -1;
        ls.save_slot = -1;
    }
    ++st.frames;
    {                                                              // R5: the base's list of every registered effect (the same from its first frame on)
        unsigned count = 0;
        const Op1fRackEntry* list = op1f_rack_list(&op1f_card_fx_train, &count);
        rack_entry_count = list ? count : 0u;
        rack_entries = list;
    }
    u32 now = cycles();                                            // H4b: the clock, whatever the frame rate
    if (st.frames > 1) {
        st.cycle_acc += now - st.last_cycles;
        while (st.cycle_acc >= (u32)CYCLES_PER_MS) { st.cycle_acc -= (u32)CYCLES_PER_MS; ++st.ms; }
    }
    st.last_cycles = now;
    for (int h = 0; h < HOSTS; ++h)
        for (u32 s = 0; s < SLOTS; ++s) if (!initialised[h][s]) init_slot(h, s);
    order_build();                                                 // R6b (M1): the picker's order, after the base's list is read
    for (int host = 0; host < HOSTS; ++host) {                     // R14: the train page's record is op1f_train_up's (fx train 17 ended it here at a change of mode or GONE_FRAMES frames away)
        frame_host(host);
        list_frame(host);                                          // H5: the T3 hold, the red hold, the flash
    }
    (void)fb;                                                      // H2: nothing is painted over the frame; the train is the GUI's own screen (Build H's train page unit)
    if (RD8(POWER_POPUP_SHOWN) != 0) save_pending();               // the countdown popup is on the frame: save now
    probes_frame();                                                // H6i: the knobs that rested say what they reached (off since H6j)
    if (&op1f_lfopage_frame) op1f_lfopage_frame();                 // I2: the value LFO's page, its wagon marker (Build I's LFO page unit)
    if (&op1f_lfo4_frame) op1f_lfo4_frame();                       // I5: the second LFO's log lines (build_i_lfo4.cpp)
    if (&op1f_lfopresets_frame) op1f_lfopresets_frame();           // I8: the LFO presets' list: the hold of shift + T4, the red hold, the flash (Build I's LFO presets unit)
    if (&op1f_patch_frame) op1f_patch_frame();                     // I9: an instrument loaded from a file that stock did not apply, and that unit's counts (Build I's patch unit)
    rows_frame();                                                  // H6j: the descriptor rows, once
    r2_lines();                                                    // R2: what a shut store did not write
    if (quiet_popup(&st.quiet, RD8(POWER_POPUP_SHOWN) != 0)) print_lines(QUIET_WHY_POPUP);   // Q1: the lines once more before the unit is off
    if (--st.print_countdown <= 0) {
        st.print_countdown = PRINT_FRAMES;
        print_lines(0);                                            // Q1: printed when the gate says so (build_q_quiet.h)
    }
    rd.in_frame = 0;
}

// ---------------------------------------------------------------- I6: what the LFO unit (build_i_lfo4.cpp) asks of this one

// The millisecond clock of the frame call (H4b), for its holds.
extern "C" u32 op1f_chain_ms() asm("op1f_chain_ms");
extern "C" u32 op1f_chain_ms() { return st.ms; }

// J5h: this unit has read its file, so what that file holds for the LFO unit is handed over. The LFOs' own file
// (J5h's unit for the LFOs without the effect train) holds its copy against this unit's only after that.
extern "C" int op1f_chain_file_loaded() asm("op1f_chain_file_loaded");
extern "C" int op1f_chain_file_loaded() { return file_loaded; }

// The chain file is written a moment after the last change (the LFO unit's store rides in it).
extern "C" void op1f_chain_touch(int host) asm("op1f_chain_touch");
extern "C" void op1f_chain_touch(int host) {
    if (host != 0 && host != 1) return;
    st.dirty[host] = 1;                                         // not mark_dirty: the effect chain did not change, the presets list keeps its lamp
    st.dirty_since[host] = (int)st.frames;
}

// The effect train leaves the screen: the LFO train takes its place on T4.
extern "C" void op1f_chain_close_train(int host) asm("op1f_chain_close_train");
extern "C" void op1f_chain_close_train(int host) { if (host == 0 || host == 1) st.train_open[host] = 0; }

// The effect wagon stock's slot LFO turns (I2) and its midi kind's four (I4): read, or written when the lamp of the
// LFO train moves and another LFO becomes stock's.
extern "C" void op1f_chain_lfo_picks(int host, u32 slot, int write, u8* pick, u8* midi) asm("op1f_chain_lfo_picks");
extern "C" void op1f_chain_lfo_picks(int host, u32 slot, int write, u8* pick, u8* midi) {
    if ((host != 0 && host != 1) || slot >= (u32)SLOTS) return;
    if (write) {
        lfo_fx_pick[host][slot] = (u8)(*pick <= WAGONS ? *pick : 0);
        for (int i = 0; i < MIDI_DESTS; ++i) lfo_midi_pick[host][slot][i] = (u8)(midi[i] <= WAGONS ? midi[i] : 0);
    } else {
        *pick = lfo_fx_pick[host][slot];
        for (int i = 0; i < MIDI_DESTS; ++i) midi[i] = lfo_midi_pick[host][slot][i];
    }
}

// I8: the knob (0..3) of the effect wagon a choice turns now: the chosen one while it carries an effect, else the
// one under the effect train's lamp (extra_knob_of, the rule the fx side runs by). An LFO preset writes this knob
// in place of a choice of 0, so it does not depend on where the lamp stood when it was saved. R14: while the place
// does not ride "train", the first wagon's (op1f_lfofx_extra's gate), the choice kept for the next ride.
extern "C" int op1f_chain_lfo_knob(int host, u32 slot, int pick) asm("op1f_chain_lfo_knob");
extern "C" int op1f_chain_lfo_knob(int host, u32 slot, int pick) {
    if ((host != 0 && host != 1) || slot >= (u32)SLOTS) return 0;
    return place_rides(host, slot) ? extra_knob_of(host, slot, pick) : mirror[host][slot];
}
