This directory contains audio processing filters (Filter class descendants)

Top-level filters are:
  DecoderGraph: simple audio decoder and processor
  DVDDecoder: audio decoder and processor with SPDIF support
  AudioDecoder: universal audio decoder
  AudioProcessor: all-in-one audio processor

All filters:
* AGC (agc.h): Auto gain control filter. Gain, overflow protection, clipping, 
  dynamic range compression, one-pass normalization.

* BassRedir (bass_redir.h): Bass redirection filter. Copies all basses
  to subwoofer channel

* Convert (convert.h): conversions between PCM audio formats and Linear 
  (separated channels) format.

* AudioDecoder (decoder.h): unversal audio decoder filter. Compresed stream
  at input linear or spdif at output.

* Dejitter (dejitter.h): Removes input timestamps jitter

* Delay (delay.h): per-channel audio delay. Supports different units:
  samples, ms, m, cm, in, ft.

* Demux (demux.h): demuxes container streams

* DVDDecoder (dvd_decoder.h): Complete audio decoder and processor.
  Now supports AC3, MPEGAudio and MPEG1/2 PES with AC3, MPEG Audio and LPCM
  streams. SPDIF passthough ability.

* FilterChain (filter_chain.h): represents filter sequence as one filter

* Levels (levels.h): Report about current audio levels. Supports levels 
  caching to sycronize levels display with audio output.

* Mixer (mixer.h): matrix mixer. Multiply input samples by conversion matrix.
  Allows to change number of channels. Automatically calculates matrices for
  common transforms.

* AudioProcessor (proc.h): audio processor. input/output format conversions,
  AGC, Mixer, Delay, input/output levels, channel reorder.

* Spdifer (spdifer.h): Encapsulates compressed stream in SPDIF 
  according to IEC 61937



Raw data
========
                NF R QSGP OEC  IP FL BUF
Convert         +  + ++-+ +-+  -  +  im  
Counter         +  + +--+ ---  +  -  ip
Decoder         +  + ++-+ +++  -  -  bl
Demux           +  + +--+ +--  +  -  ip
Spdifer         +  + +--- +-+  -  -  bl

Processing
==========
                NF R QSGP OEC  IP FL BUF
AGC             +  + ---- --+  -  +  bl
Mixer           +  - -+-- +-+  *  -  ip/im
Delay           +  + ---+ ---  +  -  ip
BassRedir       +  + -+-+ ---  +  -  ip
Levels          +  + ---- --+  +  -  ip
Dejitter        +  + ---+ --+  +  -  ip

Aggregates
==========
                NF R QSGP OEC  IP FL BUF
AudioProcessor  -  + ++++ +++         -
FilterChain     +  + ++++ +++         -
DVDDecoder      -  + ++++ +++         -



NF - NullFilter descendant

R - reset()

Q - query_input()
S - set_input()
G - get_input()
P - process()

O - get_output()
E - is_empty()
C - get_chunk()

FL  - require flushing

BUF - buffering method:
  ip - inplace
  im - immediate
  bl - block
