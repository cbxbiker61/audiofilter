#ifdef __unix__
#define _XOPEN_SOURCE
#include <unistd.h>
#endif
#include <iostream>
#include <AudioFilter/AutoFile.h>

using namespace AudioFilter;

size_t bsConvSwab16(const uint8_t *in_buf, size_t size, uint8_t *out_buf)
{
#ifdef __unix__
  swab(in_buf, out_buf, size);
  return size;
#else
  uint16_t *in16 = (uint16_t *)in_buf;
  uint16_t *out16 = (uint16_t *)out_buf;
  size_t i = size >> 1;

  while ( i-- )
    out16[i] = swab_u16(in16[i]);

  return size;
#endif
}

int main( int argc, char **argv )
{
  if ( argc < 3 )
  {
    std::cout << "Swab utility - swap bytes in the file\n"
"\n"
"Usage:\n"
"  > swab input_file output_file" << std::endl;
    return -1;
  }

  /////////////////////////////////////////////////////////
  // Open files

  char *in_filename(argv[1]);
  char *out_filename(argv[2]);

  AutoFile in_file(in_filename);

  if ( ! in_file.isOpen() )
  {
    std::cout << "Cannot open file " << in_filename << std::endl;
    return -1;
  }

  AutoFile out_file(out_filename, "wb");

  if ( ! out_file.isOpen() )
  {
    std::cout << "Cannot open file " << out_filename << std::endl;
    return -1;
  }

  const size_t buf_size(65536);
  uint8_t *buf(new uint8_t[buf_size]);

  while ( ! in_file.eof() )
  {
    size_t data_size = in_file.read(buf, buf_size);
    data_size = bsConvSwab16(buf, data_size, buf);
    out_file.write(buf, data_size);
  }

  delete[] buf;

  return 0;
}

