// warning: uber crappy code ahead

#include <cctype>
#include <cstdio>
#include <cstdint>
#include <cstring>

#include <algorithm>
#include <array>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#include "alldata.h"
#include "config.h"

#define SHOW_CHAR_COUNTS 0

using namespace std;

struct entry
{
    bool islit;
    uint8_t lit;
    uint32_t offset;
    uint32_t length;
};

static vector<entry> entries;
static vector<uint8_t> uncompressed_data;
static vector<uint8_t> compressed_data;
uint32_t cbit = 0;

static vector<int> offsets;
static vector<int> sizes;
static vector<int> comp_bit_sizes;

static std::array<std::pair<int, int>, 256> char_counts;

static void write_one(uint32_t val)
{
    uint32_t cbyte = cbit / 8;
    if(cbyte <= compressed_data.size())
        compressed_data.resize(cbyte + 1);
    if(val)
        compressed_data[cbyte] |= (1 << (cbit % 8));
    ++cbit;
}

static void write_n(uint32_t val, int n)
{
    while(n-- > 0)
    {
        write_one((val & (1 << n)) != 0);
    }
}

static void add_lit(uint8_t lit)
{
    entry e;
    e.islit = true;
    e.lit = lit;
    entries.push_back(e);
}

static void add_ref(uint32_t offset, uint32_t length)
{
    entry e;
    e.islit = false;
    e.offset = offset;
    e.length = length;
    entries.push_back(e);
}

static string convert_name(char const* name)
{
    string r = name;
    for(int j = 0; r[j]; ++j)
    {
        r[j] = char(toupper(r[j]));
        if(!isalnum(r[j]))
            r[j] = '_';
    }
    return r;
}

template<class T> T clamp(T x, T a, T b)
{
    return x < a ? a : x > b ? b : x;
}

static void load_alldata(
    struct datadecl const* alldata,
    size_t num,
    vector<uint8_t>& out)
{
    out.clear();
    offsets.clear();
    sizes.clear();
    //image_infos.clear();
    for(size_t i = 0; i < num; ++i)
    {
        size_t prev_size = out.size();
        // align each item to 4-byte address
        while(prev_size % 4 != 0)
        {
            out.push_back('\0');
            ++prev_size;
        }
        offsets.push_back((int)prev_size);
        if(alldata[i].type == STR)
        {
            for(size_t j = 0; j < alldata[i].size; ++j)
                out.push_back(alldata[i].data[j]);
        }
        else if(alldata[i].type == BIN_FILE || alldata[i].type == TXT_FILE)
        {
            ifstream f(alldata[i].name, ios::binary);
            vector<char> d(
                (istreambuf_iterator<char>(f)),
                istreambuf_iterator<char>());
            if(alldata[i].type == TXT_FILE)
                d.push_back((uint8_t)0);
            for(size_t ii = 0; ii < d.size(); ++ii)
                out.push_back((uint8_t)d[ii]);
        }
        sizes.push_back((int)(out.size() - prev_size));
    }
}

struct
{
    typedef std::pair<int, int> T;
    bool operator()(T const& a, T const& b) const
    {
        return a.second < b.second ||
            (a.second == b.second && a.first > b.first);
    }
} char_counts_predicate;

#if _MSC_VER
int __cdecl main(int argc, char** argv)
#else
int main(int argc, char** argv)
#endif
{
    (void)argc;
    (void)argv;

    load_alldata(ALLDATA, NUMDATA, uncompressed_data);

    for(int i = 0; i < 256; ++i)
    {
        char_counts[i].first = (char)i;
        char_counts[i].second = 0;
    }
    for(int i = 0; i < (int)uncompressed_data.size(); ++i)
        ++char_counts[uncompressed_data[i]].second;
    std::sort(char_counts.begin(), char_counts.end(), char_counts_predicate);
#if SHOW_CHAR_COUNTS
    printf("Uncompressed Char Counts\n");
    printf("===========================\n");
    for(int i = 255; i >= 0; --i)
    {
        unsigned char c = (unsigned char)char_counts[i].first;
        int count = char_counts[i].second;
        printf("    %3d 0x%02x %c %4d (%4.1f%%)\n",
            (int)c, (int)c, c >= 32 && c < 127 ? c : ' ',
            count, (double)count * 100 / uncompressed_data.size());
    }
#endif

    if(!uncompressed_data.empty())
        add_lit(uncompressed_data[0]);

    comp_bit_sizes.resize(sizes.size());
    for(int i = 1, j = 0; i < (int)uncompressed_data.size(); ++i)
    {
        bool found = false;
        int length, offset = 0;
        int lengthmax = min((1 << COMPRESS_LENGTH_BITS) - 1, i - 2);

        while(i > offsets[j] + sizes[j])
            ++j;

        for(length = lengthmax; length >= 0; --length)
        {
            int offsetmax = min((1 << COMPRESS_OFFSET_BITS) - 1, i - length - 2);
            for(offset = 0; offset <= offsetmax; ++offset)
            {
                if(!memcmp(
                    &uncompressed_data[i - offset - length - 2],
                    &uncompressed_data[i],
                    (size_t)(length + 2)))
                {
                    found = true;
                    break;
                }
            }
            if(found)
                break;
        }

        if(found)
        {
            comp_bit_sizes[j] += (1 + COMPRESS_OFFSET_BITS + COMPRESS_LENGTH_BITS);
            add_ref((uint32_t)offset, (uint32_t)length);
            i += (int)length + 1;
        }
        else
        {
            comp_bit_sizes[j] += 9;
            add_lit(uncompressed_data[i]);
        }
    }

    printf("%30s  Bits (Byte)  Comp (Byte)  Ratio\n", "Name");
    printf(
        "================================="
        "===============================\n");
    for(int i = 0; i < (int)sizes.size(); ++i)
    {
        printf("%30s %5d (%4d) %5d (%4d) %6.1f%%\n",
            ALLDATA[i].name,
            sizes[i] * 8,
            sizes[i],
            comp_bit_sizes[i],
            (comp_bit_sizes[i] + 4) / 8,
            (double)comp_bit_sizes[i] *100 / (sizes[i] * 8));
    }

    // convert data
    for(size_t i = 0; i < entries.size(); ++i)
    {
		entry const& e = entries[i];
        if(e.islit)
        {
            write_one((uint32_t)0);
            write_n((uint32_t)e.lit, 8);
        }
        else
        {
            write_one((uint32_t)1);
            write_n((uint32_t)e.offset, COMPRESS_OFFSET_BITS);
            write_n((uint32_t)e.length, COMPRESS_LENGTH_BITS);
        }
    }

    for(int i = 0; i < 256; ++i)
    {
        char_counts[i].first = (char)i;
        char_counts[i].second = 0;
    }
    for(int i = 0; i < (int)compressed_data.size(); ++i)
        ++char_counts[compressed_data[i]].second;
    std::sort(char_counts.begin(), char_counts.end(), char_counts_predicate);
#if SHOW_CHAR_COUNTS
    printf("Compressed Char Counts\n");
    printf("===========================\n");
    for(int i = 255; i >= 0; --i)
    {
        unsigned char c = char_counts[i].first;
        int count = char_counts[i].second;
        printf("    %3d 0x%02x %c %4d (%4.1f%%)\n",
            (int)c, (int)c, c >= 32 && c < 127 ? c : ' ',
            count, (double)count * 100 / compressed_data.size());
    }
#endif

    FILE* f = fopen(OUTFILEH, "w");
     
    fprintf(f, "#ifndef ALLDATA_COMPRESSED_H\n");
    fprintf(f, "#define ALLDATA_COMPRESSED_H\n\n");

    fprintf(f, "#ifdef _MSC_VER\n__declspec(align(4))\n");
    fprintf(f, "#elif defined(__GNUC__)\n__attribute__((aligned(4)))\n");
    fprintf(f, "#else\n#error \"Unsupported\"\n#endif\n");

    fprintf(f, "static unsigned char ALLDATA_UNCOMPRESSED[%d];\n",
        (int)uncompressed_data.size());

#define COLS 16

    if(!uncompressed_data.empty())
    {
        fprintf(f, "\nstatic unsigned char const ALLDATA_COMPRESSED[%d] =\n{\n",
            (int)compressed_data.size());
        for(size_t i = 0; i < compressed_data.size(); ++i)
        {
            if(i % COLS == 0)
                fprintf(f, "    ");
            fprintf(f, "%3d,", (int)compressed_data[i]);
            if(i % COLS == (COLS - 1) || i == compressed_data.size() - 1)
                fprintf(f, "\n");
        }
        fprintf(f, "};\n\n");

        for(size_t i = 0; i < NUMDATA; ++i)
        {
            string name;
            if(type_is_file(ALLDATA[i].type))
                name = convert_name(ALLDATA[i].name);
            else
                name = ALLDATA[i].name;
            fprintf(f,
                "#define %-25s ((%-20s const*)(ALLDATA_UNCOMPRESSED+%d))\n",
                name.c_str(),
                ALLDATA[i].type == TXT_FILE ? "char" :
                ALLDATA[i].type_str ? ALLDATA[i].type_str : "void",
                offsets[i]);
        }
        fprintf(f, "\n");

        for(size_t i = 0; i < NUMDATA; ++i)
        {
            string name;
            if(type_is_file(ALLDATA[i].type))
                name = convert_name(ALLDATA[i].name);
            else
                name = ALLDATA[i].name;
            fprintf(f, "#define %-25s %d\n",
                (name + "_SIZE").c_str(), sizes[i]);
        }
        fprintf(f, "\n");
    }

    printf("Uncompressed Size:   %d\n", (int)uncompressed_data.size());
    printf("Compressed Size:     %d\n", (int)compressed_data.size());
    printf("Compression Ratio:   %.1f%%\n",
        (float)compressed_data.size() * 100 / uncompressed_data.size());

    load_alldata(ALLDATA_DONT_COMPRESS, NUMDATA_DONT_COMPRESS, uncompressed_data);

    printf("Don't Compress Size: %d\n", (int)uncompressed_data.size());
    printf("Total Binary Size:   %d\n",
        (int)uncompressed_data.size() + (int)compressed_data.size());

    if(!uncompressed_data.empty())
    {
        fprintf(f, "\nstatic unsigned char const ALLDATA_DIDNT_COMPRESS[%d] =\n{\n",
            (int)uncompressed_data.size());
        for(size_t i = 0; i < uncompressed_data.size(); ++i)
        {
            if(i % COLS == 0)
                fprintf(f, "    ");
            fprintf(f, "%3d,", (int)uncompressed_data[i]);
            if(i % COLS == (COLS - 1) || i == uncompressed_data.size() - 1)
                fprintf(f, "\n");
        }
        fprintf(f, "};\n\n");

        for(size_t i = 0; i < NUMDATA_DONT_COMPRESS; ++i)
        {
            string name;
            if(type_is_file(ALLDATA_DONT_COMPRESS[i].type))
                name = convert_name(ALLDATA_DONT_COMPRESS[i].name);
            else
                name = ALLDATA_DONT_COMPRESS[i].name;
            fprintf(f,
                "#define %-25s ((%-20s const*)(ALLDATA_DIDNT_COMPRESS+%d))\n",
                name.c_str(),
                ALLDATA_DONT_COMPRESS[i].type == TXT_FILE ? "char" :
                ALLDATA_DONT_COMPRESS[i].type_str ? ALLDATA_DONT_COMPRESS[i].type_str : "void",
                offsets[i]);
        }
        fprintf(f, "\n");

        for(size_t i = 0; i < NUMDATA_DONT_COMPRESS; ++i)
        {
            string name;
            if(type_is_file(ALLDATA_DONT_COMPRESS[i].type))
                name = convert_name(ALLDATA_DONT_COMPRESS[i].name);
            else
                name = ALLDATA_DONT_COMPRESS[i].name;
            fprintf(f, "#define %-25s %d\n",
                (name + "_SIZE").c_str(), sizes[i]);
        }
        fprintf(f, "\n");
    }

    fprintf(f, "#endif\n");
    
    fclose(f);

    return 0;
}
