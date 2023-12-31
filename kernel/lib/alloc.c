#include "alloc.h"

#include <cpu/mmu.h>
#include <lib/string.h>
#include <lib/terminal.h>

/*   This segment-based allocator is unstable and causes page faults
 *   that are difficult to debug, so I postponed it until better times.
typedef struct heap_seg_hdr_s {
    size_t length;
    struct heap_seg_hdr_s* next;
    struct heap_seg_hdr_s* last;
    uint8_t free;
} heap_seg_hdr_t;

static void* heap_start;
static void* heap_end;
static heap_seg_hdr_t* last_header;

void heap_combine_forward(heap_seg_hdr_t* seg) {
    if (seg->next == 0) {
        return;
    }

    if (!seg->next->free) {
        return;
    }

    if (seg->next == last_header) {
        last_header = seg;
    }

    if (seg->next->next != 0) {
        seg->next->next->last = seg;
    }

    seg->length += seg->next->length + sizeof(heap_seg_hdr_t);
    seg->next = seg->next->next;
}

void heap_combine_backward(heap_seg_hdr_t* seg) {
    if (seg->last != 0 && seg->last->free) {
        heap_combine_forward(seg->last);
    }
}

heap_seg_hdr_t* heap_combine_split(heap_seg_hdr_t* seg, size_t length) {
    if (length < 0x10) {
        return 0;
    }

    size_t split_seg_length = seg->length - length - sizeof(heap_seg_hdr_t);
    if (split_seg_length < 0x10) {
        return 0;
    }

    heap_seg_hdr_t* new_header = (heap_seg_hdr_t*) ((size_t) seg + length + sizeof(heap_seg_hdr_t));
    if (seg->next) {
        seg->next->last = new_header;
    }
    new_header->next = seg->next;
    seg->next = new_header;
    new_header->last = seg;
    new_header->length = split_seg_length;
    new_header->free = seg->free;
    seg->length = length;

    if (last_header == seg) {
        last_header = new_header;
    }

    return new_header;
}

void heap_init(size_t page_count) {
    uintptr_t virtual_address = (uintptr_t) mmu_heap_base();
    for (size_t i = 0; i < page_count; i++) {
        pagemap_entry_t* page = mmu_lookup_frame(virtual_address, MMU_GET_MAKE);
        mmu_allocate_frame_ex(page, MMU_FLAG_WRITABLE | MMU_FLAG_KERNEL);
        virtual_address += 0x1000;
    }

    size_t heap_length = page_count * 0x1000;
    heap_start = mmu_heap_base();
    heap_end = (void*) ((size_t) heap_start + heap_length);
    heap_seg_hdr_t* start_seg = (heap_seg_hdr_t*) mmu_from_physical((uintptr_t) heap_start);
    start_seg->length = heap_length - sizeof(heap_seg_hdr_t);
    start_seg->next = 0;
    start_seg->last = 0;
    start_seg->free = 1;
    last_header = start_seg;
}

void heap_expand(size_t length) {
    if (length % 0x1000) {
        length -= length % 0x1000;
        length += 0x1000;
    }

    size_t page_count = length / 0x1000;
    heap_seg_hdr_t* new_segment = (heap_seg_hdr_t*) mmu_from_physical((uintptr_t) heap_end);
    for (size_t i = 0; i < page_count; i++) {
        pagemap_entry_t* page = mmu_lookup_frame((uintptr_t) heap_end, MMU_GET_MAKE);
        mmu_allocate_frame_ex(page, MMU_FLAG_WRITABLE | MMU_FLAG_KERNEL);
        heap_end = (void*) ((size_t) heap_end + 0x1000);
    }

    new_segment->free = 1;
    new_segment->last = last_header;
    last_header->next = new_segment;
    last_header = new_segment;
    new_segment->next = 0;
    new_segment->length = length - sizeof(heap_seg_hdr_t);
    heap_combine_backward(new_segment);
}

void* kmalloc(size_t size) {
    if (size % 0x10) {
        size -= (size % 0x10);
        size += 0x10;
    }

    if (size == 0) {
        return 0;
    }

    heap_seg_hdr_t* current_seg = (heap_seg_hdr_t*) mmu_from_physical((uintptr_t) heap_start);
    while (current_seg) {
        if (current_seg->free) {
            if (current_seg->length > size) {
                heap_combine_split(current_seg, size);
                current_seg->free = 0;
                return (void*) (current_seg + 1);
            } else if (current_seg->length == size) {
                current_seg->free = 0;
                return (void*) (current_seg + 1);
            }
        }

        current_seg = current_seg->next;
    }

    heap_expand(size);
    return kmalloc(size);
}
*/

#define ALLOC_MAGIC 0xA77C

struct alloc_info {
    uint16_t magic;
    uintptr_t ptr;
    size_t size;
};

void heap_init(size_t page_count) {
    // For compatibility.
}

// TODO: Write a better allocator
void* malloc(size_t size) {
    size_t unaligned_size = size;
    size += sizeof(struct alloc_info);
    if (size % 0x1000) {
        size -= size % 0x1000;
        size += 0x1000;
    }

    size_t pages = size / 0x1000;
    void* ptr = (void*) (mmu_request_frames(pages) << 12);
    for (size_t i = 0; i < pages; i++) {
        mmu_lock_frame((uintptr_t) ptr + i * 0x1000);
    }

    struct alloc_info* vptr = mmu_from_physical((uintptr_t) ptr);
    vptr->magic = ALLOC_MAGIC;
    vptr->ptr = (uintptr_t) ptr;
    vptr->size = unaligned_size;

    return vptr + 1;

//    return kmalloc(size); // FIXME
}

void* realloc(void* ptr, size_t s) {
    // TODO: Better implementation
    if (!ptr) {
        return 0;
    }

    if (!s) {
        free(ptr);
        return 0;
    }

    void* new = malloc(s);
//    heap_seg_hdr_t* segment = (heap_seg_hdr_t*) ptr - 1; // FIXME
//    size_t to_copy = segment->length < s ? segment->length : s;
    struct alloc_info* info = (struct alloc_info*) ptr - 1;
    if (info->magic != ALLOC_MAGIC) {
        puts("realloc: invalid pointer");
        return 0;
    }
    memcpy(new, ptr, info->size < s ? info->size : s);
    free(ptr);
    return new;
}

void free(void* ptr) {
    if (!ptr) {
        return;
    }

    struct alloc_info* info = (struct alloc_info*) ptr - 1;
    if (info->magic != ALLOC_MAGIC) {
        puts("free: invalid pointer");
        return;
    }

    size_t phys = info->ptr;
    size_t size = info->size + sizeof(struct alloc_info);
    if (size % 0x1000) {
        size -= size % 0x1000;
        size += 0x1000;
    }

    size_t pages = size / 0x1000;
    for (size_t i = 0; i < pages; i++) {
        mmu_release_frame(phys + i * 0x1000);
    }

//    if (!address) { // FIXME
//        return;
//    }
//
//    heap_seg_hdr_t* segment = (heap_seg_hdr_t*) address - 1;
//    if (segment->free) {
//        puts("free: invalid segement");
//        return;
//    }
//    segment->free = 1;
//    heap_combine_forward(segment);
//    heap_combine_backward(segment);
}