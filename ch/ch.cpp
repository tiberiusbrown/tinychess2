#include "ch.h"

// poor man's LTO >:-D

#include "ch_c_intf.hpp"
#include "ch_cpuid.hpp"
#include "ch_genmoves_unaccel.hpp"
#include "ch_genmoves_sse.hpp"
#include "ch_genmoves_avx.hpp"
#include "ch_init.hpp"
#include "ch_position.hpp"
#include "ch_print_bbs.hpp"
