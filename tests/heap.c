#include <rotary/tests/heap.h>

uint32 test_heap() {

    printk(LOG_DEBUG, "STARTING KERNEL HEAP TEST..\n");
    test_kmalloc();
    
    return TEST_SUCCESS;
}

int32 test_kmalloc() {
    printk(LOG_DEBUG, "Starting kmalloc() tests..\n");

    // Invalid Size: 0
    void * retval = kmalloc(0);
    if(retval != (void*)VMM_ERR_INVALID_SIZE) {
        printk(LOG_DEBUG, "FAIL: kmalloc() did not return an error when size=0\n");
        return TEST_FAILURE;
    }

    // Invalid Size: Negative
    retval = kmalloc(-1);
    if(retval != (void*)VMM_ERR_INVALID_SIZE) {
        printk(LOG_DEBUG, "FAIL: kmalloc() did not return an error when size is negative (returned %d)\n");
        return TEST_FAILURE;
    }

    printk(LOG_DEBUG, "All kmalloc() tests passed!\n");
    return TEST_SUCCESS;
}
