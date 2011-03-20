#include <stdio.h>
#include "dvd_graph.h"

DVDGraph::DVDGraph(int _nsamples, const Sink *_sink)
:FilterGraph(-1), proc(_nsamples)
{
  user_spk = Speakers(FORMAT_PCM16, 0, 0);

  use_spdif = false;
  use_detector = false;
  spdif_pt = FORMAT_MASK_AC3;
  spdif_as_pcm = false;
  spdif_encode = true;
  spdif_stereo_pt = true;
  spdif_bitrate = 640000;

  spdif_check_sr = false;
  spdif_allow_48 = true;
  spdif_allow_44 = false;
  spdif_allow_32 = false;

  spdif_status = SPDIF_MODE_NONE;
  spdif_err = SPDIF_MODE_NONE;

  sink = _sink;
  query_sink = true;
};

///////////////////////////////////////////////////////////
// User format

bool
DVDGraph::query_user(Speakers _user_spk) const
{
  if (!user_spk.format)
    return false;

  return proc.query_user(_user_spk);
}

bool
DVDGraph::set_user(Speakers _user_spk)
{
  if (!query_user(_user_spk))
    return false;

  if (user_spk != _user_spk)
  {
    user_spk = _user_spk;
    rebuild_node(state_proc);
    rebuild_node(state_proc_enc);
    rebuild_node(state_detector);
  }

  return true;
}

Speakers              
DVDGraph::get_user() const
{
  return user_spk;
}

///////////////////////////////////////////////////////////
// Sink to query

void 
DVDGraph::set_sink(const Sink *_sink)
{
  sink = _sink;
}

const Sink *
DVDGraph::get_sink() const
{
  return sink;
}

bool
DVDGraph::get_query_sink() const
{
  return query_sink;
}

void
DVDGraph::set_query_sink(bool _query_sink)
{
  query_sink = _query_sink;
  invalidate_chain();
  rebuild_node(state_detector);
}


///////////////////////////////////////////////////////////
// SPDIF options

bool
DVDGraph::get_use_detector() const
{
  return use_detector;
}

void
DVDGraph::set_use_detector(bool _use_detector)
{
  use_detector = _use_detector;
  invalidate_chain();
}

///////////////////////////////////////////////////////////
// SPDIF options

void
DVDGraph::set_spdif(bool _use_spdif, int _spdif_pt, bool _spdif_as_pcm, bool _spdif_encode, bool _spdif_stereo_pt)
{
  use_spdif = _use_spdif;
  spdif_pt = _spdif_pt;
  spdif_as_pcm = _spdif_as_pcm;
  spdif_encode = _spdif_encode;
  spdif_stereo_pt = _spdif_stereo_pt;
  invalidate_chain();
  rebuild_node(state_detector);
}

bool
DVDGraph::get_use_spdif() const
{
  return use_spdif;
}

void
DVDGraph::set_use_spdif(bool _use_spdif)
{
  use_spdif = _use_spdif;
  invalidate_chain();
  rebuild_node(state_detector);
}

int
DVDGraph::get_spdif_pt() const
{
  return spdif_pt;
}

void
DVDGraph::set_spdif_pt(int _spdif_pt)
{
  spdif_pt = _spdif_pt;
  invalidate_chain();
  rebuild_node(state_detector);
}

bool
DVDGraph::get_spdif_as_pcm() const
{
  return spdif_as_pcm;
}

void
DVDGraph::set_spdif_as_pcm(bool _spdif_as_pcm)
{
  spdif_as_pcm = _spdif_as_pcm;
  invalidate_chain();
  rebuild_node(state_detector);
}

bool
DVDGraph::get_spdif_encode() const
{
  return spdif_encode;
}

void
DVDGraph::set_spdif_encode(bool _spdif_encode)
{
  spdif_encode = _spdif_encode;
  invalidate_chain();
  rebuild_node(state_detector);
}

bool
DVDGraph::get_spdif_stereo_pt() const
{
  return spdif_stereo_pt;
}

void
DVDGraph::set_spdif_stereo_pt(bool _spdif_stereo_pt)
{
  spdif_stereo_pt = _spdif_stereo_pt;
  invalidate_chain();
  rebuild_node(state_detector);
}

int
DVDGraph::get_spdif_bitrate() const
{
  return spdif_bitrate;
}

void
DVDGraph::set_spdif_bitrate(int _spdif_bitrate)
{
  spdif_bitrate = _spdif_bitrate;
  invalidate_chain();
  rebuild_node(state_encode);
}

///////////////////////////////////////////////////////////
// SPDIF sample rate check

void
DVDGraph::set_spdif_sr(bool _spdif_check_sr, bool _spdif_allow_48, bool _spdif_allow_44, bool _spdif_allow_32)
{
  spdif_check_sr = _spdif_check_sr;
  spdif_allow_48 = _spdif_allow_48;
  spdif_allow_44 = _spdif_allow_44;
  spdif_allow_32 = _spdif_allow_32;
  invalidate_chain();
  rebuild_node(state_detector);
}

bool
DVDGraph::get_spdif_check_sr() const
{
  return spdif_check_sr;
}
void
DVDGraph::set_spdif_check_sr(bool _spdif_check_sr)
{
  spdif_check_sr = _spdif_check_sr;
  invalidate_chain();
  rebuild_node(state_detector);
}

bool
DVDGraph::get_spdif_allow_48() const
{
  return spdif_allow_48;
}
void
DVDGraph::set_spdif_allow_48(bool _spdif_allow_48)
{
  spdif_allow_48 = _spdif_allow_48;
  invalidate_chain();
  rebuild_node(state_detector);
}

bool
DVDGraph::get_spdif_allow_44() const
{
  return spdif_allow_44;
}
void
DVDGraph::set_spdif_allow_44(bool _spdif_allow_44)
{
  spdif_allow_44 = _spdif_allow_44;
  invalidate_chain();
  rebuild_node(state_detector);
}

bool
DVDGraph::get_spdif_allow_32() const
{
  return spdif_allow_32;
}
void
DVDGraph::set_spdif_allow_32(bool _spdif_allow_32)
{
  spdif_allow_32 = _spdif_allow_32;
  invalidate_chain();
  rebuild_node(state_detector);
}

///////////////////////////////////////////////////////////
// SPDIF/DTS mode

int
DVDGraph::get_dts_mode() const
{
  return spdifer_pt.get_dts_mode();
}
void
DVDGraph::set_dts_mode(int _dts_mode)
{
  spdifer_pt.set_dts_mode(_dts_mode);
  spdifer_enc.set_dts_mode(_dts_mode);
  invalidate_chain();
  rebuild_node(state_detector);
}

int
DVDGraph::get_dts_conv() const
{
  return spdifer_pt.get_dts_conv();
}
void
DVDGraph::set_dts_conv(int _dts_conv)
{
  spdifer_pt.set_dts_conv(_dts_conv);
  spdifer_enc.set_dts_conv(_dts_conv);
  invalidate_chain();
  rebuild_node(state_detector);
}

///////////////////////////////////////////////////////////
// SPDIF status

int 
DVDGraph::get_spdif_status() const
{
  return spdif_status;
}

size_t
DVDGraph::get_info(char *_buf, size_t _len) const
{
  Speakers spk;
  static const size_t buf_size = 2048;
  char buf[buf_size];
  size_t pos = 0;

  spk = get_input();
  pos += sprintf(buf + pos, "Input format: %s %s %i\n", spk.format_text(), spk.mode_text(), spk.sample_rate);

  spk = user_spk;
  pos += sprintf(buf + pos, "User format: %s %s %i\n", spk.format_text(), spk.mode_text(), spk.sample_rate);

  spk = get_output();
  pos += sprintf(buf + pos, "Output format: %s %s %i\n", spk.format_text(), spk.mode_text(), spk.sample_rate);

  if (use_spdif)
  {
    pos += sprintf(buf + pos, "\nUse SPDIF\n");

    pos += sprintf(buf + pos, "  SPDIF status: ");
    switch (spdif_status)
    {
      case SPDIF_MODE_NONE:        pos += sprintf(buf + pos, "No data\n"); break;
      case SPDIF_MODE_DISABLED:    pos += sprintf(buf + pos, "Disabled "); break;
      case SPDIF_MODE_PASSTHROUGH: pos += sprintf(buf + pos, "SPDIF passthrough\n"); break;
      case SPDIF_MODE_ENCODE:      pos += sprintf(buf + pos, "AC3 encode\n"); break;
      default:                     pos += sprintf(buf + pos, "Unknown\n"); break;
    }

    if (spdif_status == SPDIF_MODE_DISABLED)
    {
      switch (spdif_err)
      {
        case SPDIF_ERR_STEREO_PCM:       pos += sprintf(buf + pos, "(Do not encode stereo PCM)\n"); break;
        case SPDIF_ERR_FORMAT:           pos += sprintf(buf + pos, "(Format is not allowed for passthrough)\n"); break;
        case SPDIF_ERR_SAMPLE_RATE:      pos += sprintf(buf + pos, "(Disallowed sample rate)\n"); break;
        case SPDIF_ERR_SINK:             pos += sprintf(buf + pos, "(SPDIF output is not supported)\n"); break;
        case SPDIF_ERR_ENCODER_DISABLED: pos += sprintf(buf + pos, "(AC3 encoder disabled)\n"); break;
        case SPDIF_ERR_PROC:             pos += sprintf(buf + pos, "(Cannot determine format to encode)\n"); break;
        case SPDIF_ERR_ENCODER:          pos += sprintf(buf + pos, "(Encoder does not support the format given)\n"); break;
        default:                         pos += sprintf(buf + pos, "\n"); break;
      }
    }

    pos += sprintf(buf + pos, "  SPDIF passthrough for:");
    if (spdif_pt & FORMAT_MASK_MPA) pos += sprintf(buf + pos, " MPA");
    if (spdif_pt & FORMAT_MASK_AC3) pos += sprintf(buf + pos, " AC3");
    if (spdif_pt & FORMAT_MASK_DTS) pos += sprintf(buf + pos, " DTS");
    pos += sprintf(buf + pos, spdif_pt? "\n": " -\n");

    if (spdif_encode)
      pos += sprintf(buf + pos, "  Use AC3 encoder (%s)\n",
        spdif_stereo_pt? "do not encode stereo PCM": "encode stereo PCM");
    else
      pos += sprintf(buf + pos, "  Do not use AC3 encoder\n");

    if (spdif_as_pcm)
      pos += sprintf(buf + pos, "  SPDIF as PCM output");

    if (spdif_check_sr)
    {
      if (!spdif_allow_48 && !spdif_allow_44 && !spdif_allow_32)
        pos += sprintf(buf + pos, "  Check SPDIF sample rate: NO ONE SAMPLE RATE ALLOWED!\n");
      else
      {
        pos += sprintf(buf + pos, "  Check SPDIF sample rate (allow:");
        if (spdif_allow_48) pos += sprintf(buf + pos, " 48kHz");
        if (spdif_allow_44) pos += sprintf(buf + pos, " 44.1kHz");
        if (spdif_allow_32) pos += sprintf(buf + pos, " 32kHz");
        pos += sprintf(buf + pos, ")\n");
      }
    }
    else
      pos += sprintf(buf + pos, "  Do not check SPDIF sample rate\n");

    if (query_sink)
      pos += sprintf(buf + pos, "  Query for SPDIF output support\n");
    else
      pos += sprintf(buf + pos, "  Do not query for SPDIF output support\n");
  }

  if (chain_next(node_start) != node_end)
  {
    pos += sprintf(buf + pos, "\nDecoding chain:\n");
    pos += chain_text(buf + pos, buf_size - pos);

    pos += sprintf(buf + pos, "\n\nFilters info (in order of processing):\n\n");
    int node = chain_next(node_start);
    while (node != node_end)
    {
      const char *filter_name = get_name(node);
      if (!filter_name) filter_name = "Unknown filter";
      pos += sprintf(buf + pos, "%s:\n", filter_name);

      switch (node)
      {
      case state_spdif_pt:
        pos += spdifer_pt.get_info(buf + pos, buf_size - pos);
        break;

      case state_decode:
        pos += dec.get_info(buf + pos, buf_size - pos);
        break;

      case state_proc:
      case state_proc_enc:
        pos += proc.get_info(buf + pos, buf_size - pos);
        pos += sprintf(buf + pos, "\n");
        break;

      case state_spdif_enc:
        pos += spdifer_enc.get_info(buf + pos, buf_size - pos);
        break;

      default:
        pos += sprintf(buf + pos, "-\n");
        break;
      }
      pos += sprintf(buf + pos, "\n");
      node = chain_next(node);
    }
  }

  if (pos + 1 > _len) pos = _len - 1;
  memcpy(_buf, buf, pos + 1);
  _buf[pos] = 0;
  return pos;
}


/////////////////////////////////////////////////////////////////////////////
// Filter overrides

void 
DVDGraph::reset()
{
  spdif_status = use_spdif? SPDIF_MODE_NONE: SPDIF_MODE_DISABLED;
  spdif_err = use_spdif? SPDIF_MODE_NONE: SPDIF_MODE_DISABLED;

  demux.reset();
  detector.reset();
  despdifer.reset();
  spdifer_pt.reset();
  dec.reset();
  proc.reset();
  enc.reset();
  spdifer_enc.reset();
  spdif2pcm.reset();

  FilterGraph::reset();
}

/////////////////////////////////////////////////////////////////////////////
// FilterGraph overrides

const char *
DVDGraph::get_name(int node) const
{
  switch (node)
  {
    case state_demux:       return "Demux";
    case state_detector:    return "Detector";
    case state_despdif:     return "Despdif";
    case state_spdif_pt:    return "Spdifer";
    case state_decode:      return "Decoder";
    case state_proc:
    case state_proc_enc:
                            return "Processor";
    case state_encode:      return "Encoder";
    case state_spdif_enc:   return "Spdifer";
    case state_spdif2pcm:   return "SPDIF->PCM";
    case state_dejitter:    return "Dejitter";
  }
  return 0;
}

Filter *
DVDGraph::init_filter(int node, Speakers spk)
{
  switch (node)
  {
    case state_demux:
      return &demux;

    case state_detector:
      return &detector;

    case state_despdif:
      return &despdifer;

    case state_spdif_pt:    
      // reset AudioProcessor to indicate no processing 
      // activity in spdif passthrough mode
      proc.reset();

      spdif_status = SPDIF_MODE_PASSTHROUGH;
      return &spdifer_pt;

    case state_decode:
      return &dec;

    case state_proc:
    {
      spdif_status = SPDIF_MODE_DISABLED;

      // Setup audio processor
      Speakers agreed_spk = agree_output_pcm(spk, user_spk);
      if (!proc.set_user(agreed_spk))
        return 0;

      return &proc;
    }

    case state_proc_enc:
    {
      spdif_status = SPDIF_MODE_ENCODE;

      // Setup audio processor
      Speakers proc_user = user_spk;
      proc_user.format = FORMAT_LINEAR;
      if (!proc.set_user(proc_user))
        return 0;

      return &proc;
    }

    case state_encode:
      if (enc.set_bitrate(spdif_bitrate))
        return &enc;
      else
        return 0;

    case state_spdif_enc:
      return &spdifer_enc;

    case state_spdif2pcm:
      return &spdif2pcm;

    case state_dejitter:
      return &syncer;
  }
  return 0;
}

int 
DVDGraph::get_next(int node, Speakers spk) const
{
  ///////////////////////////////////////////////////////
  // When get_next() is called graph must guarantee
  // that all previous filters was initialized
  // so we may use upstream filters' status
  // (here we use spdif_status updated at init_filter() )
  // First filter in the chain must not use any
  // information from other filters (it has no upstream)

  switch (node)
  {
    /////////////////////////////////////////////////////
    // input -> state_demux
    // input -> state_detector
    // input -> state_despdif
    // input -> state_spdif_pt
    // input -> state_decode
    // input -> state_proc
    // input -> state_proc_enc

    case node_start:
      if (demux.query_input(spk)) 
        return state_demux;

      if (use_detector && spk.format == FORMAT_PCM16 && spk.mask == MODE_STEREO)
        return state_detector;

      if (despdifer.query_input(spk))
        return state_despdif;

      spdif_err = check_spdif_passthrough(spk);
      if (spdif_err == SPDIF_MODE_PASSTHROUGH)
        return state_spdif_pt;

      if (dec.query_input(spk))
        return state_decode;

      if (proc.query_input(spk))
      {
        spdif_err = check_spdif_encode(spk);
        if (spdif_err == SPDIF_MODE_ENCODE)
          return state_proc_enc;
        else
          return state_proc;
      }

      return node_err;

    /////////////////////////////////////////////////////
    // state_detector -> state_despdif
    // state_detector -> state_spdif_pt
    // state_detector -> state_decode
    // state_detector -> state_proc
    // state_detector -> state_proc_enc

    case state_detector:
      if (despdifer.query_input(spk))
        return state_despdif;

      spdif_err = check_spdif_passthrough(spk);
      if (spdif_err == SPDIF_MODE_PASSTHROUGH)
        return state_spdif_pt;

      if (dec.query_input(spk))
        return state_decode;

      if (proc.query_input(spk))
      {
        spdif_err = check_spdif_encode(spk);
        if (spdif_err == SPDIF_MODE_ENCODE)
          return state_proc_enc;
        else
          return state_proc;
      }

      return node_err;

    /////////////////////////////////////////////////////
    // state_despdif -> state_spdif_pt
    // state_despdif -> state_decode

    case state_despdif:
      spdif_err = check_spdif_passthrough(spk);
      if (spdif_err == SPDIF_MODE_PASSTHROUGH)
        return state_spdif_pt;

      if (dec.query_input(spk))
        return state_decode;

      return node_err;

    /////////////////////////////////////////////////////
    // state_demux -> state_spdif_pt
    // state_demux -> state_decode
    // state_demux -> state_proc
    // state_demux -> state_proc_enc

    case state_demux:
      spdif_err = check_spdif_passthrough(spk);
      if (spdif_err == SPDIF_MODE_PASSTHROUGH)
        return state_spdif_pt;

      if (dec.query_input(spk))
        return state_decode;

      if (proc.query_input(spk))
      {
        spdif_err = check_spdif_encode(spk);
        if (spdif_err == SPDIF_MODE_ENCODE)
          return state_proc_enc;
        else
          return state_proc;
      }

      return node_err;

    /////////////////////////////////////////////////////
    // state_spdif_pt -> state_dejitter
    // state_spdif_pt -> state_decode

    case state_spdif_pt:
      // Spdifer may return return high-bitrare DTS stream
      // that is impossible to passthrough
      if (spk.format != FORMAT_SPDIF)
        return state_decode;

      if (spdif_as_pcm)
        return state_spdif2pcm;
      else
        return state_dejitter;

    /////////////////////////////////////////////////////
    // state_decode -> state_proc
    // state_decode -> state_proc_enc

    case state_decode:
      if (proc.query_input(spk))
      {
        spdif_err = check_spdif_encode(spk);
        if (spdif_err == SPDIF_MODE_ENCODE)
          return state_proc_enc;
        else
          return state_proc;
      }

      return node_err;

    /////////////////////////////////////////////////////
    // state_proc -> state_dejitter

    case state_proc:
      return state_dejitter;

    /////////////////////////////////////////////////////
    // state_proc_enc -> state_encode

    case state_proc_enc:
      return state_encode;

    /////////////////////////////////////////////////////
    // state_encode -> state_spdif_enc

    case state_encode:
      return state_spdif_enc;

    /////////////////////////////////////////////////////
    // state_spdif_enc -> state_decode
    // state_spdif_enc -> state_dejitter

    case state_spdif_enc:
      if (spdif_as_pcm)
        return state_spdif2pcm;
      else
        return state_dejitter;

    /////////////////////////////////////////////////////
    // state_spdif2pcm -> state_dejitter

    case state_spdif2pcm:
      return state_dejitter;

    /////////////////////////////////////////////////////
    // state_dejitter -> output

    case state_dejitter:
      return node_end;

  }

  // never be here...
  return node_err;
}

int
DVDGraph::check_spdif_passthrough(Speakers _spk) const
{
  // SPDIF-1 check (passthrough)

  if (!use_spdif)
    return SPDIF_MODE_DISABLED;

  // check format
  if ((spdif_pt & FORMAT_MASK(_spk.format)) == 0)
    return SPDIF_ERR_FORMAT;

  // check sample rate
  if (spdif_check_sr && _spk.sample_rate)
    if ((!spdif_allow_48 || _spk.sample_rate != 48000) && 
        (!spdif_allow_44 || _spk.sample_rate != 44100) && 
        (!spdif_allow_32 || _spk.sample_rate != 32000))
      return SPDIF_ERR_SAMPLE_RATE;

  // check sink
  if (sink && query_sink && !spdif_as_pcm)
    if (!sink->query_input(Speakers(FORMAT_SPDIF, _spk.mask, _spk.sample_rate)))
      return SPDIF_ERR_SINK;

  return SPDIF_MODE_PASSTHROUGH;
}

int
DVDGraph::check_spdif_encode(Speakers _spk) const
{
  // SPDIF-2 check (encode)

  if (!use_spdif)
    return SPDIF_MODE_DISABLED;

  if (!spdif_encode)
    return SPDIF_ERR_ENCODER_DISABLED;

  // determine encoder's input format
  Speakers enc_spk = proc.user2output(_spk, user_spk);
  if (enc_spk.is_unknown())
    return SPDIF_ERR_PROC;
  enc_spk.format = FORMAT_LINEAR;
  enc_spk.level = 1.0;

  // do not encode stereo PCM
  if (spdif_stereo_pt && (enc_spk.mask == MODE_STEREO || enc_spk.mask == MODE_MONO))
    return SPDIF_ERR_STEREO_PCM;

  // check sample rate
  if (spdif_check_sr && enc_spk.sample_rate)
    if ((!spdif_allow_48 || enc_spk.sample_rate != 48000) && 
        (!spdif_allow_44 || enc_spk.sample_rate != 44100) && 
        (!spdif_allow_32 || enc_spk.sample_rate != 32000))
      return SPDIF_ERR_SAMPLE_RATE;

  // check encoder
  if (!enc.query_input(enc_spk))
    return SPDIF_ERR_ENCODER;

  // check sink
  if (sink && query_sink && !spdif_as_pcm)
    if (!sink->query_input(Speakers(FORMAT_SPDIF, enc_spk.mask, enc_spk.sample_rate)))
      return SPDIF_ERR_SINK;

  return SPDIF_MODE_ENCODE;
}

Speakers
DVDGraph::agree_output_pcm(Speakers _spk, Speakers _user_spk) const
{
  if (!sink || !query_sink)
    return _user_spk;

  // Apply user_spk to the input format

  if (_user_spk.format != FORMAT_UNKNOWN)
  {
    _spk.format = _user_spk.format;
    _spk.level = _user_spk.level;
  }

  if (_user_spk.mask)
    _spk.mask = _user_spk.mask;

  if (_user_spk.sample_rate)
    _spk.sample_rate = _user_spk.sample_rate;

  _spk.relation = _user_spk.relation;

  // Query direct user format

  if (sink->query_input(_spk) && proc.query_user(_spk))
    return _spk;

  // We're failed. 
  // Try to downgrade the format on the first pass.
  // Try do downgrage the format and the number of channels on the second pass.

  for (int i = 0; i < 2; i++)
  {
    Speakers enum_spk = _spk;
    if (i > 0) enum_spk.mask = MODE_STEREO;

    while (!enum_spk.is_unknown())
    {
      if (sink->query_input(enum_spk) && proc.query_user(enum_spk))
        return enum_spk;

      switch (enum_spk.format)
      {
        case FORMAT_LINEAR:   enum_spk.format = FORMAT_PCMFLOAT; break;
        case FORMAT_PCMFLOAT: enum_spk.format = FORMAT_PCM32; break;
        case FORMAT_PCM32:    enum_spk.format = FORMAT_PCM24; break;
        case FORMAT_PCM24:    enum_spk.format = FORMAT_PCM16; break;
        case FORMAT_PCM16:    enum_spk.format = FORMAT_UNKNOWN; break;

        case FORMAT_PCM32_BE: enum_spk.format = FORMAT_PCM24_BE; break;
        case FORMAT_PCM24_BE: enum_spk.format = FORMAT_PCM16_BE; break;
        case FORMAT_PCM16_BE: enum_spk.format = FORMAT_UNKNOWN; break;
        default:
          enum_spk.format = FORMAT_UNKNOWN; break;
      }
    }
  }

  // Failed to downgrade the format and number of channels.
  // Try to use format of the sink.

  _spk = sink->get_input();
  if (proc.query_user(_spk))
    return _spk;

  // Everything failed. Use user_spk in the hope that it still may work...
  return _user_spk;
}


///////////////////////////////////////////////////////////////////////////////
// Spdif2PCM class
///////////////////////////////////////////////////////////////////////////////

Spdif2PCM::Spdif2PCM(): NullFilter(-1) 
{};

Speakers
Spdif2PCM::get_output() const
{
  if (spk.format == FORMAT_SPDIF)
    return Speakers(FORMAT_PCM16, MODE_STEREO, spk.sample_rate);
  else
    return spk;
}
bool
Spdif2PCM::get_chunk(Chunk *_chunk)
{ 
  send_chunk_inplace(_chunk, size);
  if (_chunk->spk.format == FORMAT_SPDIF)
    _chunk->spk = Speakers(FORMAT_PCM16, MODE_STEREO, spk.sample_rate);
  return true;
}
