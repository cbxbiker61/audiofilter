#include <cstdlib>
#include <cstring>
#include <fstream>
#include <math.h>
#include <AudioFilter/SpdifWrapper.h>
#include <AudioFilter/FileParser.h>

#define FLOAT_THRESHOLD 1e-20

namespace {

const size_t max_buf_size(65536);

int compactSize(AudioFilter::AutoFile::fsize_t size)
{
  for ( int i = 0; size >= 10000 && i < 5; ++i )
  {
    size /= 1024;
  }

  return (int)size;
}

const char *compactSuffix(AudioFilter::AutoFile::fsize_t size)
{
  static const char *suffixes[] = { "", "K", "M", "G", "T", "P" };

  int i;

  for ( i = 0; size >= 10000 && i < 5; ++i )
  {
    size /= 1024;
  }

  return suffixes[i];
}

}; // anonymous namespace

namespace AudioFilter {

FileParser::FileParser()
  : filename()
{
  init();
}

FileParser::FileParser(const char *_filename, HeaderParser *_parser, size_t _max_scan)
  : filename()
{
  init();
  open(_filename, _parser, _max_scan);
}

void FileParser::init(void)
{
  buf = new uint8_t[max_buf_size];
  buf_size = max_buf_size;
  buf_data = 0;
  buf_pos = 0;

  stat_size = 0;
  avg_frame_interval = 0;
  avg_bitrate = 0;

  max_scan = 0;
  _intHeaderParser = 0;
}

FileParser::~FileParser()
{
  close();

  if ( _intHeaderParser )
    delete _intHeaderParser;

  if ( buf )
    delete[] buf;
}

///////////////////////////////////////////////////////////////////////////////
// File operations

bool FileParser::open(const char *_filename, HeaderParser *_parser, size_t _max_scan)
{
  if ( ! _filename )
    return false;

  if ( isOpen() )
    close();

  if ( ! _parser )
    ; // TODO: this is where I'm supposed to select a parser

  if ( stream.setParser(_parser) && f.open(_filename) )
  {
    max_scan = _max_scan;
    filename = _filename;

    reset();
    return true;
  }

  return false;
}

void FileParser::close(void)
{
  stream.releaseParser();
  f.close();
  stat_size = 0;
  avg_frame_interval = 0;
  avg_bitrate = 0;
  max_scan = 0;
}

bool FileParser::probe(void)
{
  if ( f )
  {
    fsize_t old_pos(f.pos());
    bool result(loadFrame());
    f.seek(old_pos);
    return result;
  }

  return false;
}

bool FileParser::stats(unsigned max_measurements, vtime_t precision)
{
  if ( ! f )
    return false;

  fsize_t old_pos = f.pos();

  /* If we cannot load a frame we will not gather any stats.
   * (If file format is unknown, measurements may take a lot of time)
   */
  if ( ! loadFrame() )
  {
    f.seek(old_pos);
    return false;
  }

  stat_size = 0;
  avg_frame_interval = 0;
  avg_bitrate = 0;

  vtime_t old_length;
  vtime_t new_length;

  old_length = 0;

  for ( unsigned int i = 0; i < max_measurements; ++i )
  {
    fsize_t file_pos = fsize_t((double)rand() * f.size() / RAND_MAX);
    f.seek(file_pos);

    if ( ! loadFrame() )
      continue;

    ///////////////////////////////////////////////////////
    // Update stats

    HeaderInfo hinfo(stream.getHeaderInfo());

    ++stat_size;
    avg_frame_interval += stream.getFrameInterval();
    avg_bitrate += float(stream.getFrameInterval() * 8 * hinfo.spk.sample_rate)
                        / hinfo.nsamples;

    ///////////////////////////////////////////////////////
    // Finish scanning if we have enough accuracy

    if ( precision > FLOAT_THRESHOLD )
    {
      new_length = double(f.size()) * 8 * stat_size / avg_bitrate;

      if ( stat_size > 10 && fabs(old_length - new_length) < precision )
        break;

      old_length = new_length;
    }
  }

  if ( stat_size )
  {
    avg_frame_interval /= stat_size;
    avg_bitrate        /= stat_size;
  }

  seek(old_pos);
  return stat_size > 0;
}

std::string FileParser::getFileInfo(void) const
{
  char info[1024];
  info[0] = 0;

  size_t len = sprintf(info,
    "File: %s\n"
    "Size: %.0f (%i %sB)\n"
    , filename.c_str()
    , (double)f.size()
    , compactSize(f.size())
    , compactSuffix(f.size()));

  if ( stat_size )
  {
    len += sprintf(info + len,
      "Length: %i:%02i:%02i\n"
      "Frames: %i\n"
      "Frame interval: %i\n"
      "Bitrate: %ikbps\n",
      int(getSize(time)) / 3600, int(getSize(time)) % 3600 / 60, int(getSize(time)) % 60,
      int(getSize(frames)),
      int(avg_frame_interval),
      int(avg_bitrate / 1000));
  }

  return info;
}

///////////////////////////////////////////////////////////////////////////////
// Positioning

inline double FileParser::getUnitsFactor(units_t units) const
{
  switch ( units )
  {
    case bytes:
      return 1.0;
      break;
    case relative:
      return 1.0 / f.size();
      break;
    default:
      break;
  }

  if ( stat_size )
  {
    switch ( units )
    {
      case frames:
        return 1.0 / avg_frame_interval;
        break;
      case time:
        return 8.0 / avg_bitrate;
        break;
      default:
        break;
    }
  }

  return 0.0;
}

FileParser::fsize_t FileParser::getPos(void) const
{
  return f.isOpen() ? fsize_t(f.pos() - buf_data + buf_pos) : 0;
}

double FileParser::getPos(units_t units) const
{
  return getPos() * getUnitsFactor(units);
}

FileParser::fsize_t FileParser::getSize(void) const
{
  return f.size();
}

double FileParser::getSize(units_t units) const
{
  return f.size() * getUnitsFactor(units);
}

int FileParser::seek(fsize_t pos)
{
  const int result(f.seek(pos));

  reset();
  return result;
}

int FileParser::seek(double pos, units_t units)
{
  const double factor(getUnitsFactor(units));

  if ( factor > FLOAT_THRESHOLD )
    return seek(fsize_t(pos / factor));

  return -1;
}

///////////////////////////////////////////////////////////////////////////////
// Frame-level interface (StreamBuffer interface wrapper)

void FileParser::reset(void)
{
  buf_data = 0;
  buf_pos = 0;
  stream.reset();
}

bool FileParser::loadFrame(void)
{
  size_t sync_size(0);

  for ( ; ; )
  {
    ///////////////////////////////////////////////////////
    // Load a frame

    uint8_t *pos(buf + buf_pos);
    uint8_t *end(buf + buf_data);

    if ( stream.loadFrame(&pos, end) )
    {
      buf_pos = pos - buf;
      return true;
    }

    ///////////////////////////////////////////////////////
    // Stop file scanning if scanned too much

    sync_size += (pos - buf) - buf_pos;
    buf_pos = pos - buf;

    if ( max_scan > 0 ) // do limiting
    {
      if ( (sync_size > stream.getParser()->getMaxFrameSize() * 3) // min req to sync and load a frame
          && (sync_size > max_scan) ) // limit scanning
        return false;
    }

    ///////////////////////////////////////////////////////
    // Fill the buffer

    if ( ! buf_data || buf_pos >= buf_data )
    {
      /* Move the data
      if (buf_data && buf_pos)
        memmove(buf, buf + buf_pos, buf_data - buf_pos);
      buf_pos = 0;
      buf_data -= buf_pos;
      */

      buf_pos = 0;
      buf_data = fread(buf, 1, max_buf_size, f);

      if ( ! buf_data )
        return false;
    }
  }

  return false; // never be here
}

}; // namespace AudioFilter

// vim: ts=2 sts=2 et

