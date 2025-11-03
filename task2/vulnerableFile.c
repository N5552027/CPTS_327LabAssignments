/* This is the vulnerable file that we will be attacking
for task 2 of Lab 1 */

/*
 * stack-L1.c ----> 32-bit vulnerable Set-UID style program for Level 1
 * prints buffer/EBP/RET locations at runtime (no gdb needed)
 * 
 * If PROBE is set, it only prints addresses and exits
 * otherwise, it copies the contents of "badfile" int a fixed stack buffer and causes
 *      an strcpy() overflow
 * 
 * Build (32-bit, execstack, no SSP, no PIE)
 *  gcc -m32 -O0 -fno-omit-frame-pointer -z execstack -fno-stack-protector -no-pie -o stackL1 stackL1.c
 * 
 * Optional Set UID demo (requires user to be in root):
 * sudo chown root:root stack-L1 && sudo chmod 4755 stack-L1
 * 
 * Recommended for Level 1 shell spamming:
 *      sudo ln -sf /bin/zsh /bin/sh
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#ifndef BUFFER_SIZE 
#define BUFFER_SIZE 100
#endif

// Use built-in ro read the current function's fram address (EBP).
static inline void *get_ebp(void)
{
    return __builtin_fram_address(0);  
}

static void bof(const char* src)
{
    char buffer[BUFFER_SIZE];

    // compute and print buffer address, current EBP, saved RET location, and offset

    void *ebpPtr = get_ebp();                   // current EBP 
    uintptr_t buf = (uintptr_t)buffer;
    uintptr_t ebp = (uintptr_t)ebpPtr;      
    uintptr_t retloc = ebp + 4;                 // saved return address

    unsigned offset = (unsigned)(retloc - buf); // From buffer start to saved RET address

    printf("[ADDR] : BUF=0x%08lx    EBP=0x%08lx     OFFSET=%u\n\n",
        (unsigned long)buf, (unsigned long)ebp, offset);

    fflush(stdout); // precautionary flush of keyboard inputs

    // In PROBE mode, we only print addresses and return early.
    if (getenv("PROBE")) return;

    // Vulnerable copuy: overlows 'buffer' when fed with a 517 byte bad file
    strcpy(buffer, src);

    // If exploitation fails, the function returns normally and prints this:
    printf("[EXPLOITATION FAILED] -> Returning normally");
}

int main(int argc, char *argv[])
{
    // IN PROBE MODE, CALL BOF() WITH A DUMMY STRING; 
    // Probe mode does not require a file input

    if (getenv("PROBE"))
    {
        bof("DUMMY");
        return 0;
    }


    // otherwise read attacker-controlled input from "badfile"
    char str[517];

    FILE *filePointer = fopen("badfile", "rb");

    if (!filePointer) 
    {
        perror("failed to open badfile");
        return 1;
    }

    int n = (int)fread(str, 1, sizeof(str)- 1, filePointer);

    fclose(filePointer);
    str[n] = '\0';

    bof(str);
    return 0;
}