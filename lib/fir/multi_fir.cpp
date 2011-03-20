#include <string.h>
#include "multi_fir.h"

MultiFIR::MultiFIR()
:list(0), count(0), ver(0), list_ver(0)
{}

MultiFIR::MultiFIR(const FIRGen *const *list_, size_t count_)
:list(0), count(0), ver(0), list_ver(0)
{
  set(list_, count_);
}

MultiFIR::~MultiFIR()
{
  release();
}


void
MultiFIR::set(const FIRGen *const *list_, size_t count_)
{
  release();

  list = new const FIRGen *[count_];
  if (!list) return;

  count = count_;
  for (size_t i = 0; i < count_; i++)
    list[i] = list_[i];
}

void
MultiFIR::release()
{
  safe_delete(list);
  count = 0;
  ver++;
}

int
MultiFIR::version() const
{
  int sum = 0;
  for (size_t i = 0; i < count; i++)
    sum += list[i]->version();

  if (sum != list_ver)
    list_ver = sum, ver++;

  return ver;
}

const FIRInstance *
MultiFIR::make(int sample_rate) const
{
  size_t i;

  if (count == 0) return 0;
  const FIRInstance *result = 0;
  const FIRInstance **fir = new const FIRInstance *[count];
  if (!fir) return 0;

  /////////////////////////////////////////////////////////
  // Build FIR instances

  int length = 1;
  int center = 0;
  size_t fir_count = 0;

  for (i = 0; i < count; i++)
  {
    if (list[i] == 0) continue;
    fir[fir_count] = list[i]->make(sample_rate);
    if (fir[fir_count])
    {
      length += fir[fir_count]->length - 1;
      center += fir[fir_count]->center;
      if (fir[fir_count]->type == firt_zero)
      {
        // no need to think more
        // cleanup and return zero FIR
        for (size_t j = 0; j <= fir_count; j++)
          safe_delete(fir[j]);
        safe_delete(fir);
        return new ZeroFIRInstance(sample_rate);
      }
      fir_count++;
    }
  }

  /////////////////////////////////////////////////////////
  // Convolve each

  if (fir_count == 0)
    result = 0;
  else if (fir_count == 1)
    result = fir[0];
  else if (length == 1)
  {
    // Zero, Gain or Identity response
    double gain = 1.0;
    for (i = 0; i < fir_count; i++)
      gain *= fir[i]->data[0];
    result = new GainFIRInstance(sample_rate, gain);
  }
  else
  {
    // Custom response
    double *data = new double[length];
    if (data)
    {
      int current_length = fir[0]->length;
      memset(data, 0, length * sizeof(double));
      memcpy(data, fir[0]->data, fir[0]->length * sizeof(fir[0]->data[0]));

      for (i = 1; i < fir_count; i++)
      {
        int n = fir[i]->length - 1;
        for (int j = current_length + n - 1; j >= 0; j--)
        {
          double sum = 0;
          if (j >= n)
            for (int k = 0; k <= n; k++)
              sum += data[j - n + k] * fir[i]->data[n - k];
          else
            for (int k = n - j; k <= n; k++)
              sum += data[j - n + k] * fir[i]->data[n - k];
          data[j] = sum;
        }
        current_length += n - 1;
      }
      result = new DynamicFIRInstance(sample_rate, firt_custom, length, center, data);
    }
  }

  /////////////////////////////////////////////////////////
  // Cleanup and return result

  if (fir_count > 1)
    for (i = 0; i < fir_count; i++)
      safe_delete(fir[i]);
  safe_delete(fir);

  return result;
}
