#include <math.h>
#include "ac3_defs.h"
#include "ac3_mdct.h"

#define N (AC3_BLOCK_SAMPLES * 2)
#define MDCT_NBITS 9


inline int16_t fix15(double a)
{
  int v;
  v = (int)(a * (double)(1 << 15));
  if (v < -32767)
    v = -32767;
  else if (v > 32767) 
    v = 32767;
  return v;
}


MDCT::MDCT(int ln)
{
  int i, j, m, n;
  float alpha;
  
  n = 1 << ln;
  
  for(i=0;i<(n/2);i++) 
  {
    alpha = 2 * (float)M_PI * (float)i / (float)n;
    costab[i] = fix15(cos(alpha));
    sintab[i] = fix15(sin(alpha));
  }
  
  for(i = 0; i < n; i++) 
  {
    m = 0;
    for(j = 0; j < ln; j++) 
      m |= ((i >> j) & 1) << (ln-j-1);

    fft_rev[i] = m;
  }

  //
  for(i = 0; i < N/4; i++) 
  {
    alpha = 2 * (float)M_PI * (i + 1.0f / 8.0f) / (float)N;
    xcos1[i] = fix15(-cos(alpha));
    xsin1[i] = fix15(-sin(alpha));
  }
}
  

/* butter fly op */
#define BF(pre, pim, qre, qim, pre1, pim1, qre1, qim1) \
{\
  int ax, ay, bx, by;\
  bx=pre1;\
  by=pim1;\
  ax=qre1;\
  ay=qim1;\
  pre = (bx + ax) >> 1;\
  pim = (by + ay) >> 1;\
  qre = (bx - ax) >> 1;\
  qim = (by - ay) >> 1;\
}

#define MUL16(a,b) ((a) * (b))

#define CMUL(pre, pim, are, aim, bre, bim) \
{\
   pre = (MUL16(are, bre) - MUL16(aim, bim)) >> 15;\
   pim = (MUL16(are, bim) + MUL16(bre, aim)) >> 15;\
}


/* do a 2^n point complex fft on 2^ln points. */
void MDCT::fft(IComplex *z, int ln)
{
  int j, l, np, np2;
  int nblocks, nloops;
  register IComplex *p,*q;
  int tmp_re, tmp_im;
  
  np = 1 << ln;
  
  /* reverse */
  for(j=0;j<np;j++) {
    int k;
    IComplex tmp;
    k = fft_rev[j];
    if (k < j) {
      tmp = z[k];
      z[k] = z[j];
      z[j] = tmp;
    }
  }
  
  /* pass 0 */
  
  p=&z[0];
  j=(np >> 1);
  do {
    BF(p[0].re, p[0].im, p[1].re, p[1].im, 
      p[0].re, p[0].im, p[1].re, p[1].im);
    p+=2;
  } while (--j != 0);
  
  /* pass 1 */
  
  p=&z[0];
  j=np >> 2;
  do {
    BF(p[0].re, p[0].im, p[2].re, p[2].im, 
      p[0].re, p[0].im, p[2].re, p[2].im);
    BF(p[1].re, p[1].im, p[3].re, p[3].im, 
      p[1].re, p[1].im, p[3].im, -p[3].re);
    p+=4;
  } while (--j != 0);
  
  /* pass 2 .. ln-1 */
  
  nblocks = np >> 3;
  nloops = 1 << 2;
  np2 = np >> 1;
  do {
    p = z;
    q = z + nloops;
    for (j = 0; j < nblocks; ++j) {
      
      BF(p->re, p->im, q->re, q->im,
        p->re, p->im, q->re, q->im);
      
      p++;
      q++;
      for(l = nblocks; l < np2; l += nblocks) {
        CMUL(tmp_re, tmp_im, costab[l], -sintab[l], q->re, q->im);
        BF(p->re, p->im, q->re, q->im,
          p->re, p->im, tmp_re, tmp_im);
        p++;
        q++;
      }
      p += nloops;
      q += nloops;
    }
    nblocks = nblocks >> 1;
    nloops = nloops << 1;
  } while (nblocks != 0);
}

/* do a 512 point mdct */
void MDCT::mdct512(int32_t *out, int16_t *in)
{
  int i, re, im, re1, im1;
  int16_t rot[N]; 
  IComplex x[N/4];
  
  /* shift to simplify computations */
  for(i=0;i<N/4;i++)
    rot[i] = -in[i + 3*N/4];
  for(i=N/4;i<N;i++)
    rot[i] = in[i - N/4];
  
  /* pre rotation */
  for(i=0;i<N/4;i++) {
    re = ((int)rot[2*i] - (int)rot[N-1-2*i]) >> 1;
    im = -((int)rot[N/2+2*i] - (int)rot[N/2-1-2*i]) >> 1;
    CMUL(x[i].re, x[i].im, re, im, -xcos1[i], xsin1[i]);
  }
  
  fft(x, MDCT_NBITS - 2);
  
  /* post rotation */
  for(i=0;i<N/4;i++) {
    re = x[i].re;
    im = x[i].im;
    CMUL(re1, im1, re, im, xsin1[i], xcos1[i]);
    out[2*i] = im1;
    out[N/2-1-2*i] = re1;
  }
}
