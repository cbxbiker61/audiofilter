#pragma once
#ifndef AUDIOFILTER_MPADEFS_H
#define AUDIOFILTER_MPADEFS_H

namespace AudioFilter {
/*
 * MPEG Audio parser definitions
 */

#define MPA_MIN_FRAME_SIZE 4096   // max frame size = ?????
#define MPA_MAX_FRAME_SIZE 4096   // max frame size = ?????

#define MPA_NCH      2            // number of channels
#define MPA_NSAMPLES 1152         // number of samples per frame
#define SBLIMIT      32           // number of samples per subband
#define SCALE_BLOCK  12           // number of fractions in frame

#define MPA_MODE_STEREO  0
#define MPA_MODE_JOINT   1
#define MPA_MODE_DUAL    2
#define MPA_MODE_SINGLE  3

#define MPA_LAYER_I   0
#define MPA_LAYER_II  1
#define MPA_LAYER_III 2

}; // namespace AudioFilter

#endif

// vim: ts=2 sts=2 et

