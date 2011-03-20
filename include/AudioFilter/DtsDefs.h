#pragma once
#ifndef AUDIOFILTER_DTSDEFS_H
#define AUDIOFILTER_DTSDEFS_H

/*
  DTS parser definitions
*/

namespace AudioFilter {

struct huff_entry_t
{
  int length;
  int code;
  int value;
};

#define DTS_NCHANNELS      6
#define DTS_MAX_SAMPLES    4096
#define DTS_MAX_FRAME_SIZE 16384

#define DTS_SUBFRAMES_MAX     16
#define DTS_PRIM_CHANNELS_MAX 5
#define DTS_SUBBANDS          32
#define DTS_ABITS_MAX         32 // Should be 28
#define DTS_SUBSUBFAMES_MAX   4
#define DTS_LFE_MAX           3

/*
#define DTS_HEADER_SIZE 14
#define DTS_MAX_NCHANNELS 6
#define DTS_MAX_NSAMPLES  4096
*/

// DTS audio coding modes
#define DTS_MODE_MONO           0
#define DTS_MODE_CHANNEL        1
#define DTS_MODE_STEREO         2
#define DTS_MODE_STEREO_SUMDIFF 3
#define DTS_MODE_STEREO_TOTAL   4
#define DTS_MODE_3F             5
#define DTS_MODE_2F1R           6
#define DTS_MODE_3F1R           7
#define DTS_MODE_2F2R           8
#define DTS_MODE_3F2R           9
#define DTS_MODE_4F2R           10

//#define DTS_DOLBY 101 // FIXME
/*
#define DTS_MODE_MAX  DTS_3F2R // We don't handle anything above that
#define DTS_CHANNEL_BITS 6
#define DTS_CHANNEL_MASK 0x3F

#define DTS_LFE 0x80
#define DTS_ADJUST_LEVEL 0x100
*/
/*
dts_state_t * dts_init (uint32_t mm_accel);

int dts_syncinfo (dts_state_t *state, uint8_t * buf, int * flags,
                  int * sample_rate, int * bit_rate, int *frame_length);

int dts_frame (dts_state_t * state, uint8_t * buf, int * flags,
               level_t * level, sample_t bias);

void dts_dynrng (dts_state_t * state,
                 level_t (* call) (level_t, void *), void * data);

int dts_blocks_num (dts_state_t * state);
int dts_block (dts_state_t * state);

sample_t * dts_samples (dts_state_t * state);

void dts_free (dts_state_t * state);
*/

}; // namespace AudioFilter

#endif

// vim: ts=2 sts=2 et

