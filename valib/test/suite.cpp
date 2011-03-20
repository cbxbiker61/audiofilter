#include <math.h>
#include "source/raw_source.h"
#include "suite.h"

const TestResult test_passed(true);
const TestResult test_failed(false);

const char *compact_suffix(size_t size)
{
  if (size > 10000000) return "M";
  if (size > 10000) return "K";
  return "";
}

size_t compact_size(size_t size)
{
  if (size > 10000000) return size / 1000000;
  if (size > 10000) return size / 1000;
  return size;
}

int compare(Log *log, Source *src, Source *ref)
{
  // if reference format is FORMAT_RAWDATA then filter output
  // format is supposed to be raw format (not FORMAT_LINEAR)

  // todo: add couters to prevent infinite loop

  Chunk src_chunk;
  Chunk ref_chunk;
  size_t pos = 0;

  Speakers spk;
  size_t len;
  int ch;

  while (1)
  {
    if (!src_chunk.size)
    {
      if (src->is_empty()) break;
      if (!src->get_chunk(&src_chunk)) return log->err("src->get_chunk() fails");
      if (src_chunk.is_dummy()) continue;
    }

    if (!ref_chunk.size)
    {
      if (ref->is_empty()) break;
      if (!ref->get_chunk(&ref_chunk)) return log->err("ref->get_chunk() fails");
      if (ref_chunk.is_dummy()) continue;
    }

    ///////////////////////////////////////////////////////
    // Check that stream configurstions are equal
    // Do not check if output is raw and reference format is FORMAT_RAWDATA 
    // (reference format is unspecified)

    if (ref_chunk.spk != src_chunk.spk)
      if (src_chunk.spk.format == FORMAT_LINEAR || ref_chunk.spk.format != FORMAT_RAWDATA)
        return log->err("Different speaker configurations");

    ///////////////////////////////////////////////////////
    // Compare data

    spk = src_chunk.spk;
    len = MIN(ref_chunk.size, src_chunk.size);
    if (spk.format == FORMAT_LINEAR)
    {
      // compare linear
      for (ch = 0; ch < spk.nch(); ch++)
        if (memcmp(src_chunk.samples[ch], ref_chunk.samples[ch], len * sizeof(sample_t)))
          return log->err("Data differs");
    }
    else
    {
      for (size_t i = 0; i < len; i++)
        if (src_chunk.rawdata[i] != ref_chunk.rawdata[i])
          return log->err("Data differs at pos %i (0x%x), chunk pos %i (0x%x)", pos + i, pos + i, i, i);
    }
    pos += len;
    src_chunk.drop(len);
    ref_chunk.drop(len);

    ///////////////////////////////////////////////////////
    // Statistics

    log->status("Pos: %u%s      ", compact_size(pos), compact_suffix(pos));

  } // while (1)

  /////////////////////////////////////////////////////////
  // Verify stream lengths

  if (!src->is_empty() || src_chunk.size)
    return log->err("output is longer than reference");

  if (!ref->is_empty() || ref_chunk.size)
    return log->err("reference is longer than output");

  return 0;
}

int compare(Log *log, Source *src, Filter *src_filter, Source *ref, Filter *ref_filter)
{
  SourceFilter sf(src, src_filter);
  SourceFilter rf(ref, ref_filter);
  return compare(log, &sf, &rf);
}

int compare_file(Log *log, Speakers spk_src, const char *fn_src, Filter *filter, const char *fn_ref)
{
  RAWSource src;
  RAWSource ref;

  Speakers spk_ref(FORMAT_RAWDATA, 0, 0);

  log->msg("Testing transform %s => %s", fn_src, fn_ref);

  if (!src.open(spk_src, fn_src))
    return log->err("cannot open source file '%s' of format %s %s %ikHz", fn_src, spk_src.format_text(), spk_src.mode_text(), spk_src.sample_rate);

  if (!ref.open(spk_ref, fn_ref))
    return log->err("cannot open reference file '%s'", fn_ref);

  if (!filter->set_input(spk_src))
    return log->err("cannot set filter input format to %s %s %ikHz", spk_src.format_text(), spk_src.mode_text(), spk_src.sample_rate);

  int result = compare(log, &src, filter, &ref);

  // Now we MUST reset the filter to drop 
  // possible dependency on our private source.
  // If compare function detects an error
  // filter may be full and hold reference to
  // buffer allocated by our file source.
  filter->reset();

  return result;
}


///////////////////////////////////////////////////////////////////////////////

sample_t calc_peak(Source *source)
{
  assert(source != 0);

  Chunk chunk;
  sample_t peak = 0.0;
  while (!source->is_empty())
  {
    if (!source->get_chunk(&chunk)) return -1.0;
    if (!chunk.spk.is_linear()) return -1.0;

    for (int ch = 0; ch < chunk.spk.nch(); ch++)
      for (size_t i = 0; i < chunk.size; i++)
        if (peak < fabs(chunk.samples[ch][i]))
          peak = fabs(chunk.samples[ch][i]);
  }

  return peak;
}

sample_t calc_peak(Source *source, Filter *filter)
{
  assert(source != 0);
  SourceFilter source_filter(source, filter);
  return calc_peak(&source_filter);
}

///////////////////////////////////////////////////////////////////////////////

double calc_rms(Source *source)
{
  assert(source != 0);

  Chunk chunk;

  size_t n = 0;
  double sum = 0.0;
  while (!source->is_empty())
  {
    if (!source->get_chunk(&chunk)) return -1.0;
    if (!chunk.spk.is_linear()) return -1.0;

    for (int ch = 0; ch < chunk.spk.nch(); ch++)
      for (size_t i = 0; i < chunk.size; i++)
        sum += chunk.samples[ch][i] * chunk.samples[ch][i];

    n += chunk.size * chunk.spk.nch();
  }

  return n? sqrt(sum / n): 0.0;
}

double calc_rms(Source *source, Filter *filter)
{
  assert(source != 0);
  SourceFilter source_filter(source, filter);
  return calc_rms(&source_filter);
}

///////////////////////////////////////////////////////////////////////////////

sample_t calc_diff(Source *s1, Source *s2)
{
  assert(s1 != 0 && s2 != 0);

  Chunk chunk1;
  Chunk chunk2;

  size_t len;
  sample_t diff = 0.0;
  while (1)
  {
    if (chunk1.size == 0)
    {
      if (s1->is_empty()) break;
      if (!s1->get_chunk(&chunk1)) return -1.0;
      if (chunk1.is_dummy()) continue;
      if (!chunk1.spk.is_linear()) return -1.0;
    }
    if (chunk2.size == 0)
    {
      if (s2->is_empty()) break;
      if (!s2->get_chunk(&chunk2)) return -1.0;
      if (chunk2.is_dummy()) continue;
      if (!chunk2.spk.is_linear()) return -1.0;
    }
    if (chunk1.spk != chunk2.spk)
      return -1.0;

    len = MIN(chunk1.size, chunk2.size);
    for (int ch = 0; ch < chunk1.spk.nch(); ch++)
      for (size_t i = 0; i < len; i++)
        if (diff < fabs(chunk1.samples[ch][i] - chunk2.samples[ch][i]))
          diff = fabs(chunk1.samples[ch][i] - chunk2.samples[ch][i]);

    chunk1.drop(len);
    chunk2.drop(len);
  }

  return diff;
}

sample_t calc_diff(Source *s1, Filter *f1, Source *s2, Filter *f2)
{
  assert(s1 != 0 && s2 != 0);
  SourceFilter sf1(s1, f1);
  SourceFilter sf2(s2, f2);
  return calc_diff(&sf1, &sf2);
}

///////////////////////////////////////////////////////////////////////////////

double calc_rms_diff(Source *s1, Source *s2)
{
  assert(s1 != 0 && s2 != 0);

  Chunk chunk1;
  Chunk chunk2;

  size_t len;
  size_t n = 0;
  double sum = 0.0;
  while (1)
  {
    if (chunk1.size == 0)
    {
      if (s1->is_empty()) break;
      if (!s1->get_chunk(&chunk1)) return -1.0;
      if (chunk1.is_dummy()) continue;
      if (!chunk1.spk.is_linear()) return -1.0;
    }
    if (chunk2.size == 0)
    {
      if (s2->is_empty()) break;
      if (!s2->get_chunk(&chunk2)) return -1.0;
      if (chunk2.is_dummy()) continue;
      if (!chunk2.spk.is_linear()) return -1.0;
    }
    if (chunk1.spk != chunk2.spk)
      return -1.0;

    len = MIN(chunk1.size, chunk2.size);
    for (int ch = 0; ch < chunk1.spk.nch(); ch++)
      for (size_t i = 0; i < len; i++)
        sum += (chunk1.samples[ch][i] - chunk2.samples[ch][i]) * (chunk1.samples[ch][i] - chunk2.samples[ch][i]);

    n += len * chunk1.spk.nch();
    chunk1.drop(len);
    chunk2.drop(len);
  }

  return n? sqrt(sum / n): 0.0;
}

double calc_rms_diff(Source *s1, Filter *f1, Source *s2, Filter *f2)
{
  assert(s1 != 0 && s2 != 0);
  SourceFilter sf1(s1, f1);
  SourceFilter sf2(s2, f2);
  return calc_rms_diff(&sf1, &sf2);
}
