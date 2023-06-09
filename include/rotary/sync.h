#include <stdatomic.h>
#include <rotary/types.h>

void lock(volatile atomic_flag * lock);
void unlock(volatile atomic_flag * lock);

