#pragma once
#ifndef VALIB_FILTER_GRAPH_H
#define VALIB_FILTER_GRAPH_H
/*
  FilterGraph - base class for other aggregate filters. Implements logic of
    dynamic filter chain rebuilding.

  FilterChain - an easy way to combine a set of filters into one filter. Useful
    for applications to build own chains:

    // Source, sink and filters
    SomeSource src;
    SomeSink sink;

    Filter1 f1;
    Filter2 f2;
    Filter3 f3;

    // Build chain
    FilterChain chain;
    chain.add_back(&f1, "f1");
    chain.add_back(&f2, "f2");
    chain.add_back(&f3, "f3");

    // Setup chain
    chain.set_input(src.get_output());

    // Print chain
    char chain_text[255];
    chain.chain_text(chain_text, sizeof(chain_text));
    printf("Transform chain: %s", chain_text);

    // Transform data with chain of filters
    chain.transform(src, sink);
*/

#include <AudioFilter/Filter.h>

/* statics here is wrong,
 * change it when you get a chance
 */
static const int graph_nodes(32);
static const int node_start(graph_nodes);
static const int node_end(graph_nodes + 1);
static const int node_err(-1);

namespace AudioFilter {

class FilterGraph : public Filter
{
public:
  FilterGraph(int _format_mask);
  virtual ~FilterGraph();

  /////////////////////////////////////////////////////////
  // Print chain
  //
  // int chain_text(char *buf, size_t buf_size)
  // buf - output buffer
  // buf_size - output buffer size
  // returns number of printed bytes

  size_t chainText(char *buf, size_t buf_size) const;

  /////////////////////////////////////////////////////////
  // Filter interface

  virtual void reset(void);

  virtual bool isOfdd(void) const;
  virtual bool queryInput(Speakers spk) const;
  virtual bool setInput(Speakers spk);
  virtual Speakers getInput(void) const;

  virtual bool process(const Chunk *chunk);
  virtual Speakers getOutput(void) const;
  virtual bool isEmpty(void) const;
  virtual bool getChunk(Chunk *chunk);

protected:
  /////////////////////////////////////////////////////////
  // public chain operations

  inline int chainNext(int node) const
  {
    return next[node];
  }

  inline int chainPrev(int node) const
  {
    return prev[node];
  }

  void dropChain(void);
  void setDirty(int node);
  void invalidateChain(void);
  void rebuildNode(int node);

  /////////////////////////////////////////////////////////
  // Overridable functions (placeholders)

  virtual const char *getName(int node) const
  {
    // This function should return the name of the node filter
    return 0;
  }

  virtual Filter *initFilter(int node, Speakers spk)
  {
    // This function must initialize node filter and return pointer to it
    return 0;
  }

  virtual int getNext(int node, Speakers spk) const
  {
    // This function determines what node is next to given node
    // * It must return node_end after the last graph node
    // * It must return node_err when it detects graph error. In this case
    //   processing cannot continue and will be stopped.
    // * To determine the first node in the graph following call is used:
    //     get_next(node_start, spk)
    // * It cannot return node_start
    // * If get_next() returns correct node value it must guarantee that
    //   format given to get_next() is supported by node filter.
    // * get_next(spk, node_start) is used in query_input() to determine
    //   format support. If get_next() returns correct node value it
    //   means that graph can handle this format. Therefore get_next()
    //   should always call query_input() on the first filter.

    return node_end;
  }

private:
  bool ofdd; // we have ofdd filter in the chain flag

  /////////////////////////////////////////////////////////
  // Two-way filter chain structure
  //
  // next[]       - points to the next node
  // prev[]       - points to the previous node
  // filter[]     - node filter
  // node_state[] - node state
  //
  // Last 2 elements contain special start and end nodes
  // (just null filters used as fixed chain start and end).
  //
  // node_start index constant points to the start-node
  // node_end index constant points to the end-node
  //
  // Constant values stored at start and the end nodes:
  //   prev[node_start] = node_end;
  //   next[node_end] = node_start;
  //   filter[node_start] = &start;
  //   filter[node_end] = &end;
  //
  // Node state is used for chain rebuilding:
  //   ns_ok      - nothing to do with this node
  //   ns_dirty   - we must check get_next() for this node
  //   ns_flush   - we must flush this node
  //   ns_rebuild - node is flushed and ready to be rebuilt
  //
  // Note that states are listed in order of 'importance'.
  // So we may safely change node state from ns_ok to
  // ns_dirty or ns_flush but should not change ns_rebuild
  // to ns_dirty.
  //
  // ns_rebuild is the last state in the node's life. Node
  // with this state is destroyed at next process_internal()
  // call (and may be possibly re-created after).
  //
  // Typical node state change scenarios:
  //
  // * ns_ok -> ns_dirty -> process_internal() -> ns_ok
  // * ns_ok -> ns_dirty -> process_internal() -> ns_flush ->
  //   -> process_internal() -> ns_rebuild
  // * ns_ok -> ns_flush -> process_internal() -> ns_rebuild
  //
  // Only following functions can modify the chain:
  //   add_node()
  //   drop_chain()
  //   build_chain() uses add_node and do not change chain
  //     directly
  //   process_internal() and rebuild() functions can
  //     change node_state[]

  NullFilter start;
  NullFilter end;

  int next[graph_nodes + 2];
  int prev[graph_nodes + 2];
  Filter *filter[graph_nodes + 2];
  enum { ns_ok, ns_dirty, ns_flush, ns_rebuild } node_state[graph_nodes + 2];

  /////////////////////////////////////////////////////////
  // Chain operations

  bool buildChain(int node);
  bool addNode(int node, Speakers spk);

  /////////////////////////////////////////////////////////
  // Chain data flow

  bool processInternal(bool rebuild);
};

class FilterChain : public FilterGraph
{
public:
  FilterChain(int _format_mask = -1);
  virtual ~FilterChain();

  /////////////////////////////////////////////////////////
  // FilterChain interface

  bool addFront(Filter *_filter, const char *_desc);
  bool addBack(Filter *_filter, const char *_desc);

  void drop(void);

protected:
  Filter *chain[graph_nodes];
  char   *desc[graph_nodes];
  int     chain_size;

  /////////////////////////////////////////////////////////
  // FilterGraph overrides

  const char *getName(int _node) const;
  Filter *initFilter(int _node, Speakers _spk);
  int getNext(int _node, Speakers _spk) const;
};

}; // namespace AudioFilter

#endif

// vim: ts=2 sts=2 et

