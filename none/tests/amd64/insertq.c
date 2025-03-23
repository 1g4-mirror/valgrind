
#include <stdio.h>

typedef  unsigned long long int  ULong;
typedef  unsigned int            UInt;
typedef  unsigned char            UChar;

const UChar numbits = 8;
const UChar offset = 24;

__attribute__((noinline))
void do_insertq (ULong* data1, ULong* data2 /* OUTPUT */)
{
    asm(
        "movdqu %0, %%xmm0\n"
        "movdqu %1, %%xmm1\n"
        "insertq %3, %2, %%xmm1, %%xmm0\n"
        "movdqu %%xmm0, %0\n"
        "movdqu %%xmm1, %1\n"
        : "=m" (*data2), "=m" (*data1) : "i" (numbits), "i" (offset) : "cc", "memory", "xmm0", "xmm1");
}

int main ( void )
{
    ULong data1[2] = {0x00000000000000AA, 0x1234567890ABCDEF};
    ULong data2[2] = {0xFFFFFFFFFFFFFFFF, 0x1234567890ABCDEF};

    printf("insertq [%016llx %016llx] [%016llx %016llx] %u %u\n", data1[0], data1[1], data2[0], data2[1], numbits, offset);
    do_insertq(&data1[0], &data2[0]);
    printf("insertq [%016llx %016llx] [%016llx %016llx] %u %u\n", data1[0], data1[1], data2[0], data2[1], numbits, offset);
    if (data1[1] != 0x1234567890ABCDEF) {
        fprintf(stderr, "insertq failed1\n");
        return 1;
    };

    if (data1[0] != 0x00000000000000AA) {
        fprintf(stderr, "insertq failed3\n");
        return 1;
    };

    if (data2[0] != 0xFFFFFFFFAAFFFFFF) {
        fprintf(stderr, "insertq failed4 %08llx\n", data2[1]);
        return 1;
    };

    return 0;
}
