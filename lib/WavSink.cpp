#include <memory.h>
#include <AudioFilter/WavSink.h>
#include "WinSpk.h"

namespace {

// Positions of header fields
const int riff_size32_pos(4);

}; // anonymous namespace

namespace AudioFilter {

WavSink::WavSink()
{
  spk = spk_unknown;
  header_size = 0;
  data_size = 0;
  file_format = (uint8_t*) new WAVEFORMATEXTENSIBLE;
  memset(file_format, 0, sizeof(WAVEFORMATEXTENSIBLE));
}

WavSink::WavSink(const char *_filename)
{
  spk = spk_unknown;
  header_size = 0;
  data_size = 0;
  file_format = (uint8_t*) new WAVEFORMATEXTENSIBLE;
  memset(file_format, 0, sizeof(WAVEFORMATEXTENSIBLE));

  open(_filename);
}

WavSink::~WavSink()
{
  close();
  delete (WAVEFORMATEXTENSIBLE*)file_format;
}

void WavSink::initRiff(void)
{
  WAVEFORMATEX *wfx = (WAVEFORMATEX *)file_format;
  uint32_t format_size = sizeof(WAVEFORMATEX) + wfx->cbSize;
  f.seek(0);

  // RIFF header
  f.write("RIFF\0\0\0\0WAVE", 12);

#if 0
  // RF64 junk chunk
  // (to be replaced with ds64 chunk if necessary)
  f.write("JUNK\x1c\0\0\0", 8);    // header
  f.write("\0\0\0\0\0\0\0\0", 8); // RIFF size
  f.write("\0\0\0\0\0\0\0\0", 8); // data size
  f.write("\0\0\0\0\0\0\0\0", 8); // sample count
  f.write("\0\0\0\0", 4);         // table length (unused)
#endif

  // Format chunk
  f.write("fmt ", 4);
  f.write(&format_size, 4);
  f.write(file_format, format_size);

  // Data chunk
  f.write("data\0\0\0\0", 8);

  header_size = (uint32_t) f.pos();
}

void WavSink::closeRiff(void)
{
  uint64_t riff_size = header_size + data_size - 8;

  if ( riff_size <= 0xffffffff )
  {
    // Write usual RIFF sizes
    uint32_t riff_size32 = (uint32_t) riff_size;
    uint32_t data_size32 = (uint32_t) data_size;
    // RIFF size
    f.seek(riff_size32_pos);
    f.write(&riff_size32, 4);
    // data size
    f.seek(header_size - 4);
    f.write(&data_size32, 4);
  }
  else
  {
    // Make RF64 header
    uint32_t riff_size32 = 0xffffffff;
    uint32_t data_size32 = 0xffffffff;

    WAVEFORMATEX *wfx = (WAVEFORMATEX *)file_format;
    uint64_t sample_count = data_size / wfx->nBlockAlign;

    // RF64 header
    f.seek(0);
    f.write("RF64", 4);
    f.write(&riff_size32, 4);
    f.write("WAVE", 4);

    // ds64 chunk
    f.write("ds64\x1c\0\0\0", 8); // header
    f.write(&riff_size, 8);
    f.write(&data_size, 8);
    f.write(&sample_count, 8);

    // data size
    f.seek(header_size - 4);
    f.write(&data_size32, 4);
  }
}

bool WavSink::open(const char *_filename)
{
  close();

  if ( f.open(_filename, "wb") )
  {
    spk = spk_unknown;
    data_size = 0;
    memset(file_format, 0, sizeof(WAVEFORMATEXTENSIBLE));
    return true;
  }

  return false;
}

void WavSink::close(void)
{
  if ( f.isOpen() )
  {
    closeRiff();
    f.close();

    spk = spk_unknown;
    header_size = 0;
    data_size = 0;
    memset(file_format, 0, sizeof(WAVEFORMATEXTENSIBLE));
  }
}

bool WavSink::isOpen(void) const
{
  return f.isOpen();
}

///////////////////////////////////////////////////////////
// Sink interface

bool WavSink::queryInput(Speakers _spk) const
{
  WAVEFORMATEXTENSIBLE wfx;
  bool use_wfx = false;

  if ( _spk.format == FORMAT_LINEAR )
    return false;

  if ( _spk.format & FORMAT_CLASS_PCM )
  {
    if ( _spk.mask != MODE_MONO && _spk.mask != MODE_STEREO )
      use_wfx = true;
  }

  if ( ! spk2wfx(_spk, (WAVEFORMATEX *)&wfx, use_wfx) )
    return false;

  return true;
}

bool WavSink::setInput(Speakers _spk)
{
  // Determine file format

  WAVEFORMATEXTENSIBLE wfx;
  bool use_wfx = false;

  if ( _spk.format == FORMAT_LINEAR )
    return false;

  if ( FORMAT_MASK(_spk.format) & FORMAT_CLASS_PCM )
  {
    if ( _spk.mask != MODE_MONO && _spk.mask != MODE_STEREO )
      use_wfx = true;
  }

  if ( ! spk2wfx(_spk, (WAVEFORMATEX *)&wfx, use_wfx) )
    return false;

  // Reset the file only in case when formats are not compatible
  if ( memcmp(&wfx, file_format, sizeof(WAVEFORMATEX) + wfx.Format.cbSize) )
  {
    spk = _spk;
    data_size = 0;
    memcpy(file_format, &wfx, sizeof(WAVEFORMATEX) + wfx.Format.cbSize);
    initRiff();
  }

  return true;
}

Speakers WavSink::getInput(void) const
{
  return spk;
}

bool WavSink::process(const Chunk *_chunk)
{
  if ( ! _chunk->isDummy() )
  {
    if ( _chunk->spk == spk || setInput(_chunk->spk) )
    {
      data_size += f.write(_chunk->rawdata, _chunk->size);
    }
    else
    {
      return false;
    }
  }

  return true;
}

}; // namespace AudioFilter

// vim: ts=2 sts=2 et

