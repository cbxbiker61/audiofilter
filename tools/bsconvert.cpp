#include <iostream>
#include <AudioFilter/AutoFile.h>
#include <AudioFilter/BitStream.h>
#include <AudioFilter/SpdifFrameParser.h>

using namespace AudioFilter;

void printInfo(StreamBuffer &stream, int bs_type);

inline const char *getBsName(int bs_type)
{
  switch ( bs_type )
  {
    case BITSTREAM_8:
      return "byte";
      break;
    case BITSTREAM_16BE:
      return "16bit big endian";
      break;
    case BITSTREAM_16LE:
      return "16bit little endian";
      break;
    case BITSTREAM_32BE:
      return "32bit big endian";
      break;
    case BITSTREAM_32LE:
      return "32bit little endian";
      break;
    case BITSTREAM_14BE:
      return "14bit big endian";
      break;
    case BITSTREAM_14LE:
      return "14bit little endian";
      break;
    default:
      return "??";
      break;
  }
}

inline bool is14Bit(int bs_type)
{
  return bs_type == BITSTREAM_14LE || bs_type == BITSTREAM_14BE;
}

int main(int argc, char **argv)
{
  if ( argc < 2 )
  {
    std::cout <<
"Bitstream converter\n"
"===================\n"
"This utility conversts files between numerous MPA/AC3/DTS stream types:\n"
"SPDIF padded, 8/14/16bit big/little endian. By default, it converts any\n"
"stream type to the most common byte stream.\n"
"\n"
"This utility is a part of AC3Filter project (http://ac3filter.net)\n"
"Copyright (c) 2007-2009 by Alexander Vigovsky\n"
"\n"
"Usage:\n"
"  Detect file type and print file information:\n"
"  > bsconvert input_file\n"
"\n"
"  Convert a file:\n"
"  > bsconvert input_file output_file [format]\n"
"\n"
"Options:\n"
"  input_file  - file to convert\n"
"  output_file - file to write result to\n"
"  format      - output file format:\n"
"    8     - byte stream (default)\n"
"    16le  - 16bit little endian\n"
"    14be  - 14bit big endian (DTS only)\n"
"    14le  - 14bit little endian (DTS only)\n"
"\n"
"Notes:\n"
"  File captured from SPDIF input may contain several parts of different type.\n"
"  For example SPDIF transmission may be started with 5.1 ac3 format then\n"
"  switch to 5.1 dts and then to stereo ac3. In this case all stream parts\n"
"  will be converted and writed to the same output file\n"
"\n"
"  SPDIF stream is padded with zeros, therefore output file size may be MUCH\n"
"  smaller than input. It is normal and this does not mean that some data was\n"
"  lost. This conversion is lossless! You can recreate SPDIF stream back with\n"
"  'spdifer' utility.\n"
"\n"
"  14bit streams are supported only for DTS format. Note, that conversion\n"
"  between 14bit and non-14bit changes actual frame size (frame interval),\n"
"  but does not change the frame header." << std::endl;
    return -1;
  }

  /////////////////////////////////////////////////////////
  // Parse command line

  char *inFilename(0);
  char *outFilename(0);
  int bs_type = BITSTREAM_8;

  switch (argc)
  {
  case 2:
    inFilename = argv[1];
    break;

  case 3:
    inFilename = argv[1];
    outFilename = argv[2];
    break;

  case 4:
    inFilename = argv[1];
    outFilename = argv[2];
    if (!strcmp(argv[3], "8"))
      bs_type = BITSTREAM_8;
    else if (!strcmp(argv[3], "16le"))
      bs_type = BITSTREAM_16LE;
    else if (!strcmp(argv[3], "14be"))
      bs_type = BITSTREAM_14BE;
    else if (!strcmp(argv[3], "14le"))
      bs_type = BITSTREAM_14LE;
    else
    {
      std::cout << "Unknown stream format: " << argv[3] << std::endl;
      return -1;
    }
    break;

  default:
    std::cout << "Wrong number of arguments" << std::endl;
    return -1;
  }

  /////////////////////////////////////////////////////////
  // Allocate buffers

  MultiHeaderParser mhp;
  mhp.addParser(new SpdifHeaderParser());
  mhp.addParser(new DtsHeaderParser());
  mhp.addParser(new Ac3HeaderParser());
  mhp.addParser(new MpaHeaderParser());

  SpdifFrameParser spdifFrameParser(false);
  StreamBuffer sb(&mhp);

  const size_t bufSize(512*1024);
  uint8_t *buf(new uint8_t[bufSize]);

  const size_t framebufSize((mhp.getMaxFrameSize()) / 7 * 8 + 8);
  uint8_t *framebuf(new uint8_t[framebufSize]);

  /////////////////////////////////////////////////////////
  // Open input file

  AutoFile inFile(inFilename);

  if ( ! inFile.isOpen() )
  {
    std::cout << "Cannot open file " << inFilename << std::endl;
    return -1;
  }

  /////////////////////////////////////////////////////////
  // Detect input file format and print stream info

  while ( inFile.pos() < 1000000 && ! sb.isInSync() )
  {
    size_t dataSize = inFile.read(buf, bufSize);
    uint8_t *ptr = buf;
    uint8_t *end = ptr + dataSize;
    sb.loadFrame(&ptr, end);
  }

  if ( ! sb.isInSync() )
  {
    std::cout << "Cannot detect file format" << std::endl;
    return -1;
  }
  else
  {
    std::cout << inFilename << std::endl;
    printInfo(sb, bs_type);
  }

  /////////////////////////////////////////////////////////
  // Open output file

  if ( ! outFilename )
    return 0;

  AutoFile outFile(outFilename, "wb");

  if ( ! outFile.isOpen() )
  {
    std::cout << "Cannot open file " << outFilename << std::endl;
    return -1;
  }

  /////////////////////////////////////////////////////////
  // Process data

  bool show_info(false);

  int frames(0);
  int bs_target(bs_type);
  bool isSpdif(false);
  inFile.seek(0);
  sb.reset();

  bs_conv_t conv = 0;

  while ( ! inFile.eof() )
  {
    size_t dataSize = inFile.read(buf, bufSize);
    uint8_t *ptr = buf;
    uint8_t *end = ptr + dataSize;

    while ( ptr < end )
    {
      if ( sb.loadFrame(&ptr, end) )
      {
        ++frames;

        if ( sb.isNewStream() )
        {
          if ( show_info )
          {
            std::cout << "\n" << std::endl;
            printInfo(sb, bs_type);
          }
          else
          {
            show_info = true;
          }

          // find conversion function
          HeaderInfo hi = sb.getHeaderInfo();
          isSpdif = hi.spk.format == FORMAT_SPDIF;

          if ( isSpdif )
          {
            if ( spdifFrameParser.parseFrame(sb.getFrame(), sb.getFrameSize()) )
              hi = spdifFrameParser.getHeaderInfo();
            else
              hi.drop();
          }

          bs_target = bs_type;

          if ( is14Bit(bs_target) && hi.spk.format != FORMAT_DTS )
            bs_target = BITSTREAM_8;

          std::cout << "Conversion from " << getBsName(hi.bs_type)
                  << " to " << getBsName(bs_target) << std::endl;
          conv = bs_conversion(hi.bs_type, bs_target);

          if ( ! conv )
            std::cout << "Cannot convert this stream!" << std::endl;
        }

        // Do the job here
        if ( conv )
        {
          const uint8_t *frame = sb.getFrame();
          size_t frameSize = sb.getFrameSize();

          if ( isSpdif )
          {
            if ( spdifFrameParser.parseFrame(sb.getFrame(), sb.getFrameSize()) )
            {
              frame = spdifFrameParser.getRawData();
              frameSize = spdifFrameParser.getRawSize();
            }
            else
            {
              frame = 0;
              frameSize = 0;
            }
          }

          size_t framebuf_data = (*conv)(frame, frameSize, framebuf);

          // Correct DTS header
          if ( bs_target == BITSTREAM_14LE )
            framebuf[3] = 0xe8;
          else if ( bs_target == BITSTREAM_14BE )
            framebuf[2] = 0xe8;

          outFile.write(framebuf, framebuf_data);
        }
      }
    }

    if ( conv )
      std::cerr << "Frame: " << frames << "\r";
    else
      std::cerr << "Skipping: " << frames << "\r";
  }

  std::cout << "Frames found: " << frames << "\n" << std::endl;

  delete[] framebuf;
  delete[] buf;
  return 0;
}

void printInfo(StreamBuffer &sb, int bs_type)
{
  std::cout << sb.getStreamInfo() << std::endl;

  SpdifFrameParser spdifFrameParser(false);
  HeaderInfo hi = sb.getHeaderInfo();

  bool isSpdif(hi.spk.format == FORMAT_SPDIF);

  if ( isSpdif )
  {
    if ( spdifFrameParser.parseFrame(sb.getFrame(), sb.getFrameSize()) )
      hi = spdifFrameParser.getHeaderInfo();
    else
      std::cout << "\nERROR!!!\nCannot parse SPDIF frame" << std::endl;
  }

  if ( is14Bit(bs_type) && hi.spk.format != FORMAT_DTS )
  {
    std::cout << "\nWARNING!!!\n"
            << hi.spk.getFormatText() << " does not support 14bit stream format!\n"
"It will be converted to byte stream.\n" << std::endl;
  }
}

// vim: ts=2 sts=2 et

