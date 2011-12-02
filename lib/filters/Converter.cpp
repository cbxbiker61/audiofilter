#include <math.h>
#include "Converter.h"

// todo: PCM-to-PCM conversions

namespace {

const int converter_formats =
FORMAT_MASK_LINEAR
| FORMAT_MASK_PCM16
| FORMAT_MASK_PCM24
| FORMAT_MASK_PCM32
| FORMAT_MASK_PCM16_BE
| FORMAT_MASK_PCM24_BE
| FORMAT_MASK_PCM32_BE
| FORMAT_MASK_PCMFLOAT
| FORMAT_MASK_PCMDOUBLE
| FORMAT_MASK_LPCM20
| FORMAT_MASK_LPCM24;

}; // anonymous namespace

namespace AudioFilter {

void passthrough(uint8_t *, samples_t, size_t) {}

Converter::Converter(size_t _nsamples)
  :NullFilter(0) // use own query_input()
{
  convert = 0;
  format = FORMAT_UNKNOWN;
  memcpy(order, std_order, sizeof(order));
  nsamples = _nsamples;
  out_size = 0;
  part_size = 0;
}

convert_t Converter::findConversion(int _format, Speakers _spk) const
{
  if ( _spk.getFormat() == _format )
    // no conversion required but we have to return conversion
    // function to indicate that we can proceed
    return passthrough;

  if ( _format == FORMAT_LINEAR )
    return find_pcm2linear(_spk.getFormat(), _spk.getChannelCount());
  else if ( _spk.isLinear() )
    return find_linear2pcm(_format, _spk.getChannelCount());

  return 0;
}

bool Converter::initialize(void)
{
  /////////////////////////////////////////////////////////
  // Initialize convertor:
  // * reset filter state
  // * find conversion function
  // * allocate buffer
  //
  // If we cannot find conversion we have to drop current
  // input format to Speakers::UNKNOWN to indicate that we
  // cannot proceed with current setup so forcing appli-
  // cation to call set_input() with new input format.

  /////////////////////////////////////////////////////////
  // reset filter state

  reset();

  /////////////////////////////////////////////////////////
  // check if no conversion required

  if ( spk.isUnknown() )
  {
    // input format is not set
    convert = passthrough;
    return true;
  }

  if ( spk.getFormat() == format )
  {
    // no conversion required
    // no buffer required
    convert = passthrough;
    return true;
  }

  /////////////////////////////////////////////////////////
  // find conversion function

  if ( ! (convert = findConversion(format, spk)) )
  {
    spk = Speakers::UNKNOWN;
    return false;
  }

  /////////////////////////////////////////////////////////
  // allocate buffer

  if ( ! buf.allocate(spk.getChannelCount() * nsamples * getSampleSize(format)) )
  {
    convert = 0;
    spk = Speakers::UNKNOWN;
    return false;
  }

  if ( format == FORMAT_LINEAR )
  {
    // set channel pointers
    out_samples[0] = (sample_t *)buf.data();

    for ( int ch = 1; ch < spk.getChannelCount(); ++ch )
      out_samples[ch] = out_samples[ch-1] + nsamples;

    out_rawdata = 0;
  }
  else
  {
    // set rawdata pointer
    out_rawdata = buf.data();
    out_samples.zero();
  }

  return true;
}

void Converter::convertPcm2linear(void)
{
  const size_t sample_size(spk.getSampleSize() * spk.getChannelCount());

  samples_t dst = out_samples;
  out_size = 0;

  /////////////////////////////////////////////////////////
  // Process part of a sample

  if ( part_size )
  {
    assert(part_size < sample_size);
    size_t delta = sample_size - part_size;

    if ( size < delta )
    {
      // not enough data to fill sample buffer
      memcpy(part_buf + part_size, rawdata, size);
      part_size += size;
      size = 0;
      return;
    }
    else
    {
      // fill the buffer & convert incomplete sample
      memcpy(part_buf + part_size, rawdata, delta);
      dropRawData(delta);
      part_size = 0;

      convert(part_buf, dst, 1);

      dst += 1;
      ++out_size;

      if ( spk.isLpcm() )
      {
        dst += 1;
        ++out_size;
      }
    }
  }

  /////////////////////////////////////////////////////////
  // Determine the number of samples to convert and
  // the size of raw data required for conversion
  // Note: LPCM sample size is doubled because one block
  // contains 2 samples. Also, decoding is done in 2 sample
  // blocks, thus we have to specify less cycles.

  size_t n = nsamples - out_size;

  if ( spk.isLpcm() )
    n /= 2;

  size_t n_size = n * sample_size;

  if ( n_size > size )
  {
    n = size / sample_size;
    n_size = n * sample_size;
  }

  /////////////////////////////////////////////////////////
  // Convert

  convert(rawdata, dst, n);

  dropRawData(n_size);
  out_size += n;

  if ( spk.isLpcm() )
    out_size += n;

  /////////////////////////////////////////////////////////
  // Remember the remaining part of a sample

  if ( size && size < sample_size )
  {
    memcpy(part_buf, rawdata, size);
    part_size = size;
    size = 0;
  }
}

void Converter::convertLinear2pcm(void)
{
  const size_t sample_size(AudioFilter::getSampleSize(format) * spk.getChannelCount());
  size_t n(MIN(size, nsamples));

  convert(out_rawdata, samples, n);

  dropSamples(n);
  out_size = n * sample_size;
}

///////////////////////////////////////////////////////////
// Converter interface

size_t Converter::getBuffer(void) const
{
  return nsamples;
}

bool Converter::setBuffer(size_t _nsamples)
{
  nsamples = _nsamples;
  return initialize();
}

int Converter::getFormat(void) const
{
  return format;
}

bool Converter::setFormat(int _format)
{
  if ( (FORMAT_MASK(_format) & converter_formats) == 0 )
    return false;

  format = _format;
  return initialize();
}

void Converter::getOrder(int _order[NCHANNELS]) const
{
  memcpy(_order, order, sizeof(order));
}

void Converter::setOrder(const int _order[NCHANNELS])
{
  if ( format == FORMAT_LINEAR )
    out_samples.reorder(spk, order, _order);

  memcpy(order, _order, sizeof(order));
}

///////////////////////////////////////////////////////////
// Filter interface

void Converter::reset(void)
{
  NullFilter::reset();
  part_size = 0;
}

bool Converter::queryInput(Speakers _spk) const
{
  if ( (FORMAT_MASK(_spk.getFormat()) & converter_formats) == 0 )
    return false;

  if ( _spk.getChannelCount() == 0 )
    return false;

  if ( ! findConversion(format, _spk) )
    return false;

  return true;
}

bool Converter::setInput(Speakers _spk)
{
  if ( ! NullFilter::setInput(_spk) )
    return false;

  return initialize();
}

Speakers Converter::getOutput(void) const
{
  if ( convert == 0 )
    return Speakers::UNKNOWN;

  Speakers out = spk;
  out.setFormat(format);
  return out;
}

bool Converter::process(const Chunk *_chunk)
{
  if ( ! _chunk->isDummy() ) // ignore dummy chunks
  {
    if ( receiveChunk(_chunk) )
    {
      if ( spk.isLinear() )
        samples.reorderFromStd(spk, order);

      return true;
    }
    else
      return false;
  }

  return true;
}

bool Converter::getChunk(Chunk *_chunk)
{
  const Speakers out_spk = getOutput();

  if ( spk.getFormat() == format )
  {
    sendChunkInplace(_chunk, size);
    return true;
  }

  if ( ! convert )
    return false;

  if ( format == FORMAT_LINEAR )
    convertPcm2linear();
  else
    convertLinear2pcm();

  _chunk->set ( out_spk, out_rawdata, out_samples, out_size
                , sync, time, flushing && ! size);

  if ( out_spk.isLinear() )
    _chunk->samples.reorderToStd(spk, order);

  flushing = flushing && size;
  sync = false;
  time = 0;
  return true;
}

}; // namespace AudioFilter

// vim: ts=2 sts=2 et

