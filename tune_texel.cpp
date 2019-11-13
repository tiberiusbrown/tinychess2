#include "ch/ch.h"

#include "ch/ch_params.hpp"

#include <algorithm>
#include <chrono>
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <random>
#include <thread>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>

#include <cmath>

typedef double FT;

static int num_params;

typedef std::function<void(void)> fp_t;
static std::queue<fp_t> jobs;
static std::vector<std::thread> workers;
std::mutex job_lock;
std::condition_variable job_cv;

std::vector<int> done;
std::vector<FT> results;

static void worker_thread()
{
    std::unique_lock<std::mutex> lock(job_lock);
    for(;;)
    {
        job_cv.wait(lock, []{ return (jobs.size()); });
        if(!jobs.empty())
        {
            auto op = std::move(jobs.front());
            jobs.pop();
            lock.unlock();
            op();
            lock.lock();
        }
    }
}
static void dispatch(fp_t const& op)
{
    std::unique_lock<std::mutex> lock(job_lock);
    jobs.push(op);
    lock.unlock();
    job_cv.notify_all();
}

static uint32_t get_ms(void)
{
    static std::chrono::steady_clock::time_point start_time =
        std::chrono::steady_clock::now();
    return (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start_time).count();
}

static void* alloc(uint32_t bytes) { return malloc((size_t)bytes); }
static void dealloc(void* p) { free(p); }

static ch_system_info const INIT_INFO =
{
    &alloc,
    &dealloc,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
};


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
    TUNABLE_PARAM_ARRAY_SUB(PIECE_VALUES_MAG, 100, 1500, 1, 1, 5),

    //TUNABLE_PARAM(MATERIAL_MG, 1500, 4000),
    //TUNABLE_PARAM(MATERIAL_EG, 100, 1400),
    
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
    
    //TUNABLE_PARAM_ARRAY(KING_DEFENDERS_MG, -100, 100, 2),
    
    TUNABLE_PARAM_ARRAY_SUB(INIT_TABLE_PAWN_MG, -300, 300, 8, 4, 28),
    TUNABLE_PARAM_ARRAY_SUB(INIT_TABLE_PAWN_EG, -300, 300, 8, 4, 28),
    TUNABLE_PARAM_ARRAY(INIT_TABLE_KNIGHT_MG, -300, 300, 8),
    TUNABLE_PARAM_ARRAY(INIT_TABLE_KNIGHT_EG, -300, 300, 8),
    TUNABLE_PARAM_ARRAY(INIT_TABLE_BISHOP_MG, -300, 300, 8),
    TUNABLE_PARAM_ARRAY(INIT_TABLE_BISHOP_EG, -300, 300, 8),
    TUNABLE_PARAM_ARRAY(INIT_TABLE_ROOK_MG, -300, 300, 8),
    TUNABLE_PARAM_ARRAY(INIT_TABLE_ROOK_EG, -300, 300, 8),
    TUNABLE_PARAM_ARRAY(INIT_TABLE_QUEEN_MG, -300, 300, 8),
    TUNABLE_PARAM_ARRAY(INIT_TABLE_QUEEN_EG, -300, 300, 8),
    TUNABLE_PARAM_ARRAY(INIT_TABLE_KING_MG, -300, 300, 8),
    TUNABLE_PARAM_ARRAY(INIT_TABLE_KING_EG, -300, 300, 8),
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

FT run_eval(std::vector<ch_game*> gs, FT k)
{
    ch_init(&INIT_INFO);

    done.resize(gs.size());
    for(auto& d : done) d = 0;
    results.resize(gs.size());
    size_t m = tests.size() / gs.size();
    for(size_t n = 0; n < gs.size(); ++n)
    {
        dispatch([&,m,n]()
        {
            FT t = 0;
            for(size_t i = m * n; i < m * (n + 1); ++i)
            {
                auto const& test = tests[i];
                //printf("gs[%d] = %p\n", (int)n, gs[n]);
                ch_load_fen(gs[n], test.fen.c_str());
                FT e = (FT)ch_qsearch(gs[n]);
                FT d = FT(1) + pow(FT(10), -k * e / 400);
                FT s = FT(1) / d - test.result;
                t += (s * s);
            }
            results[n] = t;
            done[n] = 1;
        });
    }

    for(;;)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        bool all_done = true;
        for(int d : done) if(!d) all_done = false;
        if(all_done) break;
    }

    FT t = 0.f;

    //for(auto const& test : tests)
    //{
    //    ch_load_fen(gs[0], test.fen.c_str());
    //    //FT e = (FT)ch_evaluate_white(gs[0]);
    //    FT e = (FT)ch_qsearch(gs[0]);
    //    FT d = FT(1) + pow(FT(10), -k * e / 400);
    //    FT s = FT(1) / d - test.result;
    //    t += (s * s);
    //}

    for(auto r : results) t += r;

    return t / (m * gs.size());
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
            else
                v = tp.p[i];
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

FT mutate_val(std::vector<ch_game*> gs, FT k, FT tv, FT mv, int i, int d)
{
    FT v = tv;
    int vvv = val_cur[i];
    while(v == tv && v > mv)
    {
        int pv = val_cur[i];
        val_cur[i] += d;
        val_cur[i] = std::min(val_cur[i], val_b[i]);
        val_cur[i] = std::max(val_cur[i], val_a[i]);
        if(val_cur[i] == pv) break;
        set_vals(val_cur);
        printf("   %-30s: %+d     \r", val_name[i].c_str(), d);
        v = run_eval(gs, k);
        d *= 2;
        if(v >= mv) val_cur[i] = vvv;
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

    int num_threads = (int)std::thread::hardware_concurrency();
    num_threads = std::max(1, num_threads);
    printf("Tuning with %d threads\n", num_threads);
    std::vector<ch_game*> gs;
    ch_init(&INIT_INFO);
    for(int n = 0; n < num_threads; ++n)
        gs.push_back(ch_create());
    workers.resize((size_t)num_threads);
    for(int n = 0; n < num_threads; ++n)
        workers[n] = std::thread(worker_thread);

    // find best k
    FT k, mv;
    {
        printf("Finding best K\n");
        FT const gr = (sqrt(FT(5)) + 1) / 2;
        FT const tol = FT(1e-5);
        FT a = FT(0);
        FT b = FT(2);
        FT c = b - (b - a) / gr;
        FT d = a + (b - a) / gr;
        int i = 0;
        printf("   iteration %2d: a = %+8.5f, b = %+8.5f\n", i, a, b);
        while(std::abs(c - d) > tol)
        {
            FT ce = run_eval(gs, c);
            FT de = run_eval(gs, d);
            if(ce < de)
                b = d;
            else
                a = c;
            c = b - (b - a) / gr;
            d = a + (b - a) / gr;
            printf("   iteration %2d: a = %+8.5f, b = %+8.5f (%.6f, %.6f)\n", ++i, a, b, ce, de);
        }
        k = (a + b) / 2;
        mv = run_eval(gs, k);
        printf("   final       : K = %+8.5f, e = %f\n", k, mv);
    }

    for(int iter = 1; ; ++iter)
    {
        bool improved = false;
        printf("iteration %d%30s\n", iter, "");
        for(int i = 0; i < num_params; ++i)
        {
            fflush(stdout);
            val_old = val_cur;
            FT tv = 100;
            tv = mutate_val(gs, k, tv, mv, i, -1);
            tv = mutate_val(gs, k, tv, mv, i, +1);
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
                printf("   %-30s   %4d -> %4d   %.6f\n",
                    val_name[i].c_str(), val_old[i], val_cur[i], tv);
                mv = tv;
                write_params(val_cur);
                improved = true;
                continue;
            }
            val_cur = val_old;
            set_vals(val_cur);
        }
        if(!improved) break;
    }

    for(ch_game* g : gs)
        ch_destroy(g);
    gs.clear();

    return 0;
}
