/* ========================================================================= */
/* x86 Paging                                                                */
/* ========================================================================= */
#pragma once

#include <rotary/types.h>
#include <rotary/logging.h>
#include <rotary/pmm.h>

/* ========================================================================= */

#undef  LOG_TAG
#define LOG_TAG "paging"

#define PAGE_SIZE 4096

#define SIZE_1K 1024
#define SIZE_1M (1024 * 1024)
#define SIZE_1G (1024 * 1024 * 1024)
#define SIZE_4K 4096
#define SIZE_4M (4096 * 1024)

#define PAGE_NOT_PRESENT 0
#define PAGE_PRESENT     1

#define PAGE_READ_ONLY   0
#define PAGE_WRITABLE    1

#define PAGE_KERNEL_ONLY     0
#define PAGE_USER_AND_KERNEL 1

#define PAGE_SIZE_4K 0
#define PAGE_SIZE_4M 1

#define PAGE_DIRECTORY_INDEX(x) (((uint32)(x) >> 22) & 0x3FF)
#define PAGE_TABLE_INDEX(x) (((uint32)(x) >> 12) & 0x3FF)

#define IS_4K_ALIGNED(value) ((value & 0xFFF) == 0)
#define ALIGN_UP_4K(value) (((value) + 0xFFF) & ~0xFFF)

/* ========================================================================= */
/* Physical Page Allocation                                                  */
/* ========================================================================= */

typedef struct {
    uint32 use_count;   // How many times is this page currently in use?
    uint32 page_number; // A unique identifier for this page.
} page;

/* ========================================================================= */
/* 32-bit Page Tables                                                        */
/* ========================================================================= */

// Note: Intel SDM, Vol. 3A 4-13 features the formats of these tables

// First 10 bits of an address index into the Page Directory
// Next 10 bits index into the 

typedef struct {
    uint32 present       : 1; // If set, the page is in physical memory.
    uint32 writable      : 1; // If set, the pages are writable. Otherwise, read-only.
    uint32 user          : 1; // If set, the pages can be accessed by usermode and kernel. If not, kernel only.
    uint32 writethrough  : 1; // If set, writethrough caching is enabled. Otherwise, writeback instead.
    uint32 cache_disable : 1; // If set, caching is disabled.
    uint32 accessed      : 1; // Set if the PDE was used in a virtual address translation.
    uint32 ignored       : 1; // Ignored 
    uint32 page_size     : 1; // If set, this PDE maps to a 4MB page. If not, it maps to a 4KB page table.
    uint32 ignored2      : 4; // Ignored
    uint32 address       : 20; // Physical address of 4KB aligned page table referenced by this entry.
} page_directory_entry;

typedef struct {
    uint32 present       : 1; // If set, the page is in physical memory.
    uint32 writable      : 1; // If set, the page are writable. Otherwise, read-only.
    uint32 user          : 1; // If set, the page can be accessed by usermode and kernel. If not, kernel only.
    uint32 writethrough  : 1; // If set, writethrough caching is enabled. Otherwise, writeback instead.
    uint32 cache_disable : 1; // If set, caching is disabled.
    uint32 accessed      : 1; // Set if the PTE was used in a virtual address translation.
    uint32 dirty         : 1; // Set if this 4KB page referenced has been written to.
    uint32 pat           : 1; // Set if PAT is enabled?
    uint32 global        : 1; // If CR4.PGE = 1, determines whether the translation is global, otherwise ignored
    uint32 ignored       : 3; // Ignored
    uint32 address       : 20; // Physical address of the 4KB page referenced by this entry
} page_table_entry;

/* ========================================================================= */

uint32 paging_load_directory(page_directory_entry * page_directory);
uint32 paging_init_directory(page_directory_entry * page_directory);
uint32 paging_init_table(page_table_entry * page_table);
uint32 paging_map(page_directory_entry * page_directory, void * virtual_addr, void * physical_addr, uint32 size);

/* ========================================================================= */
