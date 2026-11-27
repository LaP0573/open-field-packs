// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Step R2: the presets' list and save screens, in the packs that show them. A sibling of Build H's presets unit,
// which stays as it was (the base packs 17 to 19 hold it): every function that dresses the two is that unit's,
// instruction for instruction. What changes is where their documents come from: made at the pack's first frame call
// from the pack's own fxpresets.svg and fxsave.svg (op1f_picture with the pack's card), each in a holder of sixteen
// words whose second word is the document and the rest zero, which is all the firmware's lookup and label maker read
// of a screen. No screen is registered (the elder unit took the free numbers 37 and 38): the two documents are shown
// as the pack's page under the base's screen, asked by the train's unit of the same pack (fx train 11's train unit, which
// asks op1f_fx_presets_document, or op1f_lfo_presets_document in lfo, for the document of a screen's number).
//
// 2026-11-27: fx train 19 [D]: the save screen's number gets 40 pixels in fx train's copy (NUMBER_WIDTH), where 24 cut
// "02" to "0" with a mark; lfo's copy keeps 24, so lfo 12 builds to the same file.
//
// This one text is compiled into fx train and, through lfo/build_r2_presets_lfo.cpp with OP1F_R2_LFO_COPY
// defined, into lfo. R15 (lfo 12): lfo's copy makes and dresses its own two documents whether fx train runs or not,
// from lfo's view alone (op1f_lfo12_presets_view, a name fx train does not know), and its op1f_presets_available
// answers for them. (lfo 11's stood back while fx train ran.) fx train's copy compiles as before.
//
// What follows is the elder unit's description, which holds but for the screens' numbers.
//
// H5: the chain presets list; H5b adds the save screen (stock's save page redrawn) and the font's lowercase. The
// chain unit keeps the presets store, the file and the key grammar and fills Op1fPresetsView every frame; this unit
// owns the two documents and dresses them: the names, the wagons, the editor's cells and the save screen's number
// through stock's own label objects, the cursor bar, the lamp, the slider bar and the caret through matrices, the
// hint rows, the boxes, the clock and the check through display bytes.
//
// What it uses of the firmware:
//   * Dynamic text is a label object, made from a placeholder part by 0x8fc25abc(screen, node, kind), which puts the
//     label in the node's place in the document and answers it; 0x8fc3aa3c(label, text, max_width) sets its text,
//     shortened with a mark while wider than max_width pixels. The glyphs take the placeholder's stroke colour. The
//     font keeps its letters at the lowercase codes, so every text this unit sets is lowercase (H5b).
//   * A label's display byte is a node's (+0x5c); this unit hides a label by an empty text. The word at +0x60 is the
//     x where its glyphs are laid out from, and 0x8fc3820c(label) answers the laid-out box (x min, x max, y min,
//     y max in Q10) by value: the buffer's address travels in P0 (H6d bound it with the buffer as a second argument
//     and crashed). The box is the glyphs without their stroke, so a single upright stroke measures 0 wide. A text
//     equal to the one set is not laid out again, so a moved origin takes effect only after an empty text (H6f).
//
// The design: the stock browser's shape, eight rows at the 22.5-pixel pitch (the ninth row's room goes to the hint
// pills), the names in ochre at x 304, the highlighted preset's four wagons in the knob colours at x 112 (one row per
// knob), the cursor bar behind the highlighted row, the lamp at the left of the loaded preset's row while the chain
// still matches it, the slider bar at the right showing the window's share of the list, the rename editor's twelve
// cells over the first row with the caret under the current one (the chain unit scrolls the edited preset to the
// top), the pills LOAD / RENAME / SAVE / DELETE on the list and CURSOR / LETTER / KEEP / CANCEL in the editor. The
// save screen (H5b): stock's save.svg with the host's box (the synth's blue or the drum's ochre) carrying the
// preset's number as a label, "saving" with the clock's spokes lit one by one while the hold runs, then "saved!" with
// the check. Texts are set only when they change (a set lays out glyphs and allocates).
// The display thread only.

#include "build_r1_card.h"
#ifdef OP1F_HOST
#include "mock_page_r2.h"                                   // the PC's stand-ins for the firmware (for tests this tree does not hold)
#endif
#include "build_h_chain.h"

typedef unsigned int u32;
typedef unsigned char u8;
typedef int i32;

typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
typedef void* (*FindNodeFn)(void* screen, const char* id, int optional);   // 0x8fc25cc0: 0 when optional and missing
typedef void (*ApplyMatrixFn)(void* node, const u32* matrix);               // 0x8c03dbbe
typedef void* (*LabelFromNodeFn)(void* screen, void* node, int kind);      // 0x8fc25abc: the node replaced by a label object in the document, the label returned
typedef void (*LabelTextFn)(void* label, const char* text, int max_width); // 0x8fc3aa3c: the text laid out, shortened with a mark while wider than max_width pixels
struct LabelBox { i32 x_min, x_max, y_min, y_max; };                       // the laid-out glyphs' box in Q10 (the layout keeps it at label + 0x9c..0xa8)
typedef LabelBox (*LabelBoundsFn)(void* label);                            // 0x8fc3820c: the box returned by value, the buffer's address in P0

#ifndef OP1F_HOST
#define STOCK_LOG ((LogFn)0x8c053a20u)
#define STOCK_FIND_NODE ((FindNodeFn)0x8fc25cc0u)
#define STOCK_APPLY_MATRIX ((ApplyMatrixFn)0x8c03dbbeu)
#define STOCK_LABEL_FROM_NODE ((LabelFromNodeFn)0x8fc25abcu)
#define STOCK_LABEL_TEXT ((LabelTextFn)0x8fc3aa3cu)
#define STOCK_LABEL_BOUNDS ((LabelBoundsFn)0x8fc3820cu)
#define RD32(a) (*(volatile u32*)(a))
#endif

// The pack's own card: fx train's, or lfo's in lfo's copy (tools/pack_build.py writes it from the recipe).
#ifdef OP1F_R2_LFO_COPY
#define OWN_CARD op1f_card_lfo
extern "C" const Op1fCard op1f_card_lfo OP1F_NAMED("op1f_card_lfo");
#else
#define OWN_CARD op1f_card_fx_train
extern "C" const Op1fCard op1f_card_fx_train OP1F_NAMED("op1f_card_fx_train");
#endif

#ifdef OP1F_R2_LFO_COPY
extern "C" void op1f_lfo12_presets_view(Op1fPresetsView* out) OP1F_NAMED("op1f_lfo12_presets_view");   // R15: the LFO presets' list and save state (build_r2_lfopresets.cpp)
#else
extern "C" void __attribute__((weak)) op1f_lfopresets_view(Op1fPresetsView* out) OP1F_NAMED("op1f_lfopresets_view");   // the LFO presets (I8): their list and save state, in the same view
// J5h: the LFOs stand without the pack `fx train`, and the chain unit's view is optional here: with that unit away
// the two documents show the LFO presets alone, and where neither unit is there nothing is made.
extern "C" void __attribute__((weak)) op1f_presets_view(int host, Op1fPresetsView* out) OP1F_NAMED("op1f_presets_view");
#endif

static const char LOG_FILE[] = "../src/main/op1f_presets.cpp";
static const u32 MODE_GUI = 0x84d21088u;

enum { LIST_ID = OP1F_PRESETS_SCREEN_ID, SAVE_ID = OP1F_SAVE_SCREEN_ID, HOSTS = 2, WAGONS = OP1F_CHAIN_WAGONS, ROWS = OP1F_PRESET_ROWS, CELLS = OP1F_PRESET_NAME, SPOKES = OP1F_SAVE_SPOKES };
enum { NODE_DISPLAY = 0x5c, MATRIX_WORDS = 9, Q = 1024, LABEL_KIND = 0, PRINT_FRAMES = 232, TEXT_MAX = 24 };
// the artwork's geometry (the artwork tool's), in Q10 pixels where marked
enum { ROW_Y0_Q = 2 * Q, PITCH_Q = 23040, CURSOR_DY_Q = 9728 };                     // the rows' top at 2 + 22.5 r; the cursor bar drawn at y -10, its top 2.5 above the glyphs: +9.5 for the first row
enum { ROW_WIDTH = 168, WAGON_WIDTH = 160, CELL_WIDTH = 14 };                      // the labels' maximum widths in pixels (304 to 472, 112 to 272, the cell pitch)
#ifdef OP1F_R2_LFO_COPY
enum { NUMBER_WIDTH = 24 };                                                          // the save screen's number, as lfo 12 has it
#else
enum { NUMBER_WIDTH = 40 };                                                          // 19: the save screen's number whole (24 cut "02" to "0" with a mark)
#endif
enum { SLIDER_TOP_Q = 2 * Q, SLIDER_LENGTH_Q = 177 * Q };                           // the slider from y 2 to 179
enum { CELL_X0_Q = 304 * Q, CELL_PITCH_Q = 14 * Q, CARET_Y_Q = 21 * Q, CARET_LENGTH_Q = 10 * Q, CARET_INSET_Q = 2 * Q };   // H6c: the caret centred under its cell ((14 - 10) / 2)

static u32 list_object[16], save_object[16];       // R2: two holders, no screens: the second word the document, the rest 0
static int list_built, save_built, screens_failed;
static u32 nodes_wanted, nodes_found, labels_made, text_sets, frames_shown, frames_period, save_frames, print_countdown;

static void* l_row[ROWS];
static void* l_wagon[WAGONS];
static void* l_cell[CELLS];
static void* l_number[HOSTS];
static char t_row[ROWS][TEXT_MAX], t_wagon[WAGONS][TEXT_MAX], t_cell[CELLS][TEXT_MAX], t_number[HOSTS][TEXT_MAX];   // the texts last set
static u8 *n_cursor, *n_lamp, *n_bar, *n_caret, *n_hints_list, *n_hints_edit;
static u8 *n_box[HOSTS], *n_clock, *n_spoke[SPOKES], *n_ok;
static int last_open_host = -1;
static int cells_logged;                                                    // H6c: the editor's letter widths logged once
static i32 cell_base[CELLS];                                                // H6d: each cell label's origin word (+0x60) as constructed, the cell's left edge
static int cell_shift[CELLS];                                               // H6d: the pixels each cell's letter was moved right to sit in the middle
enum { LABEL_ORIGIN_WORD = 0x60 / 4 };                                      // the label's x origin in pixels (the constructor's box x less the GUI offset)
static char id_buffer[24];

static char* put(char* p, const char* s) { while (*s) *p++ = *s++; return p; }

static u8* find(void* screen, const char* id) {
    ++nodes_wanted;
    u8* n = (u8*)STOCK_FIND_NODE(screen, id, 1);
    if (n) ++nodes_found;
    return n;
}

static u8* find_numbered(void* screen, const char* prefix, int number) {
    char* p = put(id_buffer, prefix);
    if (number >= 10) { *p++ = '1'; number -= 10; }                               // 0..12 (no division: the module links no runtime)
    *p++ = (char)('0' + number);
    *p = 0;
    return find(screen, id_buffer);
}

static void* label_of(void* screen, u8* node) {
    if (!node) return 0;
    void* label = STOCK_LABEL_FROM_NODE(screen, node, LABEL_KIND);
    if (label) ++labels_made;
    return label;
}

static void show(u8* n, int on) { if (n) n[NODE_DISPLAY] = (u8)(on ? 1 : 0); }

static void apply(u8* n, const u32* m) { if (n) STOCK_APPLY_MATRIX(n, m); }

static void place(u8* n, i32 tx_q, i32 ty_q) {                                     // a translation, Q10 pixels
    u32 m[MATRIX_WORDS] = { Q, 0, 0, 0, Q, 0, (u32)tx_q, (u32)ty_q, Q };
    apply(n, m);
}

static void segment(u8* n, i32 x1, i32 y1, i32 x2, i32 y2) {                      // the unit line (0,0)-(1,0) onto a bar, Q10 (flat here)
    u32 m[MATRIX_WORDS] = { (u32)(x2 - x1), (u32)(y2 - y1), 0, 0, Q, 0, (u32)x1, (u32)y1, Q };
    apply(n, m);
}

static void scale_y(u8* n, i32 s_q, i32 f_q) {                                     // y' = s·y + f, x unchanged (the slider bar cut to the window's share)
    u32 m[MATRIX_WORDS] = { Q, 0, 0, 0, (u32)s_q, 0, 0, (u32)f_q, Q };
    apply(n, m);
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

// The text on a label, only when it differs from the text last set (a set lays the glyphs out afresh). Capitals
// are lowered on the way: the OS font keeps the letters at the lowercase codes (H5b).
static void set_text(void* label, char* last, const char* text, int max_width) {
    if (!label) return;
    const char* t = text ? text : "";
    char lowered[TEXT_MAX];
    int i;
    for (i = 0; i < TEXT_MAX - 1 && t[i]; ++i) lowered[i] = (t[i] >= 'A' && t[i] <= 'Z') ? (char)(t[i] + 32) : t[i];
    lowered[i] = 0;
    int same = 1;
    for (i = 0; i < TEXT_MAX; ++i) {
        if (last[i] != lowered[i]) { same = 0; break; }
        if (!lowered[i]) break;
    }
    if (same) return;
    for (i = 0; i < TEXT_MAX; ++i) { last[i] = lowered[i]; if (!lowered[i]) break; }
    STOCK_LABEL_TEXT(label, last, max_width);
    ++text_sets;
}

static void build_list() {
    for (int i = 0; i < 16; ++i) list_object[i] = 0;
    void* document = op1f_picture(&OWN_CARD, "fxpresets.svg");    // R2: from the pack's own picture, in a holder registered nowhere
    if (document == 0) {
        STOCK_LOG(1, LOG_FILE, 1, "presets art: '%s' could not make its fxpresets.svg, the list stays off (shift + T3 does nothing)", OWN_CARD.name);
        return;
    }
    list_object[1] = (u32)document;
    void* s = list_object;
    for (int r = 0; r < ROWS; ++r) l_row[r] = label_of(s, find_numbered(s, "fxp_row", r + 1));
    for (int k = 0; k < WAGONS; ++k) l_wagon[k] = label_of(s, find_numbered(s, "fxp_wagon", k + 1));
    for (int c = 0; c < CELLS; ++c) {
        l_cell[c] = label_of(s, find_numbered(s, "fxp_cell", c + 1));
        cell_base[c] = l_cell[c] ? ((i32*)l_cell[c])[LABEL_ORIGIN_WORD] : 0;   // H6d: the origin the constructor set from the placeholder's box
        cell_shift[c] = 0;
    }
    n_cursor = find(s, "fxp_cursor");
    n_lamp = find(s, "fxp_lamp");
    n_bar = find(s, "fxp_bar");
    n_caret = find(s, "fxp_caret");
    n_hints_list = find(s, "fxp_hints_list");
    n_hints_edit = find(s, "fxp_hints_edit");
    show(n_cursor, 0); show(n_lamp, 0); show(n_caret, 0); show(n_hints_edit, 0);    // every placed node hidden until the first dressing (the parser has no display attribute)
    list_built = 1;
}

static void build_save() {
    for (int i = 0; i < 16; ++i) save_object[i] = 0;
    void* document = op1f_picture(&OWN_CARD, "fxsave.svg");       // R2: the same
    if (document == 0) {
        STOCK_LOG(1, LOG_FILE, 4, "presets art: '%s' could not make its fxsave.svg, the save screen stays off", OWN_CARD.name);
        return;
    }
    save_object[1] = (u32)document;
    void* s = save_object;
    n_box[0] = find(s, "fxs_synth");
    n_box[1] = find(s, "fxs_drum");
    l_number[0] = label_of(s, find(s, "fxs_nr_synth"));
    l_number[1] = label_of(s, find(s, "fxs_nr_drum"));
    n_clock = find(s, "fxs_clock");
    for (int i = 0; i < SPOKES; ++i) n_spoke[i] = find_numbered(s, "fxs_d", i);
    n_ok = find(s, "fxs_ok");
    show(n_box[0], 0); show(n_box[1], 0); show(n_clock, 0); show(n_ok, 0);
    save_built = 1;
}

static void build_screens() {
    if ((list_built && save_built) || screens_failed) return;
    build_list();
    build_save();
    if (!list_built || !save_built) screens_failed = 1;              // the chain unit sees no screens and shift + T3 does nothing
    STOCK_LOG(3, LOG_FILE, 2, "presets art: the list's and the save screen's documents made from '%s''s pictures (%s, %s), %u labels, %u of %u nodes found",
              OWN_CARD.name, list_built ? "list ok" : "list off", save_built ? "save ok" : "save off", labels_made, nodes_found, nodes_wanted);
}

// H6d/H6f: a cell's letter centred in its 14-pixel cell by the label's own origin (as seen on the unit after H6b,
// H6c and H6e: an i and a 1 stood at the left). When a cell's letter changes, the glyph is laid out from the cell's
// left edge (the origin the constructor set), the laid-out box is read, and the label's origin word is moved right
// by (14 - width) / 2 less the glyph's own offset from the pen, then the letter is laid out again from there. The
// box is the geometry without the stroke, so a single upright stroke measures 0 wide and is put at the cell's middle
// by the same rule (H6c skipped a zero width). H6f: the layout 0x8fc3a436 compares the new text with the label's
// string and lays nothing out again when they match (0x8fc38e48: the lengths, then the bytes), so H6e's second set
// of the same letter did nothing and the letters stayed at the edge; the label is cleared with an empty text before
// the second layout. The shift of each character is kept once measured (glyph_shift), so a letter seen before is
// laid out from the middle at once. An empty text lays out no glyph. The first dressed editor logs the shifts.
static signed char glyph_shift[128];                                        // H6f: the shift measured for each character, -1 until measured
static int glyph_shift_known;

static int measure_shift(void* label, int c) {                               // the letter laid out from the cell's left edge: the pixels it is moved right to sit in the middle
    LabelBox box = STOCK_LABEL_BOUNDS(label);                                  // by value (the buffer in P0, H6e)
    i32 width_q = box.x_max - box.x_min;
    if (width_q < 0 || width_q > CELL_PITCH_Q) return 0;                       // nothing laid out, or the shortening mark: left at the edge
    i32 cell_left_q = CELL_X0_Q + CELL_PITCH_Q * c;
    int width_px = (int)((width_q + 512) >> 10);
    int offset_px = (int)((box.x_min - cell_left_q + 512) >> 10);              // the glyph's own distance from the pen (0 for the letters seen so far)
    int shift = ((CELL_WIDTH - width_px) >> 1) - offset_px;
    return shift < 0 ? 0 : (shift > CELL_WIDTH ? CELL_WIDTH : shift);
}

static void set_cell_text(int c, const char* text) {
    void* label = l_cell[c];
    if (!label) return;
    if (!glyph_shift_known) { for (int i = 0; i < 128; ++i) glyph_shift[i] = -1; glyph_shift_known = 1; }
    const char* t = text ? text : "";
    char ch = t[0];
    if (ch >= 'A' && ch <= 'Z') ch = (char)(ch + 32);                           // set_text lowers on the way as well
    int code = (int)(u8)ch & 127;
    i32* origin = (i32*)label + LABEL_ORIGIN_WORD;
    if (!ch) {                                                                 // an empty cell: no glyph, the origin back at the edge
        *origin = cell_base[c];
        set_text(label, t_cell[c], "", CELL_WIDTH);
        cell_shift[c] = 0;
        return;
    }
    if (t_cell[c][0] == ch && t_cell[c][1] == 0) { *origin = cell_base[c] + cell_shift[c]; return; }   // unchanged: the origin where the last layout put it
    if (glyph_shift[code] >= 0) {                                              // a letter measured before: laid out from the middle at once (the text changed, so the layout runs)
        cell_shift[c] = glyph_shift[code];
        *origin = cell_base[c] + cell_shift[c];
        set_text(label, t_cell[c], t, CELL_WIDTH);
        return;
    }
    *origin = cell_base[c];
    set_text(label, t_cell[c], t, CELL_WIDTH);                                 // laid out from the cell's left edge first
    int shift = measure_shift(label, c);
    glyph_shift[code] = (signed char)shift;
    cell_shift[c] = shift;
    if (shift) {
        set_text(label, t_cell[c], "", CELL_WIDTH);                            // cleared: the layout lays nothing out again for an unchanged text (H6f)
        *origin = cell_base[c] + shift;
        set_text(label, t_cell[c], t, CELL_WIDTH);                             // laid out again from the middle
    }
}

static void log_cells_once() {
    if (cells_logged) return;
    int any = 0;
    for (int c = 0; c < CELLS; ++c) if (t_cell[c][0]) any = 1;
    if (!any) return;
    cells_logged = 1;
    STOCK_LOG(3, LOG_FILE, 5, "presets art: editor cells centred by their origins and laid out again; the shifts in px %d %d %d %d %d %d %d %d %d %d %d %d; the first cell's origin word %d (its base %d), %u text sets",
              cell_shift[0], cell_shift[1], cell_shift[2], cell_shift[3], cell_shift[4], cell_shift[5], cell_shift[6], cell_shift[7], cell_shift[8], cell_shift[9], cell_shift[10], cell_shift[11],
              l_cell[0] ? (int)((i32*)l_cell[0])[LABEL_ORIGIN_WORD] : 0, (int)cell_base[0], text_sets);
}

static void dress_list(const Op1fPresetsView& v) {
    for (int r = 0; r < ROWS; ++r) set_text(l_row[r], t_row[r], v.row[r], ROW_WIDTH);
    for (int k = 0; k < WAGONS; ++k) set_text(l_wagon[k], t_wagon[k], v.wagon[k], WAGON_WIDTH);
    for (int c = 0; c < CELLS; ++c) set_cell_text(c, v.editing ? v.cell[c] : "");    // H6d: each letter laid out from its cell's middle
    if (v.editing) log_cells_once();
    int row = v.cursor - v.first;
    int cursor_on = (v.count > 0 || v.editing) && row >= 0 && row < ROWS;
    show(n_cursor, cursor_on);
    if (cursor_on) place(n_cursor, 0, CURSOR_DY_Q + PITCH_Q * row);
    int lamp_row = v.loaded - v.first;
    int lamp_on = v.loaded >= 0 && lamp_row >= 0 && lamp_row < ROWS && !v.editing;
    show(n_lamp, lamp_on);
    if (lamp_on) place(n_lamp, 0, PITCH_Q * lamp_row);
    if (v.count > ROWS) {                                                            // the bar: the window's share of the list, from the window's start
        i32 s = (i32)udiv((u32)ROWS * Q, (u32)v.count);
        i32 start = (i32)udiv((u32)SLIDER_LENGTH_Q * (u32)v.first, (u32)v.count);
        scale_y(n_bar, s, SLIDER_TOP_Q + start - ((SLIDER_TOP_Q * s) >> 10));
    } else {
        scale_y(n_bar, Q, 0);
    }
    show(n_caret, v.editing);
    if (v.editing) {
        i32 x = CELL_X0_Q + CELL_PITCH_Q * v.caret + CARET_INSET_Q;                   // H6c: under the cell's middle
        segment(n_caret, x, CARET_Y_Q, x + CARET_LENGTH_Q, CARET_Y_Q);
    }
    show(n_hints_list, !v.editing);
    show(n_hints_edit, v.editing);
}

static void dress_save(const Op1fPresetsView& v) {
    int host = v.save_host ? 1 : 0;
    char number[4];
    int n = v.save_number < 1 ? 1 : (v.save_number > 99 ? 99 : v.save_number);
    number[0] = (char)('0' + udiv((u32)n, 10u));
    number[1] = (char)('0' + (u32)n - udiv((u32)n, 10u) * 10u);
    number[2] = 0;
    for (int h = 0; h < HOSTS; ++h) {
        show(n_box[h], h == host);
        set_text(l_number[h], t_number[h], h == host ? number : "", NUMBER_WIDTH);
    }
    int saving = v.save_state == OP1F_SAVE_SAVING;
    show(n_clock, saving);
    for (int i = 0; i < SPOKES; ++i) show(n_spoke[i], saving && i < v.save_progress);
    show(n_ok, v.save_state == OP1F_SAVE_SAVED);
}

// ---------------------------------------------------------------- the frame, the documents

// The two documents are made (R15: in lfo's copy its own, with fx train or without).
extern "C" int op1f_presets_available() OP1F_NAMED("op1f_presets_available");
extern "C" int op1f_presets_available() {
    return list_built && save_built;
}

#ifndef OP1F_R2_LFO_COPY
// R2: for lfo's copy, which answers by it while this pack runs.
extern "C" int op1f_fx_train_presets_ready() OP1F_NAMED("op1f_fx_train_presets_ready");
extern "C" int op1f_fx_train_presets_ready() { return list_built && save_built; }
#endif

// R2: the document of the list (LIST_ID) or of the save screen (SAVE_ID), for the train unit's page; 0 for any other
// number and while the two are not made.
#ifdef OP1F_R2_LFO_COPY
extern "C" void* op1f_lfo_presets_document(int id) OP1F_NAMED("op1f_lfo_presets_document");
extern "C" void* op1f_lfo_presets_document(int id) {
#else
extern "C" void* op1f_fx_presets_document(int id) OP1F_NAMED("op1f_fx_presets_document");
extern "C" void* op1f_fx_presets_document(int id) {
#endif
    if (!list_built || !save_built) return 0;
    if (id == (int)LIST_ID) return (void*)list_object[1];
    if (id == (int)SAVE_ID) return (void*)save_object[1];
    return 0;
}

extern "C" void op1f_presets_frame() OP1F_NAMED("op1f_presets_frame");
extern "C" void op1f_presets_frame() {
#ifndef OP1F_R2_LFO_COPY
    if (&op1f_presets_view == 0 && &op1f_lfopresets_view == 0) return;   // J5h: neither the train's presets nor the LFOs': nothing is made, nothing is said
#endif
    build_screens();
    if (!list_built || !save_built) return;
    int mode = (int)RD32(MODE_GUI);
    int host = (mode == 0 || mode == 1) ? mode : -1;
    if (host >= 0) {
        Op1fPresetsView v;
#ifdef OP1F_R2_LFO_COPY
        op1f_lfo12_presets_view(&v);                                     // R15: the LFO presets of the place on the screen, a synth's or a drum's
#else
        v.open = 0;                                                      // J5h: what the lines below read of it when the train's unit is away
        v.save_state = OP1F_SAVE_NONE;
        v.save_host = host;
        if (&op1f_presets_view != 0) op1f_presets_view(host, &v);
        if (host == 0 && !v.open && v.save_state == OP1F_SAVE_NONE && &op1f_lfopresets_view != 0) op1f_lfopresets_view(&v);   // I8: the LFO presets show on the same two screens (no id is free below the power popup's): while the effect presets show nothing, the LFO library's view is dressed
#endif
        if (v.open) {
            dress_list(v);
            ++frames_shown;
            ++frames_period;
            last_open_host = host;
        }
        if (v.save_state != OP1F_SAVE_NONE && v.save_host == host) {
            dress_save(v);
            ++save_frames;
        }
    }
    if (++print_countdown >= (u32)PRINT_FRAMES) {
        print_countdown = 0;
        if (frames_period) {
            STOCK_LOG(3, LOG_FILE, 3, "presets art: %u frames shown this period (%u in all), host %d, %u text sets, %u save frames", frames_period, frames_shown, last_open_host, text_sets, save_frames);
            frames_period = 0;
        }
    }
}
