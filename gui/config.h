#ifndef CONFIG_H
#define CONFIG_H

/* LZSS config for embedded compressed data */
#define COMPRESS_OFFSET_BITS 7
#define COMPRESS_LENGTH_BITS 6

#define ENABLE_RBUTTON 1
#define ENABLE_MS 1
#define ENABLE_THREAD 1

#define ENABLE_DLCLOSE 0

#define ENABLE_OPENGL 1

#define ENABLE_XSHM 1

#define MAX_DIJKSTRA_PATH_NODES 1024

/* use 8-bit colormap */
#if defined(_WIN32)
#define USE_COLORMAP 0
#else
#define USE_COLORMAP 0
#endif

/* use 6,6,6 = 216 style colormap instead of R3G3B2 */
#define COLORMAP666 0

/* framebuffer dimensions */
#define FBW 320
#define FBH 180

/* level dimensions */
#define LEVEL_W 128
#define LEVEL_H 64
#define LEVEL_TILES (LEVEL_W * LEVEL_H)
#define NUM_LEVELS 16

/* allows fractional vision values */
#define VISION_MULT 16

#define ENABLE_RESIZE_SNAP 1
#define RESIZE_SNAP_PIXELS 32

#endif
