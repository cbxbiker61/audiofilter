#include <AudioFilter/Parser.h>

namespace AudioFilter {

MultiHeaderParser::MultiHeaderParser()
  : parserVec()
  , currentParser(0)
  , iOwnParsers(true)
  , f_header_size(0)
  , f_min_frame_size(0)
  , f_max_frame_size(0)
{
}

MultiHeaderParser::~MultiHeaderParser()
{
  releaseParsers();
}

void MultiHeaderParser::initSizes(void)
{
  zeroSizes();

  for ( size_t i = 0; i < parserVec.size(); ++i )
  {
    const HeaderParser *hp = parserVec[i];

    if ( i == 0 )
    {
      f_header_size = hp->getHeaderSize();
      f_min_frame_size = hp->getMinFrameSize();
      f_max_frame_size = hp->getMaxFrameSize();
    }
    else
    {
      if ( f_header_size < hp->getHeaderSize() )
        f_header_size = hp->getHeaderSize();

      if ( f_min_frame_size > hp->getMinFrameSize() )
        f_min_frame_size = hp->getMinFrameSize();

      if ( f_max_frame_size < hp->getMaxFrameSize() )
        f_max_frame_size = hp->getMaxFrameSize();
    }
  }

  if ( f_min_frame_size < f_header_size )
    f_min_frame_size = f_header_size;
}

void MultiHeaderParser::addParser(HeaderParser *hp)
{
  parserVec.push_back(hp);
  initSizes();
}

void MultiHeaderParser::releaseParsers(void)
{
  currentParser = 0;

  if ( iOwnParsers )
  {
    for ( HeaderParserVec::iterator i = parserVec.begin()
          ; i < parserVec.end(); i++ )
      delete *i;
  }

  parserVec.clear();
  zeroSizes();
}

void MultiHeaderParser::zeroSizes(void)
{
  f_header_size = 0;
  f_min_frame_size = 0;
  f_max_frame_size = 0;
}

bool MultiHeaderParser::canParse(int format)
{
  if ( currentParser && currentParser->canParse(format) )
    return true;

  for ( HeaderParserVec::iterator i = parserVec.begin()
        ; i < parserVec.end(); i++ )
  {
    if ( (*i)->canParse(format) )
    {
      currentParser = *i;
      return true;
    }
  }

  return false;
}

bool MultiHeaderParser::parseHeader(const uint8_t *hdr, HeaderInfo *hinfo)
{
  if ( currentParser && currentParser->parseHeader(hdr, hinfo) )
    return true;

  for ( HeaderParserVec::iterator i = parserVec.begin()
        ; i < parserVec.end(); i++ )
  {
    if ( (*i)->parseHeader(hdr, hinfo) )
    {
      currentParser = *i;
      return true;
    }
  }

  return false;
}

bool MultiHeaderParser::compareHeaders(const uint8_t *hdr1 , const uint8_t *hdr2)
{
  if ( currentParser && currentParser->compareHeaders(hdr1, hdr2) )
    return true;

  for ( HeaderParserVec::iterator i = parserVec.begin()
        ; i < parserVec.end(); i++ )
  {
    if ( (*i)->compareHeaders(hdr1, hdr2) )
    {
      currentParser = *i;
      return true;
    }
  }

  return false;
}

std::string MultiHeaderParser::getHeaderInfo(const uint8_t *hdr)
{
  if ( currentParser && currentParser->parseHeader(hdr) )
    return currentParser->getHeaderInfo(hdr);

  for ( HeaderParserVec::iterator i = parserVec.begin()
        ; i < parserVec.end(); i++ )
  {
    if ( (*i)->parseHeader(hdr) )
    {
      currentParser = *i;
      return (*i)->getHeaderInfo(hdr);
    }
  }

  return "";
}

}; // namespace AudioFilter

// vim: ts=2 sts=2 et

