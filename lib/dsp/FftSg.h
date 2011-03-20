#pragma once
#ifndef AUDIOFILTER_FFTSG_H
#define AUDIOFILTER_FFTSG_H

#include <AudioFilter/Defs.h>

#ifdef __cplusplus
extern "C" {
#endif

void cdft(int, int, sample_t *, int *, sample_t *);
void rdft(int, int, sample_t *, int *, sample_t *);
void ddct(int, int, sample_t *, int *, sample_t *);
void ddst(int, int, sample_t *, int *, sample_t *);
void dfct(int, sample_t *, sample_t *, int *, sample_t *);
void dfst(int, sample_t *, sample_t *, int *, sample_t *);

#ifdef __cplusplus
}
#endif

#endif

