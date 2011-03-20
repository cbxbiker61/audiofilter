#pragma once
#ifndef VALIB_FILTER_TESTER_H
#define VALIB_FILTER_TESTER_H
/*
  FilterTester class

  This class is proposed to test correctness of calls to filter interface and
  check that format change rules are fulfilled (see Format change rules).

  This class is a filter wrapper and may be used anywhere instead of the real
  filter. So if some filter has a strange behaviour we may wrap it and check
  for possible problems of filter usage.

  Filter must exist during the tester lifetime.
  Filter is not destroyed with the tester.
*/

#include "filter.h"
#include "log.h"

class FilterTester : public Filter
{
protected:
  Filter *filter;
  Log    *log;

  Speakers spk_input;  // input format
  Speakers spk_output; // output format

  bool     stream;     // filter has started a stream
  bool     flushing;   // filter must flush started stream

  int      streams;    // number of eos chunks found

  void updateFormats(void)
  {
    spk_input = filter->getInput();
    spk_output = filter->getOutput();
  }

  void checkFormats(const char *caller)
  {
    // check input format
    if ( filter->getInput() != spk_input )
      log->err("[k2] %s: input format was illegaly changed", caller);

    spk_input = filter->get_input(); // suppress this error report afterwards

    // check output format
    if ( filter->isOfdd() )
    {
      if ( stream && filter->getOutput() != spk_output )
        log->err("[x] %s: output format was illegaly changed", caller);
    }
    else
    {
      if ( filter->getOutput() != spk_output )
        log->err("[f2] %s: output format was illegaly changed", caller);
    }
    spk_output = filter->getOutput(); // suppress this error report afterwards

    // check unininitialized state
    if ( (filter->getInput() == spk_unknown) && ! filter->isEmpty() )
      log->err("[f5] %s: filter is not empty in uninitialized state", caller);
  }

  void checkReset(const char *caller)
  {
    if ( filter->isOfdd() && (filter->getOutput() != spk_unknown) )
      log->err("[f1] %s: output format did not change to spk_unknown", caller);

    if ( ! filter->isEmpty() )
      log->err("[f5] %s: filter is not empty", caller);

    // todo: check buffered data
  }

public:
  FilterTester()
  {
    filter = 0;
    log = 0;
  }

  FilterTester(Filter *_filter, Log *_log)
  {
    filter = _filter;
    log = _log;
    update_formats();
  }

  void link(Filter *_filter, Log *_log)
  {
    filter = _filter;
    log = _log;
    update_formats();
  }

  void unlink(void)
  {
    filter = 0;
    log = 0;
  }

  void resetStreams(void)
  {
    streams = 0;
  }

  int getStreams(void) const
  {
    return streams;
  }

  void reset(void)
  {
    if ( ! filter )
      return;

    stream = false;
    flushing = false;

    checkFormats("before reset()");

    filter->reset();

    checkReset("after reset()");
    checkFormats("after reset()");
  }

  bool isOfdd(void) const
  {
    if ( ! filter )
      return false;

    return filter->isOfdd();
  }

  /////////////////////////////////////////////////////////
  // Sink interface

  bool queryInput(Speakers _spk) const
  {
    return filter ? filter->queryInput(_spk) : false;
  }

  bool setInput(Speakers _spk)
  {
    if ( ! filter )
      return false;

    stream = false;
    flushing = false;

    checkFormats("before set_input()");

    bool query = filter->queryInput(_spk);
    bool result = filter->setInput(_spk);

    if ( query != result )
      log->err("[k3] set_input(): query_input() lies");

    if ( result )
    {
      // after successful format change filter must
      // update input and output formats
      if ( filter->getInput() != _spk )
        log->err("[k4] set_input(): new format was not set");

      updateFormats();
    }
    else
    {
      // if format change failed input and output must
      // reamin unchanged or require initialization
      if ( filter->getInput() == spk_unknown )
        // filter requires reinit so formats was changed
        updateFormats();
      else
        // formats stay unchanged
        checkFormats("after set_input()");
    }

    // filter must reset in either case
    checkReset("after set_input()");
    return result;
  }

  Speakers getInput() const
  {
    return return filter ? filter->getInput() : spk_unknown;
  }

  bool process(const Chunk *_chunk)
  {
    if ( ! filter )
      return false;

    bool dummy = false;
    bool input_format_change = false;
    bool query = true;

    // check input parameters
    if ( !_chunk )
    {
      log->err("process(): null chunk pointer!!!");
      return false;
    }

    // process() must be called only in empty state
    if ( ! isEmpty() )
    {
      log->err("process() is called in full state");
      return false;
    }

    // detect input format change
    if ( _chunk->isDummy() )
    {
      // dummy chunk
      // filter must be empty after processing
      dummy = true;
    }
    else if ( _chunk->spk != getInput() )
    {
      input_format_change = true;
      query = filter->query_input(_chunk->spk);
    }

    check_formats("before process()");

    bool result = filter->process(_chunk);

    if ( input_format_change )
    {
      stream = false;
      flushing = false;

      if ( query != result )
        log->err("[k3] process(): query_input() lies");

      if ( result )
      {
        // successful format change
        // filter must update input and output formats
        if ( filter->getInput() != _chunk->spk )
          log->err("[k4] process(): new format was not set");

        // if filter have received empty chunk with no data
        // but with format change it must just reset the filter
        // (except flushing chunks because flushing may force
        // the filter to generate flushing)
        if ( _chunk->isEmpty() && ! _chunk->eos )
          check_reset("process()");

        updateFormats();
      }
      else
      {
        // if format change failed input and output must
        // reamin unchanged or require initialization
        if ( filter->getInput() == spk_unknown )
          // filter requires reinit so formats was changed
          updateFormats();
        else
          // formats must stay unchanged
          checkFormats("after set_input()");
      }
    }
    else
    {
      if ( ! result && filter->getInput() == spk_unknown )
        // filter requires reinit so formats was changed
        updateFormats();
      else
        // formats must stay unchanged
        checkFormats("after process()");
    }

    // filter must ignore dummy chunks
    if ( dummy && ! isEmpty() )
      log->err("process(): filter is not empty after dummy chunk processing");

    // filter starts a stream when it fills
    if ( ! isEmpty() )
      stream = true;

    // filter must flush a started stream
    if ( _chunk->eos && stream && ! dummy )
      if ( isEmpty() )
        log->err("process(): filter does not want to end the stream");
      else
        flushing = true;

    return result;
  }

  /////////////////////////////////////////////////////////
  // Source interface

  Speakers getOutput(void) const
  {
    return filter ? filter->getOutput() : spk_unknown;
  }

  bool isEmpty(void) const
  {
    return filter ? filter->isEmpty() : true;
  }

  bool getChunk(Chunk *_chunk)
  {
    if ( ! filter )
      return false;

    // check input parameters
    if ( !_chunk )
    {
      log->err("getChunk(): null chunk pointer!!!");
      return false;
    }

    // getChunk() must be called only in full state
    if ( filter->isEmpty() )
    {
      log->err("getChunk() is called in empty state");
      return false;
    }

    checkFormats("before getChunk()");

    Speakers spk = filter->getOutput();

    if ( ! filter->getChunk(_chunk) )
      return false;

    if ( ! _chunk->isDummy() )
    {
      if ( _chunk->spk != spk )
        log->err("[s1] getChunk(): getOutput() lies");

      // filter has ended a stream
      if ( _chunk->eos )
      {
        stream = false;
        ++streams;
      }

      checkFormats("after getChunk()");

      // filter continues a stream or starts a new one
      if ( ! isEmpty() )
        stream = true;

      // filter must end the stream in flushing mode
      if ( flushing && stream && isEmpty() )
        log->err("getChunk(): filter did not end the stream");
    }
    else
    {
      // we must check formats even after dummy chunks
      checkFormats("after getChunk()");
    }

    return true;
  }
};

#endif

