#include <stdio.h> 
#include <stdlib.h> 
#include <stdint.h>



int *FramePointer; // Frame pointer


/* ---- Option A: inline getebp(): 32-bit x86 only ----
* Option A: GCC builtin (also requires -fno-omit-frame-pointer)
*/
// static inline int* getebp(void) {
//      return (int *)__builin_frame_address(0);
// }


/* Option B: inline assembly (matches your ts.s exactly)*/
static inline int* getebp(void) {
    #if defined(__i386__)
        int* ebp;
        __asm__ __volatile__ ("movl %%ebp, %0" : "=r"(ebp));
    
    #else 
    #     error "getebp() inline asm is for 32-bit x86 (i386) only."
    #endif
}

int A(int x, int y);
int B(int x, int y);
int C(int x, int y);

static void printFrameChain(int* framePointer)
{
    printf("\n[Frame Chain]\n");
    int *currentFrame = framePointer;

    for (int frameDepth = 0; currentFrame && frameDepth < 20; ++frameDepth)
    {
        printf("\t #%02d    FP=0x%08x   prevFP=0x%08x\n",
            frameDepth, (unsigned int)currentFrame, (unsigned int)(*(unsigned int *)currentFrame));

        currentFrame = (int *)(*(unsigned int*)currentFrame);
    }

    printf("\t ... -> NULL (or limit reached)");
}

static void dump_stack_from(int *stackPointer, int maxWords)
{
    printf("\n[Stack Dump] from p=%p, %d words (4B each)\n", (void*)stackPointer, maxWords);

    for (int stackIterator = 0; stackIterator < maxWords; stackIterator++) {
        int* frameAddress = stackPointer + stackIterator;
        unsigned int frameValue = *(unsigned int*)frameAddress;

        printf("\t p[%03d] @ %p : 0x%08x \n", stackIterator, (void *)frameAddress, frameValue);
    }

}

int main (int argc, char *argv[], char *env[])
{
    int a, b, c;
    printf("[Entering Main]: \n");

    printf("&a=%08x, &b=%08x, &c=%08x\n",
        (unsigned int)&a, (unsigned int)&b, (unsigned int)&c);

    // Print argc and argv entries (values + addresses)
    printf("\n[ARGV]\n");
    printf("argc=%d\n", argc);

    for (int i = 0; i < argc; ++i)
    {
        printf("\t argv[%d] = \"%s\"   &argv[%d]=%08x    argb[%d]_ptr=%08x\n",
            i, argv[i],
        i, (unsigned int)&argv[i],
    i, (unsigned int)argv[i]);
    }

    a = 1; b = 2; c = 3;

    A(a,b);
    printf("\n\n[EXITING MAIN]");
    return 0;
}

int A(int x, int y)
{
    int d, e, f;
    printf("\n\n[ENTERING FUNCTION A]\n");
    
    printf("[A Locals]: &d=0x%08x    &e=0x%08x     &f=0x%08x\n\n",
        (unsigned int)&d, (unsigned int)&e, (unsigned int)&f);
    printf("[A Input Params]:  &x=0x%08x    &y=0x%08x",
        (unsigned int)&x, (unsigned int)&y);

    d = 4; e = 5; f = 6;

    B(d,e);

    printf("\n\n[EXITING A]");
    return 0;
}

int B (int x, int y)
{
    int g, h, i;
    printf("\n\n[ENTERING B]:\n");

    printf("[B Locals]: &g=0x%08x    &h=0x%08x     &i=0x%08x\n\n",
        (unsigned int)&g, (unsigned int)&h, (unsigned int)&i);
    printf("[B Input Params]:  &x=0x%08x    &y=0x%08x",
        (unsigned int)&x, (unsigned int)&y);


    g = 7; h = 8; i = 9;

    C(g,h);

    printf("\n\n[EXITING B]");
    return 0;
}

int C(int x, int y)
{
    int u, v, w, i, *p;

    printf("\n\n[ENTER C]:\n");
    printf("[C Locals]: &u=%08x   &v=%08x    &w=%08x    i=%08x    &p=%08x\n",
        (unsigned int)&u, (unsigned int)&v, (unsigned int)&w,
        (unsigned int)&i, (unsigned int)&p);

    printf("[C Input Parameters]: &x=%08x    &y=%08x\n",
        (unsigned int)&x, (unsigned int)&y);

    u = 10; v = 11; w = 12;

    FramePointer = getebp();

    unsigned int saved_ret = *((unsigned int*)FramePointer + 1); // *(EBP+4)

    printf("\n[Frame Pointer/Saved RET]\n");

    printf("\t FP(C.EBP) = 0x%08x\n", (unsigned int)FramePointer);
    printf("\t RET(EBP+4) = 0x%08x\n", saved_ret);

    printFrameChain(FramePointer);

    p = &u;
    dump_stack_from(p, 128);

    printf("\n\n[EXITING C]\n");
    return 0;
}