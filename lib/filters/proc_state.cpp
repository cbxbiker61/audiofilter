#include "proc_state.h"

AudioProcessorState::AudioProcessorState()
{
  eq_master_bands = 0;
  for (int ch = 0; ch < NCHANNELS; ch++)
    eq_bands[ch] = 0;
}

AudioProcessorState::~AudioProcessorState()
{
  safe_delete(eq_master_bands);
  for (int ch = 0; ch < NCHANNELS; ch++)
    safe_delete(eq_bands[ch]);
}
