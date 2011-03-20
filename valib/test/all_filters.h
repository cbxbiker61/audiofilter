/*
  Raw data
  ========
  Convert
  Counter
  Decoder
  Demux
  Spdifer

  Processing
  ==========
  AGC
  Mixer
  Resample
  Delay
  BassRedir
  Levels
  Dejitter

  Aggregates
  ==========
  AudioProcessor
  FilterChain
  DVDDecoder
*/


// Raw data
#include "filters\convert.h"
#include "filters\counter.h"
#include "filters\decoder.h"
#include "filters\demux.h"
#include "filters\spdifer.h"
#include "filters\detector.h"
#include "parsers\ac3\ac3_enc.h"

// Processing
#include "filters\agc.h"
#include "filters\mixer.h"
#include "filters\delay.h"
#include "filters\bass_redir.h"
#include "filters\levels.h"
#include "filters\dejitter.h"
#include "filters\resample.h"
#include "filters\convolver.h"

// Aggregates
#include "filter_graph.h"
#include "filters\dvd_graph.h"
#include "filters\proc.h"
