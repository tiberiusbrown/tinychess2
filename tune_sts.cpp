#include "ch/ch.h"

#include "ch/ch_params.hpp"

#include <algorithm>
#include <chrono>
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <random>

#define SEARCH_NODES 50000
#define SEARCH_DEPTH 0
#define SEARCH_TIME 0

#define GENERATION_KEEP 3

#define GENERATION_SIZE (GENERATION_KEEP * GENERATION_KEEP)

#define NUM_MUTATE 1
#define MUTATE_FACTOR 0.05

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
    //TUNABLE_PARAM(ASPIRATION_BASE_DELTA, 4, 50),
    //TUNABLE_PARAM(ASPIRATION_MIN_DEPTH, 2, 10),
    //TUNABLE_PARAM(ASPIRATION_C0, 1, 10),
    //TUNABLE_PARAM(ASPIRATION_C1, 1, 20),
    //TUNABLE_PARAM(PROBCUT_MIN_DEPTH, 3, 10),
    //TUNABLE_PARAM(PROBCUT_MARGIN, 10, 500),
    //TUNABLE_PARAM(RAZOR_MARGIN, 10, 500),
    //TUNABLE_PARAM(RAZOR_MAX_DEPTH, 1, 3),
    //TUNABLE_PARAM(DEEP_RAZOR_MARGIN, 50, 600),
    //TUNABLE_PARAM(DEEP_RAZOR_MAX_DEPTH, 3, 5),
    //TUNABLE_PARAM(FUTILITY_PRUNING_MAX_DEPTH, 3, 10),
    //TUNABLE_PARAM(FUTILITY_PRUNING_C0, 10, 400),
    //TUNABLE_PARAM(FUTILITY_PRUNING_C1, 10, 400),
    //TUNABLE_PARAM(NULL_MOVE_MIN_DEPTH, 3, 8),
    //TUNABLE_PARAM(IID_MIN_DEPTH, 4, 12),
    //TUNABLE_PARAM(IID_C0, 2, 4),
    //TUNABLE_PARAM(LMP_MAX_DEPTH, 2, 8),
    //TUNABLE_PARAM(LMP_C0, 3, 12),
    //TUNABLE_PARAM(LMP_C1, -5, 5),
    //TUNABLE_PARAM(LMR_MIN_DEPTH, 2, 5),
    //TUNABLE_PARAM(LMR_C0, 3, 6),
    //TUNABLE_PARAM(LMR_C1, 3, 12),
    //TUNABLE_PARAM(LMR_C2, 2, 8),



    //TUNABLE_PARAM_ARRAY(INIT_TABLE_PAWN_MG, -127, 127, 8),
    //TUNABLE_PARAM_ARRAY(INIT_TABLE_PAWN_EG, -127, 127, 8),
    //TUNABLE_PARAM_ARRAY(INIT_TABLE_KNIGHT_MG, -127, 127, 8),
    //TUNABLE_PARAM_ARRAY(INIT_TABLE_KNIGHT_EG, -127, 127, 8),
    //TUNABLE_PARAM_ARRAY(INIT_TABLE_BISHOP_MG, -127, 127, 8),
    //TUNABLE_PARAM_ARRAY(INIT_TABLE_BISHOP_EG, -127, 127, 8),
    //TUNABLE_PARAM_ARRAY(INIT_TABLE_ROOK_MG, -127, 127, 8),
    //TUNABLE_PARAM_ARRAY(INIT_TABLE_ROOK_EG, -127, 127, 8),
    //TUNABLE_PARAM_ARRAY(INIT_TABLE_QUEEN_MG, -127, 127, 8),
    //TUNABLE_PARAM_ARRAY(INIT_TABLE_QUEEN_EG, -127, 127, 8),
    //TUNABLE_PARAM_ARRAY(INIT_TABLE_KING_MG, -127, 127, 8),
    //TUNABLE_PARAM_ARRAY(INIT_TABLE_KING_EG, -127, 127, 8),

    //TUNABLE_PARAM(HALF_OPEN_FILE, 0, 50),

    //TUNABLE_PARAM(PAWN_PROTECT_ANY, 0, 100),
    //TUNABLE_PARAM(PAWN_PROTECT_PAWN, 0, 100),
    //TUNABLE_PARAM(PAWN_THREATEN_KNIGHT, 0, 100),
    //TUNABLE_PARAM(PAWN_THREATEN_BISHOP, 0, 100),
    //TUNABLE_PARAM(PAWN_THREATEN_ROOK, 0, 200),
    //TUNABLE_PARAM(PAWN_THREATEN_QUEEN, 0, 140),
    //TUNABLE_PARAM_ARRAY_SUB(PASSED_PAWN_MG, 0, 200, 1, 1, 7),
    //TUNABLE_PARAM_ARRAY_SUB(PASSED_PAWN_EG, 0, 500, 1, 1, 7),
    //TUNABLE_PARAM_ARRAY_SUB(PASSED_PAWN_FREE_EG, 0, 1000, 1, 1, 7),
    //TUNABLE_PARAM(PASSED_PAWN_KING_ESCORT, 0, 100),

    //TUNABLE_PARAM(KNIGHT_PAWN_BONUS_MG, 0, 100),
    //TUNABLE_PARAM(KNIGHT_PAWN_BONUS_EG, 0, 100),
    //TUNABLE_PARAM(KNIGHT_MOBILITY_BONUS_MG, 0, 100),
    //TUNABLE_PARAM(KNIGHT_MOBILITY_BONUS_EG, 0, 100),
    //TUNABLE_PARAM(KNIGHT_THREATEN_BISHOP, 0, 50),
    //TUNABLE_PARAM(KNIGHT_THREATEN_ROOK, 0, 100),
    //TUNABLE_PARAM(KNIGHT_THREATEN_QUEEN, 0, 100),
    //TUNABLE_PARAM(KNIGHT_THREATEN_KING, 0, 100),
    //TUNABLE_PARAM(KNIGHT_OUTPOST, 0, 100),
    //TUNABLE_PARAM(KNIGHT_OUTPOST_HALF_OPEN_FILE, 0, 100),
    //TUNABLE_PARAM(KNIGHT_OUTPOST_OPEN_FILE, 0, 100),

    //TUNABLE_PARAM(BISHOP_MOBILITY_BONUS_MG, 0, 100),
    //TUNABLE_PARAM(BISHOP_MOBILITY_BONUS_EG, 0, 100),
    //TUNABLE_PARAM(BISHOP_THREATEN_ROOK, 0, 100),
    //TUNABLE_PARAM(BISHOP_THREATEN_QUEEN, 0, 100),
    //TUNABLE_PARAM(BISHOP_THREATEN_KING, 0, 100),

    //TUNABLE_PARAM(ROOK_MOBILITY_BONUS_MG, 0, 100),
    //TUNABLE_PARAM(ROOK_MOBILITY_BONUS_EG, 0, 100),
    //TUNABLE_PARAM(ROOK_THREATEN_QUEEN, 0, 100),
    //TUNABLE_PARAM(ROOK_THREATEN_KING, 0, 100),
    //TUNABLE_PARAM(ROOK_ON_OPEN_FILE, 0, 200),

    //TUNABLE_PARAM(QUEEN_ON_OPEN_FILE, 0, 200),

    //TUNABLE_PARAM_ARRAY(KING_DEFENDERS_MG, -100, 100, 2),
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

struct sort_param_values_descending
{
    bool operator()(param_values const &a, param_values const &b) const
    {
        return a.score > b.score;
    }
};

static param_values values[GENERATION_SIZE];

static int mutate_value(int src, int a, int b)
{
    static std::default_random_engine generator;
    double sigma = std::max(0.5, double(b - a) * MUTATE_FACTOR);
    std::normal_distribution<double> distribution(0, sigma);
    double d = distribution(generator);
    int nd = (int)std::round(d);
    if(nd == 0) nd = (d < 0 ? -1 : 1);
    return std::min(b, std::max(a, src + nd));
}

static void mutate(param_values& dst, param_values const& src)
{
    static std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(0, num_params - 1);
    dst = src;
    for(int m = 0; m < NUM_MUTATE; ++m)
    {
        int k = distribution(generator);
        int n = 0;
        for(auto const& tp : params)
        {
            //for(int i = 0; i < tp.n; ++i)
            for(int i = tp.ai; i < tp.bi; ++i)
            {
                if(n == k)
                    dst.d[n] = mutate_value(src.d[n], tp.a, tp.b);
                ++n;
            }
        }
    }
}

#ifdef _MSC_VER
#define CDECL __cdecl
#else
#define CDECL
#endif

struct sts_test
{
    std::string fen;
    std::map<std::string, int> moves;
};
static std::vector<sts_test> tests;

static ch_search_limits const LIMITS =
{
    SEARCH_NODES, SEARCH_DEPTH, SEARCH_TIME, { 0, 0 }, { 0, 0 }, 0,
};

static ch_move best_move = 0;

static void set_best_move(ch_move m)
{
    best_move = m;
}

static ch_system_info const INIT_INFO =
{
    &get_ms,
    nullptr,
    nullptr,
    &set_best_move
};

static uint64_t hash_mem[(1 << 20) / 8];

static int run_sts()
{
    int n = 0;
    int i = 0;

    ch_init(&INIT_INFO);
    ch_set_hash(hash_mem, 0);

    for(auto const& test : tests)
    {
        ch_clear_caches();
        ch_load_fen(test.fen.c_str());
        ch_search(&LIMITS);
        std::string m = ch_extended_algebraic(best_move);
        int s = test.moves.count(m) != 0 ? test.moves.at(m) : 0;
        n += s;
        ++i;
    }

    return n;
}

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

int CDECL main()
{
    {
        printf("Reading \"%s\"...\n", STSFILE);
        std::ifstream f(STSFILE, std::ios::in);
        std::string line;
        while(!f.eof())
        {
            std::getline(f, line);
            auto x = line.find(" bm");
            if(x == std::string::npos) break;
            tests.resize(tests.size() + 1);
            auto& test = tests.back();
            test.fen = line.substr(0, x);

            x = line.find(" c8 ");
            auto y = line.find('"', x + 5);
            auto vals = line.substr(x + 5, y - (x + 5));
            auto vals_split = split(vals, " ");
            
            x = line.find(" c9 ");
            y = line.find('"', x + 5);
            auto mvs = line.substr(x + 5, y - (x + 5));
            auto mvs_split = split(mvs, " ");

            for(int i = 0; i < (int)mvs_split.size(); ++i)
                test.moves[mvs_split[i]] = atoi(vals_split[i].c_str());
        }
        printf("DONE\n");
    }

    // initial generation
    {
        num_params = 0;
        //for(auto const& tp : params) num_params += tp.n;
        for(auto const& tp : params) num_params += (tp.bi - tp.ai);
        for(auto& v : values) v.d.resize(num_params);
    }

    {
        int s = run_sts();
        for(int i = 0; i < GENERATION_KEEP; ++i)
        {
            auto& v = values[i];
            v.score = s;
            int n = 0;
            for(auto const& tp : params)
                //for(int m = 0; m < tp.n; ++m)
                for(int m = tp.ai; m < tp.bi; ++m)
                    v.d[n++] = tp.p[m];
        }
    }

    for(int g = 0;; ++g)
    {
        printf("Generation: %d\n", g);
        printf("Top Scores:");
        for(int i = 0; i < GENERATION_KEEP; ++i)
            printf(" %d", values[i].score);
        printf("\n");

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
                        v = values[0].d[n++];
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

        // mutate
        for(int m = 0; m < GENERATION_KEEP; ++m)
        {
            for(int n = 0; n < GENERATION_KEEP - 1; ++n)
            {
                mutate(values[m * (GENERATION_KEEP - 1) + GENERATION_KEEP + n], values[m]);
            }
        }

        for(int n = GENERATION_KEEP; n < GENERATION_SIZE; ++n)
        {
            printf("   Running test for unit %3d... ", n);
            fflush(stdout);
            values[n].set();
            int v = run_sts();
            values[n].score =
                (v == values[(n - GENERATION_KEEP) / (GENERATION_KEEP - 1)].score ?
                0 : v);
            printf("%d\n", values[n].score);
        }

        std::sort(std::begin(values), std::end(values), sort_param_values_descending());
    }

    return 0;
}
