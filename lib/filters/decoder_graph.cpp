#include <stdio.h>
#include "decoder_graph.h"

DecoderGraph::DecoderGraph()
:FilterGraph(-1), proc(4096)
{
  user_spk = Speakers(FORMAT_PCM16, 0, 0);
};

///////////////////////////////////////////////////////////
// User format

bool
DecoderGraph::query_user(Speakers _user_spk) const
{
  if (!user_spk.format)
    return false;

  return proc.query_user(_user_spk);
}

bool
DecoderGraph::set_user(Speakers _user_spk)
{
  if (!query_user(_user_spk))
    return false;

  _user_spk.sample_rate = 0;
  if (user_spk != _user_spk)
  {
    user_spk = _user_spk;
    rebuild_node(state_proc);
  }

  return true;
}

Speakers              
DecoderGraph::get_user() const
{
  return user_spk;
}

size_t
DecoderGraph::get_info(char *_buf, size_t _len) const
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
      case state_decode:
        pos += dec.get_info(buf + pos, buf_size - pos);
        break;

      case state_proc:
        pos += proc.get_info(buf + pos, buf_size - pos);
        pos += sprintf(buf + pos, "\n");
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
DecoderGraph::reset()
{
  despdifer.reset();
  dec.reset();
  proc.reset();

  FilterGraph::reset();
}

/////////////////////////////////////////////////////////////////////////////
// FilterGraph overrides

const char *
DecoderGraph::get_name(int node) const
{
  switch (node)
  {
    case state_despdif:     return "Despdif";
    case state_decode:      return "Decoder";
    case state_proc:        return "Processor";
  }
  return 0;
}

Filter *
DecoderGraph::init_filter(int node, Speakers spk)
{
  switch (node)
  {
    case state_despdif:
      return &despdifer;

    case state_decode:
      return &dec;

    case state_proc:
    {
      // Setup audio processor
      if (!proc.set_user(user_spk))
        return 0;

      return &proc;
    }
  }
  return 0;
}

int 
DecoderGraph::get_next(int node, Speakers spk) const
{
  switch (node)
  {
    /////////////////////////////////////////////////////
    // input -> state_despdif
    // input -> state_decode
    // input -> state_proc

    case node_start:
      if (despdifer.query_input(spk))
        return state_despdif;

      if (dec.query_input(spk))
        return state_decode;

      if (proc.query_input(spk))
        return state_proc;

      return node_err;

    /////////////////////////////////////////////////////
    // state_despdif -> state_decode

    case state_despdif:
      if (dec.query_input(spk))
        return state_decode;

      return node_err;

    /////////////////////////////////////////////////////
    // state_decode -> state_proc

    case state_decode:
      if (proc.query_input(spk))
        return state_proc;

      return node_err;

    /////////////////////////////////////////////////////
    // state_proc -> output

    case state_proc:
      return node_end;
  }

  // never be here...
  return node_err;
}
