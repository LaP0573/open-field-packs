// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Build B, step B0a: the tape chunk cache moves to a six-track layout in the second C heap.
// Build F, image F1: the layout grows to eight tracks without a new memory region.
//
// Replaces stock `tape_chunks_init()` in tape_chunks.cpp (OS 1.7.0+0, 0x8fc2140c, its only caller the
// tape engine init 0x8f01cc10) through a JUMP.a at its entry (profile group "build-b0"). Stock's
// version lays out 24 slots of 4 tracks × 128 KiB in the 12 MiB fill at 0x8f021280 (the one place
// that address appears) and links them into the LRU list. B0a did the same with slots of 6 × 128 KiB (768 KiB) in an 18 MiB
// block taken once from C heap 1 (24 MiB at 0x82000000, 1 KiB in use) through stock's
// heap_malloc(index, size), aligned to 32 bytes for the eMMC DMA.
//
// Since F1 a slot holds eight tracks (1 MiB) and the 24 slots sit in two regions: slots 0-11 in the
// stock 12 MiB fill at 0x8f021280, which no code has touched since B0a (its only referrer is stock's
// slot_init, called only by the replaced function), and
// slots 12-23 in a 12 MiB heap-1 block. The slots need no common block: every stock and module
// function reaches a slot's data through the slot descriptor's own pointer and addresses track t at
// data + t × 128 KiB (stock's read_block, the L1 record function at 0x11a07cfa..0x11a07d60, the
// crossfade compute, the module's service loop, play pass and tape I/O), so only the 1 MiB inside a
// slot has to be contiguous. Both regions are 32-byte aligned (0x8f021280 = 60 × 32 from a MiB
// boundary; the heap block is rounded up).
//
// If the heap allocation fails the stock layout and base are used, the extension tracks stay off
// (op1f_cache_tracks = 4) and a warning is logged, as B0a did.

typedef unsigned int u32;
typedef unsigned short u16;
typedef short i16;
typedef int i32;

typedef void* (*HeapMallocFn)(int heap_index, u32 size);
typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
#define STOCK_HEAP_MALLOC ((HeapMallocFn)0x8fcc5a68u)
#define STOCK_LOG ((LogFn)0x8c053a20u)

struct Slot {                 // tape_chunks.cpp slot descriptor, 28 bytes
    unsigned char valid;
    unsigned char stale;
    unsigned char write_pending;
    unsigned char pad;
    u16 dirty_tracks;
    i16 logical_chunk;
    u16 index;
    u16 pad2;
    i32 frame_pos;
    unsigned char* data;
    Slot* next;
    Slot* prev;
};

#define SLOTS ((Slot*)0x84c68ae8u)                       // 24 descriptors
#define LRU_HEAD (*(volatile Slot**)0x848ced58u)
#define LRU_TAIL (*(volatile Slot**)0x848ced5cu)
#define SLOT_BY_LOGICAL ((volatile Slot**)0x848ced60u)   // 969 entries
#define INIT_FLAG (*(volatile unsigned char*)0x848cfc84u)

enum {
    SLOT_COUNT = 24,
    CHUNKS_PER_TRACK = 969,
    TRACK_BYTES = 0x20000,
    EIGHT_TRACKS = 8,
    STOCK_TRACKS = 4,
    STOCK_REGION_SLOTS = 12,             // the 12 MiB stock fill holds twelve 1 MiB slots
    HEAP_INDEX = 1,
    ALIGN = 32
};
static const u32 STOCK_CACHE_BASE = 0x8f021280u;
static const char LOG_FILE[] = "../src/dsp/op1f_tape_cache.cpp";

// op1f_cache_tracks is read by the storage, play and key units (8 with both regions, 4 after the fallback).
// op1f_cache_base is the heap-1 block (slots 12-23); the stock region holds slots 0-11.
extern "C" unsigned char* op1f_cache_base asm("op1f_cache_base");
extern "C" u32 op1f_cache_tracks asm("op1f_cache_tracks");
unsigned char* op1f_cache_base;
u32 op1f_cache_tracks;

static void allocate_cache() {
    if (op1f_cache_base) return;                     // stock calls init once; keep the block if it ever runs again
    u32 bytes = (SLOT_COUNT - STOCK_REGION_SLOTS) * EIGHT_TRACKS * TRACK_BYTES;   // 12 MiB
    unsigned char* block = (unsigned char*)STOCK_HEAP_MALLOC(HEAP_INDEX, bytes + ALIGN);
    if (block) {
        u32 address = (u32)block;
        address = (address + ALIGN - 1) & ~(u32)(ALIGN - 1);
        op1f_cache_base = (unsigned char*)address;
        op1f_cache_tracks = EIGHT_TRACKS;
        STOCK_LOG(3, LOG_FILE, 1, "tape cache: %d slots x %d tracks: %d at 0x%08x (stock region), %d at 0x%08x (heap %d block 0x%08x, %u bytes)",
                  SLOT_COUNT, EIGHT_TRACKS, STOCK_REGION_SLOTS, STOCK_CACHE_BASE, SLOT_COUNT - STOCK_REGION_SLOTS,
                  address, HEAP_INDEX, (u32)block, bytes + ALIGN);
    } else {
        op1f_cache_base = (unsigned char*)STOCK_CACHE_BASE;
        op1f_cache_tracks = STOCK_TRACKS;
        STOCK_LOG(1, LOG_FILE, 2, "tape cache: heap %d allocation of %u bytes failed, stock 4-track layout kept",
                  HEAP_INDEX, bytes + ALIGN);
    }
}

static unsigned char* slot_data(u32 i) {
    u32 stride = op1f_cache_tracks * TRACK_BYTES;    // 1 MiB with eight tracks, 512 KiB after the fallback
    if (op1f_cache_tracks != EIGHT_TRACKS) return (unsigned char*)STOCK_CACHE_BASE + i * stride;   // stock's layout
    if (i < STOCK_REGION_SLOTS) return (unsigned char*)STOCK_CACHE_BASE + i * stride;
    return op1f_cache_base + (i - STOCK_REGION_SLOTS) * stride;
}

extern "C" void op1f_tape_chunks_init() asm("op1f_tape_chunks_init");
extern "C" void op1f_tape_chunks_init() {
    INIT_FLAG = 0;
    allocate_cache();
    LRU_HEAD = 0;
    LRU_TAIL = 0;
    Slot* previous = 0;
    for (u32 i = 0; i < SLOT_COUNT; ++i) {
        Slot* slot = SLOTS + i;
        slot->valid = 0;
        slot->stale = 0;
        slot->write_pending = 0;
        slot->pad = 0;
        slot->dirty_tracks = 0;
        slot->logical_chunk = -1;
        slot->index = (u16)i;
        slot->pad2 = 0;
        slot->frame_pos = 0;
        slot->data = slot_data(i);
        slot->next = 0;
        slot->prev = previous;
        if (previous) previous->next = slot; else LRU_HEAD = slot;
        LRU_TAIL = slot;
        previous = slot;
    }
    for (u32 l = 0; l < CHUNKS_PER_TRACK; ++l) SLOT_BY_LOGICAL[l] = 0;
    INIT_FLAG = 1;
}
