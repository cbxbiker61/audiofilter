#pragma once
#ifndef AUDIOFILTER_AC3TABLES_H
#define AUDIOFILTER_AC3TABLES_H

namespace AudioFilter {

///////////////////////////////////////////////////////////////////////////////
/////////////////////// Bitstream information tables //////////////////////////
///////////////////////////////////////////////////////////////////////////////

const int acmod2mask_tbl[] =
{
  MODE_2_0,
  MODE_1_0,
  MODE_2_0,
  MODE_3_0,
  MODE_2_1,
  MODE_3_1,
  MODE_2_2,
  MODE_3_2,
  MODE_2_0 | CH_MASK_LFE,
  MODE_1_0 | CH_MASK_LFE,
  MODE_2_0 | CH_MASK_LFE,
  MODE_3_0 | CH_MASK_LFE,
  MODE_2_1 | CH_MASK_LFE,
  MODE_3_1 | CH_MASK_LFE,
  MODE_2_2 | CH_MASK_LFE,
  MODE_3_2 | CH_MASK_LFE,
};

///////////////////////////////////////////////////////////////////////////////
// Bitrates table
//
// Mnemonics: bitrate = rate[code];
// Usage: used to obtain bitrate in kbps from bitrate code

const int bitrate_tbl[] =
{
  32,  40,  48,  56,  64,  80,  96, 112,
 128, 160, 192, 224, 256, 320, 384, 448,
 512, 576, 640
};

// Center/surround mix levels
//
// Mnemonics:
//   clev = clev_tbl[code];
//   slev = slev_tbl[code];
// Usage: used to obtain center/surround mix levels from code

const sample_t clev_tbl[4] = {LEVEL_3DB, LEVEL_45DB, LEVEL_6DB, LEVEL_45DB};
const sample_t slev_tbl[4] = {LEVEL_3DB, LEVEL_6DB, 0, LEVEL_6DB};

// Half-rate
//
// Mnemonics: halfrate = halfrate_tbl[bsid];
// Usage: used to obtain halfrate factor from bsid

const uint8_t halfrate_tbl[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3};


///////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Block parsing /////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Rematrix band

const int rematrix_tbl[4] = { 25, 37, 61, 253 };


///////////////////////////////////////////////////////////////////////////////
// Number of full-bandwidth channels

const int nfchans_tbl[8] =  { 2, 1, 2, 3, 3, 4, 4, 5 };


///////////////////////////////////////////////////////////////////////////////
// bit allocation tables
const int8_t  sdecay_tbl[4] = { 0x0f, 0x11, 0x13, 0x15 };
const int8_t  fdecay_tbl[4] = { 0x3f, 0x53, 0x67, 0x7b };
const int16_t sgain_tbl[4]  = { 0x0540, 0x04d8, 0x0478, 0x0410 };
const int16_t dbknee_tbl[4] = { 0x0000, 0x0700, 0x0900, 0x0b00 };
const int16_t floor_tbl[8]  = { 0x02f0, 0x02b0, 0x0270, 0x0230, 0x01f0, 0x0170, 0x00f0, -0x0800 };
const int16_t fgain_tbl[8]  = { 0x0080, 0x0100, 0x0180, 0x0200, 0x0280, 0x0300, 0x0380, 0x0400 };


///////////////////////////////////////////////////////////////////////////////
// Scale factors
//
// Used in: parse_block()
// Mnemonics: d = scale_factor[i]
// Definition: scale_factor[i] = 1/2^(i+15)
// Usage:
//   Used to obtain floating-point value from integer mantissa
//   and exponent values:
//     coef = mant * scale_factor[exp]
//
//     mant - integer 16-bit signed value interpreted as fixed
//            point value in range [-1;1]; or
//            floating-point value multiplied by 2^15
//     exp -  negative binary exponent
//

const sample_t scale_factor[25] =
{
  0.000030517578125,
  0.0000152587890625,
  0.00000762939453125,
  0.000003814697265625,
  0.0000019073486328125,
  0.00000095367431640625,
  0.000000476837158203125,
  0.0000002384185791015625,
  0.00000011920928955078125,
  0.000000059604644775390625,
  0.0000000298023223876953125,
  0.00000001490116119384765625,
  0.000000007450580596923828125,
  0.0000000037252902984619140625,
  0.00000000186264514923095703125,
  0.000000000931322574615478515625,
  0.0000000004656612873077392578125,
  0.00000000023283064365386962890625,
  0.000000000116415321826934814453125,
  0.0000000000582076609134674072265625,
  0.00000000002910383045673370361328125,
  0.000000000014551915228366851806640625,
  0.0000000000072759576141834259033203125,
  0.00000000000363797880709171295166015625,
  0.000000000001818989403545856475830078125
};

///////////////////////////////////////////////////////////////////////////////
// Symmetric quantization tables
//
// Used in: parse_block() (decoding of mantissas)
// Mnemonics: mant = qx_tbl[code]
// Definition: qx_tbl[code] = mant[x][code] / quant_levels[x] * 2^15
// Usage:
//   Used to obtain mantissa value from coded representation
//   coef = qx_tbl[code] * scale_factor[exp];
//   code - mantissa code given from bitstream
//   exp - negative binary exponent

const sample_t q3_tbl[3] =
{
  sample_t(- 2 << 15) / 3,
  0,
  sample_t(+ 2 << 15) / 3
};

const sample_t q5_tbl[5] =
{
  sample_t(- 4 << 15) / 5,
  sample_t(- 2 << 15) / 5,
  0,
  sample_t(+ 2 << 15) / 5,
  sample_t(+ 4 << 15) / 5
};

const sample_t q7_tbl[7] =
{
  sample_t(- 6 << 15) / 7,
  sample_t(- 4 << 15) / 7,
  sample_t(- 2 << 15) / 7,
  0,
  sample_t(+ 2 << 15) / 7,
  sample_t(+ 4 << 15) / 7,
  sample_t(+ 6 << 15) / 7
};

const sample_t q11_tbl[11] =
{
  sample_t(-10 << 15) / 11,
  sample_t(- 8 << 15) / 11,
  sample_t(- 6 << 15) / 11,
  sample_t(- 4 << 15) / 11,
  sample_t(- 2 << 15) / 11,
  0,
  sample_t(+ 2 << 15) / 11,
  sample_t(+ 4 << 15) / 11,
  sample_t(+ 6 << 15) / 11,
  sample_t(+ 8 << 15) / 11,
  sample_t(+10 << 15) / 11,
};

const sample_t q15_tbl[15] =
{
  sample_t(-14 << 15) / 15,
  sample_t(-12 << 15) / 15,
  sample_t(-10 << 15) / 15,
  sample_t(- 8 << 15) / 15,
  sample_t(- 6 << 15) / 15,
  sample_t(- 4 << 15) / 15,
  sample_t(- 2 << 15) / 15,
  0,
  sample_t(+ 2 << 15) / 15,
  sample_t(+ 4 << 15) / 15,
  sample_t(+ 6 << 15) / 15,
  sample_t(+ 8 << 15) / 15,
  sample_t(+10 << 15) / 15,
  sample_t(+12 << 15) / 15,
  sample_t(+14 << 15) / 15
};

#define Q0 ((-2 << 15) / 3.0)
#define Q1 (0)
#define Q2 ((2 << 15) / 3.0)

const sample_t q3_1_tbl[32] = {
    Q0,Q0,Q0,Q0,Q0,Q0,Q0,Q0,Q0,
    Q1,Q1,Q1,Q1,Q1,Q1,Q1,Q1,Q1,
    Q2,Q2,Q2,Q2,Q2,Q2,Q2,Q2,Q2,
    0,0,0,0,0
};

const sample_t q3_2_tbl[32] = {
    Q0,Q0,Q0,Q1,Q1,Q1,Q2,Q2,Q2,
    Q0,Q0,Q0,Q1,Q1,Q1,Q2,Q2,Q2,
    Q0,Q0,Q0,Q1,Q1,Q1,Q2,Q2,Q2,
    0,0,0,0,0
};

const sample_t q3_3_tbl[32] = {
    Q0,Q1,Q2,Q0,Q1,Q2,Q0,Q1,Q2,
    Q0,Q1,Q2,Q0,Q1,Q2,Q0,Q1,Q2,
    Q0,Q1,Q2,Q0,Q1,Q2,Q0,Q1,Q2,
    0,0,0,0,0
};

#undef Q0
#undef Q1
#undef Q2

#define Q0 ((-4 << 15) / 5.0)
#define Q1 ((-2 << 15) / 5.0)
#define Q2 (0)
#define Q3 ((2 << 15) / 5.0)
#define Q4 ((4 << 15) / 5.0)

static const sample_t q5_1_tbl[128] = {
    Q0,Q0,Q0,Q0,Q0,Q0,Q0,Q0,Q0,Q0,Q0,Q0,Q0,Q0,Q0,Q0,Q0,Q0,Q0,Q0,Q0,Q0,Q0,Q0,Q0,
    Q1,Q1,Q1,Q1,Q1,Q1,Q1,Q1,Q1,Q1,Q1,Q1,Q1,Q1,Q1,Q1,Q1,Q1,Q1,Q1,Q1,Q1,Q1,Q1,Q1,
    Q2,Q2,Q2,Q2,Q2,Q2,Q2,Q2,Q2,Q2,Q2,Q2,Q2,Q2,Q2,Q2,Q2,Q2,Q2,Q2,Q2,Q2,Q2,Q2,Q2,
    Q3,Q3,Q3,Q3,Q3,Q3,Q3,Q3,Q3,Q3,Q3,Q3,Q3,Q3,Q3,Q3,Q3,Q3,Q3,Q3,Q3,Q3,Q3,Q3,Q3,
    Q4,Q4,Q4,Q4,Q4,Q4,Q4,Q4,Q4,Q4,Q4,Q4,Q4,Q4,Q4,Q4,Q4,Q4,Q4,Q4,Q4,Q4,Q4,Q4,Q4,
    0,0,0
};

static const sample_t q5_2_tbl[128] = {
    Q0,Q0,Q0,Q0,Q0,Q1,Q1,Q1,Q1,Q1,Q2,Q2,Q2,Q2,Q2,Q3,Q3,Q3,Q3,Q3,Q4,Q4,Q4,Q4,Q4,
    Q0,Q0,Q0,Q0,Q0,Q1,Q1,Q1,Q1,Q1,Q2,Q2,Q2,Q2,Q2,Q3,Q3,Q3,Q3,Q3,Q4,Q4,Q4,Q4,Q4,
    Q0,Q0,Q0,Q0,Q0,Q1,Q1,Q1,Q1,Q1,Q2,Q2,Q2,Q2,Q2,Q3,Q3,Q3,Q3,Q3,Q4,Q4,Q4,Q4,Q4,
    Q0,Q0,Q0,Q0,Q0,Q1,Q1,Q1,Q1,Q1,Q2,Q2,Q2,Q2,Q2,Q3,Q3,Q3,Q3,Q3,Q4,Q4,Q4,Q4,Q4,
    Q0,Q0,Q0,Q0,Q0,Q1,Q1,Q1,Q1,Q1,Q2,Q2,Q2,Q2,Q2,Q3,Q3,Q3,Q3,Q3,Q4,Q4,Q4,Q4,Q4,
    0,0,0
};

static const sample_t q5_3_tbl[128] = {
    Q0,Q1,Q2,Q3,Q4,Q0,Q1,Q2,Q3,Q4,Q0,Q1,Q2,Q3,Q4,Q0,Q1,Q2,Q3,Q4,Q0,Q1,Q2,Q3,Q4,
    Q0,Q1,Q2,Q3,Q4,Q0,Q1,Q2,Q3,Q4,Q0,Q1,Q2,Q3,Q4,Q0,Q1,Q2,Q3,Q4,Q0,Q1,Q2,Q3,Q4,
    Q0,Q1,Q2,Q3,Q4,Q0,Q1,Q2,Q3,Q4,Q0,Q1,Q2,Q3,Q4,Q0,Q1,Q2,Q3,Q4,Q0,Q1,Q2,Q3,Q4,
    Q0,Q1,Q2,Q3,Q4,Q0,Q1,Q2,Q3,Q4,Q0,Q1,Q2,Q3,Q4,Q0,Q1,Q2,Q3,Q4,Q0,Q1,Q2,Q3,Q4,
    Q0,Q1,Q2,Q3,Q4,Q0,Q1,Q2,Q3,Q4,Q0,Q1,Q2,Q3,Q4,Q0,Q1,Q2,Q3,Q4,Q0,Q1,Q2,Q3,Q4,
    0,0,0
};

#undef Q0
#undef Q1
#undef Q2
#undef Q3
#undef Q4

#define Q0 ((-10 << 15) / 11.0)
#define Q1 ((-8 << 15) / 11.0)
#define Q2 ((-6 << 15) / 11.0)
#define Q3 ((-4 << 15) / 11.0)
#define Q4 ((-2 << 15) / 11.0)
#define Q5 (0)
#define Q6 ((2 << 15) / 11.0)
#define Q7 ((4 << 15) / 11.0)
#define Q8 ((6 << 15) / 11.0)
#define Q9 ((8 << 15) / 11.0)
#define QA ((10 << 15) / 11.0)

static const sample_t q11_1_tbl[128] = {
    Q0, Q0, Q0, Q0, Q0, Q0, Q0, Q0, Q0, Q0, Q0,
    Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1,
    Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2,
    Q3, Q3, Q3, Q3, Q3, Q3, Q3, Q3, Q3, Q3, Q3,
    Q4, Q4, Q4, Q4, Q4, Q4, Q4, Q4, Q4, Q4, Q4,
    Q5, Q5, Q5, Q5, Q5, Q5, Q5, Q5, Q5, Q5, Q5,
    Q6, Q6, Q6, Q6, Q6, Q6, Q6, Q6, Q6, Q6, Q6,
    Q7, Q7, Q7, Q7, Q7, Q7, Q7, Q7, Q7, Q7, Q7,
    Q8, Q8, Q8, Q8, Q8, Q8, Q8, Q8, Q8, Q8, Q8,
    Q9, Q9, Q9, Q9, Q9, Q9, Q9, Q9, Q9, Q9, Q9,
    QA, QA, QA, QA, QA, QA, QA, QA, QA, QA, QA,
    0,  0,  0,  0,  0,  0,  0
};

static const sample_t q11_2_tbl[128] = {
    Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, Q8, Q9, QA,
    Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, Q8, Q9, QA,
    Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, Q8, Q9, QA,
    Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, Q8, Q9, QA,
    Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, Q8, Q9, QA,
    Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, Q8, Q9, QA,
    Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, Q8, Q9, QA,
    Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, Q8, Q9, QA,
    Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, Q8, Q9, QA,
    Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, Q8, Q9, QA,
    Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, Q8, Q9, QA,
    0,  0,  0,  0,  0,  0,  0
};

#undef Q0
#undef Q1
#undef Q2
#undef Q3
#undef Q4
#undef Q5
#undef Q6
#undef Q7
#undef Q8
#undef Q9
#undef QA


///////////////////////////////////////////////////////////////////////////////
// Exponent decoding tables
//
// Used in: parse_exponents()
// Mnemonics: exp = exp_x[exp_group]
// Definition:
//   exp_1[exp_group] = (exp_group / 25) - 2
//   exp_2[exp_group] = (exp_group % 25) / 5 - 2
//   exp_3[exp_group] = (exp_group %  5) % 5 - 2
// Usage:
//   Used to ungroup differential exponents from exponent group.

const int8_t exp1_tbl[128] = {
    -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    25,25,25
};

const int8_t exp2_tbl[128] = {
    -2,-2,-2,-2,-2,-1,-1,-1,-1,-1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
    -2,-2,-2,-2,-2,-1,-1,-1,-1,-1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
    -2,-2,-2,-2,-2,-1,-1,-1,-1,-1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
    -2,-2,-2,-2,-2,-1,-1,-1,-1,-1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
    -2,-2,-2,-2,-2,-1,-1,-1,-1,-1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
    25,25,25
};

const int8_t exp3_tbl[128] = {
    -2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,
    -2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,
    -2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,
    -2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,
    -2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,
    25,25,25
};



///////////////////////////////////////////////////////////////////////////////
////////////////////////////// Bit allocation /////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

const int slowgain[4] = {0x540, 0x4d8, 0x478, 0x410};
const int dbpbtab[4]  = {0xc00, 0x500, 0x300, 0x100};
const int floortab[8] = {0x910, 0x950, 0x990, 0x9d0,
                         0xa10, 0xa90, 0xb10, 0x1400};


}; // namespace AudioFilter

// vim: ts=2 sts=2 et

#endif

