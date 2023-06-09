#include <rotary/tests/heap.h>

extern void * heap_start_vaddr;
extern void * heap_end_vaddr;

uint32 test_heap() {

    printk(LOG_DEBUG, "STARTING KERNEL HEAP TEST..\n");
    test_kmalloc();
    
    return TEST_SUCCESS;
}

int32 test_kmalloc() {
    printk(LOG_DEBUG, "Starting kmalloc() tests..\n");

    // Invalid Size: 0
    void * retval = kmalloc(0);
    if((uint32)retval >= 1) {
        printk(LOG_DEBUG, "FAIL: kmalloc() did not return an error when size=0\n");
        return TEST_FAILURE;
    } else {
        printk(LOG_DEBUG, "PASS: kmalloc() errors when size=0\n");
    }

    void * kfree_addr = heap_start_vaddr - 256;
    uint32 kfree_retval = kfree(kfree_addr);
    if(kfree_retval >= 1) {
        printk(LOG_DEBUG, "FAIL: kfree() did not error when ptr (0x%x) out of heap bounds (0x%x -> 0x%x)\n",
                kfree_addr,
                heap_start_vaddr,
                heap_end_vaddr);
        return TEST_FAILURE;
    } else {
        printk(LOG_DEBUG, "PASS: kfree() errors when ptr (0x%x) out of heap bounds (0x%x -> 0x%x)\n",
                kfree_addr,
                heap_start_vaddr,
                heap_end_vaddr);
    }

    printk(LOG_DEBUG, "All kmalloc() tests passed!\n");
    return TEST_SUCCESS;
}
