#ifndef A52_PARSER_H

#include "parser.h"
#include "a52.h"

class A52Parser : public FrameParser
{
public:
  a52_state_t *a52_state;
  Speakers  spk;
  SampleBuf samples;

public:
  int frames;
  int errors;

  A52Parser();
  ~A52Parser();

  /////////////////////////////////////////////////////////
  // FrameParser overrides

  virtual const HeaderParser *header_parser() const;

  virtual void reset();
  virtual bool parse_frame(uint8_t *frame, size_t size);

  virtual Speakers  get_spk()      const { return spk;     }
  virtual samples_t get_samples()  const { return samples; }
  virtual size_t    get_nsamples() const { return 1536;    }
  virtual uint8_t  *get_rawdata()  const { return 0;       }
  virtual size_t    get_rawsize()  const { return 0;       }

  virtual size_t stream_info(char *buf, size_t size) const;
  virtual size_t frame_info(char *buf, size_t size) const;
};

#endif
