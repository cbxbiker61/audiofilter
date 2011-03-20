#include "multi_frame.h"
#include "multi_header.h"

MultiFrame::MultiFrame()
{
  parsers = 0;
  nparsers = 0;
  reset();
}

MultiFrame::MultiFrame(FrameParser **_parsers, size_t _nparsers)
{
  parsers = 0;
  nparsers = 0;
  reset();

  set_parsers(_parsers, _nparsers);
}

MultiFrame::~MultiFrame()
{
  release_parsers();
}

bool
MultiFrame::set_parsers(FrameParser **_parsers, size_t _nparsers)
{
  release_parsers();

  if (!multi_header.set_parsers(_parsers, _nparsers))
    return false;

  parsers = new FrameParser *[_nparsers];
  if (!parsers)
  {
    multi_header.release_parsers();
    return false;
  }

  nparsers = _nparsers;
  for (size_t i = 0; i < nparsers; i++)
    parsers[i] = _parsers[i];

  reset();
  return true;
}

void
MultiFrame::release_parsers()
{
  multi_header.release_parsers();

  safe_delete(parsers);
  nparsers = 0;

  reset();
}

/////////////////////////////////////////////////////////
// FrameParser overrides

void
MultiFrame::reset()
{
  spk = spk_unknown;
  nsamples = 0;
  samples.zero();
  rawdata = 0;
  rawsize = 0;

  parser = 0;
  hparser = 0;

  for (size_t i = 0; i < nparsers; i++)
    parsers[i]->reset();
}

bool
MultiFrame::parse_frame(uint8_t *frame, size_t size)
{
  if (!parser)
    if (!switch_parser(frame, size))
      return false;

  HeaderInfo hinfo;
  if (!hparser->parse_header(frame, &hinfo))
    if (!switch_parser(frame, size))
      return false;

  if (parser->parse_frame(frame, size))
  {
    spk = parser->get_spk();
    samples = parser->get_samples();
    nsamples = parser->get_nsamples();
    rawdata = parser->get_rawdata();
    rawsize = parser->get_rawsize();
    return true;
  }
  else
    return false;
}

bool
MultiFrame::switch_parser(uint8_t *frame, size_t size)
{
  parser = 0;
  hparser = 0;
  for (size_t i = 0; i < nparsers; i++)
    if (parsers[i]->header_parser()->parse_header(frame))
    {
      parser = parsers[i];
      hparser = parsers[i]->header_parser();
      break;
    }

  return parser != 0;
}

size_t
MultiFrame::stream_info(char *buf, size_t size) const
{
  if (parser)
    return parser->stream_info(buf, size);

  if (buf && size) buf[0] = 0;
  return 0;
}

size_t
MultiFrame::frame_info(char *buf, size_t size) const
{
  if (parser)
    return parser->frame_info(buf, size);

  if (buf && size) buf[0] = 0;
  return 0;
}
