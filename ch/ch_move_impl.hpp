#pragma once

#include "ch_move.hpp"

namespace ch
{

char const* move::extended_algebraic() const
{
    static char buf[16];
    char* bp = buf;

    int a = from();
    int b = to();

    *bp++ = char('a' + (a % 8));
    *bp++ = char('8' - (a / 8));
    *bp++ = char('a' + (b % 8));
    *bp++ = char('8' - (b / 8));

    if(is_promotion())
    {
        switch((d >> 16) & 0xff)
        {
        case WHITE + KNIGHT: case BLACK + KNIGHT: *bp++ = 'n'; break;
        case WHITE + BISHOP: case BLACK + BISHOP: *bp++ = 'b'; break;
        case WHITE + ROOK  : case BLACK + ROOK  : *bp++ = 'r'; break;
        case WHITE + QUEEN : case BLACK + QUEEN : *bp++ = 'q'; break;
        }
    }

    *bp = '\0';
    return buf;
}

}
