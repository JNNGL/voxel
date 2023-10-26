#include "mmu.h"

#include <lib/string.h>

#define PAGE_SHIFT     12
#define PAGE_SIZE      0x1000UL
#define PAGE_SIZE_MASK 0xFFFFFFFFFFFFF000UL
#define PAGE_LOW_MASK  0x0000000000000FFFUL

#define LARGE_PAGE_SIZE 0x200000UL

#define USER_PML_ACCESS   0x07
#define KERNEL_PML_ACCESS 0x03
#define LARGE_PAGE_BIT    0x80

#define PDP_MASK 0x3FFFFFFFUL
#define PD_MASK 0x1FFFFFUL
#define PT_MASK PAGE_LOW_MASK
#define ENTRY_MASK 0x1FF

#define PHYS_MASK 0x7FFFFFFFFFUL
#define CANONICAL_MASK 0xFFFFFFFFFFFFUL

#define bit_index(b) ((b) >> 5)
#define bit_offset(b) ((b) & 0x1F)

static volatile pagemap_entry_t* current_pml;
static volatile uint32_t* frames;
static size_t n_frames;
static size_t total_memory = 0;
static size_t unavailable_memory = 0;
static uint8_t* mem_refcounts = 0;

static uintptr_t lowest_available = 0;
static uintptr_t mmio_base_address = MMIO_BASE_START;

extern pagemap_entry_t initial_pages[3][512];
pagemap_entry_t high_base_pml[512] __attribute__((aligned(PAGE_SIZE))) = {0};
pagemap_entry_t heap_base_pml[512] __attribute__((aligned(PAGE_SIZE))) = {0};
pagemap_entry_t heap_base_pd[512] __attribute__((aligned(PAGE_SIZE))) = {0};
pagemap_entry_t heap_base_pt[512 * 3] __attribute__((aligned(PAGE_SIZE))) = {0};
pagemap_entry_t low_base_pmls[34][512] __attribute__((aligned(PAGE_SIZE))) = {0};
pagemap_entry_t twom_high_pds[64][512] __attribute__((aligned(PAGE_SIZE))) = {0};

static char* heap_start = 0;
extern char end[];

void mmu_init(void(*memory_marker)(), size_t mem_size, uintptr_t first_page) {
    current_pml = (pagemap_entry_t*) &initial_pages[0];

    asm volatile("movq %%cr0, %%rax\n"
                 "orq $0x10000, %%rax\n"
                 "movq %%rax, %%cr0\n" ::: "rax");

    *(uint64_t*) &initial_pages[0][511] = (uintptr_t) &high_base_pml | KERNEL_PML_ACCESS;
    *(uint64_t*) &initial_pages[0][510] = (uintptr_t) &heap_base_pml | KERNEL_PML_ACCESS;

    for (size_t i = 0; i < 64; ++i) {
        *(uint64_t*) &high_base_pml[i] = (uintptr_t) &twom_high_pds[i] | KERNEL_PML_ACCESS;
        for (size_t j = 0; j < 512; ++j) {
            *(uint64_t*) &twom_high_pds[i][j] = ((i << 30) + (j << 21)) | LARGE_PAGE_BIT | KERNEL_PML_ACCESS;
        }
    }

    *(uint64_t*) &low_base_pmls[0][0] = (uintptr_t) &low_base_pmls[1] | USER_PML_ACCESS;

    uintptr_t end_ptr = ((uintptr_t) &end + PAGE_LOW_MASK) & PAGE_SIZE_MASK;
    size_t low_pages = end_ptr >> PAGE_SHIFT;
    size_t pd_count = (low_pages + ENTRY_MASK) >> 9;

    for (size_t j = 0; j < pd_count; ++j) {
        *(uint64_t*) &low_base_pmls[1][j] = (uintptr_t) &low_base_pmls[j + 2] | KERNEL_PML_ACCESS;
        for (int i = 0; i < 512; ++i) {
            *(uint64_t*) &low_base_pmls[j + 2][i] = (uintptr_t) (LARGE_PAGE_SIZE * j + PAGE_SIZE * i) | KERNEL_PML_ACCESS;
        }
    }

    *(uint64_t*) &low_base_pmls[2][0] = 0;

    *(uint64_t*) &initial_pages[0][0] = (uintptr_t) &low_base_pmls[0] | USER_PML_ACCESS;

    n_frames = mem_size >> 12;
    size_t frame_bytes = bit_index(n_frames * 8);
    frame_bytes = (frame_bytes + PAGE_LOW_MASK) & PAGE_SIZE_MASK;
    first_page = (first_page + PAGE_LOW_MASK) & PAGE_SIZE_MASK;
    size_t frame_pages = frame_bytes >> 12;

    *(uint64_t*) &heap_base_pml[0] = (uintptr_t) &heap_base_pd | KERNEL_PML_ACCESS;
    *(uint64_t*) &heap_base_pd[0] = (uintptr_t) &heap_base_pt[0] | KERNEL_PML_ACCESS;
    *(uint64_t*) &heap_base_pd[1] = (uintptr_t) & heap_base_pt[512] | KERNEL_PML_ACCESS;
    *(uint64_t*) &heap_base_pd[2] = (uintptr_t) & heap_base_pt[1024] | KERNEL_PML_ACCESS;

    for (size_t i = 0; i < frame_pages; i++) {
        *(uint64_t*) &heap_base_pt[i] = (first_page + (i << 12)) | KERNEL_PML_ACCESS;
    }

    asm volatile("" ::: "memory");
    current_pml = mmu_from_physical((uintptr_t) current_pml);
    asm volatile("" ::: "memory");

    frames = (void*) ((uintptr_t) KERNEL_HEAP_START);
    memset((void*) frames, 0xFF, frame_bytes);

    memory_marker();

    size_t unavailable = 0;
    size_t available = 0;
    for (size_t i = 0; i < bit_index(n_frames); ++i) {
        for (size_t j = 0; j < 32; ++j) {
            uint32_t frame = 1U << j;
            if (frames[i] & frame) {
                ++unavailable;
            } else {
                ++available;
            }
        }
    }

    total_memory = available * 4;
    unavailable_memory = unavailable * 4;

    for (uintptr_t i = 0; i < first_page + frame_bytes; i += PAGE_SIZE) {
        mmu_lock_frame(i);
    }

    heap_start = (char*) KERNEL_HEAP_START + frame_bytes;

    size_t refcounts_size = (n_frames & PAGE_LOW_MASK) ? (n_frames + PAGE_SIZE - (n_frames & PAGE_LOW_MASK)) : n_frames;
    mem_refcounts = (void*) heap_start;

    for (uintptr_t p = (uintptr_t) mem_refcounts; p < (uintptr_t) mem_refcounts + refcounts_size; p += PAGE_SIZE) {
        pagemap_entry_t* page = mmu_lookup_frame(p, MMU_GET_MAKE);
        mmu_allocate_frame_ex(page, MMU_FLAG_WRITABLE | MMU_FLAG_KERNEL);
    }

    memset(mem_refcounts, 0, refcounts_size);
    heap_start += refcounts_size;
}

void mmu_lock_frame(uintptr_t frame_addr) {
    if (frame_addr < n_frames * PAGE_SIZE) {
        uint64_t frame = frame_addr >> 12;
        uint64_t index = bit_index(frame);
        uint64_t offset = bit_offset(frame);
        frames[index] |= (1U << offset);
        asm("" ::: "memory");
    }
}

void mmu_release_frame(uintptr_t frame_addr) {
    if (frame_addr < n_frames * PAGE_SIZE) {
        uint64_t frame = frame_addr >> PAGE_SHIFT;
        uint64_t index = bit_index(frame);
        uint32_t offset = bit_offset(frame);
        frames[index] &= ~(1U << offset);
        asm("" ::: "memory");
        if (frame < lowest_available) {
            lowest_available = frame;
        }
    }
}

int mmu_probe_frame(uintptr_t frame_addr) {
    if (frame_addr >= n_frames * PAGE_SIZE) {
        return 1;
    }

    uint64_t frame = frame_addr >> PAGE_SHIFT;
    uint64_t index = bit_index(frame);
    uint32_t offset = bit_offset(frame);
    asm("" ::: "memory");
    return !!(frames[index] & (1U << offset));
}

uintptr_t mmu_request_frames(int n) {
    for (uint64_t i = lowest_available; i < n_frames; ++i) {
        int free = 1;
        for (int j = 0; j < n; ++j) {
            if (mmu_probe_frame( PAGE_SIZE * (i + j))) {
                free = 0;
                break;
            }
        }

        if (free) {
            return i;
        }
    }

    return (uintptr_t) -1;
}

uintptr_t mmu_request_frame() {
    for (uintptr_t i = bit_index(lowest_available); i < bit_index(n_frames); ++i) {
        if (frames[i] != (uint32_t) -1) {
            for (uintptr_t j = 0; j < 32; ++j) {
                uint32_t frame = 1U << j;
                if (!(frames[i] & frame)) {
                    uintptr_t out = (i << PAGE_SHIFT) + j;
                    lowest_available = out;
                    return out;
                }
            }
        }
    }

    return (uintptr_t) -1;
}

void mmu_allocate_frame_ex(pagemap_entry_t* pml, uint32_t flags) {
    if (pml->page == 0) {
        uintptr_t index = mmu_request_frame();
        mmu_lock_frame(index << PAGE_SHIFT);
        pml->page = index;
    }

    pml->size = 0;
    pml->present = 1;
    pml->writable = (flags & MMU_FLAG_WRITABLE) ? 1 : 0;
    pml->user = (flags & MMU_FLAG_KERNEL) ? 0 : 1;
    pml->nocache = (flags & MMU_FLAG_NOCACHE) ? 1 : 0;
    pml->writethrough = (flags & MMU_FLAG_WRITETHROUGH) ? 1 : 0;
    pml->size = (flags & MMU_FLAG_SPEC) ? 1 : 0;
    pml->noexec = (flags & MMU_FLAG_NOEXECUTE) ? 1 : 0;
}

void mmu_map_frame(pagemap_entry_t* pml, uint32_t flags, uintptr_t phys_addr) {
    mmu_lock_frame(phys_addr);
    pml->page = phys_addr >> PAGE_SHIFT;
    mmu_allocate_frame_ex(pml, flags);
}

uintptr_t mmu_lookup_address(pagemap_entry_t* pml, uintptr_t virt_addr) {
    pagemap_entry_t* frame = mmu_lookup_frame_from(pml, virt_addr);
    if (!frame->present) {
        return 0;
    }

    return ((uintptr_t) frame->page << PAGE_SHIFT) | (virt_addr & PT_MASK);
}

pagemap_entry_t* mmu_lookup_frame(uintptr_t virt_addr, int flags) {
    uintptr_t real_bits = virt_addr & CANONICAL_MASK;
    uintptr_t page_addr = real_bits >> PAGE_SHIFT;
    uint32_t pml4_entry = (page_addr >> 27) & ENTRY_MASK;
    uint32_t pdp_entry = (page_addr >> 18) & ENTRY_MASK;
    uint32_t pd_entry = (page_addr >> 9) & ENTRY_MASK;
    uint32_t pt_entry = page_addr & ENTRY_MASK;

    pagemap_entry_t* root = (pagemap_entry_t*) current_pml;
    if (!root[pml4_entry].present) {
        if (!(flags & MMU_GET_MAKE)) {
            return 0;
        }

        uintptr_t new_page = mmu_request_frame() << PAGE_SHIFT;
        mmu_lock_frame(new_page);
        memset(mmu_from_physical(new_page), 0, PAGE_SIZE);
        *(uint64_t*) &root[pml4_entry] = new_page | USER_PML_ACCESS;
    }

    pagemap_entry_t* pdp = mmu_from_physical((uintptr_t) root[pml4_entry].page << PAGE_SHIFT);

    if (!pdp[pdp_entry].present) {
        if (!(flags & MMU_GET_MAKE)) {
            return 0;
        }

        uintptr_t new_page = mmu_request_frame() << PAGE_SHIFT;
        mmu_lock_frame(new_page);
        memset(mmu_from_physical(new_page), 0, PAGE_SIZE);
        *(uint64_t*) &pdp[pdp_entry] = new_page | USER_PML_ACCESS;
    }

    if (pdp[pdp_entry].size) {
        return 0;
    }

    pagemap_entry_t* pd = mmu_from_physical((uintptr_t) pdp[pdp_entry].page << PAGE_SHIFT);

    if (!pd[pd_entry].present) {
        if (!(flags & MMU_GET_MAKE)) {
            return 0;
        }

        uintptr_t new_page = mmu_request_frame() << PAGE_SHIFT;
        mmu_lock_frame(new_page);
        memset(mmu_from_physical(new_page), 0, PAGE_SIZE);
        *(uint64_t*) &pdp[pdp_entry] = new_page | USER_PML_ACCESS;
    }

    if (pd[pd_entry].size) {
        return NULL;
    }

    pagemap_entry_t* pt = mmu_from_physical((uintptr_t) pd[pd_entry].page << PAGE_SHIFT);
    return (pagemap_entry_t*) &pt[pt_entry];
}

int _refcount_increase(uintptr_t frame) {
    if (frame >= n_frames) {
        return -1;
    }

    if (mem_refcounts[frame] == 255) {
        return 1;
    }

    ++mem_refcounts[frame];
    return 0;
}

int _refcount_decrease(uintptr_t frame) {
    if (frame >= n_frames) {
        return -1;
    }

    if (mem_refcounts[frame] == 0) {
        return -1;
    }

    return --mem_refcounts[frame];
}

int _copy_page(pagemap_entry_t* in, pagemap_entry_t* out, size_t index, uintptr_t address) {
    if (in[index].writable) {
        if (mem_refcounts[in[index].page] != 0) {
            return 1;
        }

        mem_refcounts[in[index].page] = 2;
        in[index].writable = 0;
        in[index].cow = 1;
        *(uint64_t*) &out[index] = *(uint64_t*) &in[index];
        asm("" ::: "memory");
        mmu_invalidate(address);
        return 0;
    }

    if (_refcount_increase(in[index].page)) {
        char* page_in = mmu_from_physical((uintptr_t) in[index].page << PAGE_SHIFT);
        uintptr_t new_page = mmu_request_frame() << PAGE_SHIFT;
        char* page_out = mmu_from_physical(new_page);
        memcpy(page_out, page_in, PAGE_SIZE);
        *(uint64_t*) &out[index] = 0;
        out[index].present = 1;
        out[index].user = 1;
        out[index].page = new_page >> PAGE_SHIFT;
        out[index].writable = 1;
        out[index].cow = 0;
        asm("" ::: "memory");
    } else {
        *(uint64_t*) &out[index] = *(uint64_t*) &in[index];
    }

    return 0;
}

int _free_page(pagemap_entry_t* in, size_t index, uintptr_t address) {
    if (in[index].writable || _refcount_decrease(in[index].page) == 0) {
        mmu_release_frame((uintptr_t) in[index].page << PAGE_SHIFT);
    }

    return 0;
}

void mmu_set_directory(pagemap_entry_t* pml) {
    if (!pml) {
        pml = mmu_kernel_directory();
    }

    current_pml = pml;
    asm volatile("movq %0, %%cr3" : : "r"((uintptr_t) pml & PHYS_MASK));
}

void mmu_free(pagemap_entry_t* pml) {
    if (!pml) {
        return;
    }

    for (size_t i = 0; i < 256; ++i) {
        if (pml[i].present) {
            pagemap_entry_t* pdp = mmu_from_physical((uintptr_t) pml[i].page << PAGE_SHIFT);
            for (size_t j = 0; j < 512; ++j) {
                if (pdp[j].present) {
                    pagemap_entry_t* pd = mmu_from_physical((uintptr_t) pdp[j].page << PAGE_SHIFT);
                    for (size_t k = 0; k < 512; ++k) {
                        if (pd[k].present) {
                            pagemap_entry_t* pt = mmu_from_physical((uintptr_t) pd[k].page << PAGE_SHIFT);
                            for (size_t l = 0; l < 512; ++l) {
                                uintptr_t address = ((i << (9 * 3 + 12)) | (j << (9*2 + 12)) | (k << (9 + 12)) | (l << PAGE_SHIFT));
                                if (pt[l].present && pt[l].user) {
                                    _free_page(pt, l, address);
                                }
                            }
                            mmu_release_frame((uintptr_t) pd[k].page << PAGE_SHIFT);
                        }
                    }
                    mmu_release_frame((uintptr_t) pdp[j].page << PAGE_SHIFT);
                }
            }
            mmu_release_frame((uintptr_t) pml[i].page << PAGE_SHIFT);
        }
    }

    mmu_release_frame(((uintptr_t) pml) & PHYS_MASK);
}

pagemap_entry_t* mmu_clone(pagemap_entry_t* pml) {
    if (!pml) {
        pml = (pagemap_entry_t*) current_pml;
    }

    uintptr_t new_page = mmu_request_frame() << PAGE_SHIFT;
    mmu_lock_frame(new_page);
    pagemap_entry_t* pml4_out = mmu_from_physical(new_page);

    memset(&pml4_out[0], 0, 256 * sizeof(pagemap_entry_t));
    memcpy(&pml4_out[256], &pml[256], 256 * sizeof(pagemap_entry_t));

    for (size_t i = 0; i < 256; ++i) {
        if (pml[i].present) {
            pagemap_entry_t* pdp_in = mmu_from_physical((uintptr_t) pml[i].page << PAGE_SHIFT);
            uintptr_t new_page = mmu_request_frame() << PAGE_SHIFT;
            pagemap_entry_t* pdp_out = mmu_from_physical(new_page);
            memset(pdp_out, 0, 512 * sizeof(pagemap_entry_t));
            *(uint64_t*) &pml4_out[i] = new_page | USER_PML_ACCESS;

            for (size_t j = 0; j < 512; ++j) {
                if (pdp_in[j].present) {
                    pagemap_entry_t* pd_in = mmu_from_physical((uintptr_t) pdp_in[j].page << PAGE_SHIFT);
                    uintptr_t new_page = mmu_request_frame() << PAGE_SHIFT;
                    pagemap_entry_t* pd_out = mmu_from_physical(new_page);
                    memset(pd_out, 0, 512 * sizeof(pagemap_entry_t));
                    *(uint64_t*) &pdp_out[j] = new_page | USER_PML_ACCESS;

                    for (size_t k = 0; k < 512; ++k) {
                        if (pd_in[k].present) {
                            pagemap_entry_t* pt_in = mmu_from_physical((uintptr_t) pd_in[k].page << PAGE_SHIFT);
                            uintptr_t new_page = mmu_request_frame() << PAGE_SHIFT;
                            pagemap_entry_t* pt_out = mmu_from_physical(new_page);
                            memset(pt_out, 0, 512 * sizeof(pagemap_entry_t));
                            *(uint64_t*) &pdp_out[k] = new_page | USER_PML_ACCESS;

                            for (size_t l = 0; l < 512; ++l) {
                                uintptr_t address = (i << (9 * 3 + 12)) | (j << (9 * 2 + 12)) | (k << (9 + 12)) | (l << PAGE_SHIFT);
                                if (pt_in[l].present) {
                                    if (pt_in[l].user) {
                                        _copy_page(pt_in, pt_out, l, address);
                                    } else {
                                        *(uint64_t*) &pt_out[l] = *(uint64_t*) &pt_in[l];
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return pml4_out;
}

void mmu_invalidate(uintptr_t addr) {
    asm volatile("invlpg (%0)" : : "r"(addr));
}

uintptr_t mmu_allocate_frame() {
    uintptr_t index = mmu_request_frame();
    mmu_lock_frame(index << PAGE_SHIFT);
    return index;
}

uintptr_t mmu_allocate_frames(int n) {
    uintptr_t index = mmu_request_frames(n);
    for (int i = 0; i < n; ++i) {
        mmu_lock_frame((index + i) << PAGE_SHIFT);
    }

    return index;
}

pagemap_entry_t* mmu_kernel_directory() {
    return mmu_from_physical((uintptr_t) &initial_pages[0]);
}

void* mmu_from_physical(uintptr_t frame) {
    return (void*) (frame | HIGH_MAP_REGION);
}

void* mmu_map_mmio(uintptr_t phys_addr, size_t size) {
    if (size & PAGE_LOW_MASK) {
        return 0;
    }

    void* out = (void*) mmio_base_address;
    for (size_t i = 0; i < size; i += PAGE_SIZE) {
        pagemap_entry_t* page = mmu_lookup_frame(mmio_base_address + i, MMU_GET_MAKE);
        mmu_map_frame(page, MMU_FLAG_KERNEL | MMU_FLAG_WRITABLE | MMU_FLAG_NOCACHE | MMU_FLAG_WRITETHROUGH, phys_addr + i);
    }

    mmio_base_address += size;
    return out;
}

pagemap_entry_t* mmu_lookup_frame_from(pagemap_entry_t* pml, uintptr_t virt_addr) {
    uintptr_t real_bits = virt_addr & CANONICAL_MASK;
    uintptr_t page_addr = real_bits >> PAGE_SHIFT;
    uint32_t pml4_entry = (page_addr >> 27) & ENTRY_MASK;
    uint32_t pdp_entry = (page_addr >> 18) & ENTRY_MASK;
    uint32_t pd_entry = (page_addr >> 9) & ENTRY_MASK;
    uint32_t pt_entry = page_addr & ENTRY_MASK;

    if (!pml[pml4_entry].present) {
        return 0;
    }

    pagemap_entry_t* pdp = mmu_from_physical((uintptr_t) pml[pml4_entry].page << PAGE_SHIFT);
    if (!pdp[pdp_entry].present) {
        return 0;
    }

    if (pdp[pdp_entry].size) {
        return 0;
    }

    pagemap_entry_t* pd = mmu_from_physical((uintptr_t) pdp[pdp_entry].page << PAGE_SHIFT);
    if (!pd[pd_entry].present) {
        return 0;
    }

    if (pd[pd_entry].size) {
        return 0;
    }

    pagemap_entry_t* pt = mmu_from_physical((uintptr_t) pd[pd_entry].page << PAGE_SHIFT);
    return &pt[pt_entry];
}

size_t mmu_count_user(pagemap_entry_t* pml) {
    size_t out = 0;

    for (size_t i = 0; i < 256; ++i) {
        if (pml[i].present) {
            ++out;
            pagemap_entry_t* pdp = mmu_from_physical((uintptr_t) pml[i].page << PAGE_SHIFT);
            for (size_t j = 0; j < 512; ++j) {
                if (pdp[j].present) {
                    ++out;
                    pagemap_entry_t* pd = mmu_from_physical((uintptr_t) pml[j].page << PAGE_SHIFT);
                    for (size_t k = 0; k < 512; ++k) {
                        if (pd[k].present) {
                            ++out;
                            pagemap_entry_t* pt = mmu_from_physical((uintptr_t) pd[k].page << PAGE_SHIFT);
                            for (size_t l = 0; l < 512; ++l) {
                                uintptr_t address = (i << (9 * 3 + 12)) | (j << (9 * 2 + 12)) | (k << (9 + 12)) | (l << PAGE_SHIFT);
                                if (pt[l].present && pt[l].user) {
                                    ++out;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return out;
}

size_t mmu_total_memory() {
    return total_memory;
}

size_t mmu_used_memory() {
    size_t out = 0;
    for (size_t i = 0; i < bit_index(n_frames); ++i) {
        for (size_t j = 0; j < 32; ++j) {
            uint32_t frame = 1U << j;
            if (frames[i] & frame) {
                ++out;
            }
        }
    }

    return out * 4 - unavailable_memory;
}

void* mmu_heap_base() {
    return (void*) heap_start;
}