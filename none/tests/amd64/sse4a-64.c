
/* A program to test SSE4.1/SSE4.2 instructions.
   Revisions:  Nov.208     - wrote this file
               Apr.10.2010 - added PEXTR* tests
               Apr.16.2010 - added PINS*  tests
*/

/* HOW TO COMPILE:
   gcc -m64 -g -O -Wall -o sse4-64 sse4-64.c
*/

#include <stdio.h>
#include <assert.h>
#include "tests/malloc.h"
#include <string.h>


typedef  unsigned char           V128[16];
typedef  unsigned int            UInt;
typedef  signed int              Int;
typedef  unsigned char           UChar;
typedef  unsigned long long int  ULong;


typedef
   struct {
      V128 arg1;
      V128 arg2;
      V128 res;
   }
   RRArgs;

typedef
   struct {
      V128 arg1;
      V128 res;
   }
   RMArgs;

static UChar randUChar ( void )
{
   static UInt seed = 80021;
   seed = 1103515245 * seed + 12345;
   return (seed >> 17) & 0xFF;
}

static void randV128 ( V128* v )
{
   Int i;
   for (i = 0; i < 16; i++)
      (*v)[i] = randUChar();
}

static void showV128 ( V128* v )
{
   Int i;
   for (i = 15; i >= 0; i--)
      printf("%02x", (Int)(*v)[i]);
}

static void showMaskedV128 ( V128* v, V128* mask )
{
   Int i;
   for (i = 15; i >= 0; i--)
      printf("%02x", (Int)( ((*v)[i]) & ((*mask)[i]) ));
}

static void showIIA ( char* rOrM, char* op, Int imm1,  Int imm2, RRArgs* rra, V128* rmask )
{
   printf("%s %10s $%d $%d ", rOrM, op, imm1, imm2);
   showV128(&rra->arg1);
   printf(" ");
   showMaskedV128(&rra->res, rmask);
   printf("\n");
}

static void showIIAA ( char* rOrM, char* op, Int imm1,  Int imm2, RRArgs* rra, V128* rmask )
{
   printf("%s %10s $%d $%d ", rOrM, op, imm1, imm2);
   showV128(&rra->arg1);
   printf(" ");
   showV128(&rra->arg2);
   printf(" ");
   showMaskedV128(&rra->res, rmask);
   printf("\n");
}

static void showAA ( char* rOrM, char* op, RRArgs* rra, V128* rmask )
{
   printf("%s %10s ", rOrM, op);
   showV128(&rra->arg1);
   printf(" ");
   showV128(&rra->arg2);
   printf(" ");
   showMaskedV128(&rra->res, rmask);
   printf("\n");
}

/* Note: these are little endian.  Hence first byte is the least
   significant byte of lane zero. */

/* Mask for insns where all result bits are non-approximated. */
static V128 AllMask  = { 0xFF,0xFF,0xFF,0xFF, 0xFF,0xFF,0xFF,0xFF,
                         0xFF,0xFF,0xFF,0xFF, 0xFF,0xFF,0xFF,0xFF };

#define DO_r_r(_opname, _src, _dst)  \
   {  \
      V128 _tmp;  \
      __asm__ __volatile__(  \
         "movupd (%0), %%xmm2"    "\n\t"  \
         "movupd (%1), %%xmm11"   "\n\t"  \
         _opname " %%xmm2, %%xmm11"  "\n\t"  \
         "movupd %%xmm11, (%2)" "\n"  \
         : /*out*/ : /*in*/ "r"(&(_src)), "r"(&(_dst)), "r"(&(_tmp))  \
         : "cc", "memory", "xmm2", "xmm11"  \
      );  \
      RRArgs rra;  \
      memcpy(&rra.arg1, &(_src), sizeof(V128));  \
      memcpy(&rra.arg2, &(_dst), sizeof(V128));  \
      memcpy(&rra.res,  &(_tmp), sizeof(V128));  \
      showAA("r", (_opname), &rra, &AllMask);  \
   }

#define DO_r_m(_opname, _src, _dst)  \
   {  \
      V128 _tmp;  \
      memcpy(&(_tmp), &(_dst), sizeof(V128));  \
      __asm__ __volatile__(  \
         "movupd (%0), %%xmm11"   "\n\t"  \
         _opname " %%xmm11, (%1)"  "\n\t"  \
         : /*out*/ : /*in*/ "r" (&(_src)), "r" (&(_dst)) \
         : "cc", "memory", "xmm11"  \
      );  \
      RRArgs rra;  \
      memcpy(&rra.arg1, &(_src), sizeof(V128));  \
      memcpy(&rra.arg2, &(_tmp), sizeof(V128));  \
      memcpy(&rra.res,  &(_dst), sizeof(V128));  \
      showAA("m", (_opname), &rra, &AllMask);  \
   }

#define DO_imm_imm_r_r(_opname, _imm1, _imm2, _src, _dst)  \
   {  \
      V128 _tmp;  \
      __asm__ __volatile__(  \
         "movupd (%0), %%xmm2"    "\n\t"  \
         "movupd (%1), %%xmm11"   "\n\t"  \
         _opname " $" #_imm1 ", $" #_imm2 ", %%xmm2, %%xmm11"  "\n\t"  \
         "movupd %%xmm11, (%2)" "\n"  \
         : /*out*/ : /*in*/ "r"(&(_src)), "r"(&(_dst)), "r"(&(_tmp))  \
         : "cc", "memory", "xmm2", "xmm11"                            \
      );  \
      RRArgs rra;  \
      memcpy(&rra.arg1, &(_src), sizeof(V128));  \
      memcpy(&rra.arg2, &(_dst), sizeof(V128));  \
      memcpy(&rra.res,  &(_tmp), sizeof(V128));  \
      showIIAA("r", (_opname), (_imm1), (_imm2), &rra, &AllMask);  \
   }


#define DO_imm_imm_r(_opname, _imm1, _imm2, _src)  \
   {  \
      V128 _tmp;  \
      __asm__ __volatile__(  \
         "movupd (%0), %%xmm11"   "\n\t"  \
         _opname " $" #_imm1 ", $" #_imm2 ", %%xmm11"  "\n\t"  \
         "movupd %%xmm11, (%1)" "\n"  \
         : /*out*/ : /*in*/ "r"(&(_src)), "r"(&(_tmp))  \
         : "cc", "memory", "xmm11"                            \
      );  \
      RRArgs rra;  \
      memcpy(&rra.arg1, &(_src), sizeof(V128));  \
      memcpy(&rra.res,  &(_tmp), sizeof(V128));  \
      showIIA("r", (_opname), (_imm1), (_imm2), &rra, &AllMask);  \
   }

static void test_INSERTQ ( void )
{
   V128 src, dst;
   {
      for (Int i = 0; i < 5; i++) {
         randV128(&src);
         memset(&dst[0], 0xAA,sizeof(dst));
         #define GEN(a,b) \
         if (a + b <= 64) { \
            /*immediate form*/\
            DO_imm_imm_r_r("insertq", a, b, src, dst); memset(&dst[0], 0xAA,sizeof(dst));\
            /*register form*/\
            src[8] = b; \
            src[9] = a; \
            DO_r_r("insertq", src, dst); memset(&dst[0], 0xAA,sizeof(dst));\
         }

         #define GEN_B1(a,b) GEN(a, b) GEN(a, b+1)
         #define GEN_B2(a,b) GEN_B1(a, b) GEN_B1(a, b+2)
         #define GEN_B4(a,b) GEN_B2(a, b) GEN_B2(a, b+4)
         #define GEN_B8(a,b) GEN_B4(a, b) GEN_B4(a, b+8)
         #define GEN_B16(a,b) GEN_B8(a, b) GEN_B8(a, b+16)
         #define GEN_B32(a,b) GEN_B16(a, b) GEN_B16(a, b+32)
         #define GEN_A1(a,b) GEN_B32(a, b) GEN_B32(a+1, b)
         #define GEN_A2(a,b) GEN_A1(a, b) GEN_A1(a+2, b)
         #define GEN_A4(a,b) GEN_A2(a, b) GEN_A2(a+4, b)
         #define GEN_A8(a,b) GEN_A4(a, b) GEN_A4(a+8, b)
         #define GEN_A16(a,b) GEN_A8(a, b) GEN_A8(a+16, b)
         #define GEN_A32(a,b) GEN_A16(a, b) GEN_A16(a+32, b)
         GEN_A32(0,0);
         #undef GEN
         #undef GEN_B1
         #undef GEN_B2
         #undef GEN_B4
         #undef GEN_B8
         #undef GEN_B16
         #undef GEN_B32
         #undef GEN_A1
         #undef GEN_A2
         #undef GEN_A4
         #undef GEN_A8
         #undef GEN_A16
         #undef GEN_A32
      }
   }
}


static void test_EXTRQ ( void )
{
   V128 src, op;
   {
      for (Int i = 0; i < 5; i++) {
         V128 orig;
         randV128(&orig);
         memcpy(&src[0], &orig[0], sizeof(op));

         #define GEN(a,b) \
         if (a + b <= 64) { \
            /*immediate form*/\
            DO_imm_imm_r("extrq", a, b, src); memcpy(&src[0], &orig[0], sizeof(op));\
            /*register form*/\
            op[0] = b; \
            op[1] = a; \
            DO_r_r("extrq", op, src); memcpy(&src[0], &orig[0], sizeof(op));\
         }

         #define GEN_B1(a,b) GEN(a, b) GEN(a, b+1)
         #define GEN_B2(a,b) GEN_B1(a, b) GEN_B1(a, b+2)
         #define GEN_B4(a,b) GEN_B2(a, b) GEN_B2(a, b+4)
         #define GEN_B8(a,b) GEN_B4(a, b) GEN_B4(a, b+8)
         #define GEN_B16(a,b) GEN_B8(a, b) GEN_B8(a, b+16)
         #define GEN_B32(a,b) GEN_B16(a, b) GEN_B16(a, b+32)
         #define GEN_A1(a,b) GEN_B32(a, b) GEN_B32(a+1, b)
         #define GEN_A2(a,b) GEN_A1(a, b) GEN_A1(a+2, b)
         #define GEN_A4(a,b) GEN_A2(a, b) GEN_A2(a+4, b)
         #define GEN_A8(a,b) GEN_A4(a, b) GEN_A4(a+8, b)
         #define GEN_A16(a,b) GEN_A8(a, b) GEN_A8(a+16, b)
         #define GEN_A32(a,b) GEN_A16(a, b) GEN_A16(a+32, b)
         GEN_A32(0,0);
         #undef GEN
         #undef GEN_B1
         #undef GEN_B2
         #undef GEN_B4
         #undef GEN_B8
         #undef GEN_B16
         #undef GEN_B32
         #undef GEN_A1
         #undef GEN_A2
         #undef GEN_A4
         #undef GEN_A8
         #undef GEN_A16
         #undef GEN_A32

      }
   }
}

static void test_MOVNTSS ( void )
{
   V128 src;
   for (Int i = 0; i < 20; i++) {
      randV128(&src);
      V128 dst = {0};

      DO_r_m("movntss", src, dst);
   }
}

static void test_MOVNTSD ( void )
{
   V128 src;
   for (Int i = 0; i < 20; i++) {
      randV128(&src);
      V128 dst = {0};

      DO_r_m("movntsd", src, dst);
   }
}

/* ------------ main ------------ */

int main ( int argc, char** argv )
{
   // ------ SSE 4a ------
   test_INSERTQ();
   test_EXTRQ();
   test_MOVNTSS();
   test_MOVNTSD();
   return 0;
}

