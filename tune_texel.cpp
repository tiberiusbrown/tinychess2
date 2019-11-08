#include "ch/ch.h"

#include "ch/ch_params.hpp"

#include <algorithm>
#include <chrono>
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <random>

#include <cmath>

typedef double FT;

static int num_params;

static uint32_t get_ms(void)
{
    static std::chrono::steady_clock::time_point start_time =
        std::chrono::steady_clock::now();
    return (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start_time).count();
}

struct tunable_param
{
    char const* s;
    int* p;
    int n;
    int a, b;
    int nr;
    int ai, bi;
};

#define TUNABLE_PARAM(s_, a_, b_) { #s_, &ch::s_, 1, a_, b_, 1, 0, 1 }
#define TUNABLE_PARAM_ARRAY(s_, a_, b_, nr_) { \
    #s_, ch::s_, sizeof(ch::s_) / sizeof(int), a_, b_, nr_, 0, sizeof(ch::s_) / sizeof(int) }
#define TUNABLE_PARAM_ARRAY_SUB(s_, a_, b_, nr_, ai_, bi_) { \
    #s_, ch::s_, sizeof(ch::s_) / sizeof(int), a_, b_, nr_, ai_, bi_ }

static tunable_param const params[] =
{
    TUNABLE_PARAM_ARRAY(INIT_TABLE_PAWN_MG, -127, 127, 8),
    TUNABLE_PARAM_ARRAY(INIT_TABLE_PAWN_EG, -127, 127, 8),
    TUNABLE_PARAM_ARRAY(INIT_TABLE_KNIGHT_MG, -127, 127, 8),
    TUNABLE_PARAM_ARRAY(INIT_TABLE_KNIGHT_EG, -127, 127, 8),
    TUNABLE_PARAM_ARRAY(INIT_TABLE_BISHOP_MG, -127, 127, 8),
    TUNABLE_PARAM_ARRAY(INIT_TABLE_BISHOP_EG, -127, 127, 8),
    TUNABLE_PARAM_ARRAY(INIT_TABLE_ROOK_MG, -127, 127, 8),
    TUNABLE_PARAM_ARRAY(INIT_TABLE_ROOK_EG, -127, 127, 8),
    TUNABLE_PARAM_ARRAY(INIT_TABLE_QUEEN_MG, -127, 127, 8),
    TUNABLE_PARAM_ARRAY(INIT_TABLE_QUEEN_EG, -127, 127, 8),
    TUNABLE_PARAM_ARRAY(INIT_TABLE_KING_MG, -127, 127, 8),
    TUNABLE_PARAM_ARRAY(INIT_TABLE_KING_EG, -127, 127, 8),

    TUNABLE_PARAM(HALF_OPEN_FILE, 0, 50),

    TUNABLE_PARAM(PAWN_PROTECT_ANY, 0, 100),
    TUNABLE_PARAM(PAWN_PROTECT_PAWN, 0, 100),
    TUNABLE_PARAM(PAWN_THREATEN_KNIGHT, 0, 100),
    TUNABLE_PARAM(PAWN_THREATEN_BISHOP, 0, 100),
    TUNABLE_PARAM(PAWN_THREATEN_ROOK, 0, 200),
    TUNABLE_PARAM(PAWN_THREATEN_QUEEN, 0, 140),
    TUNABLE_PARAM_ARRAY_SUB(PASSED_PAWN_MG, 0, 200, 1, 1, 7),
    TUNABLE_PARAM_ARRAY_SUB(PASSED_PAWN_EG, 0, 500, 1, 1, 7),
    TUNABLE_PARAM_ARRAY_SUB(PASSED_PAWN_FREE_EG, 0, 1000, 1, 1, 7),
    TUNABLE_PARAM(PASSED_PAWN_KING_ESCORT, 0, 100),

    TUNABLE_PARAM(KNIGHT_PAWN_BONUS_MG, 0, 100),
    TUNABLE_PARAM(KNIGHT_PAWN_BONUS_EG, 0, 100),
    TUNABLE_PARAM(KNIGHT_MOBILITY_BONUS_MG, 0, 100),
    TUNABLE_PARAM(KNIGHT_MOBILITY_BONUS_EG, 0, 100),
    TUNABLE_PARAM(KNIGHT_THREATEN_BISHOP, 0, 50),
    TUNABLE_PARAM(KNIGHT_THREATEN_ROOK, 0, 100),
    TUNABLE_PARAM(KNIGHT_THREATEN_QUEEN, 0, 100),
    TUNABLE_PARAM(KNIGHT_THREATEN_KING, 0, 100),
    TUNABLE_PARAM(KNIGHT_OUTPOST, 0, 100),
    TUNABLE_PARAM(KNIGHT_OUTPOST_HALF_OPEN_FILE, 0, 100),
    TUNABLE_PARAM(KNIGHT_OUTPOST_OPEN_FILE, 0, 100),

    TUNABLE_PARAM(BISHOP_MOBILITY_BONUS_MG, 0, 100),
    TUNABLE_PARAM(BISHOP_MOBILITY_BONUS_EG, 0, 100),
    TUNABLE_PARAM(BISHOP_THREATEN_ROOK, 0, 100),
    TUNABLE_PARAM(BISHOP_THREATEN_QUEEN, 0, 100),
    TUNABLE_PARAM(BISHOP_THREATEN_KING, 0, 100),

    TUNABLE_PARAM(ROOK_MOBILITY_BONUS_MG, 0, 100),
    TUNABLE_PARAM(ROOK_MOBILITY_BONUS_EG, 0, 100),
    TUNABLE_PARAM(ROOK_THREATEN_QUEEN, 0, 100),
    TUNABLE_PARAM(ROOK_THREATEN_KING, 0, 100),
    TUNABLE_PARAM(ROOK_ON_OPEN_FILE, 0, 200),

    TUNABLE_PARAM(QUEEN_ON_OPEN_FILE, 0, 200),

    TUNABLE_PARAM_ARRAY(KING_DEFENDERS_MG, -100, 100, 2),
};

struct param_values
{
    int score;
    std::vector<int> d;

    void set()
    {
        int i = 0;
        for(auto const& tp : params)
            //for(int m = 0; m < tp.n; ++m)
            for(int m = tp.ai; m < tp.bi; ++m)
                tp.p[m] = d[i++];
    }
};

#ifdef _MSC_VER
#define CDECL __cdecl
#else
#define CDECL
#endif

struct texel_test
{
    std::string fen;
    FT result;
};
static std::vector<texel_test> tests;

static std::vector<std::string> split(std::string str, std::string const& token)
{
    std::vector<std::string> result;
    while(str.size())
    {
        auto index = str.find(token);
        if(index != std::string::npos)
        {
            result.push_back(str.substr(0, index));
            str = str.substr(index + token.size());
            if(str.size() == 0)
                result.push_back(str);
        }
        else
        {
            result.push_back(str);
            str = "";
        }
    }
    return result;
}

FT run_eval(FT k)
{
    FT t = 0.f;

    ch_init(nullptr);

    for(auto const& test : tests)
    {
        ch_load_fen(test.fen.c_str());
        FT e = (FT)ch_evaluate_white();
        FT d = FT(1) + pow(FT(10), -k * e / num_params);
        FT s = FT(1) / d - test.result;
        t += (s * s);
    }

    return t / tests.size();
}

int CDECL main()
{
    {
        printf("Reading \"%s\"...\n", TEXELFILE);
        std::ifstream f(TEXELFILE, std::ios::in);
        std::string line;
        int w = 0, b = 0, d = 0;
        while(!f.eof())
        {
            std::getline(f, line);
            auto x = line.find("\"");
            if(x == std::string::npos) break;
            tests.resize(tests.size() + 1);
            auto& test = tests.back();
            test.fen = line.substr(0, x);

            if(line[x + 3] == '0')
                ++w, test.result = 1.f;
            else if(line[x + 3] == '1')
                ++b, test.result = 0.f;
            else if(line[x + 3] == '2')
                ++d, test.result = 0.5f;
            else
                __debugbreak();
        }
        printf("DONE: read %d tests\n", (int)tests.size());
        printf("   White: %d\n", w);
        printf("   Black: %d\n", b);
        printf("   Draw : %d\n", d);
    }

    num_params = 0;
    for(auto const& tp : params)
        num_params += (tp.bi - tp.ai);
    printf("Number of parameters: %d\n", num_params);

    // find best k
    FT k;
    {
        printf("Finding best K\n");
        FT const gr = (sqrt(FT(5)) + 1) / 2;
        FT const tol = FT(1e-5);
        FT a = FT(1);
        FT b = FT(2);
        FT c = b - (b - a) / gr;
        FT d = a + (b - a) / gr;
        int i = 0;
        printf("   iteration %2d: a = %+8.5f, b = %+8.5f\n", i, a, b);
        while(abs(c - d) > tol)
        {
            FT ce = run_eval(c);
            FT de = run_eval(d);
            if(ce < de)
                b = d;
            else
                a = c;
            c = b - (b - a) / gr;
            d = a + (b - a) / gr;
            printf("   iteration %2d: a = %+8.5f, b = %+8.5f\n", ++i, a, b);
        }
        k = (a + b) / 2;
        printf("   final       : K = %+8.5f, e = %f\n", k, run_eval(k));
    }



    return 0;
}
