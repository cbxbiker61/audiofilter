#include <cstring>
#include <AudioFilter/MpaDefs.h>
#include <AudioFilter/MpaSynth.h>
#include "MpaSynthFilter.h"

namespace AudioFilter {

SynthBufferFPU::SynthBufferFPU()
{
  synth_offset = 64;
  memset(synth_buf, 0, sizeof(synth_buf));
}

void
SynthBufferFPU::reset(void)
{
  synth_offset = 64;
  memset(synth_buf, 0, sizeof(synth_buf));
}


void
SynthBufferFPU::synth(sample_t samples[32])
{
  //int i;
  //int j;

  synth_offset = (synth_offset - 64) & 0x3ff;
  sample_t *bufOffsetPtr = &(synth_buf[synth_offset]);

#define cos1_64  (sample_t)0.500602998235196
#define cos3_64  (sample_t)0.505470959897544
#define cos5_64  (sample_t)0.515447309922625
#define cos7_64  (sample_t)0.531042591089784
#define cos9_64  (sample_t)0.553103896034445
#define cos11_64 (sample_t)0.582934968206134
#define cos13_64 (sample_t)0.622504123035665
#define cos15_64 (sample_t)0.674808341455006
#define cos17_64 (sample_t)0.744536271002299
#define cos19_64 (sample_t)0.839349645415527
#define cos21_64 (sample_t)0.972568237861961
#define cos23_64 (sample_t)1.169439933432885
#define cos25_64 (sample_t)1.484164616314166
#define cos27_64 (sample_t)2.057781009953411
#define cos29_64 (sample_t)3.407608418468719
#define cos31_64 (sample_t)10.190008123548033
#define cos1_32  (sample_t)0.502419286188156
#define cos3_32  (sample_t)0.522498614939689
#define cos5_32  (sample_t)0.566944034816358
#define cos7_32  (sample_t)0.646821783359990
#define cos9_32  (sample_t)0.788154623451250
#define cos11_32 (sample_t)1.060677685990347
#define cos13_32 (sample_t)1.722447098238334
#define cos15_32 (sample_t)5.101148618689155
#define cos1_16  (sample_t)0.509795579104159
#define cos3_16  (sample_t)0.601344886935045
#define cos5_16  (sample_t)0.899976223136416
#define cos7_16  (sample_t)2.562915447741505
#define cos1_8   (sample_t)0.541196100146197
#define cos3_8   (sample_t)1.306562964876376
#define cos1_4   (sample_t)0.707106781186547

    {
      //int i;
      register sample_t tmp;
      sample_t p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13,p14,p15;
      sample_t pp0,pp1,pp2,pp3,pp4,pp5,pp6,pp7,pp8,pp9,pp10,pp11,pp12,pp13,pp14,pp15;

      // Compute new values via a fast cosine transform
      p0  = samples[0] + samples[31];
      p1  = samples[1] + samples[30];
      p2  = samples[2] + samples[29];
      p3  = samples[3] + samples[28];
      p4  = samples[4] + samples[27];
      p5  = samples[5] + samples[26];
      p6  = samples[6] + samples[25];
      p7  = samples[7] + samples[24];
      p8  = samples[8] + samples[23];
      p9  = samples[9] + samples[22];
      p10 = samples[10]+ samples[21];
      p11 = samples[11]+ samples[20];
      p12 = samples[12]+ samples[19];
      p13 = samples[13]+ samples[18];
      p14 = samples[14]+ samples[17];
      p15 = samples[15]+ samples[16];

      pp0 = p0 + p15;
      pp1 = p1 + p14;
      pp2 = p2 + p13;
      pp3 = p3 + p12;
      pp4 = p4 + p11;
      pp5 = p5 + p10;
      pp6 = p6 + p9;
      pp7 = p7 + p8;
      pp8 = cos1_32  * (p0 - p15);
      pp9 = cos3_32  * (p1 - p14);
      pp10= cos5_32  * (p2 - p13);
      pp11= cos7_32  * (p3 - p12);
      pp12= cos9_32  * (p4 - p11);
      pp13= cos11_32 * (p5 - p10);
      pp14= cos13_32 * (p6 - p9);
      pp15= cos15_32 * (p7 - p8);

      p0  = pp0 + pp7;
      p1  = pp1 + pp6;
      p2  = pp2 + pp5;
      p3  = pp3 + pp4;
      p4  = cos1_16 * (pp0 - pp7);
      p5  = cos3_16 * (pp1 - pp6);
      p6  = cos5_16 * (pp2 - pp5);
      p7  = cos7_16 * (pp3 - pp4);
      p8  = pp8 + pp15;
      p9  = pp9 + pp14;
      p10 = pp10 + pp13;
      p11 = pp11 + pp12;
      p12 = cos1_16 * (pp8  - pp15);
      p13 = cos3_16 * (pp9  - pp14);
      p14 = cos5_16 * (pp10 - pp13);
      p15 = cos7_16 * (pp11 - pp12);

      pp0 = p0 + p3;
      pp1 = p1 + p2;
      pp2 = cos1_8 * (p0 - p3);
      pp3 = cos3_8 * (p1 - p2);
      pp4 = p4 + p7;
      pp5 = p5 + p6;
      pp6 = cos1_8 * (p4 - p7);
      pp7 = cos3_8 * (p5 - p6);
      pp8 = p8 + p11;
      pp9 = p9 + p10;
      pp10= cos1_8 * (p8 - p11);
      pp11= cos3_8 * (p9 - p10);
      pp12= p12 + p15;
      pp13= p13 + p14;
      pp14= cos1_8 * (p12 - p15);
      pp15= cos3_8 * (p13 - p14);

      p0  = pp0 + pp1;
      p1  = cos1_4 * (pp0 - pp1);
      p2  = pp2 + pp3;
      p3  = cos1_4 * (pp2 - pp3);
      p4  = pp4 + pp5;
      p5  = cos1_4 * (pp4 - pp5);
      p6  = pp6 + pp7;
      p7  = cos1_4 * (pp6 - pp7);
      p8  = pp8 + pp9;
      p9  = cos1_4 * (pp8 - pp9);
      p10 = pp10 + pp11;
      p11 = cos1_4 * (pp10 - pp11);
      p12 = pp12 + pp13;
      p13 = cos1_4 * (pp12 - pp13);
      p14 = pp14 + pp15;
      p15 = cos1_4 * (pp14 - pp15);

      tmp              = p6 + p7;
      bufOffsetPtr[36] = -(p5 + tmp);
      bufOffsetPtr[44] = -(p4 + tmp);
      tmp              = p11 + p15;
      bufOffsetPtr[10] = tmp;
      bufOffsetPtr[6]  = p13 + tmp;
      tmp              = p14 + p15;
      bufOffsetPtr[46] = -(p8  + p12 + tmp);
      bufOffsetPtr[34] = -(p9  + p13 + tmp);
      tmp             += p10 + p11;
      bufOffsetPtr[38] = -(p13 + tmp);
      bufOffsetPtr[42] = -(p12 + tmp);
      bufOffsetPtr[2]  = p9 + p13 + p15;
      bufOffsetPtr[4]  = p5 + p7;
      bufOffsetPtr[48] = -p0;
      bufOffsetPtr[0]  = p1;
      bufOffsetPtr[8]  = p3;
      bufOffsetPtr[12] = p7;
      bufOffsetPtr[14] = p15;
      bufOffsetPtr[40] = -(p2  + p3);

      p0  = cos1_64  * (samples[0] - samples[31]);
      p1  = cos3_64  * (samples[1] - samples[30]);
      p2  = cos5_64  * (samples[2] - samples[29]);
      p3  = cos7_64  * (samples[3] - samples[28]);
      p4  = cos9_64  * (samples[4] - samples[27]);
      p5  = cos11_64 * (samples[5] - samples[26]);
      p6  = cos13_64 * (samples[6] - samples[25]);
      p7  = cos15_64 * (samples[7] - samples[24]);
      p8  = cos17_64 * (samples[8] - samples[23]);
      p9  = cos19_64 * (samples[9] - samples[22]);
      p10 = cos21_64 * (samples[10]- samples[21]);
      p11 = cos23_64 * (samples[11]- samples[20]);
      p12 = cos25_64 * (samples[12]- samples[19]);
      p13 = cos27_64 * (samples[13]- samples[18]);
      p14 = cos29_64 * (samples[14]- samples[17]);
      p15 = cos31_64 * (samples[15]- samples[16]);

      pp0 = p0 + p15;
      pp1 = p1 + p14;
      pp2 = p2 + p13;
      pp3 = p3 + p12;
      pp4 = p4 + p11;
      pp5 = p5 + p10;
      pp6 = p6 + p9;
      pp7 = p7 + p8;
      pp8 = cos1_32  * (p0 - p15);
      pp9 = cos3_32  * (p1 - p14);
      pp10= cos5_32  * (p2 - p13);
      pp11= cos7_32  * (p3 - p12);
      pp12= cos9_32  * (p4 - p11);
      pp13= cos11_32 * (p5 - p10);
      pp14= cos13_32 * (p6 - p9);
      pp15= cos15_32 * (p7 - p8);

      p0  = pp0 + pp7;
      p1  = pp1 + pp6;
      p2  = pp2 + pp5;
      p3  = pp3 + pp4;
      p4  = cos1_16 * (pp0 - pp7);
      p5  = cos3_16 * (pp1 - pp6);
      p6  = cos5_16 * (pp2 - pp5);
      p7  = cos7_16 * (pp3 - pp4);
      p8  = pp8  + pp15;
      p9  = pp9  + pp14;
      p10 = pp10 + pp13;
      p11 = pp11 + pp12;
      p12 = cos1_16 * (pp8  - pp15);
      p13 = cos3_16 * (pp9  - pp14);
      p14 = cos5_16 * (pp10 - pp13);
      p15 = cos7_16 * (pp11 - pp12);

      pp0 = p0 + p3;
      pp1 = p1 + p2;
      pp2 = cos1_8 * (p0 - p3);
      pp3 = cos3_8 * (p1 - p2);
      pp4 = p4 + p7;
      pp5 = p5 + p6;
      pp6 = cos1_8 * (p4 - p7);
      pp7 = cos3_8 * (p5 - p6);
      pp8 = p8 + p11;
      pp9 = p9 + p10;
      pp10= cos1_8 * (p8 - p11);
      pp11= cos3_8 * (p9 - p10);
      pp12= p12 + p15;
      pp13= p13 + p14;
      pp14= cos1_8 * (p12 - p15);
      pp15= cos3_8 * (p13 - p14);

      p0  = pp0 + pp1;
      p1  = cos1_4 * (pp0 - pp1);
      p2  = pp2 + pp3;
      p3  = cos1_4 * (pp2 - pp3);
      p4  = pp4 + pp5;
      p5  = cos1_4 * (pp4 - pp5);
      p6  = pp6 + pp7;
      p7  = cos1_4 * (pp6 - pp7);
      p8  = pp8 + pp9;
      p9  = cos1_4 * (pp8 - pp9);
      p10 = pp10 + pp11;
      p11 = cos1_4 * (pp10 - pp11);
      p12 = pp12 + pp13;
      p13 = cos1_4 * (pp12 - pp13);
      p14 = pp14 + pp15;
      p15 = cos1_4 * (pp14 - pp15);

      tmp              = p13 + p15;
      bufOffsetPtr[1]  = p1 + p9 + tmp;
      bufOffsetPtr[5]  = p5 + p7 + p11 + tmp;
      tmp             += p9;
      bufOffsetPtr[33] = -(p1 + p14 + tmp);
      tmp             += p5 + p7;
      bufOffsetPtr[3]  = tmp;
      bufOffsetPtr[35] = -(p6 + p14 + tmp);
      tmp              = p10 + p11 + p12 + p13 + p14 + p15;
      bufOffsetPtr[39] = -(p2 + p3 + tmp - p12);
      bufOffsetPtr[43] = -(p4 + p6 + p7 + tmp - p13);
      bufOffsetPtr[37] = -(p5 + p6 + p7 + tmp - p12);
      bufOffsetPtr[41] = -(p2 + p3 + tmp - p13);
      tmp              = p8 + p12 + p14 + p15;
      bufOffsetPtr[47] = -(p0 + tmp);
      bufOffsetPtr[45] = -(p4 + p6 + p7 + tmp);
      tmp              = p11 + p15;
      bufOffsetPtr[11] = p7  + tmp;
      tmp             += p3;
      bufOffsetPtr[9]  = tmp;
      bufOffsetPtr[7]  = p13 + tmp;
      bufOffsetPtr[13] = p7 + p15;
      bufOffsetPtr[15] = p15;

      bufOffsetPtr[16] = 0.0;

      for ( int i = 0; i < 16; ++i )
      {
        bufOffsetPtr[32-i] = -bufOffsetPtr[i];
        bufOffsetPtr[63-i] = bufOffsetPtr[33+i];
      }
  }

  {
    sample_t* dt[16];

    for ( int i = 0; i < 16; ++i )
    {
      dt[i] = &(synth_buf[( (((i << 5) + (((i+1) >> 1) << 6)) +
        synth_offset) & 0x3ff)]);
    }

    for ( int j = 0; j < 32; ++j )
    {
      samples[j] =
        window[j] * dt[0][j]+
        window[j + 32] * dt[1][j]+
        window[j + 64] * dt[2][j]+
        window[j + 96] * dt[3][j]+
        window[j + 128] * dt[4][j]+
        window[j + 160] * dt[5][j]+
        window[j + 192] * dt[6][j]+
        window[j + 224] * dt[7][j]+
        window[j + 256] * dt[8][j]+
        window[j + 288] * dt[9][j]+
        window[j + 320] * dt[10][j]+
        window[j + 352] * dt[11][j]+
        window[j + 384] * dt[12][j]+
        window[j + 416] * dt[13][j]+
        window[j + 448] * dt[14][j]+
        window[j + 480] * dt[15][j];
    }
  }
}

}; // namespace AudioFilter

// vim: ts=2 sts=2 et

