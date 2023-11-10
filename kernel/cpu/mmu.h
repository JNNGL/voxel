#pragma once

#include <stdint.h>
#include <stddef.h>

#define KERNEL_HEAP_START 0xFFFFFF0000000000UL
#define MMIO_BASE_START   0xFFFFFF1FC0000000UL
#define HIGH_MAP_REGION   0xFFFFFF8000000000UL

#define MMU_FLAG_KERNEL       0x01
#define MMU_FLAG_WRITABLE     0x02
#define MMU_FLAG_NOCACHE      0x04
#define MMU_FLAG_WRITETHROUGH 0x08
#define MMU_FLAG_SPEC         0x10
#define MMU_FLAG_WC           (MMU_FLAG_NOCACHE | MMU_FLAG_WRITETHROUGH | MMU_FLAG_SPEC)
#define MMU_FLAG_NOEXECUTE    0x20

#define MMU_GET_MAKE 0x01

#define MMU_PTR_NULL  1
#define MMU_PTR_WRITE 2

typedef struct pagemap_entry_s {
    uint64_t present : 1;
    uint64_t writable : 1;
    uint64_t user : 1;
    uint64_t writethrough : 1;
    uint64_t nocache : 1;
    uint64_t accessed : 1;
    uint64_t available0 : 1;
    uint64_t size : 1;
    uint64_t global : 1;
    uint64_t cow : 1;
    uint64_t available1 : 2;
    uint64_t page : 28;
    uint64_t reserved : 12;
    uint64_t available2 : 11;
    uint64_t noexec : 1;
} pagemap_entry_t;

extern volatile pagemap_entry_t* current_pml;

void mmu_init(void(*memory_marker)(), size_t mem_size, uintptr_t first_page);

void mmu_lock_frame(uintptr_t frame_addr);
void mmu_release_frame(uintptr_t frame_addr);
int mmu_probe_frame(uintptr_t frame_addr);
uintptr_t mmu_request_frames(int n);
uintptr_t mmu_request_frame();
void mmu_allocate_frame_ex(pagemap_entry_t* pml, uint32_t flags);
void mmu_map_frame(pagemap_entry_t* pml, uint32_t flags, uintptr_t phys_addr);
uintptr_t mmu_lookup_address(pagemap_entry_t* pml, uintptr_t virt_addr);
pagemap_entry_t* mmu_lookup_frame(uintptr_t virt_addr, int flags);
void mmu_set_directory(pagemap_entry_t* pml);
void mmu_free(pagemap_entry_t* pml);
pagemap_entry_t* mmu_clone(pagemap_entry_t* pml);
void mmu_invalidate(uintptr_t addr);
uintptr_t mmu_allocate_frame();
uintptr_t mmu_allocate_frames(int n);
pagemap_entry_t* mmu_kernel_directory();
void* mmu_from_physical(uintptr_t frame);
void* mmu_map_mmio(uintptr_t phys_addr, size_t size);
pagemap_entry_t* mmu_lookup_frame_from(pagemap_entry_t* pml, uintptr_t virt_addr);
int mmu_copy_on_write(uintptr_t address);
int mmu_validate_user(void* addr, size_t size, int flags);

size_t mmu_count_user(pagemap_entry_t* pml);
size_t mmu_total_memory();
size_t mmu_used_memory();
void* mmu_heap_base();