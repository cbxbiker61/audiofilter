#include <AudioFilter/LinearFilter.h>

namespace {

const int FLUSH_NONE(0);
const int FLUSH_EOS(1);
const int FLUSH_REINIT(2);

}; // anonymous namespace

namespace AudioFilter {

LinearFilter::LinearFilter()
  : size(0)
  , out_size(0)
  , buffered_samples(0)
  , flushing(FLUSH_NONE)
{}

LinearFilter::~LinearFilter()
{}

///////////////////////////////////////////////////////////////////////////////
// Filter interface
///////////////////////////////////////////////////////////////////////////////

void LinearFilter::reset(void)
{
  if ( flushing & FLUSH_REINIT )
  {
    out_spk = in_spk;
    init(in_spk, out_spk);

    // Timing won't work correctly with SRC, because it's
    // hard to track the internal buffer size of the
    // filter in this case.
    assert(in_spk.getSampleRate() == out_spk.getSampleRate());
  }
  flushing = FLUSH_NONE;
  resetState();

  samples.zero();
  size = 0;
  out_samples.zero();
  out_size = 0;
  buffered_samples = 0;
  sync_helper.reset();
}

bool LinearFilter::isOfdd(void) const
{
  return false;
}

bool LinearFilter::queryInput(Speakers spk) const
{
  if ( ! spk.isLinear() || ! spk.getSampleRate() || ! spk.getMask() )
    return false;

  return query(spk);
}

bool LinearFilter::setInput(Speakers spk)
{
  if ( ! queryInput(spk) )
  {
    reset();
    return false;
  }

  in_spk = spk;
  out_spk = spk;

  if ( ! init(spk, out_spk) )
    return false;

  assert(in_spk.getSampleRate() == out_spk.getSampleRate());
  reset();
  return true;
}

Speakers LinearFilter::getInput(void) const
{
  return in_spk;
}

bool LinearFilter::process(const Chunk *chunk)
{
  if ( ! chunk->isDummy() )
  {
    if ( in_spk != chunk->spk && ! setInput(chunk->spk) )
      return false;

    if ( chunk->sync )
      sync(chunk->time);

    sync_helper.receiveSync(chunk, buffered_samples);
    samples = chunk->samples;
    size = chunk->size;

    if ( chunk->eos )
      flushing |= FLUSH_EOS;

    out_size = 0;
    out_samples.zero();
    return process();
  }

  return true;
}

Speakers LinearFilter::getOutput(void) const
{
  return out_spk;
}

bool LinearFilter::isEmpty(void) const
{
  if ( size > 0 || out_size > 0 || flushing != FLUSH_NONE )
    return false;

  return true;
}

bool LinearFilter::getChunk(Chunk *chunk)
{
  if ( size > 0 && out_size == 0 && ! process() )
    return false;

  if ( out_size )
  {
    chunk->setLinear(out_spk, out_samples, out_size);
    sync_helper.sendSync(chunk, 1.0 / in_spk.getSampleRate());
    sync_helper.drop(out_size);
    out_size = 0;
    return true;
  }

  if ( flushing != FLUSH_NONE && needFlushing() )
  {
    if ( ! flush() )
      return false;

    chunk->setLinear(out_spk, out_samples, out_size);
    sync_helper.sendSync(chunk, 1.0 / in_spk.getSampleRate());
    sync_helper.drop(out_size);
    out_size = 0;
    return true;
  }

  chunk->setEmpty(out_spk);

  if ( flushing != FLUSH_NONE )
  {
    assert(buffered_samples == 0); // incorrect number of samples flushed

    if ( flushing & FLUSH_EOS )
    {
      chunk->setEos();
      reset();
    }
    else
    {
      Speakers old_out_spk = out_spk;
      out_spk = in_spk;
      init(in_spk, out_spk);
      resetState();
      buffered_samples = 0;

      assert(old_out_spk == out_spk); // format change is not allowed
      assert(in_spk.getSampleRate() == out_spk.getSampleRate()); // sample rate conversion is not allowed
    }

    flushing = FLUSH_NONE;
  }

  return true;
}

bool LinearFilter::process(void)
{
  out_size = 0;

  while ( size > 0 && out_size == 0 )
  {
    size_t gone = 0;

    if ( ! processSamples(samples, size, out_samples, out_size, gone) )
      return false;

    // detect endless loop
    assert(gone > 0 || out_size > 0);

    if ( gone > size )
    {
      // incorrect number of samples processed
      assert(false);
      gone = size;
    }

    size -= gone;
    samples += gone;

    buffered_samples += gone;

    if ( buffered_samples < out_size )
    {
      // incorrect number of output samples
      assert(false);
      buffered_samples = out_size;
    }

    buffered_samples -= out_size;
  }

  return true;
}

bool LinearFilter::flush(void)
{
  out_size = 0;

  while ( out_size == 0 && needFlushing() )
  {
    if ( ! flush(out_samples, out_size) )
      return false;

    // detect endless loop
    assert( out_size > 0 || ! needFlushing() );

    if ( buffered_samples > out_size )
    {
      // incorrect number of samples flushed
      assert(false);
      buffered_samples = out_size;
    }

    buffered_samples -= out_size;
  }

  return true;
}

bool LinearFilter::reinit(bool format_change)
{
  if ( ! needFlushing() && ! format_change )
  {
    assert(buffered_samples == 0); // we should flush if we have buffered samples

    Speakers old_out_spk = out_spk;
    out_spk = in_spk;
    init(in_spk, out_spk);
    resetState();
    buffered_samples = 0;

    assert(old_out_spk == out_spk); // format change is not allowed
    assert(in_spk.getSampleRate() == out_spk.getSampleRate()); // sample rate conversion is not allowed
  }
  else if ( format_change )
    flushing |= FLUSH_EOS;
  else
    flushing |= FLUSH_REINIT;

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// LinearFilter interface
///////////////////////////////////////////////////////////////////////////////

bool LinearFilter::query(Speakers spk) const
{
  return true;
}

bool LinearFilter::init(Speakers spk, Speakers &out_spk)
{
  out_spk = spk;
  return true;
}

void LinearFilter::resetState(void)
{}

void LinearFilter::sync(vtime_t time)
{}

bool LinearFilter::processSamples(samples_t in, size_t in_size
  , samples_t &out, size_t &out_size, size_t &gone)
{
  out = in;
  out_size = in_size;
  gone = in_size;

  return processInplace(in, in_size);
}

bool LinearFilter::processInplace(samples_t in, size_t in_size)
{
  return true;
}

bool LinearFilter::flush(samples_t &out, size_t &out_size)
{
  out.zero();
  out_size = 0;
  return true;
}

bool LinearFilter::needFlushing(void) const
{
  return false;
}

}; // namespace AudioFilter

// vim: ts=2 sts=2 et

