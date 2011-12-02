#include <stdlib.h>
#include <string.h>
#include <Ac3Filter/Crc.h>
#include "Ac3BitAlloc.h"
#include "Ac3Enc.h"

#define EXP_REUSE 0
#define EXP_D15   1
#define EXP_D25   2
#define EXP_D45   3

#define DELTA_BIT_REUSE    0
#define DELTA_BIT_NEW      1
#define DELTA_BIT_NONE     2
#define DELTA_BIT_RESERVED 3

// exponent variation threshold
// exponent set will be reused if variation between blocks is less
#define EXP_DIFF_THRESHOLD 5000

#define CRC16_POLY ((1 << 0) | (1 << 2) | (1 << 15) | (1 << 16))

#define min(a, b) ((a) < (b)? (a): (b))
#define max(a, b) ((a) > (b)? (a): (b))
inline int bits_left(int32_t v);
inline unsigned int mul_poly(unsigned int a, unsigned int b, unsigned int poly);
inline unsigned int pow_poly(unsigned int a, unsigned int n, unsigned int poly);

// freq_tbl[halfratecod][fscod]
// freq_tbl[halfratecod][fscod] = freq_tbl[0][fscod] >> halfratecod;

const int freq_tbl[9] =
{ 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025,  8000 };

const int bitrate_tbl[19] =
{
   32000,  40000,  48000,  56000,  64000,  80000,  96000, 112000, 128000,
  160000, 192000, 224000, 256000, 320000, 384000, 448000, 512000, 576000, 640000
};

const sample_t ac3_window[256] =
{
  0.00014, 0.00024, 0.00037, 0.00051, 0.00067, 0.00086, 0.00107, 0.00130,
  0.00157, 0.00187, 0.00220, 0.00256, 0.00297, 0.00341, 0.00390, 0.00443,
  0.00501, 0.00564, 0.00632, 0.00706, 0.00785, 0.00871, 0.00962, 0.01061,
  0.01166, 0.01279, 0.01399, 0.01526, 0.01662, 0.01806, 0.01959, 0.02121,
  0.02292, 0.02472, 0.02662, 0.02863, 0.03073, 0.03294, 0.03527, 0.03770,
  0.04025, 0.04292, 0.04571, 0.04862, 0.05165, 0.05481, 0.05810, 0.06153,
  0.06508, 0.06878, 0.07261, 0.07658, 0.08069, 0.08495, 0.08935, 0.09389,
  0.09859, 0.10343, 0.10842, 0.11356, 0.11885, 0.12429, 0.12988, 0.13563,
  0.14152, 0.14757, 0.15376, 0.16011, 0.16661, 0.17325, 0.18005, 0.18699,
  0.19407, 0.20130, 0.20867, 0.21618, 0.22382, 0.23161, 0.23952, 0.24757,
  0.25574, 0.26404, 0.27246, 0.28100, 0.28965, 0.29841, 0.30729, 0.31626,
  0.32533, 0.33450, 0.34376, 0.35311, 0.36253, 0.37204, 0.38161, 0.39126,
  0.40096, 0.41072, 0.42054, 0.43040, 0.44030, 0.45023, 0.46020, 0.47019,
  0.48020, 0.49022, 0.50025, 0.51028, 0.52031, 0.53033, 0.54033, 0.55031,
  0.56026, 0.57019, 0.58007, 0.58991, 0.59970, 0.60944, 0.61912, 0.62873,
  0.63827, 0.64774, 0.65713, 0.66643, 0.67564, 0.68476, 0.69377, 0.70269,
  0.71150, 0.72019, 0.72877, 0.73723, 0.74557, 0.75378, 0.76186, 0.76981,
  0.77762, 0.78530, 0.79283, 0.80022, 0.80747, 0.81457, 0.82151, 0.82831,
  0.83496, 0.84145, 0.84779, 0.85398, 0.86001, 0.86588, 0.87160, 0.87716,
  0.88257, 0.88782, 0.89291, 0.89785, 0.90264, 0.90728, 0.91176, 0.91610,
  0.92028, 0.92432, 0.92822, 0.93197, 0.93558, 0.93906, 0.94240, 0.94560,
  0.94867, 0.95162, 0.95444, 0.95713, 0.95971, 0.96217, 0.96451, 0.96674,
  0.96887, 0.97089, 0.97281, 0.97463, 0.97635, 0.97799, 0.97953, 0.98099,
  0.98236, 0.98366, 0.98488, 0.98602, 0.98710, 0.98811, 0.98905, 0.98994,
  0.99076, 0.99153, 0.99225, 0.99291, 0.99353, 0.99411, 0.99464, 0.99513,
  0.99558, 0.99600, 0.99639, 0.99674, 0.99706, 0.99736, 0.99763, 0.99788,
  0.99811, 0.99831, 0.99850, 0.99867, 0.99882, 0.99895, 0.99908, 0.99919,
  0.99929, 0.99938, 0.99946, 0.99953, 0.99959, 0.99965, 0.99969, 0.99974,
  0.99978, 0.99981, 0.99984, 0.99986, 0.99988, 0.99990, 0.99992, 0.99993,
  0.99994, 0.99995, 0.99996, 0.99997, 0.99998, 0.99998, 0.99998, 0.99999,
  0.99999, 0.99999, 0.99999, 1.00000, 1.00000, 1.00000, 1.00000, 1.00000,
  1.00000, 1.00000, 1.00000, 1.00000, 1.00000, 1.00000, 1.00000, 1.00000
};

// bit allocation tables
const uint8_t  sdecay_tbl[4] = { 0x0f, 0x11, 0x13, 0x15 };
const uint8_t  fdecay_tbl[4] = { 0x3f, 0x53, 0x67, 0x7b };
const uint16_t sgain_tbl[4]  = { 0x0540, 0x04d8, 0x0478, 0x0410 };
const uint16_t dbknee_tbl[4] = { 0x0000, 0x0700, 0x0900, 0x0b00 };
const uint16_t floor_tbl[8]  = { 0x02f0, 0x02b0, 0x0270, 0x0230, 0x01f0, 0x0170, 0x00f0, 0xf800 };
const uint16_t fgain_tbl[8]  = { 0x0080, 0x0100, 0x0180, 0x0200, 0x0280, 0x0300, 0x0380, 0x0400 };

namespace AudioFilter {

Ac3Enc::Ac3Enc()
  : NullFilter(0) // use own query_input()
  , mdct(7)
{
  frames = 0;
  bitrate = 640000;
  frame_samples.allocate(AC3_NCHANNELS, AC3_FRAME_SAMPLES);
  frame_buf.allocate(AC3_MAX_FRAME_SIZE);
  window.allocate(1, AC3_BLOCK_SAMPLES);
  reset();
}

int Ac3Enc::get_bitrate(void) const
{
  return bitrate;
}

bool Ac3Enc::set_bitrate(int _bitrate)
{
  // check bitrate
  int i;
  for (i = 0; i < sizeof(bitrate_tbl) / sizeof(bitrate_tbl[0]); i++)
    if (bitrate_tbl[i] == _bitrate)
    {
      bitrate = _bitrate;
      reset();
      return true;
    }

  return false;
}

bool Ac3Enc::fill_buffer(void)
{
  size_t n(AC3_FRAME_SAMPLES - sample);

  if ( size < n )
  {
    for (int ch = 0; ch < spk.getChannelCount(); ++ch )
      memcpy(frame_samples[ch] + sample, samples[ch], size * sizeof(sample_t));

    sample += size;
    time += vtime_t(size) / spk.sample_rate;
    drop_samples(size);

    return false;
  }
  else
  {
    for ( int ch = 0; ch < spk.getChannelCount(); ++ch )
      memcpy(frame_samples[ch] + sample, samples[ch], n * sizeof(sample_t));

    sample = 0;
    time += vtime_t(n) / spk.sample_rate;
    drop_samples(n);

    return true;
  }
}

void Ac3Enc::reset(void)
{
  sample = 0;

  memset(delay, 0, sizeof(delay));

  for ( int ch = 0; ch < NCHANNELS; ++ch )
    delay_exp[ch] = 15; // indicate that we have 0 bits in delay array

  // reset bit allocation
  sdcycod   = 2;
  fdcycod   = 1;
  sgaincod  = 1;
  dbpbcod   = 2;
  floorcod  = 4;
  fgaincod  = 4;

  csnroffst = 0;
  fsnroffst = 0;
}

bool Ac3Enc::query_input(Speakers _spk) const
{
  if (_spk.format != FORMAT_LINEAR)
    return false;

  // check sample rate
  int i;
  for (i = 0; i < sizeof(freq_tbl) / sizeof(freq_tbl[0]); i++)
    if (freq_tbl[i] == _spk.sample_rate)
      break;

  if ( i == sizeof(freq_tbl) / sizeof(freq_tbl[0]) )
    return false;

  // check mask
  switch (_spk.mask)
  {
    case MODE_1_0: case MODE_1_0 | CH_MASK_LFE:
    case MODE_2_0: case MODE_2_0 | CH_MASK_LFE:
    case MODE_3_0: case MODE_3_0 | CH_MASK_LFE:
    case MODE_2_1: case MODE_2_1 | CH_MASK_LFE:
    case MODE_3_1: case MODE_3_1 | CH_MASK_LFE:
    case MODE_2_2: case MODE_2_2 | CH_MASK_LFE:
    case MODE_3_2: case MODE_3_2 | CH_MASK_LFE:
      break;
    default: return false;
  }

  // everything is ok
  return true;
}

bool Ac3Enc::setInput(Speakers _spk)
{
  if (!NullFilter::set_input(_spk))
    return false;

  // sample rate
  fscod = 0;
  for (fscod = 0; fscod < sizeof(freq_tbl) / sizeof(freq_tbl[0]); fscod++)
    if (freq_tbl[fscod] == _spk.sample_rate)
      break;

  if (fscod == sizeof(freq_tbl) / sizeof(freq_tbl[0]))
    return false;

  // bitrate
  for (frmsizecod = 0; frmsizecod < sizeof(bitrate_tbl) / sizeof(bitrate_tbl[0]); frmsizecod++)
    if (bitrate_tbl[frmsizecod] == bitrate)
      break;

  if (frmsizecod == sizeof(bitrate_tbl) / sizeof(bitrate_tbl[0]))
    return false;

  frmsizecod <<= 1;
  halfratecod = fscod / 3;
  fscod %= 3;
  bsid = 8 + halfratecod;

  spk         = _spk;
  sample_rate = _spk.sample_rate;
  frame_size  = (bitrate * AC3_BLOCK_SAMPLES * AC3_NBLOCKS) / sample_rate / 8;

  switch (spk.mask)
  {
    case MODE_1_0: case MODE_1_0 | CH_MASK_LFE: acmod = AC3_MODE_1_0; break;
    case MODE_2_0: case MODE_2_0 | CH_MASK_LFE: acmod = AC3_MODE_2_0; break;
    case MODE_3_0: case MODE_3_0 | CH_MASK_LFE: acmod = AC3_MODE_3_0; break;
    case MODE_2_1: case MODE_2_1 | CH_MASK_LFE: acmod = AC3_MODE_2_1; break;
    case MODE_3_1: case MODE_3_1 | CH_MASK_LFE: acmod = AC3_MODE_3_1; break;
    case MODE_2_2: case MODE_2_2 | CH_MASK_LFE: acmod = AC3_MODE_2_2; break;
    case MODE_3_2: case MODE_3_2 | CH_MASK_LFE: acmod = AC3_MODE_3_2; break;
    default: return false;
  }
  lfe     = spk.hasLfe();
  dolby   = (spk.mask == MODE_STEREO) && ((spk.relation == RELATION_DOLBY) || (spk.relation == RELATION_DOLBY2));
  nfchans = spk.hasLfe()? spk.getChannelCount() - 1: spk.getChannelCount();

  // scale window
  sample_t factor = 32768.0 / spk.level;
  for (int s = 0; s < AC3_BLOCK_SAMPLES; s++)
    window[0][s] = ac3_window[s] * factor;

  reset();

  return true;
}

bool Ac3Enc::get_chunk(Chunk *_chunk)
{
  // todo: output partially filled frame on flushing

  if (fill_buffer())
  {
    // encode frame
    if (!encode_frame())
      return false;

    // fill chunk
    _chunk->set_rawdata
    (
      get_output(),
      frame_buf, frame_size,
      sync, time,
      flushing && !size
    );
  }
  else
  {
    // dummy chunk
    _chunk->set_dummy();
  }

  // reset after flushing
  if (flushing && !size)
  {
    flushing = false;
    reset();
  }

  return true;
}

Speakers Ac3Enc::get_output(void) const
{
  return Speakers(FORMAT_AC3, spk.mask, spk.sample_rate, 1.0, spk.relation);
}

int Ac3Enc::encodeFrame(void)
{
  // todo: support non-standard channel ordering given with spk
  // todo: support for 24/32/float input sample formats
  // todo: support coupling (basic encoder)

  const int maxCh(spk.getChannelCount());

  // frame-wide data

  // channel-wide data
  int exp_norm[AC3_NBLOCKS]; // normalization

  // block-wide data
  int16_t mdct_buf[AC3_BLOCK_SAMPLES * 2];

  for ( int ch = 0; ch < maxCh; ++ch )
  {
    int endmant;

    if ( spk.hasLfe() && (ch == nfchans) )
      // lfe channel
      endmant = 7;
    else
    {
      // fbw channels
      chbwcod[ch] = 50;
      endmant = ((chbwcod[ch] + 12) * 3) + 37;
    }

    nmant[ch] = endmant;

    /////////////////////////////////////////////////////////////////
    // Compute exponents and mdct coeffitients
    // for all blocks from input data
    //
    for ( int b = 0; b < AC3_NBLOCKS; ++b )
    {
      // todo: silence threshold at absolute level of 128
      int exp_norm1 = 0;  // normalization for this block
      int exp_norm2 = 0;  // normalization for next block delay

      memcpy(mdct_buf, delay[ch], sizeof(delay[ch]));

      ///////////////////////////////////////////////////////////////
      // Form input for MDCT
      //

      sample_t v;
      sample_t *sptr = frame_samples[ch];
      sptr += b * AC3_BLOCK_SAMPLES;

      // * copy samples from input buffer to mdct and delay
      // * apply ac3 window to both mdct and delay halves
      // * compute the normalization for mdct (using
      //   delay normalization computed at previous block)
      //   and delay
      // optimize: unroll cycle
      for ( int s = 0; s < AC3_BLOCK_SAMPLES; ++s )
      {
        v = *sptr++;
        mdct_buf[s + 256] = int32_t(v * window[0][AC3_BLOCK_SAMPLES - s - 1]);
        delay[ch][s] = int32_t(v * window[0][s]);
        exp_norm1 |= abs(mdct_buf[s + 256]);
        exp_norm2 |= abs(delay[ch][s]);
      }

      // compute normalization
      exp_norm1 = 15 - bits_left(exp_norm1);
      exp_norm2 = 15 - bits_left(exp_norm2);

      exp_norm1     = min(exp_norm1, delay_exp[ch]);

      exp_norm[b]   = exp_norm1;
      delay_exp[ch] = exp_norm2;

      // normalize
      for ( int s = 0; s < AC3_BLOCK_SAMPLES * 2; ++s )
        mdct_buf[s] <<= exp_norm1;

      // finished with input
      // now we have normalized mdct_buf[] buffer
      // and noramlization exponent 'exp'
      ///////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////
      // MDCT and exponents computation
      // todo: mdct 256/512 switch

      mdct.mdct512(mant[ch][b], mdct_buf);

      // compute exponents
      // normalize mdct coeffitients
      // we take into account the normalization
      for ( int s = 0; s < AC3_BLOCK_SAMPLES; ++s )
      {
        exp_norm2 = 15 - bits_left(abs(mant[ch][b][s]));

        if (exp_norm2 == 15)
          exp[ch][b][s] = 24;
        else
        {
          if (exp_norm1 + exp_norm2 > 24)
          {
            exp[ch][b][s] = 24;
            mant[ch][b][s] >>= exp_norm1 + exp_norm2 - 24;
          }
          else
            exp[ch][b][s] = exp_norm1 + exp_norm2;
        }
      }

      // finished with MDCT and exponents
      ///////////////////////////////////////////////////////////////
    } // for (b = 0; b < AC3_NBLOCKS; b++)


    compute_expstr(expstr[ch], exp[ch], endmant);
    restrict_exp(expcod[ch], ngrps[ch], exp[ch], expstr[ch], endmant);

    // normalize mdct coefs
    int b1;
    for ( int b = 0; b < AC3_NBLOCKS; ++b )
    {
      if ( expstr[ch][b] != EXP_REUSE )
        b1 = b;

      for ( int s = 0; s < endmant; ++s )
        // note: it is possible that exp[ch][b1][s] < exp_norm[b]
        //       exponent may be decreased because of differential
        //       restricttions
        if (exp[ch][b1][s] - exp_norm[b] >= 0)
          mant[ch][b][s] <<= exp[ch][b1][s] - exp_norm[b];
        else
          mant[ch][b][s] >>= exp_norm[b] - exp[ch][b1][s];
    }

  } // for (ch = 0; ch < nch; ch++)


  // now we have computed exponents at exp[ch][b][s]
  // and mantissas at mant[ch][b][s]
  // mantissas are now in normalized form
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  // Compute bits left for mantissas

  //                       acmod = 0  1  2  3  4  5  6  7
  static const int bsi_plus[8] = { 0, 0, 2, 2, 2, 4, 2, 4  };
  int  bits_left;


  bits_left  = 0;
  bits_left += 40;                          // syncinfo
  bits_left += 25 + bsi_plus[acmod];        // bsi

  bits_left += 54;                          // dynrnge, cplstre, cplinu, baie,
                                            // sdcycod, fdcycod, sgaincod,
                                            // dbpbcod, floorcod, snroffste,
                                            // csnroffst, deltbaie, skiple
  bits_left += 31 * nfchans;                // blocksw, dithflag, chexpstr
                                            // fsnroffst, fgaincod
  if (acmod == 2)
  {
    bits_left += 6;                         // rematstr
    bits_left += 4;                         // rematflg[]
  }

  for ( int ch = 0; ch < nfchans; ++ch )
  {
    for ( int b = 0; b < AC3_NBLOCKS; ++b )
    }
      if ( expstr[ch][b] != EXP_REUSE )
      {
        bits_left += 12;                    // chbwcod, exps[0], gainrng
        bits_left += 7 * (ngrps[ch][b]);    // exps
      }
    }
  }

  if ( lfe )
  {
    bits_left += 13; // lfeexpstr, lfesnroffst, lfegaincod

    for ( int b = 0; b < AC3_NBLOCKS; ++b )
    {
      if ( expstr[nfchans] != EXP_REUSE )
      {
        bits_left += 4;                    // lfeexps[0]
        bits_left += 7 * ngrps[nfchans][b];// lfeexps
      }
    }
  }

  bits_left += 16; // CRC
  bits_left = frame_size * 8 - bits_left;

  // Finished with bits left
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  // Bit allocation

  int sdecay    = sdecay_tbl[sdcycod];
  int fdecay    = fdecay_tbl[fdcycod];
  int sgain     = sgain_tbl[sgaincod];
  int dbknee    = dbknee_tbl[dbpbcod];
  int floor     = floor_tbl[floorcod];
  int fgain     = fgain_tbl[fgaincod];

  const int snroffset_max = (((63 - 15) << 4) + 15) << 2;
  const int snroffset_min = (((0 - 15) << 4) + 0) << 2;
  int snroffset = (((csnroffst - 15) << 4) + fsnroffst) << 2;
  int ba_bits;
  int ba_block;
  int high, low; // bisection bounds

  BAP_BitCount block_bits;
  ba_bits = 0;
  ba_block = 0;

  for ( int ch = 0; ch < nch; ++ch )
  {
    for ( int b = 0; b < AC3_NBLOCKS; ++b )
    {
      if ( expstr[ch][b] != EXP_REUSE )
      {
        bit_alloc(
          bap[ch][b], exp[ch][b],
          DELTA_BIT_NONE, 0,
          0, nmant[ch],
          fscod, halfratecod,
          sdecay, fdecay,
          sgain, fgain,
          dbknee, floor,
          0, 0,
          snroffset);
        block_bits.reset();
        block_bits.add_bap(bap[ch][b], 0, nmant[ch]);
        ba_bits += block_bits.bits;
        ba_block = b;
      }
      else
      {
        memcpy(bap[ch][b], bap[ch][ba_block], sizeof(bap[0][0][0]) * nmant[ch]);
        ba_bits += block_bits.bits;
      }
    }
  }

  // bisection init
  if ( ba_bits > bits_left )
  {
    high = snroffset;
    low = snroffset_min;
  }
  else
  {
    high = snroffset_max;
    low = snroffset;
  }

  // bisection steps
  while ( high - low > 4 )
  {
    snroffset = ((high + low) >> 1) & ~3;

    ba_bits = 0;

    for ( int ch = 0; ch < nch; ++ch )
    {
      for ( int b = 0; b < AC3_NBLOCKS; ++b )
      {
        if ( expstr[ch][b] != EXP_REUSE )
        {
          bit_alloc(
            bap[ch][b], exp[ch][b],
            DELTA_BIT_NONE, 0,
            0, nmant[ch],
            fscod, halfratecod,
            sdecay, fdecay,
            sgain, fgain,
            dbknee, floor,
            0, 0,
            snroffset);
          block_bits.reset();
          block_bits.add_bap(bap[ch][b], 0, nmant[ch]);
          ba_bits += block_bits.bits;
          ba_block = b;
        }
        else
        {
          memcpy(bap[ch][b], bap[ch][ba_block], sizeof(bap[0][0][0]) * nmant[ch]);
          ba_bits += block_bits.bits;
        }
      }
    }

    if ( ba_bits > bits_left )
      high = snroffset;
    else
      low = snroffset;
  }

  // do bit allocation
  snroffset = low & ~3; // clear 2 last bits
  ba_bits = 0;

  for ( int ch = 0; ch < nch; ++ch )
  {
    for ( int b = 0; b < AC3_NBLOCKS; ++b )
    {
      if ( expstr[ch][b] != EXP_REUSE )
      {
        bit_alloc(
          bap[ch][b], exp[ch][b],
          DELTA_BIT_NONE, 0,
          0, nmant[ch],
          fscod, halfratecod,
          sdecay, fdecay,
          sgain, fgain,
          dbknee, floor,
          0, 0,
          snroffset);
        block_bits.reset();
        block_bits.add_bap(bap[ch][b], 0, nmant[ch]);
        ba_bits += block_bits.bits;
        ba_block = b;
      }
      else
      {
        memcpy(bap[ch][b], bap[ch][ba_block], sizeof(bap[0][0][0]) * nmant[ch]);
        ba_bits += block_bits.bits;
      }
    }
  }

  if ( ba_bits > bits_left )
    // some error happen!!!
    return 0;

  //  snroffset = (((csnroffst - 15) << 4) + fsnroffst) << 2;
  csnroffst = (snroffset >> 6) + 15;
  fsnroffst = (snroffset >> 2) - ((csnroffst - 15) << 4);

  // debug:
  assert(((((csnroffst - 15) << 4) + fsnroffst) << 2) == snroffset);

  // Finished with bit allocation
  ///////////////////////////////////////////////////////////////////


  ///////////////////////////////////////////////////////////////////
  // Output everything

  memset(frame_buf, 0, frame_size);
  bs.set(frame_buf, 0, frame_size * 8);

  ///////////////////////////////////////////////////////////////////
  // BSI

  bs.put(16, 0x0b77);// 'syncword'
  bs.put(16, 0);     // 'crc1'
  bs.put(2, fscod);  // 'fscod'
  bs.put(6, frmsizecod);
  bs.put(5, bsid);   // 'bsid'
  bs.put(3, 0);      // 'bsmod' = complete main audio service
  bs.put(3, acmod);

  if ( acmod & 1 && acmod != 1 )
    bs.put(2, 0);    // 'cmixlev' = -3dB

  if ( acmod & 4 )
    bs.put(2, 0);    // 'surmixlev' = -3dB

  if ( acmod == 2 )
  {
    if ( spk.relation )
      bs.put(2, 2);  // 'dsurmod' = Dolby surround encoded
    else
      bs.put(2, 0);  // 'dsurmod' = surround not indicated
  }

  bs.put_bool(spk.lfe()); // 'lfeon'
  bs.put(5, 31);     // 'dialnorm' = -31dB
  bs.put_bool(false);     // 'compre'
  bs.put_bool(false);     // 'langcode'
  bs.put_bool(false);     // 'audprodie'
  bs.put_bool(false);     // 'copyrightb'
  bs.put_bool(true);      // 'origbs'
  bs.put_bool(false);     // 'timecod1e'
  bs.put_bool(false);     // 'timecod2e'
  bs.put_bool(false);     // 'addbsie'


  ///////////////////////////////////////////////////////////////////
  // Audio blocks

  for ( int b = 0; b < AC3_NBLOCKS; ++b )
  {
    for ( int ch = 0; ch < nfchans; ++ch )
      bs.put_bool(false);              // 'blksw[ch]' - 512-tap mdct

    for ( int ch = 0; ch < nfchans; ++ch )
      bs.put_bool(true);               // 'dithflag[ch]' - use dithering

    bs.put_bool(false);                // 'dynrnge'

    if ( b == 0 )
    {
      bs.put_bool(true);               // 'cplstre'
      bs.put_bool(false);              // 'cplinu'
    }
    else
      bs.put_bool(false);              // 'cplstre'

    if ( acmod == AC3_MODE_STEREO )
    {
      if ( b == 0 )
      {
        bs.put_bool(true);             // 'rematstr'
        bs.put_bool(false);            // 'rematflg[0]'
        bs.put_bool(false);            // 'rematflg[1]'
        bs.put_bool(false);            // 'rematflg[2]'
        bs.put_bool(false);            // 'rematflg[3]'
      }
      else
        bs.put_bool(false);            // 'rematstr'
    }

    for ( int ch = 0; ch < nfchans; ++ch )
      bs.put(2, expstr[ch][b]);   // 'chexpstr'

    if ( lfe )
      bs.put(1, expstr[nfchans][b]); // 'lfeexpstr'

    for ( int ch = 0; ch < nfchans; ++ch )
    {
      if (expstr[ch][b] != EXP_REUSE)
        bs.put(6, chbwcod[ch]);   // 'chbwcod'
    }

    // exponents

    for ( int ch = 0; ch < nfchans; ++ch )
    {
      if ( expstr[ch][b] != EXP_REUSE )
      {
        bs.put(4, expcod[ch][b][0]);   // 'exps[ch][0]'
        for (s = 1; s < ngrps[ch][b]+1; s++)// note: include the last group!
          bs.put(7, expcod[ch][b][s]); // 'exps[ch][grp]'
        bs.put(2, 0);                  // 'gainrng[ch]'
      }
    }

    if ( lfe && expstr[nfchans][b] != EXP_REUSE )
    {
      bs.put(4, expcod[nfchans][b][0]);    // 'lfeexp[0]'

      for ( int s = 1; s < ngrps[nfchans][b]+1; ++s ) // note: include the last group!
        bs.put(7, expcod[nfchans][b][s]);  // 'lfeexp[grp]'
    }

    // bit allocation

    if ( b == 0 )
    {
      bs.put_bool(true);               // 'baie'
      bs.put(2, sdcycod);         // 'sdcycod'
      bs.put(2, fdcycod);         // 'fdcycod'
      bs.put(2, sgaincod);        // 'sgaincod'
      bs.put(2, dbpbcod);         // 'dbpbcod'
      bs.put(3, floorcod);        // 'floorcod'

      bs.put_bool(true);               // 'snroffste'
      bs.put(6, csnroffst);       // 'csnroffst'

      for ( int ch = 0; ch < nch; ++ch )
      {
        bs.put(4, fsnroffst);     // 'fsnroffst[ch]'/'lfesnroffst'
        bs.put(3, fgaincod);      // 'fgaincod[ch]'/'lfegaincod'
      }
    }
    else
    {
      bs.put_bool(false);              // 'baie'
      bs.put_bool(false);              // 'snroffste'
    }

    bs.put_bool(false);                // 'deltbaie'
    bs.put_bool(false);                // 'skiple'

    // mantissas

#ifdef _DEBUG
  int bits0 = 0;
  int bits1 = 0;
  int bits2 = 0;
  int bits4 = 0;
#define COUNT_BITS(id, n) { bits##id += n; }
#else
#define COUNT_BITS(id, n)
#endif

    int qs1 = 0; int qch1 = 0;
    int qs2 = 0; int qch2 = 0;
    int qs4 = 0; int qch4 = 0;
    int v;

    for ( int ch = 0; ch < nch; ++ch )
      for ( int s = 0; s < nmant[ch]; ++s )
        #define GROUP_NEXT(q, value, levels, mul)                        \
          while (qch##q < nch)                                           \
          {                                                              \
            while (qs##q < nmant[qch##q] && bap[qch##q][b][qs##q] != q) qs##q++;   \
            if (qs##q < nmant[qch##q]) break;                            \
            qs##q = 0;                                                   \
            qch##q++;                                                    \
          }                                                              \
          if (qs##q < nmant[qch##q] && qch##q < nch)                     \
          {                                                              \
            value += mul * sym_quant(mant[qch##q][b][qs##q], levels);    \
            qs##q++;                                                     \
          }

        switch (bap[ch][b][s])
        {
          case 0: break;

          case 1:
            // 5 bits 3 groups 3 q-levels
            if ((qch1 > ch) || (qch1 == ch && qs1 > s)) break;

            v = 9 * sym_quant(mant[ch][b][s], 3);

            // seek for next value to group
            // we start seeking from current position
            qch1 = ch;
            qs1 = s + 1;
            while (qch1 < nch)
            {
              while (qs1 < nmant[qch1] && bap[qch1][b][qs1] != 1) qs1++;
              if (qs1 < nmant[qch1]) break; // found
              qs1 = 0;
              qch1++;
            }

            // group value if found
            if (qs1 < nmant[qch1] && qch1 < nch)
            {
              v += 3 * sym_quant(mant[qch1][b][qs1], 3);
              qs1++;
            }

            // seek for next value to group
            GROUP_NEXT(1, v, 3, 1)

            bs.put(5, v);
            COUNT_BITS(1, 5);
            break;

          case 2:
            // 7 bits 3 groups 5 q-levels
            if ((qch2 > ch) || (qch2 == ch && qs2 > s)) break;
            // note: ch >= qch2 && s >= qs2;

            v = 25 * sym_quant(mant[ch][b][s], 5);

            qch2 = ch;
            qs2 = s + 1;
            GROUP_NEXT(2, v, 5, 5);
            GROUP_NEXT(2, v, 5, 1);

            bs.put(7, v);
            COUNT_BITS(2, 7);
            break;

          case 3:
            bs.put(3, sym_quant(mant[ch][b][s], 7));
            COUNT_BITS(0, 3);
            break;

          case 4:
            // 7 bits 2 groups 11 q-levels
            if ((qch4 > ch) || (qch4 == ch && qs4 > s)) break;

            v = 11 * sym_quant(mant[ch][b][s], 11);
            qch4 = ch;
            qs4 = s + 1;
            GROUP_NEXT(4, v, 11, 1);

            bs.put(7, v);
            COUNT_BITS(4, 7);
            break;

          case 5:
            bs.put(4, sym_quant(mant[ch][b][s], 15));
            COUNT_BITS(0, 4);
            break;

          case 14:
            bs.put(14, asym_quant(mant[ch][b][s], 14));
            COUNT_BITS(0, 14);
            break;

          case 15:
            bs.put(16, asym_quant(mant[ch][b][s], 16));
            COUNT_BITS(0, 16);
            break;

          default:
            bs.put(bap[ch][b][s] - 1, asym_quant(mant[ch][b][s], bap[ch][b][s] - 1));
            COUNT_BITS(0, bap[ch][b][s] - 1);
            break;
        } // switch (bap[s])
      // for (s = 0; s < nmant[ch]; s++)
    // for (ch = 0; ch < nch; ch++)

  } // for (b = 0; b < AC3_NBLOCKS; b++)
  bs.flush();

  // calc CRC
  int frame_size1 = ((frame_size >> 1) + (frame_size >> 3)) & ~1; // should be even
  int crc = calc_crc(0, frame_buf + 4,  frame_size1 - 4);
  int crc_inv = pow_poly((CRC16_POLY >> 1), (frame_size1 * 8) - 16, CRC16_POLY);
  crc = mul_poly(crc_inv, crc, CRC16_POLY);
  frame_buf[2] = crc >> 8;
  frame_buf[3] = crc & 0xff;

  crc = calc_crc(0, frame_buf + frame_size1, frame_size - frame_size1 - 2);
  frame_buf[frame_size - 2] = crc >> 8;
  frame_buf[frame_size - 1] = crc & 0xff;


  frames++;
  return frame_size;
}


inline void Ac3Enc::compute_expstr(int expstr[AC3_NBLOCKS]
  , int8_t exp[AC3_NBLOCKS][AC3_BLOCK_SAMPLES]
  , int endmant) const
{
  int b, b1, s;
  int exp_diff;

  // compute variation of exponents over time and reuse
  // old exponents if variation is too small
  expstr[0] = EXP_D15;
  for (b = 1; b < AC3_NBLOCKS; b++)
  {
    exp_diff = 0;
    for (s = 0; s < endmant; s++)
      exp_diff += abs(exp[b][s] - exp[b-1][s]);

    if (exp_diff > EXP_DIFF_THRESHOLD)
      expstr[b] = EXP_D15;
    else
      expstr[b] = EXP_REUSE;
  }

  // LFE always uses EXP_D15
  if (endmant == 7) return;

  // deicde exponent strategy based on now many blocks
  // the new exponent set is used for
  // todo: compute exponent strategy based on frequency variation
  b = 0;
  while (b < AC3_NBLOCKS)
  {
    b1 = b + 1;
    while (b1 < AC3_NBLOCKS && expstr[b1] == EXP_REUSE)
      b1++;

    switch(b1 - b)
    {
    case 1:
      expstr[b] = EXP_D45;
      break;
    case 2:
    case 3:
      expstr[b] = EXP_D25;
      break;
    default:
      expstr[b] = EXP_D15;
      break;
    }
    b = b1;
  }
}

inline void Ac3Enc::restrict_exp(int8_t expcod[AC3_NBLOCKS][AC3_BLOCK_SAMPLES]
  , int ngrps[AC3_NBLOCKS]
  , int8_t exp[AC3_NBLOCKS][AC3_BLOCK_SAMPLES]
  , int expstr[AC3_NBLOCKS]
  , int endmant) const
{
  int s, b, b1;

  b = 0;
  while (b < AC3_NBLOCKS)
  {
    // Find minimum of reused exponents
    for (b1 = b + 1; b1 < AC3_NBLOCKS && expstr[b1] == EXP_REUSE; b1++)
      for (s = 0; s < endmant; s++)
        if (exp[b][s] > exp[b1][s])
          exp[b][s] = exp[b1][s];

    // compute encoded exponents
    // and update exponents as decoder will see them
    ngrps[b] = encode_exp(expcod[b], exp[b], expstr[b], endmant);

    // copy reused exponents
    // optimize: we may not do this
    for (b1 = b + 1; b1 < AC3_NBLOCKS && expstr[b1] == EXP_REUSE; b1++)
    {
      ngrps[b1] = 0;
      memcpy(exp[b1], exp[b], sizeof(exp[b]));
    }

    b = b1;
  }
}

inline int Ac3Enc::encode_exp(int8_t expcod[AC3_BLOCK_SAMPLES]
  , int8_t exp[AC3_BLOCK_SAMPLES], int expstr, int endmant) const
{
  // Encodes exponents to differential form and applies AC3 restrictions.
  // Encodes exponents into grouped form
  // Decodes encoded exponens back to normal form for bit allocation unit.
  // Returns number of exponent groups (without DC exponent)

  int s, s1;
  int ngrps;
  int surplus;
  int8_t e;

  // find minimum in each group
  expcod[0] = min(exp[0], 15); // limit DC exponent
  switch (expstr)
  {
    default:
    case EXP_D15:
      for (s = 1; s < endmant; s++)
        expcod[s] = exp[s];
      ngrps = s;
      break;

    case EXP_D25:
      for (s = 1, s1 = 1; s < endmant; s += 2, s1++)
        expcod[s1] = min(exp[s], exp[s+1]);
      ngrps = s1;
      break;

    case EXP_D45:
      for (s = 1, s1 = 1; s < endmant; s += 4, s1++)
        expcod[s1] = min(min(exp[s+0], exp[s+1]),
                          min(exp[s+2], exp[s+3]));
      ngrps = s1;
      break;
  }

  // encode differential exponents
  // and convert it to exponent codes
  for (s = ngrps-1; s > 0; s--)
    expcod[s] -= expcod[s-1] - 2;
  expcod[ngrps]   = 2;
  expcod[ngrps+1] = 2;

  // appliy differential restrictions [-2; +2] (or, [0; 4] in codes)
  // note: we cannot increase absolute exponent values!

  // propagate positive surplus forward
  // (do not modify DC component, we cannot increase it)
  surplus = 0;
  for (s = 1; s < ngrps; s++)
  {
    e = expcod[s] += surplus;
    if (e > 4)
    {
      expcod[s] = 4;
      surplus = e - 4;
    }
    else
      surplus = 0;
  }
  // propagate negative surplus backward
  // (we may need to decrease DC component)
  surplus = 0;
  for (s = ngrps-1; s >= 0; s--)
  {
    e = expcod[s] += surplus;
    if (e < 0)
    {
      expcod[s] = 0;
      surplus = e;
    }
    else
      surplus = 0;
  }


  // decode encoded exponents back to fine-grained scale
  e = exp[0] = expcod[0];
  switch (expstr)
  {
    case EXP_D15:
      for (s = 1; s < endmant; s++)
      {
        assert(exp[s] >= e + expcod[s] - 2);

        exp[s] =
          e += expcod[s] - 2;
      }
      break;

    case EXP_D25:
      for (s = 1, s1 = 1; s < endmant; s += 2, s1++)
      {
        assert(exp[s+0] >= e + expcod[s1] - 2);
        assert(exp[s+1] >= e + expcod[s1] - 2);

        exp[s+1] = exp[s] =
          e += expcod[s1] - 2;
      }
      break;

    case EXP_D45:
      for (s = 1, s1 = 1; s < endmant; s += 4, s1++)
      {
        assert(exp[s+0] >= e + expcod[s1] - 2);
        assert(exp[s+1] >= e + expcod[s1] - 2);
        assert(exp[s+2] >= e + expcod[s1] - 2);
        assert(exp[s+3] >= e + expcod[s1] - 2);

        exp[s+3] = exp[s+2] = exp[s+1] = exp[s] =
          e += expcod[s1] - 2;
      }
      break;

    default: assert(false); // we should never be here;
  }

  // exponent grouping
  for (s = 1, s1 = 1; s < ngrps; s += 3, s1++)
    expcod[s1] = expcod[s] * 25 + expcod[s+1] * 5 + expcod[s+2];

  // withoout DC exponent
  return s1 - 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Math utils
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _M_IX86
inline int bits_left(int32_t v)
{
  __asm {
    mov ecx, v
    shl ecx, 1
    or  ecx, 1
    bsr eax, ecx
    mov v, eax
  }
  return v;
}
#else
inline int bits_left(int32_t v)
{
  int n = 0;
  if (v & 0xffff0000) { v >>= 16; n += 16; }
  if (v & 0xff00)     { v >>= 8;  n += 8;  }
  if (v & 0xf0)       { v >>= 4;  n += 4;  }
  if (v & 0xc)        { v >>= 2;  n += 2;  }

  if (v & 0x2)        { n += 2; }
  else
  if (v & 0x1)        { n++;  }
  return n;
}
#endif

inline unsigned int mul_poly(unsigned int a, unsigned int b, unsigned int poly)
{
  unsigned int c;

  c = 0;
  while (a)
  {
    if (a & 1)
      c ^= b;
    a = a >> 1;
    b = b << 1;
    if (b & (1 << 16))
      b ^= poly;
  }
  return c;
}

inline unsigned int pow_poly(unsigned int a, unsigned int n, unsigned int poly)
{
  unsigned int r;
  r = 1;
  while (n)
  {
    if (n & 1)
      r = mul_poly(r, a, poly);
    a = mul_poly(a, a, poly);
    n >>= 1;
  }
  return r;
}

}; // namespace AudioFilter

