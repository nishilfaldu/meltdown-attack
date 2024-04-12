// Task 4

#include <stdio.h>

int main()
{
    // replace the hex with address of secret from the previous task
    char *kernel_data_addr = (char *)0xfab22000;
    char kernel_data = *kernel_data_addr;
    printf("I have reached here.\n");
    return 0;
}
