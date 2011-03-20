#include <cstdio>
#include <cstring>
#include <AudioFilter/Crc.h>
#include <AudioFilter/Parsers.h>
#include "Ac3BitAlloc.h"
#include "Ac3Dither.h"
#include "Ac3Tables.h"

// todo:
// * crc check (start_decode)
// * verify bit allocation conditions:
//   do we need to make _all_ bit allocation if deltbaie = 1?
// * grouped mantissas verification

#define EXP_REUSE 0
#define EXP_D15   1
#define EXP_D25   2
#define EXP_D45   3

#define DELTA_BIT_REUSE    0
#define DELTA_BIT_NEW      1
#define DELTA_BIT_NONE     2
#define DELTA_BIT_RESERVED 3

namespace AudioFilter {

///////////////////////////////////////////////////////////////////////////////
// Quantizer class
// Decode mantissas
///////////////////////////////////////////////////////////////////////////////

class Quantizer
{
protected:
  int q3_cnt, q5_cnt, q11_cnt;
  sample_t q3[2];
  sample_t q5[2];
  sample_t q11;

public:
  Quantizer(): q3_cnt(0), q5_cnt(0), q11_cnt(0) {};

  void getCoeff(ReadBS &bs, sample_t *s, int8_t *bap, int8_t *exp, int n, bool dither);
};

///////////////////////////////////////////////////////////////////////////////
// Ac3Parser
///////////////////////////////////////////////////////////////////////////////

Ac3Parser::Ac3Parser()
  : ac3_header()
{
  frames = 0;
  errors = 0;

  do_crc = true;
  do_dither = true;
  do_imdct = true;

  // allocate buffers
  samples.allocate(AC3_NCHANNELS, AC3_FRAME_SAMPLES);
  delay.allocate(AC3_NCHANNELS, AC3_BLOCK_SAMPLES);

  reset();
}

Ac3Parser::~Ac3Parser()
{}

///////////////////////////////////////////////////////////////////////////////
// FrameParser overrides

HeaderParser *Ac3Parser::getHeaderParser(void)
{
  return &ac3_header;
}

void Ac3Parser::reset(void)
{
  memset((AC3Info*)this, 0, sizeof(AC3Info));
  memset((AC3FrameState*)this, 0, sizeof(AC3FrameState));

  spk = spk_unknown;
  frame = 0;
  frame_size = 0;
  bs_type = 0;

  block = 0;
  samples.zero();
  delay.zero();
}

bool Ac3Parser::parseFrame(uint8_t *frame, size_t size)
{
  if ( ! startParse(frame, size) )
  {
    ++errors;
    return false;
  }

  if ( ! parseHeader() )
  {
    ++errors;
    return false;
  }

  while ( block < AC3_NBLOCKS )
  {
    if ( ! decodeBlock() )
    {
      ++errors;
      return false;
    }
  }

  ++frames;

  return true;
}

std::string Ac3Parser::getStreamInfo(void) const
{
  char info[1024];
  int max_freq = (cplinu? MAX(endmant[0], cplendmant): endmant[0]) * spk.sample_rate / 512000;
  int cpl_freq = cplinu? cplstrtmant * spk.sample_rate / 512000: max_freq;
  info[0] = 0;

  sprintf(info,
    "AC3\n"
    "speakers: %s\n"
    "sample rate: %iHz\n"
    "bitrate: %ikbps\n"
    "stream: %s\n"
    "frame size: %zu bytes\n"
    "nsamples: %i\n"
    "bsid: %i\n"
    "clev: %.1fdB (%.4f)\n"
    "slev: %.1fdB (%.4f)\n"
    "dialnorm: -%idB\n"
    "bandwidth: %ikHz/%ikHz\n",
    spk.getModeText(),
    spk.sample_rate,
    bitrate,
    (bs_type == BITSTREAM_8 ? "8 bit": "16bit little endian"),
    frame_size,
    AC3_FRAME_SAMPLES,
    bsid,
    value2db(clev), clev,
    value2db(slev), slev,
    dialnorm,
    cpl_freq, max_freq);

  return info;
}

std::string Ac3Parser::getFrameInfo(void) const
{
  return "";
}

///////////////////////////////////////////////////////////////////////////////
// AC3 parse

bool Ac3Parser::startParse(uint8_t *_frame, size_t _size)
{
  HeaderInfo hinfo;

  if ( ! ac3_header.parseHeader(_frame, &hinfo) )
    return false;

  if ( hinfo.frame_size > _size )
    return false;

  spk = hinfo.spk;
  spk.format = FORMAT_LINEAR;
  frame = _frame;
  frame_size = hinfo.frame_size;
  bs_type = hinfo.bs_type;

  if ( bs_type != BITSTREAM_8 )
    if ( bs_convert(frame, _size, bs_type, frame, BITSTREAM_8) == 0 )
      return false;

  if ( do_crc && ! checkCrc() )
      return false;

  bs.set(frame, 0, frame_size * 8);
  return true;
}

bool Ac3Parser::checkCrc(void)
{
  // Note: AC3 uses standard CRC16 polynomial

  uint32_t crc;

  /////////////////////////////////////////////////////////
  // Check first 5/8 of frame
  // CRC is initialized by 0 and test result must be also 0.
  // Syncword (first 2 bytes) is not imcluded to crc calc
  // but it is included to 5/8 of frame size. So we must
  // check 5/8*frame_size - 2 bytes.

  size_t frame_size1 = ((frame_size >> 2) + (frame_size >> 4)) << 1;
  crc = crc16.calc(0, frame + 2, frame_size1 - 2);

  if ( crc )
    return false;

  /////////////////////////////////////////////////////////
  // Check the rest of frame
  // CRC is initialized by 0 (from previous point) and test
  // result must be also 0.

  crc = crc16.calc(0, frame + frame_size1, frame_size - frame_size1);

  if ( crc )
    return false;

  return true;
}

bool Ac3Parser::parseHeader(void)
// Fill AC3Info structure
{
  /////////////////////////////////////////////////////////////
  // Skip syncword

  bs.get(32);

  /////////////////////////////////////////////////////////////
  // Parse bit stream information (BSI)

  fscod      = bs.get(2);            // 'fscod' - sample rate code
  frmsizecod = bs.get(6);            // 'frmsizecod' - frame size code
  bsid       = bs.get(5);            // 'bsid' - bitstream identification
  bsmod      = bs.get(3);            // 'bsmod' - bitstreeam mode
  acmod      = bs.get(3);            // 'acmod' - audio coding mode

  halfrate = halfrate_tbl[bsid];
  bitrate = bitrate_tbl[frmsizecod >> 1];

  if ( (acmod & 1) && (acmod != 1) )
    clev = clev_tbl[bs.get(2)];      // 'clev' - center mix level
  else
    clev = 1.0;

  if ( acmod & 4 )
    slev = slev_tbl[bs.get(2)];      // 'slev' - surround mix level
  else
    slev = 1.0;

  if ( acmod == AC3_MODE_STEREO )
    dsurmod  = bs.get(2);            // 'dsurmod' - Dolby Surround mode
  else
    dsurmod  = 0;

  lfeon = bs.getBool();        // 'lfeon' - flag shows if it is LFE channel in stream
  dialnorm = bs.get(5);            // 'dialnorm' - dialog normalization

  compre = bs.getBool();        // 'compre' - compression gain word

  if ( compre )
    compr = bs.get(8);            // 'compr' - compression gain word
  else
    compr = 0;

  langcode = bs.getBool();        // 'langcode' - language code exists
  if ( langcode )
    langcod  = bs.get(8);            // 'langcod' - language code
  else
    langcod  = 0;

  audprodie  = bs.getBool();        // 'audprodie' - audio production information exists

  if ( audprodie )
  {
    mixlevel = bs.get(5) + 80;       // 'mixlevel' - mixing level in SPL
    roomtyp  = bs.get(2);            // 'roomtyp' - room type
  }
  else
  {
    mixlevel = 0;
    roomtyp  = 0;
  }

  if ( acmod == AC3_MODE_DUAL )
  {
    dialnorm2 = bs.get(5);          // 'dialnorm2' - dialog normalization

    compr2e = bs.getBool();      // 'compr2e' - compression gain word

    if ( compr2e )
      compr2   = bs.get(8);          // 'compr2' - compression gain word
    else
      compr2   = 0;

    langcod2e  = bs.getBool();      // 'langcod2e' - language code exists

    if ( langcod2e )
      langcod  = bs.get(8);          // 'langcod2' - language code
    else
      langcod  = 0;

    audprodi2e = bs.getBool();      // 'audprodi2e' - audio production information exists

    if ( audprodi2e )
    {
      mixlevel2 = bs.get(5) + 80;    // 'mixlevel2' - mixing level in SPL
      roomtyp2  = bs.get(2);         // 'roomtyp2' - room type
    }
    else
    {
      mixlevel2 = 0;
      roomtyp2  = 0;
    }
  }
  else
  {
    dialnorm2  = 0;
    compr2e    = false;
    compr2     = 0;
    langcod2e  = false;
    langcod    = 0;
    audprodi2e = false;
    mixlevel2  = 0;
    roomtyp2   = 0;
  }

  copyrightb = bs.getBool();        // 'copyrightb' - copyright bit
  origbs     = bs.getBool();        // 'origbs' - original bitstream

  if ( bs.getBool() )                 // 'timecod1e' - timecode first half exists
  {
    timecode.hours = bs.get(5);
    timecode.mins  = bs.get(6);
    timecode.secs  = bs.get(3) << 4;
  }

  if ( bs.getBool() )                 // 'timecod2e' - timecode second half exists
  {
    timecode.secs  += bs.get(3);
    timecode.frames = bs.get(5);
    timecode.fracs  = bs.get(6);
  }

  if ( bs.getBool() )                 // 'addbsie' - additional bitstream information exists
  {
    int addbsil = bs.get(6);         // 'addbsil' - additioanl bitstream information length

    while ( addbsil-- )
      bs.get(8);                     // 'addbsi' - additional bitstream information
  }

  /////////////////////////////////////////////////////////////
  // Init variables to for first block decoding

  block    = 0;

  dynrng   = 1.0;
  dynrng2  = 1.0;

  cpldeltbae = DELTA_BIT_NONE;
  deltbae[0] = DELTA_BIT_NONE;
  deltbae[1] = DELTA_BIT_NONE;
  deltbae[2] = DELTA_BIT_NONE;
  deltbae[3] = DELTA_BIT_NONE;
  deltbae[4] = DELTA_BIT_NONE;

  return true;
}

bool Ac3Parser::decodeBlock(void)
{
  samples_t d = delay;
  samples_t s = samples;
  s += (block * AC3_BLOCK_SAMPLES);

  if ( block >= AC3_NBLOCKS || ! parseBlock() )
  {
    block = AC3_NBLOCKS; // prevent further decoding
    return false;
  }

  parseCoeff(s);

  if ( do_imdct )
  {
    int nfchans = spk.lfe() ? spk.nch() - 1: spk.nch();

    for ( int ch = 0; ch < nfchans; ++ch )
    {
      if ( blksw[ch] )
        imdct.imdct_256(s[ch], delay[ch]);
      else
        imdct.imdct_512(s[ch], delay[ch]);
    }

    if ( spk.lfe() )
      imdct.imdct_512(s[nfchans], d[nfchans]);
  }

  ++block;
  return true;
}

bool Ac3Parser::parseBlock(void)
{
  int nfchans = nfchans_tbl[acmod];
  int ch, bnd;

  ///////////////////////////////////////////////
  // bit allocation bitarray; bits are:
  // 0-4 = do bit allocation for fbw channels
  // 5   = do bit allocation for lfe channel
  // 6   = do bit allocation for coupling channel
  int bitalloc = 0;

  for ( ch = 0; ch < nfchans; ++ch )
    blksw[ch] = bs.getBool();                // 'blksw[ch]' - block switch flag

  for ( ch = 0; ch < nfchans; ++ch )
    dithflag[ch] = bs.getBool();             // 'dithflag[ch]' - dither flag

  // reset dithering info
  // if we do not want to dither
  if ( ! do_dither )
  {
    for ( ch = 0; ch < nfchans; ++ch )
      dithflag[ch] = 0;
  }

  if ( bs.getBool() )                          // 'dynrnge' - dynamic range gain word exists
  {
    int32_t dynrng_word = bs.getSigned(8);   // 'dynrng' - dynamic range gain word
    dynrng = (((dynrng_word & 0x1f) | 0x20) << 13) * scale_factor[(3 - (dynrng_word >> 5)) & 7];
  }

  if ( acmod == AC3_MODE_DUAL )
  {
    if ( bs.getBool() )                        // 'dynrng2e' - dynamic range gain word 2 exists
    {
      int32_t dynrng_word = bs.getSigned(8); // 'dynrng2' - dynamic range gain word 2
      dynrng2 = (((dynrng_word & 0x1f) | 0x20) << 13) * scale_factor[(3 - (dynrng_word >> 5)) & 7];
    }
  }

  /////////////////////////////////////////////////////////
  // Coupling strategy information
  /////////////////////////////////////////////////////////

  if ( bs.getBool() )                          // 'cplstre' - coupling strategy exists
  {
    cplinu = bs.getBool();                   // 'cplinu' - coupling in use

    if ( cplinu )
    {
      if ( acmod == AC3_MODE_MONO || acmod == AC3_MODE_DUAL )
        return false;                         // this modes are not allowed for coupling
                                              // constraint p...
      for ( ch = 0; ch < nfchans; ++ch )
        chincpl[ch] = bs.getBool();          // 'chincpl[ch]' - channel in coupliing

      if ( acmod == AC3_MODE_STEREO )
        phsflginu = bs.getBool();            // 'phsflginu' - phase flags in use

      int cplbegf = bs.get(4);                // 'cplbegf' - coupling begin frequency code
      int cplendf = bs.get(4);                // 'cplendf' - coupling end frequency code

      int ncplsubnd = cplendf - cplbegf + 3;

      if ( ncplsubnd < 0 )
        return false;                         // constraint p...

      cplstrtmant = cplbegf * 12 + 37;
      cplendmant  = cplendf * 12 + 73;

      ncplbnd = 0;
      cplbnd[0] = cplstrtmant + 12;

      for ( bnd = 0; bnd < ncplsubnd - 1; ++bnd )
      {
        if ( bs.getBool() )                    // 'cplbndstrc[bnd]' - coupling band structure
          cplbnd[ncplbnd] += 12;
        else
        {
          ++ncplbnd;
          cplbnd[ncplbnd] = cplbnd[ncplbnd - 1] + 12;
        }
      }
      ++ncplbnd; // coupling band index to number to coupling bands
    }
    else
    {
      chincpl[0] = false;
      chincpl[1] = false;
      chincpl[2] = false;
      chincpl[3] = false;
      chincpl[4] = false;
    }
  }
  else // if (bs.getBool()) // 'cplstre' - coupling strategy exists
    if ( ! block ) // cplstre <> 0 for block 0 (constraint p39 s5.4.3.7)
      return false;

  /////////////////////////////////////////////////////////
  // Coupling coordinates
  // todo: constraint p.41 s5.4.3.14
  /////////////////////////////////////////////////////////

  if ( cplinu )
  {
    bool cplcoe = false;
    int  mstrcplco;
    int  cplcoexp;
    int  cplcomant;

    for ( ch = 0; ch < nfchans; ++ch )
      if ( chincpl[ch] )
        if ( bs.getBool() )                    // 'cplcoe[ch]' coupling coordinates exists
        {
          cplcoe = true;

          mstrcplco = bs.get(2) * 3;          // 'mstrcplco' - master coupling coordinate
          for ( bnd = 0; bnd < ncplbnd; ++bnd )
          {
            cplcoexp  = bs.get(4);            // 'cplcoexp' - coupling coordinate exponent
            cplcomant = bs.get(4);            // 'cplcomant' - coupling coordinate mantissa

            if ( cplcoexp == 15 )
              cplcomant <<= 14;
            else
              cplcomant = (cplcomant | 0x10) << 13;

            cplco[ch][bnd] = cplcomant * scale_factor[cplcoexp + mstrcplco];
          } // for (int bnd = 0; bnd < ncplbnd; bnd++)
        } // if (bs.get_bool())               // 'cplcoe[ch]' coupling coordinates exists

    if ( acmod == AC3_MODE_STEREO && phsflginu && cplcoe )
    {
      for ( bnd = 0; bnd < ncplbnd; ++bnd )
      {
        if ( bs.getBool() )                    // 'phsflg' - phase flag
          cplco[1][bnd] = -cplco[1][bnd];
      }
    }
  }

  /////////////////////////////////////////////////////////
  // Rematrixing
  /////////////////////////////////////////////////////////

  if ( acmod == AC3_MODE_STEREO )
    if ( bs.getBool() )                        // 'rematstr' - rematrixing strategy
    {
      bnd = 0;
      rematflg = 0;
      int endbin = cplinu ? cplstrtmant: 253;
      do
        rematflg |= bs.get(1) << bnd;         // rematflg[bnd] - rematrix flag
      while (rematrix_tbl[bnd++] < endbin);
    }
/*
    This check is disabled because some buggy encoder exists that
    breaks this rule.

    else if (block == 0)                      // rematstr <> 0 for block 0 (constraint p41 s5.4.3.19)
      return false;
*/
  /////////////////////////////////////////////////////////
  // Exponents
  /////////////////////////////////////////////////////////

  int cplexpstr = 0;
  int chexpstr[5];
  int lfeexpstr = 0;

  if ( cplinu )
  {
    cplexpstr = bs.get(2); // 'cplexpstr' coupling exponent strategy

    if ( cplexpstr == EXP_REUSE && block == 0 ) // cplexpstr <> reuse in block 0 (constraint p42 s5.4.3.21)
      return false;
  }

  for ( ch = 0; ch < nfchans; ++ch )
  {
    chexpstr[ch] = bs.get(2); // 'chexpstr[ch]' - channel exponent strategy

    if ( chexpstr[ch] == EXP_REUSE && block == 0 ) // chexpstr[ch] <> reuse in block 0 (constraint p42 s5.4.3.22)
      return false;
  }

  if ( lfeon )
  {
    lfeexpstr = bs.get(1); // 'lfeexpstr' - LFE exponent strategy

    if ( lfeexpstr == EXP_REUSE && block == 0 ) // lfeexpstr <> reuse in block 0 (constraint p42 s5.4.3.23)
      return false;
  }

  for ( ch = 0; ch < nfchans; ++ch )
  {
    if ( chexpstr[ch] != EXP_REUSE )
    {
      if ( chincpl[ch] )
        endmant[ch] = cplstrtmant;
      else
      {
        int chbwcod = bs.get(6); // 'chbwcod[ch]' - channel bandwidth code

        if ( chbwcod > 60 ) // chbwcod[ch] <= 60 (constraint p42 s5.4.3.24)
          return false;

        endmant[ch] = chbwcod * 3 + 73;
      }
    }
  }

  if ( cplinu && cplexpstr != EXP_REUSE )
  {
    bitalloc |= 1 << 6; // do bit allocation for coupling channel
    int ncplgrps = (cplendmant - cplstrtmant) / (3 << (cplexpstr - 1));
    int8_t cplabsexp = bs.get(4) << 1;

    if ( ! parseExponents(cplexps + cplstrtmant, cplabsexp, cplexpstr, ncplgrps) )
      return false;
  }

  int nexpgrps = 0; // init to something so gcc -O doesn't complain

  for ( ch = 0; ch < nfchans; ++ch )
  {
    if ( chexpstr[ch] != EXP_REUSE )
    {
      bitalloc |= 1 << ch; // do bit allocation for channel ch

      switch ( chexpstr[ch] )
      {
        case EXP_D15:
          nexpgrps = (endmant[ch] - 1) / 3;
          break;
        case EXP_D25:
          nexpgrps = (endmant[ch] - 1 + 3) / 6;
          break;
        case EXP_D45:
          nexpgrps = (endmant[ch] - 1 + 9) / 12;
          break;
      }

      exps[ch][0] = bs.get(4);

      if ( ! parseExponents(exps[ch] + 1, exps[ch][0], chexpstr[ch], nexpgrps) )
        return false;

      gainrng[ch] = bs.get(2);                // 'gainrng[ch]' - gain range code
    }
  }

  if ( lfeon && lfeexpstr != EXP_REUSE )
  {
    bitalloc |= 1 << 5; // do bit allocation for lfe channel
    lfeexps[0] = bs.get(4);

    if ( ! parseExponents(lfeexps + 1, lfeexps[0], lfeexpstr, 2))
      return false;
  }

  /////////////////////////////////////////////////////////
  // Bit allocation parametric information
  /////////////////////////////////////////////////////////

  if ( bs.getBool() ) // 'baie' - bit allocation information exists
  {
    bitalloc |= -1; // do all bit allocation
    sdecay = sdecay_tbl[bs.get(2)]; // 'sdcycod' - slow decay code
    fdecay = fdecay_tbl[bs.get(2)]; // 'fdcycod' - fast decay code
    sgain  = sgain_tbl[bs.get(2)];  // 'sgaincod' - slow gain code
    dbknee = dbknee_tbl[bs.get(2)]; // 'dbpbcod' - dB per bit code
    floor  = floor_tbl[bs.get(3)]; // 'floorcod' - masking floor code
  }
  else if ( block == 0 ) // baie <> 0 in block 0 (constraint p43 s5.4.3.30)
      return false;

  if ( bs.getBool() ) // 'snroffste' - SNR offset exists
  {
    bitalloc |= -1; // do all bit allocation
    int csnroffst = bs.get(6); // 'csnroffst' - coarse SNR offset

    if ( cplinu )
    {
      int cplfsnroffst = bs.get(4); // 'cplfsnroffst' - coupling fine SNR offset
      cplsnroffset = (((csnroffst - 15) << 4) + cplfsnroffst) << 2;
      cplfgain = fgain_tbl[bs.get(3)]; // 'cplfgaincod' - coupling fast gain code
    }

    for ( ch = 0; ch < nfchans; ++ch )
    {
      int fsnroffst = bs.get(4); // 'fsnroffst' - channel fine SNR offset
      snroffset[ch] = (((csnroffst - 15) << 4) + fsnroffst) << 2;
      fgain[ch] = fgain_tbl[bs.get(3)]; // 'fgaincod' - channel fast gain code
    }

    if ( lfeon )
    {
      int lfefsnroffst = bs.get(4); // 'lfesnroffst' - LFE channel SNR offset
      lfesnroffset = (((csnroffst - 15) << 4) + lfefsnroffst) << 2;
      lfefgain = fgain_tbl[bs.get(3)]; // 'lfegaincod' - LFE channel gain code
    }
  }
  else if ( block == 0 ) // snroffte <> 0 in block 0 (constraint p44 s5.4.3.36)
  {
    return false;
  }

  if ( cplinu )
  {
    if ( bs.getBool() ) // 'cplleake' - coupling leak initalization exists
    {
      bitalloc |= 1 << 6; // do bit allocations for coupling channel
      cplfleak = (bs.get(3) << 8) + 768; // 'cplfleak' - coupling fast leak initialization
      cplsleak = (bs.get(3) << 8) + 768; // 'cplsleak' - coupling slow leak initialization
    }
    else if ( block == 0 ) // cplleake <> 0 in block 0 (constraint p44 s5.4.3.44)
    {
      return false;
    }
  }

  /////////////////////////////////////////////////////////
  // Delta bit allocation information
  /////////////////////////////////////////////////////////

  if ( bs.getBool() ) // 'deltbaie' - delta bit information exists
  {
    bitalloc |= -1; // do all bit allocation?

    if ( cplinu )
    {
      cpldeltbae = bs.get(2); // 'cpldeltbae' - coupling delta bit allocation exists

      if ( cpldeltbae == DELTA_BIT_REUSE && block == 0 ) // cpldeltbae <> 0 in block 0 (constraint p45 s5.4.3.48)
        return false;
    }

    for ( ch = 0; ch < nfchans; ++ch )
    {
      deltbae[ch] = bs.get(2); // 'deltbae[ch]' - delta bit allocation exists

      if ( deltbae == DELTA_BIT_REUSE && block == 0 ) // deltbae[ch] <> 0 in block 0 (constraint p45 s5.4.3.49)
        return false;
    }

    if ( cplinu && cpldeltbae == DELTA_BIT_NEW )
      if ( ! parseDeltba(cpldeltba) )
        return false;

    for ( ch = 0; ch < nfchans; ++ch )
    {
      if ( deltbae[ch] == DELTA_BIT_NEW )
        if ( ! parseDeltba(deltba[ch]) )
          return false;
    }
  }

  /////////////////////////////////////////////////////////
  // Skip data
  /////////////////////////////////////////////////////////

  if ( bs.getBool() )
  {
    int skipl = bs.get(9);

    while ( skipl-- )
      bs.get(8);
  }

  /////////////////////////////////////////////////////////
  // Do bit allocation
  /////////////////////////////////////////////////////////

  if ( bitalloc )
  {
    bool got_cplchan = false;
    BAP_BitCount counter;

    for ( ch = 0; ch < nfchans; ++ch )
    {
      if ( bitalloc & (1 << ch) )
      {
        bit_alloc(
          bap[ch], exps[ch],
          deltbae[ch], deltba[ch],
          0, endmant[ch],
          fscod, halfrate,
          sdecay, fdecay,
          sgain, fgain[ch],
          dbknee, floor,
          0, 0,
          snroffset[ch]);

        counter.add_bap(bap[ch], 0, endmant[ch]);
      }

      if ( cplinu && !got_cplchan && (bitalloc & (1 << 6)) )
      {
        got_cplchan = true;
        bit_alloc(
          cplbap, cplexps,
          cpldeltbae, cpldeltba,
          cplstrtmant, cplendmant,
          fscod, halfrate,
          sdecay, fdecay,
          sgain, cplfgain,
          dbknee, floor,
          cplfleak, cplsleak,
          cplsnroffset);

        counter.add_bap(cplbap, cplstrtmant, cplendmant);
      }
    }

    if ( lfeon && bitalloc & (1 << 5) )
    {
      bit_alloc(
        lfebap, lfeexps,
        DELTA_BIT_NONE, 0,
        0, 7,
        fscod, halfrate,
        sdecay, fdecay,
        sgain, lfefgain,
        dbknee, floor,
        0, 0,
        lfesnroffset);

      counter.add_bap(lfebap, 0, 7);
    }

    if ( bs.getPosBits() + counter.bits > frame_size * 8 )
      return false;
  }

  return true;
}

bool Ac3Parser::parseExponents(int8_t *exps, int8_t absexp, int expstr, int nexpgrps)
{
  int expgrp;

  switch ( expstr )
  {
  case EXP_D15:
    while ( nexpgrps-- )
    {
      expgrp = bs.get(7);

      absexp += exp1_tbl[expgrp];
      if (absexp > 24) return false;
      *(exps++) = absexp;

      absexp += exp2_tbl[expgrp];
      if (absexp > 24) return false;
      *(exps++) = absexp;

      absexp += exp3_tbl[expgrp];
      if (absexp > 24) return false;
      *(exps++) = absexp;
    }
    break;

  case EXP_D25:
    while ( nexpgrps-- )
    {
      expgrp = bs.get(7);

      absexp += exp1_tbl[expgrp];
      if (absexp > 24) return false;
      *(exps++) = absexp;
      *(exps++) = absexp;

      absexp += exp2_tbl[expgrp];
      if (absexp > 24) return false;
      *(exps++) = absexp;
      *(exps++) = absexp;

      absexp += exp3_tbl[expgrp];

      if ( absexp > 24 )
        return false;

      *(exps++) = absexp;
      *(exps++) = absexp;
    }
    break;

  case EXP_D45:
    while ( nexpgrps-- )
    {
      expgrp = bs.get(7);

      if ( expgrp >= 125 )
        return false;

      absexp += exp1_tbl[expgrp];

      if ( absexp > 24 )
        return false;

      *(exps++) = absexp;
      *(exps++) = absexp;
      *(exps++) = absexp;
      *(exps++) = absexp;

      absexp += exp2_tbl[expgrp];

      if ( absexp > 24 )
        return false;

      *(exps++) = absexp;
      *(exps++) = absexp;
      *(exps++) = absexp;
      *(exps++) = absexp;

      absexp += exp3_tbl[expgrp];

      if ( absexp > 24 )
        return false;

      *(exps++) = absexp;
      *(exps++) = absexp;
      *(exps++) = absexp;
      *(exps++) = absexp;
    }
    break;
  }

  return true;
}

bool Ac3Parser::parseDeltba(int8_t *deltba)
{
  int deltnseg, deltlen, delta, band;

  memset(deltba, 0, 50);

  deltnseg = bs.get(3) + 1; // 'cpldeltnseg'/'deltnseg'' - coupling/channel delta bit allocation number of segments
  band = 0;
  while (deltnseg--)
  {
    band += bs.get(5); // 'cpldeltoffst'/'deltoffst' - coupling/channel delta bit allocation offset
    deltlen = bs.get(4); // 'cpldeltlen'/'deltlen' - coupling/channel delta bit allocation length
    delta = bs.get(3); // 'cpldeltba'/'deltba' - coupling/channel delta bit allocation

    if ( delta >= 4 )
      delta = (delta - 3) << 7;
    else
      delta = (delta - 4) << 7;

    if ( band + deltlen >= 50 )
      return false;

    while ( deltlen-- )
      deltba[band++] = delta;
  }

  return true;
}

void Ac3Parser::parseCoeff(samples_t samples)
{
  int bnd;
  int s;
  Quantizer q;

  int nfchans = nfchans_tbl[acmod];
  bool got_cplchan = false;

  /////////////////////////////////////////////////////////////
  // Get coeffs

  for ( int ch = 0; ch < nfchans; ++ch )
  {
    // parse channel mantissas
    q.getCoeff(bs, samples[ch], bap[ch], exps[ch], endmant[ch], dithflag[ch]);

    if ( chincpl[ch] && ! got_cplchan )
    {
      // parse coupling channel mantissas
      got_cplchan = true;
      q.getCoeff(bs, samples[ch] + cplstrtmant, cplbap + cplstrtmant, cplexps + cplstrtmant, cplendmant - cplstrtmant, false);

      // copy coupling coeffs to all coupled channels
      for ( int ch2 = ch + 1; ch2 < nfchans; ++ch2 )
      {
        if ( chincpl[ch2] )
          memcpy(samples[ch2] + cplstrtmant, samples[ch] + cplstrtmant, (cplendmant - cplstrtmant) * sizeof(sample_t));
      }
    }
  }

  if ( lfeon )
  {
    q.getCoeff(bs, samples[nfchans], lfebap, lfeexps, 7, false);
    memset(samples[nfchans] + 7, 0, 249 * sizeof(sample_t));
  }

  // Dither
  for ( int ch = 0; ch < nfchans; ++ch )
  {
    if ( chincpl[ch] && dithflag[ch] )
    {
      for ( s = cplstrtmant; s < cplendmant; ++s )
      {
        if ( ! cplbap[s] )
          samples[ch][s] = genDither() * scale_factor[cplexps[s]];
      }
    }
  }

  // Apply coupling coordinates
  for ( int ch = 0; ch < nfchans; ++ch )
  {
    if ( chincpl[ch] )
    {
      s = cplstrtmant;

      for ( bnd = 0; bnd < ncplbnd; ++bnd )
        while ( s < cplbnd[bnd] )
          samples[ch][s++] *= cplco[ch][bnd];
    }
  }

  // Clear tails
  for ( int ch = 0; ch < nfchans; ++ch )
  {
    if ( chincpl[ch] )
      memset(samples[ch] + cplendmant, 0, (256 - cplendmant) * sizeof(sample_t));
    else
      memset(samples[ch] + endmant[ch], 0, (256 - endmant[ch]) * sizeof(sample_t));
  }

  /////////////////////////////////////////////////////////////
  // Rematrixing

  if ( acmod == AC3_MODE_STEREO )
  {
    int bin = 13;
    int bnd = 0;
    int band_end = 0;
    int last_bin = MIN(endmant[0], endmant[1]);
    int remat = rematflg;
    do
    {
      if ( ! (remat & 1) )
      {
        remat >>= 1;
        bin = rematrix_tbl[bnd++];
        continue;
      }

      remat >>= 1;
      band_end = rematrix_tbl[bnd++];

      if ( band_end > last_bin )
        band_end = last_bin;

      do
      {
        sample_t tmp0 = samples[0][bin];
        sample_t tmp1 = samples[1][bin];
        samples[0][bin] = tmp0 + tmp1;
        samples[1][bin] = tmp0 - tmp1;
      } while ( ++bin < band_end );

    } while ( bin < last_bin );
  }
}

void Quantizer::getCoeff(ReadBS &bs, sample_t *s, int8_t *bap, int8_t *exp, int n, bool dither)
{
  while ( n-- )
  {
    int ibap = *bap++;

    switch ( ibap )
    {
      case 0:
        if ( dither )
          *s++ = genDither() * scale_factor[*exp++];
        else
        {
          *s++ = 0;
          exp++;
        }
        break;

      case 1:
        // 3-levels 3 values in 5 bits
        if ( q3_cnt-- )
          *s++ = q3[q3_cnt] * scale_factor[*exp++];
        else
        {
          int code = bs.get(5);
          q3[0] = q3_3_tbl[code];
          q3[1] = q3_2_tbl[code];
          q3_cnt = 2;
          *s++ = q3_1_tbl[code] * scale_factor[*exp++];
        }
        break;

      case 2:
        // 5-levels 3 values in 7 bits
        if ( q5_cnt-- )
          *s++ = q5[q5_cnt] * scale_factor[*exp++];
        else
        {
          int code = bs.get(7);
          q5[0] = q5_3_tbl[code];
          q5[1] = q5_2_tbl[code];
          q5_cnt = 2;
          *s++ = q5_1_tbl[code] * scale_factor[*exp++];
        }
        break;

      case 3:
        *s++ = q7_tbl[bs.get(3)] * scale_factor[*exp++];
        break;

      case 4:
        // 11-levels 2 values in 7 bits
        if ( q11_cnt-- )
          *s++ = q11 * scale_factor[*exp++];
        else
        {
          int code = bs.get(7);
          q11 = q11_2_tbl[code];
          q11_cnt = 1;
          *s++ = q11_1_tbl[code] * scale_factor[*exp++];
        }
        break;

      case 5:
        *s++ = q15_tbl[bs.get(4)] * scale_factor[*exp++];
        break;

      case 14:
        *s++ = (bs.getSigned(14) << 2) * scale_factor[*exp++];
        break;

      case 15:
        *s++ = bs.getSigned(16) * scale_factor[*exp++];
        break;

      default:
        *s++ = (bs.getSigned(ibap - 1) << (16 - (ibap - 1))) * scale_factor[*exp++];
        break;
    }
  }
}

}; // namespace AudioFilter

// vim: ts=2 sts=2 et

