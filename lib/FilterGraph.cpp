#include <cstdio>  // snprint
#include <cstring> // strdup
#include "FilterGraph.h"

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

inline static bool isInvalid(int node)
{
  return ( node != node_end )
    && ( node == node_err || node < 0 || node > graph_nodes );
}

namespace AudioFilter {

FilterGraph::FilterGraph(int _format_mask)
  :start(_format_mask)
  , end(-1)
{
  filter[node_start] = &start;
  filter[node_end]   = &end;
  dropChain();
}

FilterGraph::~FilterGraph()
{
}

///////////////////////////////////////////////////////////////////////////////
// Chain operatoins
///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////
// Build filter chain after the specified node
//
// bool build_chain(int node)
// node - existing node after which we must build the chain
// updates nothing (uses add_node() to update the chain)

bool
FilterGraph::buildChain(int node)
{
  Speakers spk;

  while ( node != node_end )
  {
    spk = filter[node]->getOutput();

    if ( ! addNode(node, spk) )
      return false;

    node = next[node];
  }

  return true;
}

/////////////////////////////////////////////////////////
// Drop filter chain and put graph into initial state
//
// void drop_chain()
// updates filter lists (forward and reverse)
// updates input/output formats and ofdd status

void
FilterGraph::dropChain(void)
{
  ofdd = false;

  start.reset();
  end.reset();
  next[node_start] = node_end;
  prev[node_start] = node_end;
  next[node_end] = node_start;
  prev[node_end] = node_start;
  node_state[node_start] = ns_dirty;
  node_state[node_end] = ns_ok;
}

/////////////////////////////////////////////////////////
// Mark nodes dirty
//
// set_dirty() marks node as dirty when get_next() result
// for this node may change. In this case process_internal()
// will call get_next() for this node again and rebuild
// the chain if nessesary.
//
// invalidate_chain() marks all nodes as dirty to re-check
// all chain.
//
// Note that start node can be set dirty and end node
// cannot. It is because get_next() is never called for
// the end node but always called for the start node.

void
FilterGraph::setDirty(int node)
{
  if ( node == node_end || node < 0 || node > graph_nodes )
    return;

  if ( node_state[node] == ns_ok )
    node_state[node] = ns_dirty;
}

void
FilterGraph::invalidateChain(void)
{
  int node = node_start;

  while ( node != node_end )
  {
    if (node_state[node] == ns_ok)
      node_state[node] = ns_dirty;

    node = next[node];
  }
}

/////////////////////////////////////////////////////////
// Mark node to rebuild
//
// It is nessesary when graph parameters change and some
// nodes are required to be reinitialized. In his case
// process_internal() will flush this node (and therefore
// downstream nodes will be flushed too) and then remove
// it and rebuild the chain starting from this node.
//
// Note that we can mark end node to rebuild (in this
// case flushing will be sent to output) but cannot mark
// start node.

void
FilterGraph::rebuildNode(int node)
{
  if ( node == node_start || node < 0 || node > graph_nodes )
    return;

  node_state[node] = ns_flush;
}

/////////////////////////////////////////////////////////
// Add new node into the chain
//
// bool add_node(int node, Speakers spk)
// node - parent node
// spk - input format for a new node
//
// updates filter lists (forward and reverse)
// updates output format and ofdd status

bool
FilterGraph::addNode(int node, Speakers spk)
{
  ///////////////////////////////////////////////////////
  // if ofdd filter is in transition state then drop
  // the rest of the chain and set output format to
  // spk_unknown

  if ( spk == Speakers::UNKNOWN )
  {
    ofdd = true;

    end.setInput(spk);
    next[node] = node_end;
    prev[node_end] = node;
    return true;
  }

  ///////////////////////////////////////////////////////
  // find the next node

  int next_node = getNext(node, spk);

  // runtime protection
  // we may check get_next() result only here because
  // in all other cases wrong get_next() result forces
  // chain to rebuild and we'll get here anyway

  if ( isInvalid(next_node) )
    return false;

  ///////////////////////////////////////////////////////
  // end of the filter chain
  // drop the rest of the chain and
  // set output format

  if ( next_node == node_end )
  {
    end.setInput(spk);
    next[node] = node_end;
    prev[node_end] = node;
    return true;
  }

  ///////////////////////////////////////////////////////
  // build new filter into the filter chain

  // create filter

  filter[next_node] = initFilter(next_node, spk);

  // runtime protection
  // must do it BEFORE updating of filter lists
  // otherwise filter list may be broken in case of failure

  if ( ! filter[next_node] )
    return false;

  // init filter
  // must do it BEFORE updating of filter lists
  // otherwise filter list may be broken in case of failure

  if ( ! filter[next_node]->setInput(spk) )
    return false;

  // update filter lists
  next[node] = next_node;
  prev[next_node] = node;
  next[next_node] = node_end;
  prev[node_end] = next_node;
  node_state[next_node] = ns_ok;

  // update ofdd status
  // aggregate is data-dependent if chain has
  // at least one ofdd filter

  ofdd = false;
  node = next[node_start];

  while ( node != node_end )
  {
    ofdd |= filter[node]->isOfdd();
    node = next[node];
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// Chain data flow
///////////////////////////////////////////////////////////////////////////////

bool
FilterGraph::processInternal(bool rebuild)
{
  Speakers spk;
  Chunk chunk;

  int node = prev[node_end];

  while ( node != node_end )
  {
    /////////////////////////////////////////////////////
    // find full filter

    if ( filter[node]->isEmpty() )
    {
      // we need more data from upstream.
      node = prev[node];
      continue;
    }

    /////////////////////////////////////////////////////
    // filter is full so getOutput() must always
    // report format of next output chunk

    spk = filter[node]->getOutput();

    /////////////////////////////////////////////////////
    // Rebuild the filter chain

    if ( spk != filter[next[node]]->getInput() )
    {
      // Rebuild the chain according to format changes
      // during normal data flow.
      //
      // If format was changed it means that flushing was
      // send before (see format change rules) and we may
      // rebuild the chain right now
      if ( ! buildChain(node) )
        return false;
    }
    else if ( node_state[next[node]] == ns_rebuild )
    {
      // Rebuild the chain after flushing.
      // We have flushed downstream and may rebuild it now.
      if ( ! buildChain(node) )
        return false;
    }
    else if ( rebuild && node_state[node] == ns_dirty )
    {
      // We should rebuild graph according to changes in
      // get_next() call ONLY when we do it from the top
      // of the chain, i.e. 'rebuild' flag should be set
      // in process() and clear in get_chunk() call.
      // (otherwise partially changed graph is possible)
      //
      // If chain changes without format change we must
      // flush downstream before rebuilding the chain.
      if ( next[node] != getNext(node, spk) )
        node_state[next[node]] = ns_flush;
      else
        node_state[node] = ns_ok;
    }

    /////////////////////////////////////////////////////
    // process data downstream

    if ( node_state[next[node]] == ns_flush )
    {
      // flush downstream
      chunk.setEmpty(spk, false, 0, true);

      if ( ! filter[next[node]]->process(&chunk) )
        return false;

      node_state[next[node]] = ns_rebuild;
    }
    else
    {
      // process data
      if ( ! filter[node]->getChunk(&chunk) )
        return false;

      if ( ! filter[next[node]]->process(&chunk) )
        return false;

    }

    node = next[node];
  }

  return true;
}

/////////////////////////////////////////////////////////
// Print chain
//
// int chain_text(char *buf, size_t buf_size)
// buf - output buffer
// buf_size - output buffer size
// returns number of printed bytes

size_t
FilterGraph::chainText(char *buf, size_t buf_size) const
{
  size_t i;
  Speakers spk;

  char *buf_ptr = buf;
  int node = next[node_start];

  spk = filter[node]->getInput();

  if ( spk.getMask() || spk.getSampleRate() )
    i = snprintf(buf_ptr, buf_size, "(%s %s %i)"
                    , spk.getFormatText()
                    , spk.getModeText()
                    , spk.getSampleRate());
  else
    i = snprintf(buf_ptr, buf_size, "(%s)", spk.getFormatText());

  buf_ptr += i;
  buf_size = (buf_size > i)? buf_size - i: 0;

  while ( node != node_end )
  {
    spk = filter[node]->getOutput();

    if ( spk.getMask() || spk.getSampleRate() )
      i = snprintf(buf_ptr, buf_size, " -> %s -> (%s %s %i)"
                    , getName(node)
                    , spk.getFormatText()
                    , spk.getModeText()
                    , spk.getSampleRate());
    else
      i = snprintf(buf_ptr, buf_size, " -> %s -> (%s)"
                    , getName(node)
                    , spk.getFormatText());

    buf_ptr += i;
    buf_size = (buf_size > i)? buf_size - i: 0;

    node = next[node];
  }

  return buf_ptr - buf;
}

///////////////////////////////////////////////////////////////////////////////
// Filter interface
///////////////////////////////////////////////////////////////////////////////

void
FilterGraph::reset(void)
{
  dropChain();
}

bool
FilterGraph::isOfdd(void) const
{
  return ofdd;
}

bool
FilterGraph::queryInput(Speakers _spk) const
{
  // format mask based test
  if ( ! start.queryInput(_spk) )
    return false;

  // getNext() test
  return ! isInvalid(getNext(node_start, _spk));
}

bool
FilterGraph::setInput(Speakers _spk)
{
  dropChain();

  return queryInput(_spk) && start.setInput(_spk) && buildChain(node_start);
}

Speakers
FilterGraph::getInput(void) const
{
  return start.getInput();
};

bool
FilterGraph::process(const Chunk *_chunk)
{
  if ( ! _chunk->isDummy() )
  {
    if ( _chunk->spk != filter[next[node_start]]->getInput() )
    {
      if ( ! setInput(_chunk->spk) )
        return false;
    }

    return start.process(_chunk) && processInternal(true);
  }

  return true;
};

Speakers
FilterGraph::getOutput(void) const
{
  return end.getOutput();
};

bool
FilterGraph::isEmpty(void) const
{
  ///////////////////////////////////////////////////////
  // graph is not empty if there're at least one
  // non-empty filter

  int node = node_end;

  do
  {
    if ( ! filter[node]->isEmpty() )
      return false;

    node = prev[node];
  } while ( node != node_end );

  return true;
}

bool
FilterGraph::getChunk(Chunk *chunk)
{
  ///////////////////////////////////////////////////////
  // if there're something to output from the last filter
  // get it...

  if ( ! end.isEmpty() )
    return end.getChunk(chunk);

  ///////////////////////////////////////////////////////
  // if the last filter is empty then do internal data
  // processing and try to get output afterwards

  if ( ! processInternal(false) )
    return false;

  if ( ! end.isEmpty() )
    return end.getChunk(chunk);

  ///////////////////////////////////////////////////////
  // return dummy chunk

  chunk->setDummy();

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// FilterChain
///////////////////////////////////////////////////////////////////////////////

FilterChain::FilterChain(int _format_mask)
  :FilterGraph(_format_mask)
{
  chain_size = 0;
};

FilterChain::~FilterChain()
{
  drop();
};

///////////////////////////////////////////////////////////////////////////////
// FilterChain interface
///////////////////////////////////////////////////////////////////////////////

bool
FilterChain::addFront(Filter *_filter, const char *_desc)
{
  if ( chain_size < graph_nodes )
  {
    for ( int i = chain_size; i > 0; --i )
    {
      chain[i] = chain[i-1];
      desc[i] = desc[i-1];
    }

    chain[0] = _filter;
    desc[0] = strdup(_desc);
    ++chain_size;
    return true;
  }

  return false;
}

bool
FilterChain::addBack(Filter *_filter, const char *_desc)
{
  if ( chain_size >= graph_nodes )
    return false;

  chain[chain_size] = _filter;
  desc[chain_size] = strdup(_desc);
  ++chain_size;
  return true;
}

void
FilterChain::drop(void)
{
  dropChain();

  for ( int i = 0; i < chain_size; ++i )
  {
    delete desc[i];
    desc[i] = 0;
  }

  chain_size = 0;
}

///////////////////////////////////////////////////////////////////////////////
// FilterGraph overrides
///////////////////////////////////////////////////////////////////////////////

const char *
FilterChain::getName(int _node) const
{
  if ( _node >= chain_size )
    return 0;

  return desc[_node];
}

Filter *
FilterChain::initFilter(int _node, Speakers _spk)
{
  if ( _node >= chain_size )
    return 0;

  return chain[_node];
}

int
FilterChain::getNext(int _node, Speakers _spk) const
{
  if ( _node == node_start )
    return chain_size ? 0 : node_end;

  if ( _node >= chain_size - 1 )
    return node_end;

  if ( chain[_node+1]->queryInput(_spk) )
    return _node + 1;
  else
    return node_err;
}

}; // namespace AudioFilter

// vim: ts=2 sts=2 et

