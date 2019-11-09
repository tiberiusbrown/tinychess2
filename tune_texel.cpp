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
    TUNABLE_PARAM_ARRAY(PIECE_VALUES_MAG, 100, 1500, 1),

    TUNABLE_PARAM(MATERIAL_MG, 1500, 4000),
    TUNABLE_PARAM(MATERIAL_EG, 100, 1400),

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

    TUNABLE_PARAM(QUEEN_MOBILITY_BONUS_MG, 0, 100),
    TUNABLE_PARAM(QUEEN_MOBILITY_BONUS_EG, 0, 100),
    TUNABLE_PARAM(QUEEN_ON_OPEN_FILE, 0, 200),

    TUNABLE_PARAM_ARRAY(KING_DEFENDERS_MG, -100, 100, 2),

    TUNABLE_PARAM_ARRAY_SUB(INIT_TABLE_PAWN_MG, -127, 127, 8, 4, 28),
    TUNABLE_PARAM_ARRAY_SUB(INIT_TABLE_PAWN_EG, -127, 127, 8, 4, 28),
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
};

static std::vector<int> val_cur, val_old, val_a, val_b;
static std::vector<std::string> val_name;
void set_vals(std::vector<int> const& v)
{
    int i = 0;
    for(auto const& tp : params)
        for(int m = tp.ai; m < tp.bi; ++m)
            tp.p[m] = v[i++];
}

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

void write_params(std::vector<int> const& vs)
{
    FILE* f = fopen("tuned_params.txt", "w");
    int n = 0;
    for(auto const& tp : params)
    {
        if(tp.n > 1)
            fprintf(f, "CH_PARAM_ARRAY(%s[%d],", tp.s, tp.n);
        else
            fprintf(f, "CH_PARAM(%s, ", tp.s);
        for(int i = 0; i < tp.n; ++i)
        {
            int v = 0;
            if(i >= tp.ai && i < tp.bi)
                v = vs[n++];
            if(tp.n == 1)
                fprintf(f, "%d)\n", v);
            else
            {
                if(i % (tp.n / tp.nr) == 0)
                    fprintf(f, "\n    ");
                fprintf(f, "%4d, ", v);
            }
        }
        if(tp.n > 1)
            fprintf(f, "    )\n");
    }
    fclose(f);
}

FT mutate_val(FT k, FT tv, FT mv, int i, int d)
{
    FT v = tv;
    while(v == tv && v >= mv)
    {
        int pv = val_cur[i];
        val_cur[i] += d;
        val_cur[i] = std::min(val_cur[i], val_b[i]);
        val_cur[i] = std::max(val_cur[i], val_a[i]);
        if(val_cur[i] == pv) break;
        set_vals(val_cur);
        v = run_eval(k);
        d = (d * 3) / 2 + 1;
    }
    return v;
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
        }
        printf("DONE: read %d tests\n", (int)tests.size());
        printf("   White: %d\n", w);
        printf("   Black: %d\n", b);
        printf("   Draw : %d\n", d);
    }

    num_params = 0;
    for(auto const& tp : params)
    {
        num_params += (tp.bi - tp.ai);
        for(int i = tp.ai; i < tp.bi; ++i)
        {
            val_cur.push_back(tp.p[i]);
            val_a.push_back(tp.a);
            val_b.push_back(tp.b);
            {
                std::string s = tp.s;
                if(tp.n > 1)
                {
                    s += '[';
                    s += std::to_string(i);
                    s += ']';
                }
                val_name.push_back(s);
            }
        }
    }
    val_old = val_cur;
    printf("Number of parameters: %d\n", num_params);

    // find best k
    FT k, mv;
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
        while(std::abs(c - d) > tol)
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
        mv = run_eval(k);
        printf("   final       : K = %+8.5f, e = %f\n", k, mv);
    }

    for(int iter = 0; ; ++iter)
    {
        printf("iteration %d\n", iter);
        for(int i = 0; i < num_params; ++i)
        {
            printf("   %-28s\r", val_name[i].c_str());
            val_old = val_cur;
            FT tv = 100;
            tv = mutate_val(k, tv, mv, i, +1);
            tv = mutate_val(k, tv, mv, i, -1);
            //if(tv > mv && val_cur[i] < val_b[i])
            //{
            //    ++val_cur[i];
            //    set_vals(val_cur);
            //    tv = run_eval(k);
            //}
            //if(tv > mv && val_cur[i] > val_a[i])
            //{
            //    --val_cur[i];
            //    set_vals(val_cur);
            //    tv = run_eval(k);
            //}
            if(tv < mv)
            {
                printf("   %-28s   %3d -> %3d   %.6f\n",
                    val_name[i].c_str(), val_old[i], val_cur[i], tv);
                mv = tv;
                write_params(val_cur);
                continue;
            }
            val_cur = val_old;
            set_vals(val_cur);
        }   
    }


    return 0;
}
