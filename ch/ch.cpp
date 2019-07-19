#include "ch.h"

// poor man's LTO >:-D

#include "ch_c_intf.hpp"
#include "ch_cpuid.hpp"
#include "ch_evaluate.hpp"
#include "ch_genmoves_impl.hpp"
#include "ch_init.hpp"
#include "ch_position.hpp"
#include "ch_print_bbs.hpp"
#include "ch_search.hpp"

#ifdef _MSC_VER

extern "C"
{

#if CH_ARCH_32BIT

__declspec(naked) void __cdecl _allshl()
{
    __asm
    {
        ;
        ; Handle shifts of 64 or more bits(all get 0)
            ;
        cmp     cl, 64
            jae     short RETZERO

            ;
        ; Handle shifts of between 0 and 31 bits
            ;
        cmp     cl, 32
            jae     short MORE32
            shld    edx, eax, cl
            shl     eax, cl
            ret

            ;
        ; Handle shifts of between 32 and 63 bits
            ;
    MORE32:
        mov     edx, eax
            xor     eax, eax
            and     cl, 31
            shl     edx, cl
            ret

            ;
        ; return 0 in edx : eax
            ;
    RETZERO:
        xor     eax, eax
            xor     edx, edx
            ret
    }
}

__declspec(naked) void __cdecl _allshr()
{
    __asm
    {
        ;
        ; Handle shifts of 64 bits or more(if shifting 64 bits or more, the result
            ; depends only on the high order bit of edx).
            ;
        cmp     cl, 64
            jae     short RETSIGN

            ;
        ; Handle shifts of between 0 and 31 bits
            ;
        cmp     cl, 32
            jae     short MORE32
            shrd    eax, edx, cl
            sar     edx, cl
            ret

            ;
        ; Handle shifts of between 32 and 63 bits
            ;
    MORE32:
        mov     eax, edx
            sar     edx, 31
            and cl, 31
            sar     eax, cl
            ret

            ;
        ; Return double precision 0 or -1, depending on the sign of edx
            ;
    RETSIGN:
        sar     edx, 31
            mov     eax, edx
            ret
    }
}

__declspec(naked) void __cdecl _aullshr()
{
    __asm
    {
        cmp     cl, 64
        jae     short RETZERO
        ;
        ; Handle shifts of between 0 and 31 bits
            ;
        cmp     cl, 32
            jae     short MORE32
            shrd    eax, edx, cl
            shr     edx, cl
            ret
            ;
        ; Handle shifts of between 32 and 63 bits
            ;
    MORE32:
        mov     eax, edx
            xor     edx, edx
            and     cl, 31
            shr     eax, cl
            ret
            ;
        ; return 0 in edx : eax
            ;
    RETZERO:
        xor     eax, eax
            xor     edx, edx
            ret
    }
}

#endif
}
#endif
