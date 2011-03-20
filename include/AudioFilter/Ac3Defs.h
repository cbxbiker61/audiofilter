#pragma once
#ifndef AUDIOFILTER_AC3DEFS_H
#define AUDIOFILTER_AC3DEFS_H

/*
  AC3 parser definitions
*/

#define AC3_MAX_FRAME_SIZE 3840
#define AC3_MIN_FRAME_SIZE 128

#define AC3_NBLOCKS   6
#define AC3_NCHANNELS 6
#define AC3_FRAME_SAMPLES  1536
#define AC3_BLOCK_SAMPLES  256

#define AC3_MODE_DUAL    0
#define AC3_MODE_1_0     1
#define AC3_MODE_2_0     2
#define AC3_MODE_3_0     3
#define AC3_MODE_2_1     4
#define AC3_MODE_3_1     5
#define AC3_MODE_2_2     6
#define AC3_MODE_3_2     7
#define AC3_MODE_LFE     8  // can be combined with any of previous modes

#define AC3_MODE_MONO    1
#define AC3_MODE_STEREO  2
#define AC3_MODE_QUADRO  6
#define AC3_MODE_5_1     (AC3_MODE_3_2 | AC3_MODE_LFE)

#endif

// vim: ts=2 sts=2 et

