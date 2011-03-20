/*
  Raw file source
*/

#ifndef VALIB_RAW_SOURCE
#define VALIB_RAW_SOURCE

#include "../buffer.h"
#include "../auto_file.h"
#include "../filter.h"

class RAWSource: public Source
{
protected:
  AutoFile f;
  Speakers spk;
  Rawdata  buf;
  size_t   block_size;

public:
  typedef AutoFile::fsize_t fsize_t;

  RAWSource()
  {}
 
  RAWSource(Speakers _spk, const char *_filename, size_t _block_size = 65536)
  { open (_spk, _filename, _block_size); }

  RAWSource(Speakers _spk, FILE *_f, size_t _block_size = 65536)
  { open (_spk, _f, _block_size); }

  /////////////////////////////////////////////////////////
  // FileSource interface

  bool open(Speakers _spk, const char *_filename, size_t _block_size = 65536);
  bool open(Speakers _spk, FILE *_f, size_t _block_size = 65536);
  void close();

  inline bool    is_open() const { return f.is_open(); }
  inline bool    eof()     const { return f.eof();     }
  inline fsize_t size()    const { return f.size();    }
  inline FILE   *fh()      const { return f.fh();      }

  inline int seek(fsize_t _pos) { return f.seek(_pos); }
  inline fsize_t pos() const { return f.pos(); }

  /////////////////////////////////////////////////////////
  // Source interface

  virtual Speakers get_output() const  { return spk;               }
  virtual bool is_empty() const        { return !f.is_open() || f.eof(); }
  virtual bool get_chunk(Chunk *chunk);

};

#endif
