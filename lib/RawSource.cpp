#include "raw_source.h"

bool 
RAWSource::open(Speakers _spk, const char *_filename, size_t _block_size)
{
  if (spk.format == FORMAT_LINEAR)
    return false;

  if (!f.open(_filename))
    return false;

  if (!buf.allocate(_block_size))
    return false;

  spk = _spk;
  block_size = _block_size;
  return true;
}

bool 
RAWSource::open(Speakers _spk, FILE *_f, size_t _block_size)
{
  if (spk.format == FORMAT_LINEAR)
    return false;

  if (!f.open(_f))
    return false;

  if (!buf.allocate(_block_size))
    return false;

  spk = _spk;
  block_size = _block_size;
  return true;
}

void 
RAWSource::close()
{ 
  spk = spk_unknown;
  f.close();
}

bool
RAWSource::get_chunk(Chunk *_chunk)
{
  size_t read_size = f.read(buf, block_size);
  bool eof = f.eof();
  _chunk->set_rawdata(spk, buf, read_size, false, 0, eof);
  return eof || read_size;
};
