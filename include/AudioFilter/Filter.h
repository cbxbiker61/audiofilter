#pragma once
#ifndef AUDIOFILTER_FILTER_H
#define AUDIOFILTER_FILTER_H

/*
  Abstract filter interface & base infrastructure

  Interfaces:
  * Chunk - data carrier
  * Sink - audio receiver
  * Source - audio source
  * Filter - audio filter; source and receiver at the same time

  Base classes:
  * NullFilter - filter that does nothing (passthrough filter)
                 base class for most of filters
                 contains infrastructure for descendant filters
  * NullSink - sink that drops all input
  * SourceFilter - combine source and filter as source
  * SinkFilter - combine sink and filter as sink
*/

#include "Speakers.h"

namespace AudioFilter {

class Chunk;
class Sink;
class Source;
class Filter;

class NullSink;
class NullFilter;
class SourceFilter;
class SinkFilter;

///////////////////////////////////////////////////////////////////////////////
// Chunk
//
// This structure is used to transfer audio data and stream events from audio
// source to auido sink (see Source and Sink classes below).
//
// Structure consists of:
// * Data format
// * Audio data
// * Syncronization data
// * End of stream flag
//
// Following stream events possible:
// * Format change
// * Syncronization
// * End of stream
//
// Data format
// ===========
// Chunk always carries data format in 'spk' field.
//
// Audio data
// ==========
// Chunk may carry audio data. There two kinds of data possible: raw data and
// channel samples (linear format). Raw data is a simple continious block of
// binary data. It may be PCM data, encoded data (AC3, DTS) or something else.
// Linear format data is a set of sample buffers for each channel. PCM data in
// raw form is interleaved and may have different sample format. So it's hard
// to work with it. Linear format has linear buffers for each channel and
// fixed sample format (sample_t). Most of internal data processing is done on
// linear format but input and output must be raw. So considering importance
// of both kinds of data the chunk structure may carry both. So 'rawdata'
// field is a pointer to raw buffer and 'samples' field is a set of pointers
// to sample buffers for each channel.
//
// We can find data format with 'spk' field. If it indicates FORMAT_LINEAR
// then 'samples' field must be filled and 'rawdata' field value is undefined.
// Any other format means that 'samples' field is undefined and 'rawdata'
// field has correct pointer. We must not use undefined field.
//
// Any data format has buffer size. In case of raw data it means size of data
// buffer pointed by 'rawdata' pointer in bytes. In case of linear format it
// means number of samples in each channel buffer. All channel buffers have
// the same number of samples.
//
// Also chunk may not contain any data (empty chunk). Such chunks may be used
// to inform downstream about different events without sending data. Empty
// chunk means that both pointers are considered to be invalid and must not be
// used. Empty chunk has 'size' field set to zero (size == 0). There is a
// special function is_empty() to clarify this statement.
//
// Syncronization data
// ===================
// Chunk may contain time stamp that indicates position in the stream. In this
// case it is called syncronization chunk (sync-chunk). 'sync' field indicates
// that we have correct time stamp and 'time' field is this stamp.
//
// (see Sync class at sync.h for more information).
//
// End of stream flag
// ===================
// End-of-stream chunk (eos-chunk) is method to correctly finish the stream.
// eos-chunk may contain data and stream is assumed to end after the last
// byte/sample of this chunk. 'eos' field indicates eos-chunk.
//
// Format change
// =============
// There are 2 ways to change the stream format and start new stream:
// * Forced. Send chunk with new format without sending eos-chunk before or
//   set_input() call without flush() call before. In this case sink should
//   drop all internal buffers immediately without waiting this data to
//   playback and switch to the new format. New data is considered to be not
//   connected with previous. This method provides fastest switching to the
//   new foramt.
// * Flushing. Send eos-chunk with old format before sending chunk with new
//   format or flush() call before set_input(). In this case sink should
//   guarantee that all buffered data is sent to output before receiving new
//   data. Flushing is also used to correctly finish playback. This way
//   guarantees that we don't loose audio tail.
//
// Of course new format should be supported. Unsupported format will lead to
// fail of process() or set_input() call and immediate stop of playback in
// case of forced format change or correct finish of current stream and stop
// in case of flushing.
//
// Syncronization
// ==============
// Audio source may indicate position in the audio stream. All filters must
// pass this informtion correctly. But this may be a hard task because of
// a simple rule: time stamp is applied to the first syncpoint of incoming
// data. Where syncpoint is:
// * each sample for linear format
// * first byte of interleaved PCM sample for PCM format
// * first byte of a packet for packeted format (ac3/dts/mpa/pes, etc)
// * reasonable place for other formats (should be documented)
//
// This rule comes from MPEG where presentation timestamp of PES packet is
// applied to the first syncpoint of wrapped elementary stream. In our case
// filter may receive timestamped chunk of multichannel PCM data beginning
// from half-a-sample. This requires to extend the concept of syncpoint.
//
// End of stream
// =============
// See end of stream flag and format change...

class Chunk
{
public:
  /////////////////////////////////////////////////////////
  // Data

  Speakers  spk;

  uint8_t  *rawdata;
  samples_t samples;
  size_t    size;

  bool      sync;
  vtime_t   time;

  bool      eos;

  /////////////////////////////////////////////////////////
  // Utilities

  Chunk():
    spk(spk_unknown),
    rawdata(0),
    size(0),
    sync(false),
    time(0),
    eos(false)
  {}

  Chunk(Speakers _spk, bool _sync = false,
	vtime_t _time = 0, bool _eos  = false)
  {
    setEmpty(_spk, _sync, _time, _eos);
  }

  Chunk(Speakers _spk, samples_t _samples, size_t _size,
    bool _sync = false, vtime_t _time = 0, bool _eos  = false)
  {
    setLinear(_spk, _samples, _size, _sync, _time, _eos);
  }

  Chunk(Speakers _spk, uint8_t *_rawdata, size_t _size,
    bool _sync = false, vtime_t _time = 0, bool _eos  = false)
  {
    setRawData(_spk, _rawdata, _size, _sync, _time, _eos);
  }

  Chunk(Speakers _spk, uint8_t *_rawdata, samples_t _samples, size_t _size,
    bool _sync = false, vtime_t _time = 0, bool _eos  = false)
  {
    set(_spk, _rawdata, _samples, _size, _sync, _time, _eos);
  }

  inline void setDummy(void)
  {
    spk = spk_unknown;
    rawdata = 0;
    samples.zero();
    size = 0;
    sync = false;
    time = 0;
    eos = false;
  }

  inline void setEmpty(Speakers _spk, bool _sync = false
                        , vtime_t _time = 0, bool _eos  = false)
  {
    spk = _spk;
    rawdata = 0;
    samples.zero();
    size = 0;
    sync = _sync;
    time = _time;
    eos = _eos;
  }

  inline void setLinear(Speakers _spk, samples_t _samples, size_t _size
                        , bool _sync = false, vtime_t _time = 0, bool _eos  = false)
  {
    // channel samples must be used only with linear format
    assert(_spk.format == FORMAT_LINEAR);

    spk = _spk;
    rawdata = 0;
    samples = _samples;
    size = _size;
    sync = _sync;
    time = _time;
    eos = _eos;
  }

  inline void setRawData(Speakers _spk, uint8_t *_rawdata, size_t _size,
    bool _sync = false, vtime_t _time = 0, bool _eos  = false)
  {
    // cannot use raw data with linear format
    assert(_spk.format != FORMAT_LINEAR);

    spk = _spk;
    rawdata = _rawdata;
    samples.zero();
    size = _size;
    sync = _sync;
    time = _time;
    eos = _eos;
  }

  inline void set(Speakers _spk, uint8_t *_rawdata, samples_t _samples, size_t _size,
    bool _sync = false, vtime_t _time = 0, bool _eos  = false)
  {
    // cannot use raw data with linear format
    assert((_spk.format != FORMAT_LINEAR) || (_rawdata == 0));

    spk = _spk;
    rawdata = _rawdata;
    samples = _samples;
    size = _size;
    sync = _sync;
    time = _time;
    eos = _eos;
  }

  inline void setSync(bool _sync, vtime_t _time)
  {
    sync = _sync;
    time = _time;
  }

  inline void setEos(bool _eos = true)
  {
    eos = _eos;
  }

  inline bool isDummy(void) const
  {
    return spk.format == FORMAT_UNKNOWN;
  }

  inline bool isEmpty(void) const
  {
    return size == 0;
  }

  inline void drop(size_t _size)
  {
    if ( _size > size )
      _size = size;

    if ( spk.format == FORMAT_LINEAR )
      samples += _size;
    else
      rawdata += _size;

    size -= _size;
    sync = false;
  }

};

///////////////////////////////////////////////////////////////////////////////
// Sink class
//
// Abstract audio sink.
//
// query_input() [thread-safe, fast]
//   Check if we can change format to proposed. Should work as fast as
//   possible because it may be used to try numerous formats to find
//   acceptable conversion. Also, this function may be called asynchronously
//   from other thread, so it should be thread-safe.
//
//   It is assumed that set_input() succeeds if this function returns true.
//   But it is possible that sink may change its mind because some resources
//   may be locked by other applications. For example imagine that some audio
//   device supports SPDIF output. query_input() may check it and report that
//   resource is free. But before set_input() call some other application
//   locks SPIDF output. Of course set_input() will fail.
//
//   Other scenario. Some sink has option to block some formats. So in
//   following scenario set_input() will fall:
//
//   sink.allow(format1);
//   sink.allow(format2);
//
//   if (sink.query_input(format1)) // ok. can change format to 'format1'
//     sink.set_input(format1);     // ok. format is allowed...
//
//   if (sink.query_input(format2)) // ok. can change format to 'format2'
//   {
//     sink.disallow(format2);      // change our mind about 'format2'
//     sink.set_input(format2);     // always fail!
//   }
//
//   Such scenarios with shared resources, sink options, etc must be
//   documented because it may require special ways to work with such sink.
//
// set_input() [working thread, blocking]
//   Switch format. If sink has buffered data it should immediately drop it,
//   stop playback and prepare to receive new data. Even if we switch format
//   to the same one. I.e. following scenario provides fast switch to the new
//   audio stream of the same format:
//
//   sink.set_input(pcm16); // prepare to output pcm16 data
//   sink.process(chunk1);  // buffer some data and possibly start playback
//   sink.set_input(pcm16); // stop playback and drop remaining data
//   sink.process(chunk2);  // buffer new data and possibly start playback
//
//   It is expected that call succeeds after successful query_input() call
//   (see query_input() for more info).
//
//   It may require some time to change format (initialize output device,
//   allocate buffers, etc). Sound output is not required to be continous
//   and clickless in case of format changes, but it is highly recommended
//   to produce minimum artifacts.
//
// get_input() [thread-safe, fast]
//   Just report current input format. If function reports FORMAT_UNKNOWN it
//   means that sink is not initialized. In all other cases it is supposed
//   that query_input(get_input()) returns true. If chunk format differs from
//   get_input() result it means that format change procedure will be evaluted
//
// process() [working thread, critical path]
//   Receive data chunk. Returns true on success and false otherwise. This
//   function must carry about possible events: format change, syncronization,
//   end of stream.
//
//   For audio renderers this call may block working thread during audio
//   playback.
//
// flush() [working thread, blocking]
//   Flush buffered data. Used to playback tail of an audio stream to correctly
//   finish playback or before format change. After flush() call all internal
//   buffers should be flushed and playback stopped so set_input() call should
//   just prepare to receive new format.
//
//   For audio renderers this call may block working thread during audio
//   playback.

class Sink
{
public:
  virtual ~Sink() {};

  virtual bool queryInput(Speakers spk) const = 0;
  virtual bool setInput(Speakers spk) = 0;
  virtual Speakers getInput(void) const = 0;
  virtual bool process(const Chunk *chunk) = 0;
};

class NullSink : public Sink
{
public:
  NullSink() {}

  virtual bool queryInput(Speakers _spk) const
  {
    return true;
  }

  virtual bool setInput(Speakers _spk)
  {
    spk = _spk;
    return true;
  }

  virtual Speakers getInput(void) const
  {
    return spk;
  }

  virtual bool process(const Chunk *_chunk)
  {
    spk = _chunk->spk;
    return true;
  }

protected:
  Speakers spk;
};

///////////////////////////////////////////////////////////////////////////////
// Source class
//
// Abstract audio source.
//
// get_output() [thread-safe, fast]
//   Report format of next output chunk. Primary purpose of this call is to
//   setup downstream audio sink before processing because format switch may
//   be time-consuming operation. When source is empty (is_empty() == true)
//   it may report any format and may change it. But when source is full
//   (is_empty() == false) it must report format exactly as it will appear
//   at next output chunk. So returned value may change either when
//   is_empty() == true or after get_chunk() call (standard format change).
//
// is_empty() [thread-safe, critical path]
//   Check if we can get some data from this source. Depending on source type
//   it may require some explicit actions to be filled (like filters) or it
//   may be filled asyncronously (like audio capture). May be called
//   asynchronously.
//
// get_chunk() [working thread, critical path]
//   Receive data from the source. Should be called only from working thread.
//   If empty chunk is returned it does not mean that source becomes empty.
//   Check it with is_empty() call. This call may block working thread (?).
//

class Source
{
public:
  virtual ~Source() {}

  virtual Speakers getOutput(void) const = 0;
  virtual bool isEmpty(void) const = 0;
  virtual bool getChunk(Chunk *chunk) = 0;
};

///////////////////////////////////////////////////////////////////////////////
// Filter class
//
// reset() [working thread]
//   Reset filter state to empty, drop all internal buffers and all external
//   references.
//
// set_input() [working thread]
//   If current and new configuration differs then it acts as reset() call,
//   in other case nothing happens.
//
// process() [working thread, critical path]
//   May be used for data processing. Do main processing here if filter
//   produces exactly one output chunk for one input chunk.
//
// get_chunk() [working thread, critical path]
//   May be used for data processing. Do main processing here if filter
//   may produce many output chunks for one input chunk.
//
//
// Other threads may call:
//   query_input()
//   get_output()
//   is_empty()

class Filter: public Sink, public Source
{
public:
  virtual ~Filter() {}

  virtual void reset(void) = 0;
  virtual bool isOfdd(void) const = 0;

  virtual bool queryInput(Speakers spk) const = 0;
  virtual bool setInput(Speakers spk) = 0;
  virtual Speakers getInput(void) const = 0;
  virtual bool process(const Chunk *chunk) = 0;

  virtual Speakers getOutput(void) const = 0;
  virtual bool isEmpty(void) const = 0;
  virtual bool getChunk(Chunk *chunk) = 0;

  inline bool processTo(const Chunk *_chunk, Sink *_sink)
  {
    Chunk chunk;

    while ( ! isEmpty() )
    {
      if ( ! getChunk(&chunk) || ! _sink->process(&chunk) )
        return false;
    }

    if ( ! process(_chunk) )
      return false;

    while ( ! isEmpty() )
    {
      if ( ! getChunk(&chunk) || ! _sink->process(&chunk) )
        return false;
    }

    return true;
  }

  inline bool getFrom(Chunk *_chunk, Source *_source)
  {
    Chunk chunk;

    while ( isEmpty() )
    {
      if ( ! _source->getChunk(&chunk) || ! process(&chunk) )
        return false;
    }

    return getChunk(_chunk);
  }

  inline bool transform(Source *_source, Sink *_sink)
  {
    Chunk chunk;

    while ( isEmpty() )
    {
      if ( ! _source->getChunk(&chunk) || ! process(&chunk) )
        return false;
    }

    if ( ! getChunk(&chunk) )
      return false;

    return _sink->process(&chunk);
  }
};

///////////////////////////////////////////////////////////////////////////////
// NullFilter class
//
// Simple filter implementation that does nothing (just passthrough all data)
// and fulfills all requirements.
//
// NullFilter remembers input data and makes output chunk equal to input data.
// Following funcitons may be used in descendant classes:
//
// receive_chunk()      - remember input chunk data
// send_empty_chunk()   - fill empty output chunk
// send_chunk_buffer()  - fill output chunk
// send_chunk_inplace() - fill output chunk and drop data from input buffer
// drop()               - drop data from input buffer and track time changes
//
// Time tracking:
// ==============
// todo...


class NullFilter : public Filter
{
public:
  NullFilter(int _format_mask)
  {
    spk  = spk_unknown;
    size = 0;
    time = 0;
    sync = false;
    flushing = false;
    format_mask = _format_mask;
  }

  virtual void reset(void)
  {
    size = 0;
    time = 0;
    sync = false;
    flushing = false;
    onReset();
  }

  virtual bool isOfdd(void) const
  {
    return false;
  }

  virtual bool queryInput(Speakers _spk) const
  {
    // channel mask and sample rate must be defined for linear format
    if (_spk.format == FORMAT_LINEAR)
      return (FORMAT_MASK_LINEAR & format_mask) != 0 && _spk.mask != 0 && _spk.sample_rate != 0;
    else
      return (FORMAT_MASK(_spk.format) & format_mask) != 0;
  }

  virtual bool setInput(Speakers _spk)
  {
    reset(); // required because it may be overwritten

    if ( ! queryInput(_spk) || ! onSetInput(_spk) )
      return false;

    spk = _spk;
    return true;
  }

  virtual Speakers getInput(void) const
  {
    return spk;
  }

  virtual bool process(const Chunk *_chunk)
  {
    // we must ignore dummy chunks
    if ( ! _chunk->isDummy() )
    {
      if ( ! receiveChunk(_chunk) || ! onProcess() )
        return false;
    }

    return true;
  }

  virtual Speakers getOutput(void) const
  {
    return spk;
  }

  virtual bool isEmpty(void) const
  {
    // must report false in flushing state
    return ! size && ! flushing;
  }

  virtual bool getChunk(Chunk *_chunk)
  {
    sendChunkInplace(_chunk, size);
    return true;
  }

protected:
  Speakers  spk;

  bool      sync;
  vtime_t   time;
  bool      flushing;

  uint8_t  *rawdata;
  samples_t samples;
  size_t    size;

  int       format_mask;

  virtual void onReset(void) {}

  virtual bool onProcess(void)
  {
    return true;
  }

  virtual bool onSetInput(Speakers _spk)
  {
    return true;
  }

  inline bool receiveChunk(const Chunk *_chunk)
  {
    if ( ! _chunk->isDummy() )
    {
      // format change
      if ( spk != _chunk->spk && ! setInput(_chunk->spk) )
        return false;

      // remember input chunk info
      if ( _chunk->sync ) // ignore non-sync chunks
      {
        sync     = true;
        time     = _chunk->time;
      }

      flushing = _chunk->eos;
      rawdata  = _chunk->rawdata;
      samples  = _chunk->samples;
      size     = _chunk->size;
    }

    return true;
  }

  inline void sendChunkInplace(Chunk *_chunk, size_t _size)
  {
    // fill output chunk & drop data

    if ( _size > size )
      _size = size;

    _chunk->set ( getOutput(), rawdata, samples, _size
                  , sync, time, flushing && (size == _size));

    if ( spk.format == FORMAT_LINEAR )
      samples += _size;
    else
      rawdata += _size;

    size -= _size;
    sync = false;
    flushing = flushing && size;
  }

  inline void dropRawData(size_t _size)
  {
    if ( _size > size )
      _size = size;

    rawdata += _size;
    size -= _size;
  }

  inline void dropSamples(size_t _size)
  {
    if ( _size > size )
      _size = size;

    samples += _size;
    size -= _size;
  }

};

///////////////////////////////////////////////////////////////////////////////
// SourceFilter class
// Combines a source and a filter into one source.

class SourceFilter: public Source
{
public:
  SourceFilter()
    : source(0), filter(0)
  {}

  SourceFilter(Source *_source, Filter *_filter)
    : source(0), filter(0)
  {
    set(_source, _filter);
  }

  bool set(Source *_source, Filter *_filter)
  {
    if ( _source )
    {
      source = _source;
      filter = _filter;
      return true;
    }

    return false;
  }

  void release(void)
  {
    source = 0;
    filter = 0;
  }

  Source *getSource(void) const
  {
    return source;
  }

  Filter *getFilter(void) const
  {
    return filter;
  }

  /////////////////////////////////////////////////////////
  // Source interface

  Speakers getOutput(void) const
  {
    if ( filter )
      return filter->getOutput();
    else if ( source )
      return source->getOutput();

    return spk_unknown;
  }

  bool isEmpty(void) const
  {
    if ( filter && source )
      return filter->isEmpty() && source->isEmpty();
    else if ( filter )
      return filter->isEmpty();
    else if ( source )
      return source->isEmpty();

    return true;
  }

  bool getChunk(Chunk *chunk)
  {
    if ( filter )
      return filter->getFrom(chunk, source);
    else if ( source )
      return source->getChunk(chunk);

    return false;
  }

protected:
  Source *source;
  Filter *filter;

};

///////////////////////////////////////////////////////////////////////////////
// SinkFilter class
// Combines a sink and a filter into one sink.

class SinkFilter : public Sink
{
public:
  SinkFilter()
    : sink(0), filter(0)
  {}

  SinkFilter(Sink *_sink, Filter *_filter)
    : sink(0), filter(0)
  {
    set(_sink, _filter);
  }

  bool set(Sink *_sink, Filter *_filter)
  {
    if ( _sink )
    {
      sink = _sink;
      filter = _filter;
      return true;
    }

    return false;
  }

  void release(void)
  {
    sink = 0;
    filter = 0;
  }

  Sink *getSink(void) const
  {
    return sink;
  }

  Filter *getFilter(void) const
  {
    return filter;
  }

  /////////////////////////////////////////////////////////
  // Sink interface

  bool queryInput(Speakers spk) const
  {
    if ( filter )
      return filter->queryInput(spk);
    else if ( sink )
      return sink->queryInput(spk);

    return false;
  }

  bool setInput(Speakers spk)
  {
    if ( filter )
      return filter->setInput(spk);
    else if ( sink )
      return sink->setInput(spk);

    return false;
  }

  Speakers getInput(void) const
  {
    if ( filter )
      return filter->getInput();
    else if ( sink )
      return sink->getInput();

    return spk_unknown;
  }

  bool process(const Chunk *chunk)
  {
    if ( filter )
      return filter->processTo(chunk, sink);
    else if ( sink )
      return sink->process(chunk);

    return false;
  }

protected:
  Sink   *sink;
  Filter *filter;

};

}; // namespace AudioFilter

#endif

// vim: ts=2 sts=2 et

