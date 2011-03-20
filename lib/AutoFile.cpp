#include <iostream>
#include <limits>
#include <limits.h>
#include <AudioFilter/AutoFile.h>

namespace AudioFilter {

#if defined(_MSC_VER) && (_MSC_VER >= 1400)

const AutoFile::fsize_t AutoFile::max_size((AutoFile::fsize_t)-1);

static int portable_seek(FILE *f, AutoFile::fsize_t pos, int origin)
{
  return _fseeki64(f, pos, origin);
}

static AutoFile::fsize_t portable_tell(FILE *f)
{
  return _ftelli64(f);
}

#else // ! MSC

#ifdef __GNUC__
const AutoFile::fsize_t AutoFile::max_size(std::numeric_limits<off64_t>::max());
#else
const AutoFile::fsize_t AutoFile::max_size(LONG_MAX);
#endif

static int portable_seek(FILE *f, AutoFile::fsize_t pos, int origin)
{
  assert(pos < AutoFile::max_size);
#ifdef __GNUC__
  return ::fseeko64(f, pos, origin);
#else
  return ::fseek(f, (long)pos, origin);
#endif
}

static AutoFile::fsize_t portable_tell(FILE *f)
{
#ifdef __GNUC__
  return ::ftello64(f);
#else
  return ::ftell(f);
#endif
}

#endif // ! MSC

bool AutoFile::open(const char *filename, const char *mode)
{
  if ( f )
    close();

  filesize = max_size;
#ifdef __GNUC__
  f = ::fopen64(filename, mode);
#else
  f = ::fopen(filename, mode);
#endif

  if ( f )
  {
    if ( portable_seek(f, 0, SEEK_END) == 0 )
      filesize = portable_tell(f);

    portable_seek(f, 0, SEEK_SET);
    own_file = true;
  }

  return isOpen();
}

bool AutoFile::open(FILE *_f, bool takeOwnership)
{
  if ( f )
    close();

  filesize = max_size;
  f = _f;

  if ( f )
  {
    fsize_t old_pos(pos());

    if ( portable_seek(f, 0, SEEK_END) == 0 )
      filesize = portable_tell(f);

    portable_seek(f, old_pos, SEEK_SET);
    own_file = takeOwnership;
  }

  return isOpen();
}

void AutoFile::close(void)
{
  if ( f && own_file )
    fclose(f);

  f = 0;
}

int AutoFile::seek(fsize_t _pos)
{
  return portable_seek(f, _pos, SEEK_SET);
}

AutoFile::fsize_t AutoFile::pos(void) const
{
  return portable_tell(f);
}

MemFile::MemFile(const char *filename)
  : data(0), file_size(0)
{
  AutoFile f(filename, "rb");

  if ( ! f.isOpen() || f.size() == f.max_size || f.isLarge(f.size()) )
    return;

  file_size = f.castSize( f.size() );

  data = new uint8_t[file_size];

  size_t read_size = f.read(data, file_size);

  if ( read_size != file_size )
  {
    delete[] (uint8_t*)data;
    data = 0;
    file_size = 0;
  }
}

MemFile::~MemFile()
{
  if ( data )
    delete[] (uint8_t*)data;
}

}; // namespace AudioFilter

// vim: ts=2 sts=2 et

