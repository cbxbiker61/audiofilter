#include <cstdio>
#include <cstring>
#include <AudioFilter/Crc.h>
#include <AudioFilter/MpaFrameParser.h>
#include "MpaTables.h"

namespace AudioFilter {

MpaFrameParser::MpaFrameParser()
  : _mpaHeaderParser()
{
  _samples.allocate(2, MPA_NSAMPLES);
  _synth[0] = new SynthBufferFPU();
  _synth[1] = new SynthBufferFPU();

  reset(); // always useful
}

MpaFrameParser::~MpaFrameParser()
{
  delete _synth[0];
  delete _synth[1];
}

///////////////////////////////////////////////////////////////////////////////
// FrameParser overrides

const HeaderParser *MpaFrameParser::getHeaderParser(void) const
{
  return &_mpaHeaderParser;
}

void MpaFrameParser::reset(void)
{
  _spk = spk_unknown;
  _samples.zero();

  _synth[0]->reset();
  _synth[1]->reset();
}

bool MpaFrameParser::parseFrame(uint8_t *frame, size_t size)
{
  if ( ! parseHeader(frame, size) )
    return false;

  // convert bitstream format
  if ( _bsi.bs_type != BITSTREAM_8 )
  {
    if ( bs_convert(frame, size, _bsi.bs_type, frame, BITSTREAM_8) == 0 )
      return false;
  }

  _bs.set(frame, 0, size * 8);
  _bs.get(32); // skip header

  if ( _hdr.error_protection )
    _bs.get(16); // skip crc

  switch ( _bsi.layer )
  {
    case MPA_LAYER_I:
      decodeFrame_I(frame);
      break;

    case MPA_LAYER_II:
      decodeFrame_II(frame);
      break;
  }

  return true;
}

bool MpaFrameParser::checkCrc(const uint8_t *frame, size_t protected_data_bits) const
{
  if ( ! _hdr.error_protection )
    return true;

  /////////////////////////////////////////////////////////
  // CRC check
  // Note that we include CRC word into processing AFTER
  // protected data. Due to CRC properties we must get
  // zero result in case of no errors.

  uint32_t crc(crc16.crc_init(0xffff));
  crc = crc16.calcBits(crc, frame + 2, 0, 16); // header
  crc = crc16.calcBits(crc, frame + 6, 0, protected_data_bits); // frame data
  crc = crc16.calcBits(crc, frame + 4, 0, 16); // crc
  return crc == 0;
}

std::string MpaFrameParser::getStreamInfo(void) const
{
  char info[1024];

  sprintf(info,
    "MPEG Audio\n"
    "speakers: %s\n"
    "ver: %s\n"
    "frame size: %zu bytes\n"
    "stream: %s\n"
    "bitrate: %ikbps\n"
    "sample rate: %iHz\n"
    "bandwidth: %ikHz/%ikHz\n",
    _spk.getModeText(),
    _bsi.ver ? "MPEG2 LSF": "MPEG1",
    _bsi.frame_size,
    (_bsi.bs_type == BITSTREAM_8? "8 bit": "16bit little endian"),
    _bsi.bitrate / 1000,
    _bsi.freq,
    _bsi.jsbound * _bsi.freq / SBLIMIT / 1000 / 2,
    _bsi.sblimit * _bsi.freq / SBLIMIT / 1000 / 2);

  return info;
}

std::string MpaFrameParser::getFrameInfo(void) const
{
  return "";
}

//////////////////////////////////////////////////////////////////////
// MPA parsing
//////////////////////////////////////////////////////////////////////

bool MpaFrameParser::parseHeader(const uint8_t *frame, size_t size)
{
  // 8 bit or 16 bit big endian stream sync
  if ( (frame[0] == 0xff)         && // sync
     ((frame[1] & 0xf0) == 0xf0) && // sync
     ((frame[1] & 0x06) != 0x00) && // layer
     ((frame[2] & 0xf0) != 0xf0) && // bitrate
     ((frame[2] & 0x0c) != 0x0c) )   // sample rate
  {
    uint32_t h(*(uint32_t *)frame);
    _hdr = swab_u32(h);
    _bsi.bs_type = BITSTREAM_8;
  }
  else
  // 16 bit little endian stream sync
  if ((frame[1] == 0xff)         && // sync
     ((frame[0] & 0xf0) == 0xf0) && // sync
     ((frame[0] & 0x06) != 0x00) && // layer
     ((frame[3] & 0xf0) != 0xf0) && // bitrate
     ((frame[3] & 0x0c) != 0x0c))   // sample rate
  {
    uint32_t h(*(uint32_t *)frame);
    _hdr = (h >> 16) | (h << 16);
    _bsi.bs_type = BITSTREAM_16LE;
  }
  else
    return false;

  _hdr.error_protection = ~_hdr.error_protection;

  // common information
  _bsi.ver       = 1 - _hdr.version;
  _bsi.mode      = _hdr.mode;
  _bsi.layer     = 3 - _hdr.layer;
  _bsi.bitrate   = bitrate_tbl[_bsi.ver][_bsi.layer][_hdr.bitrate_index] * 1000;
  _bsi.freq      = freq_tbl[_bsi.ver][_hdr.sampling_frequency];

  _bsi.nch       = _bsi.mode == MPA_MODE_SINGLE ? 1: 2;
  _bsi.nsamples  = _bsi.layer == MPA_LAYER_I ? SCALE_BLOCK * SBLIMIT: SCALE_BLOCK * SBLIMIT * 3;

  // frame size calculation
  if ( _bsi.bitrate )
  {
    _bsi.frame_size = _bsi.bitrate * slots_tbl[_bsi.layer] / _bsi.freq + _hdr.padding;

    if ( _bsi.layer == MPA_LAYER_I )
      _bsi.frame_size *= 4;

    if ( _bsi.frame_size > size )
      return false;
  }
  else
    _bsi.frame_size = 0;

  // layerII: table select
  _II_table = 0;

  if ( _bsi.layer == MPA_LAYER_II )
  {
    // todo: check for allowed bitrate ??? (look at sec 2.4.2.3 of ISO 11172-3)
    if ( _bsi.ver )
    {
      // MPEG2 LSF
      _II_table = 4;
    }
    else
    {
      // MPEG1
      if ( _bsi.mode == MPA_MODE_SINGLE )
        _II_table = II_table_tbl[_hdr.sampling_frequency][_hdr.bitrate_index];
      else
        _II_table = II_table_tbl[_hdr.sampling_frequency][II_half_bitrate_tbl[_hdr.bitrate_index]];
    }
  }

  // subband information
  _bsi.sblimit = ( _bsi.layer == MPA_LAYER_II )
          ? II_sblimit_tbl[_II_table]
          : SBLIMIT;

  _bsi.jsbound = ( _bsi.mode == MPA_MODE_JOINT )
          ? jsbound_tbl[_bsi.layer][_hdr.mode_ext]
          : _bsi.sblimit;

  _spk = Speakers(FORMAT_LINEAR, (_bsi.mode == MPA_MODE_SINGLE) ? MODE_MONO : MODE_STEREO, _bsi.freq);
  return true;
}

///////////////////////////////////////////////////////////////////////////////
//  Layer II
///////////////////////////////////////////////////////////////////////////////

bool MpaFrameParser::decodeFrame_II(const uint8_t *frame)
{
  const int nch(_bsi.nch);
  const int sblimit(_bsi.sblimit);
  const int jsbound(_bsi.jsbound);
  const int table(_II_table);

  /////////////////////////////////////////////////////////
  // Load bitalloc

  int16_t bit_alloc[MPA_NCH][SBLIMIT];

  const int16_t *ba_bits = II_ba_bits_tbl[table];

  if ( nch == 1 )
  {
    for ( int sb = 0; sb < sblimit; ++sb )
    {
      int bits = ba_bits[sb];
      bit_alloc[0][sb] = II_ba_tbl[table][sb][_bs.get(bits)];
    }

    for ( int sb = sblimit; sb < SBLIMIT; ++sb )
      bit_alloc[0][sb] = 0;
  }
  else
  {
    for ( int sb = 0; sb < jsbound; ++sb )
    {
      int bits = ba_bits[sb];

      if ( bits )
      {
        bit_alloc[0][sb] = II_ba_tbl[table][sb][_bs.get(bits)];
        bit_alloc[1][sb] = II_ba_tbl[table][sb][_bs.get(bits)];
      }
      else
      {
        bit_alloc[0][sb] = II_ba_tbl[table][sb][0];
        bit_alloc[1][sb] = II_ba_tbl[table][sb][0];
      }
    }

    for ( int sb = jsbound; sb < sblimit; ++sb )
    {
      int bits = ba_bits[sb];

      if ( bits )
        bit_alloc[0][sb] = bit_alloc[1][sb] = II_ba_tbl[table][sb][_bs.get(bits)];
      else
        bit_alloc[0][sb] = bit_alloc[1][sb] = II_ba_tbl[table][sb][0];
    }

    for ( int sb = sblimit; sb < SBLIMIT; ++sb )
      bit_alloc[0][sb] = bit_alloc[1][sb] = 0;
  }

  /////////////////////////////////////////////////////////
  // Load scalefactors bitalloc

  uint16_t scfsi[2][SBLIMIT];

  for ( int sb = 0; sb < sblimit; ++sb )
  {
    for ( int ch = 0; ch < nch; ++ch ) // 2 bit scfsi
    {
      if ( bit_alloc[ch][sb] )
        scfsi[ch][sb] = (uint16_t) _bs.get(2);
    }
  }

  // do we need this?
  for ( int sb = sblimit; sb < SBLIMIT; ++sb )
  {
    for ( int ch = 0; ch < nch; ++ch )
      scfsi[ch][sb] = 0;
  }

  /////////////////////////////////////////////////////////
  // CRC check

  if ( ! checkCrc(frame, _bs.getPosBits() - 32 - 16) )
    return false;

  /////////////////////////////////////////////////////////
  // Load scalefactors

  sample_t scale[MPA_NCH][3][SBLIMIT];
  sample_t c = c_tbl[0]; // init to something
                         // otherwise gcc -O complains that it may be
						 // used unitialized

  for ( int sb = 0; sb < sblimit; ++sb )
  {
    for ( int ch = 0; ch < nch; ++ch )
    {
      int ba = bit_alloc[ch][sb];

      if ( ba )
      {
        if ( ba > 0 )
          c = c_tbl[ba];
        else
        {
          switch ( ba )
          {
            case -5:
              c = c_tbl[0];
              break;
            case -7:
              c = c_tbl[1];
              break;
            case -10:
              c = c_tbl[2];
              break;
          }
        }

        switch ( scfsi[ch][sb] )
        {
          case 0 :  // all three scale factors transmitted
            scale[ch][0][sb] = scale_tbl[_bs.get(6)] * c;
            scale[ch][1][sb] = scale_tbl[_bs.get(6)] * c;
            scale[ch][2][sb] = scale_tbl[_bs.get(6)] * c;
            break;

          case 1 :  // scale factor 1 & 3 transmitted
            scale[ch][0][sb] =
            scale[ch][1][sb] = scale_tbl[_bs.get(6)] * c;
            scale[ch][2][sb] = scale_tbl[_bs.get(6)] * c;
            break;

          case 3 :  // scale factor 1 & 2 transmitted
            scale[ch][0][sb] = scale_tbl[_bs.get(6)] * c;
            scale[ch][1][sb] =
            scale[ch][2][sb] = scale_tbl[_bs.get(6)] * c;
            break;

          case 2 :    // only one scale factor transmitted
            scale[ch][0][sb] =
            scale[ch][1][sb] =
            scale[ch][2][sb] = scale_tbl[_bs.get(6)] * c;
            break;

          default : break;
        }
      }
      else
      {
        scale[ch][0][sb] = scale[ch][1][sb] = scale[ch][2][sb] = 0;
      }
    }
  }

  // do we need this?
  for ( int sb = sblimit; sb < SBLIMIT; ++sb )
  {
    for ( int ch = 0; ch < nch; ++ch )
    {
      scale[ch][0][sb] = scale[ch][1][sb] = scale[ch][2][sb] = 0;
    }
  }

  /////////////////////////////////////////////////////////
  // Decode fraction and synthesis

  sample_t *sptr[MPA_NCH];

  for ( int i = 0; i < SCALE_BLOCK; ++i )
  {
    sptr[0] = &_samples[0][i * SBLIMIT * 3];
    sptr[1] = &_samples[1][i * SBLIMIT * 3];

    decodeFraction_II(sptr, bit_alloc, scale, i >> 2);

    for ( int ch = 0; ch < nch; ++ch )
    {
      _synth[ch]->synth(&_samples[ch][i * SBLIMIT * 3              ]);
      _synth[ch]->synth(&_samples[ch][i * SBLIMIT * 3 + 1 * SBLIMIT]);
      _synth[ch]->synth(&_samples[ch][i * SBLIMIT * 3 + 2 * SBLIMIT]);
    }
  }

  return true;
}

void MpaFrameParser::decodeFraction_II(
  sample_t *fraction[MPA_NCH]
  , int16_t bit_alloc[MPA_NCH][SBLIMIT]
  , sample_t scale[MPA_NCH][3][SBLIMIT]
  , int x)
{
  const int nch(_bsi.nch);
  const int sblimit(_bsi.sblimit);
  const int jsbound(_bsi.jsbound);

  for ( int sb = 0; sb < sblimit; ++sb )
  {
    for ( int ch = 0; ch < ((sb < jsbound) ? nch : 1 ); ++ch )
    {
      // ba means number of bits to read;
      // negative numbers mean sample triplets
      int16_t ba = bit_alloc[ch][sb];

      if ( ba )
      {
        uint16_t s0, s1, s2;
        int16_t d;

        if ( ba > 0 )
        {
          d  = d_tbl[ba]; // ba > 0 => ba = quant
          s0 = (uint16_t) _bs.get(ba);
          s1 = (uint16_t) _bs.get(ba);
          s2 = (uint16_t) _bs.get(ba);

          ba = 16 - ba;  // number of bits we should shift
        }
        else // nlevels = 3, 5, 9; ba = -5, -7, -10
        {
          // packed triplet of samples
          ba = -ba;
          unsigned int pack = (unsigned int)_bs.get(ba);

          switch ( ba )
          {
          case 5:
            s0 = pack % 3; pack /= 3;
            s1 = pack % 3; pack /= 3;
            s2 = pack % 3;
            d  = d_tbl[0];
            ba = 14;
            break;

          case 7:
            s0 = pack % 5; pack /= 5;
            s1 = pack % 5; pack /= 5;
            s2 = pack % 5;
            d  = d_tbl[1];
            ba = 13;
            break;

          case 10:
            s0 = pack % 9; pack /= 9;
            s1 = pack % 9; pack /= 9;
            s2 = pack % 9;
            d  = d_tbl[2];
            ba = 12;
            break;

          default:
            assert(false);
            return;
          }
        } // if (ba > 0) .. else ..

        #define dequantize(r, s, d, bits)                     \
        {                                                     \
          s = ((unsigned short) s) << bits;                   \
          s = (s & 0x7fff) | (~s & 0x8000);                   \
          r = (sample_t)((short)(s) + d) * scale[ch][x][sb];  \
        }

        #define dequantize2(r1, r2, s, d, bits)               \
        {                                                     \
          s  = ((unsigned short) s) << bits;                  \
          s  = (s & 0x7fff) | (~s & 0x8000);                  \
          sample_t f = sample_t((short)(s) + d);              \
          r1 = f * scale[0][x][sb];                           \
          r2 = f * scale[1][x][sb];                           \
        }

        if ( nch > 1 && sb >= jsbound )
        {
          dequantize2(fraction[0][sb              ], fraction[1][sb              ], s0, d, ba);
          dequantize2(fraction[0][sb + 1 * SBLIMIT], fraction[1][sb + 1 * SBLIMIT], s1, d, ba);
          dequantize2(fraction[0][sb + 2 * SBLIMIT], fraction[1][sb + 2 * SBLIMIT], s2, d, ba);
        }
        else
        {
          dequantize(fraction[ch][sb              ], s0, d, ba);
          dequantize(fraction[ch][sb + 1 * SBLIMIT], s1, d, ba);
          dequantize(fraction[ch][sb + 2 * SBLIMIT], s2, d, ba);
        }
      }
      else // ba = 0; no sample transmitted
      {
        fraction[ch][sb              ] = 0.0;
        fraction[ch][sb + 1 * SBLIMIT] = 0.0;
        fraction[ch][sb + 2 * SBLIMIT] = 0.0;
        if (nch > 1 && sb >= jsbound)
        {
          fraction[1][sb              ] = 0.0;
          fraction[1][sb + 1 * SBLIMIT] = 0.0;
          fraction[1][sb + 2 * SBLIMIT] = 0.0;
        }
      } // if (ba) ... else ...
    } // for (ch = 0; ch < ((sb < jsbound)? nch: 1 ); ++ch )
  } // for (sb = 0; sb < sblimit; ++sb )

  for ( int ch = 0; ch < nch; ++ch )
  {
    for ( int sb = sblimit; sb < SBLIMIT; ++sb )
    {
      fraction[ch][sb              ] = 0.0;
      fraction[ch][sb +     SBLIMIT] = 0.0;
      fraction[ch][sb + 2 * SBLIMIT] = 0.0;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
//  Layer I
///////////////////////////////////////////////////////////////////////////////

bool MpaFrameParser::decodeFrame_I(const uint8_t *frame)
{
  const int nch(_bsi.nch);
  const int jsbound(_bsi.jsbound);

  /////////////////////////////////////////////////////////
  // Load bitalloc

  int16_t bit_alloc[MPA_NCH][SBLIMIT];

  for ( int sb = 0; sb < jsbound; ++sb )
  {
    for ( int ch = 0; ch < nch; ++ch )
    {
      bit_alloc[ch][sb] = _bs.get(4);
    }
  }

  for ( int sb = jsbound; sb < SBLIMIT; ++sb )
    bit_alloc[0][sb] = bit_alloc[1][sb] = _bs.get(4);

  /////////////////////////////////////////////////////////
  // CRC check

  if ( ! checkCrc(frame, _bs.getPosBits() - 32 - 16) )
    return false;

  /////////////////////////////////////////////////////////
  // Load scale

  sample_t scale[MPA_NCH][SBLIMIT];

  for ( int sb = 0; sb < SBLIMIT; ++sb )
  {
    for ( int ch = 0; ch < nch; ++ch )
    {
      if ( bit_alloc[ch][sb] )
        scale[ch][sb] = scale_tbl[_bs.get(6)]; // 6 bit per scale factor
      else
        scale[ch][sb] = 0;
    }
  }

  /////////////////////////////////////////////////////////
  // Decode fraction and synthesis

  sample_t *sptr[2];

  for ( int i = 0; i < SCALE_BLOCK * SBLIMIT; i += SBLIMIT )
  {
    sptr[0] = &_samples[0][i];
    sptr[1] = &_samples[1][i];
    decodeFraction_I(sptr, bit_alloc, scale);

    for ( int ch = 0; ch < nch; ++ch )
      _synth[ch]->synth(&_samples[ch][i]);
  }

  return true;
}

void MpaFrameParser::decodeFraction_I(
  sample_t *fraction[MPA_NCH]
  , int16_t  bit_alloc[MPA_NCH][SBLIMIT]
  , sample_t scale[MPA_NCH][SBLIMIT])
{
  const int nch(_bsi.nch);
  const int jsbound(_bsi.jsbound);

  /////////////////////////////////////////////////////////
  // buffer samples

  int sample[MPA_NCH][SBLIMIT];

  for ( int sb = 0; sb < jsbound; ++sb )
  {
    for ( int ch = 0; ch < nch; ++ch )
    {
      int ba = bit_alloc[ch][sb];

      if ( ba )
        sample[ch][sb] = (unsigned int) _bs.get(ba + 1);
      else
        sample[ch][sb] = 0;
    }
  }

  for ( int sb = jsbound; sb < SBLIMIT; ++sb )
  {
    int ba = bit_alloc[0][sb];
    int s;

    if ( ba )
      s = (unsigned int) _bs.get(ba + 1);
    else
      s = 0;

    for ( int ch = 0; ch < nch; ++ch )
      sample[ch][sb] = s;
  }

  /////////////////////////////////////////////////////////
  // Dequantize

  for ( int sb = 0; sb < SBLIMIT; ++sb )
  {
    for ( int ch = 0; ch < nch; ++ch )
    {
      if ( bit_alloc[ch][sb] )
      {
        int ba = bit_alloc[ch][sb] + 1;

        if ( ((sample[ch][sb] >> (ba-1)) & 1) == 1 )
          fraction[ch][sb] = 0.0;
        else
          fraction[ch][sb] = -1.0;

        fraction[ch][sb] += (sample_t) (sample[ch][sb] & ((1<<(ba-1))-1)) /
          (sample_t) (1L<<(ba-1));

        fraction[ch][sb] =
          (sample_t) (fraction[ch][sb]+1.0/(sample_t)(1L<<(ba-1))) *
          (sample_t) (1L<<ba) / (sample_t) ((1L<<ba)-1);
      }
      else
        fraction[ch][sb] = 0.0;
    }
  }

  /////////////////////////////////////////////////////////
  // Denormalize

  for ( int ch = 0; ch < nch; ++ch )
  {
    for ( int sb = 0; sb < SBLIMIT; ++sb )
    {
      fraction[ch][sb] *= scale[ch][sb];
    }
  }
}

}; // namespace AudioFilter

// vim: ts=2 sts=2 et

