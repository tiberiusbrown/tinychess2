#pragma once

#define CH_ARCH_32BIT 0

#ifdef _MSC_VER
#ifdef _M_IX86
#undef CH_ARCH_32BIT
#define CH_ARCH_32BIT 1
#endif
#endif

#ifdef __GNUC__
#ifdef __i386__
#undef CH_ARCH_32BIT
#define CH_ARCH_32BIT 1
#endif
#endif

#define CH_ARCH_64BIT (!CH_ARCH_32BIT)
