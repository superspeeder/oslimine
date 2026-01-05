//
// Created by andy on 1/3/26.
//

#include "phys.h"

#include "paging.h"

#include "std/list.h"
#include "std/mem.h"

#include <stddef.h>

#include "boot.h"
#include "trace.h"

/**
 * Minimum block size allowed (4KiB)
 */
#define MIN_BUDDY_BLOCK_SIZE (12)

/**
 * Number of block entries to statically allocate memory for.
 *
 * These work the same as other entries.
 */
#define NUM_STATIC_BLOCK_ALLOC 256

/**
 * Block flags
 */
enum {
    /**
     * This should be set when this specific block is allocated
     */
    BA_USED = 0b1,

    /**
     * This should be used in non-leaf blocks when there is no available space
     * (free unsets this up the tree)
     */
    BA_UNAVAILABLE = 0b10,

    /**
     * This should be used in blocks where they are either used or have a child which is.
     */
    BA_NONEMPTY = 0b100,


    // Optimization Flags
    /**
     * This indicates that there is not any free 2MiB spaces in this block.
     * This is invalid for blocks with order <= 21.
     *
     * The process for an alloc of any size includes walking back up and setting
     * this flag until reaching a node where the sibling does not have this set
     * (or is nullptr).
     *
     * This is used for optimizations since checking a densely packed tree for a
     * 2M segment when one isn't available is not ideal. Sizes larger than this
     * get exponentially easier to check for, and smaller sizes lose the benefit
     * of this due to overhead.
     */
    BA_NO_2M = 0b1000,
};

/**
 * Struct for buddy allocator blocks
 *
 * When a block is freed,
 */
typedef struct buddyalloc_block_t {
    uintptr_t            start_address;
    struct flist_node_t *parent;
    struct flist_node_t *child_left;
    struct flist_node_t *child_right;
    uint8_t              order; // which power-of-two is this block
    uint8_t              flags;
} buddyalloc_block_t;

/**
 * @brief Buddy allocator state
 */
typedef struct buddyalloc_state_t {
    /**
     * @brief List of toplevel blocks
     */
    flist_t toplevel_blocks;
} buddyalloc_state_t;

static buddyalloc_state_t buddyalloc_state;

/**
 * @brief Pool of free entries (usable memory for managing memory stuff)
 */
static flist_t buddyalloc_free_pool;

/**
 * @brief Pair of block node and list entry
 */
typedef struct buddyalloc_block_node_t {
    flist_node_t       node;
    buddyalloc_block_t block;
} buddyalloc_block_node_t;

/**
 * @brief Statically allocated block entries (used to avoid having multiple frame allocators)
 */
static buddyalloc_block_node_t buddyalloc_static_alloc[NUM_STATIC_BLOCK_ALLOC];

static void ba_init_blocknode_range(buddyalloc_block_node_t *nodes, size_t count) {
    trace_enter();
    memset(nodes, 0, NUM_STATIC_BLOCK_ALLOC * sizeof(buddyalloc_block_node_t));
    for (size_t i = 0; i < count - 1; i++) {
        buddyalloc_block_node_t *block = &nodes[i];
        buddyalloc_block_node_t *next  = &nodes[i + 1];
        block->node.next               = &(next->node);
        block->node.value              = &(block->block);
    }
    nodes[count - 1].node.value = &nodes[count - 1].block;

    flist_append(&buddyalloc_free_pool, &(nodes[0].node));
    trace_exit();
}

static void ba_alloc_new_blockpage() {
    trace_enter();
    constexpr size_t         num_items = (1ULL << 12) / sizeof(buddyalloc_block_node_t);
    buddyalloc_block_node_t *frame     = (buddyalloc_block_node_t *)palloc(1ULL << 12);
    mmap_identity((uintptr_t)frame, 1ULL << 12, nullptr);

    // Setup the page as a list
    ba_init_blocknode_range(frame, num_items);
    trace_exit();
}

static flist_node_t *ba_new_block(const uintptr_t addr, const uint8_t order) {
    trace_enter();
    if (buddyalloc_free_pool.head == nullptr) {
        ba_alloc_new_blockpage();
    }

    flist_node_t       *entry = flist_pop(&buddyalloc_free_pool);
    buddyalloc_block_t *block = entry->value;

    // zero out entry
    memset(block, 0, sizeof(buddyalloc_block_t));
    block->order         = order;
    block->start_address = addr;
    block->flags         = 0;
    block->child_left    = nullptr;
    block->child_right   = nullptr;
    trace_exit();
    return entry;
}

// This releases the block struct back into the pool
static void ba_release_block(flist_node_t *entry) {
    trace_enter();
    flist_prepend(&buddyalloc_free_pool, entry);
    trace_exit();
}

/**
 * Initialize a new toplevel block for the buddy allocator.
 *
 * @param base The base physical address of the new toplevel block
 * @param order The order of the new block (should probably be 4KiB, 2MiB, or
 * 1GiB, but that technically isn't a requirement for this to work)
 */
static void ba_init_toplevel_block(const uintptr_t base, const uint8_t order) {
    trace_enter();
    flist_node_t *node = ba_new_block(base, order);
    flist_append(&buddyalloc_state.toplevel_blocks, node);
    trace_exit();
}

static void ba_create_toplevel_from_range(uintptr_t base, const size_t len) {
    trace_enter();
    const uintptr_t end = base + len;

    while (base < end) {
        const size_t rem            = end - base;
        uint8_t      best_fit_order = 34;
        while (rem < (1ULL << best_fit_order))
            best_fit_order--;
        if (best_fit_order < 12)
            break;

        ba_init_toplevel_block(base, best_fit_order);
        base += (1ULL << best_fit_order);
    }
    trace_exit();
}

static void ba_init() {
    trace_enter();
    buddyalloc_free_pool.head             = nullptr;
    buddyalloc_state.toplevel_blocks.head = nullptr;

    // Setup statically allocated pool space.
    ba_init_blocknode_range(buddyalloc_static_alloc, NUM_STATIC_BLOCK_ALLOC);

    // Read and create blocks from the memory map
    auto const lmm = get_limine_memmap();
    for (size_t i = 0; i < lmm->entry_count; i++) {
        auto const entry = lmm->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            ba_create_toplevel_from_range(entry->base, entry->length);
        }
    }
    trace_exit();
}

static void ba_split(flist_node_t *entry) {
    trace_enter();
    buddyalloc_block_t *block = entry->value;
    // REQUIRES: block->child_left and block->child_right are both null. block->order > 12.

    const uint8_t   child_order = block->order - 1;
    const size_t    step_size   = 1ULL << child_order;
    const uintptr_t left_base   = block->start_address;
    const uintptr_t right_base  = block->start_address + step_size;

    flist_node_t *left  = ba_new_block(left_base, child_order);
    flist_node_t *right = ba_new_block(right_base, child_order);

    ((buddyalloc_block_t *)left->value)->parent  = entry;
    ((buddyalloc_block_t *)right->value)->parent = entry;

    block->child_left  = left;
    block->child_right = right;

    trace_exit();
}

static void ba_mark_used(buddyalloc_block_t *block) {
    trace_call();
    block->flags |= BA_USED;
    if (block->parent) {
        buddyalloc_block_t *parent  = block->parent->value;
        buddyalloc_block_t *sibling = parent->child_left->value == block ? parent->child_right->value : parent->child_left->value;
        if (sibling && (sibling->flags & (BA_USED | BA_UNAVAILABLE)) != 0) {
            parent->flags |= BA_UNAVAILABLE;
        }
    }
}

static void ba_mark_free(buddyalloc_block_t *block) {
    trace_call();
    block->flags &= ~BA_USED;
    buddyalloc_block_t *parent = block->parent->value;
    parent->flags &= ~BA_UNAVAILABLE;
}

static bool ba_can_allocate(buddyalloc_block_t *block) {
    trace_call();
    return !block->child_left && !block->child_right && ((block->flags & (BA_USED | BA_UNAVAILABLE)) == 0);
}

static bool ba_can_allocate_in(buddyalloc_block_t *block) {
    trace_call();
    return (block->flags & (BA_USED | BA_UNAVAILABLE)) == 0;
}

static bool ba_can_merge(buddyalloc_block_t *block) {
    trace_call();
    return !(block->flags & BA_USED) && (!block->child_left || ba_can_merge(block->child_left->value)) && (!block->child_right || ba_can_merge(block->child_right->value));
}

static void ba_merge(buddyalloc_block_t *block) {
    trace_enter();
    if (block->child_left) {
        ba_merge(block->child_left);
        ba_release_block(block->child_left);
        block->child_left = nullptr;
    }

    if (block->child_right) {
        ba_merge(block->child_right);
        ba_release_block(block->child_right);
        block->child_right = nullptr;
    }
    trace_exit();
}

static bool ba_contains(const buddyalloc_block_t *block, const uintptr_t addr) {
    trace_call();
    return addr >= block->start_address && addr < (block->start_address + (1ULL << block->order));
}

static buddyalloc_block_t *ba_tlcontaining(const uintptr_t addr) {
    trace_enter();
    for (flist_node_t *node = buddyalloc_state.toplevel_blocks.head; node != nullptr; node = node->next) {
        buddyalloc_block_t *tl = node->value;
        if (ba_contains(tl, addr)) {
            trace_exit();
            return tl;
        }
    }

    trace_exit();
    return nullptr;
}

/**
 * @brief
 *
 * @param block
 * @param laddr This address needs to be block local-compatible (basically this just means that it needs to be properly aligned to the base of the toplevel block, so that
 * bitmasking the address can be used to determine tree sides)
 * @return buddyalloc_block_t*
 */
static buddyalloc_block_t *ba_block_step_contain(const buddyalloc_block_t *block, const uintptr_t laddr) {
    trace_enter();
    const uint64_t side_bit = 1ULL << (block->order - 1);
    if (laddr & side_bit) {
        trace_exit();
        return block->child_right;
    } else {
        trace_exit();
        return block->child_left;
    }
}

static buddyalloc_block_t *ba_allocatedblock_containing(const uintptr_t addr) {
    trace_enter();
    const buddyalloc_block_t *tl    = ba_tlcontaining(addr);
    const uintptr_t           local = addr - tl->start_address;
    buddyalloc_block_t       *block = tl;
    while (!(block->flags & BA_USED)) {
        buddyalloc_block_t *cont = ba_block_step_contain(block, local);
        if (!cont) {
            trace_exit();
            return nullptr; // no allocated block containing this addr
        }
        block = cont;
    }
    trace_exit();
    return block;
}

static buddyalloc_block_t *ba_try_alloc_in(flist_node_t *entry, const size_t size) {
    buddyalloc_block_t *block = entry->value;
    trace_enter();
    if (!ba_can_allocate_in(block)) {
        trace_exit();
        return nullptr;
    }

    if (((1ULL << block->order) < size)) {
        trace_exit();
        return nullptr;
    }

    // If the block 1 order smaller isnt large enough to fit the requested size OR the current blocks order is the minimum allowed size, use the current block
    if (((1ULL << (block->order - 1)) < size || block->order == MIN_BUDDY_BLOCK_SIZE) && ba_can_allocate(block)) {
        ba_mark_used(block);
        trace_log_addr(block);
        traceint("order", block->order);
        traceinth("base", block->start_address);
        trace_exit();
        return block;
    }

    if (!block->child_left && !block->child_right) {
        ba_split(entry);
    }

    if (block->child_left) {
        buddyalloc_block_t *block_out = ba_try_alloc_in(block->child_left, size);
        if (block_out) {
            trace_exit();
            return block_out;
        }
    }

    if (block->child_right) {
        buddyalloc_block_t *block_out = ba_try_alloc_in(block->child_right, size);
        if (block_out) {
            trace_exit();
            return block_out;
        }
    }

    trace_exit();
    return nullptr;
}

static buddyalloc_block_t *ba_alloc_block(const size_t size) {
    trace_enter();
    for (flist_node_t *node = buddyalloc_state.toplevel_blocks.head; node != nullptr; node = node->next) {
        buddyalloc_block_t *alloced = ba_try_alloc_in(node, size);
        if (alloced) {
            trace_exit();
            return alloced;
        }
    }

    trace_exit();
    return nullptr;
}

void init_phys() {
    trace_enter();
    ba_init();
    trace_exit();
}

void *palloc(const size_t size) {
    trace_enter();
    buddyalloc_block_t *block = ba_alloc_block(size);
    if (block) {
        trace_exit();
        return (void *)block->start_address;
    }
    trace_exit();
    return nullptr;
}
