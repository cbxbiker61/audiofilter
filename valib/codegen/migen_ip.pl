use strict;


my $nch = 6;
my $in_nch,;
my $in_ch;
my $out_nch;
my $out_ch;

###########################################################
# function definitions

foreach $in_nch (1..$nch)
{
  foreach $out_nch (1..$nch)
  {
    print "void ip_mix$in_nch$out_nch(samples_t samples, size_t nsamples);\n";
  }
}
print "\n";

###########################################################
# lookup table

print "typedef void (Mixer::*ip_mixfunc_t)(samples_t, size_t); // in-place mixing\n\n";

print "static const ip_mixfunc_t ip_mix_tbl[NCHANNELS][NCHANNELS] = {\n";

foreach $in_nch (1..$nch)
{
  print "  { ";
  foreach $out_nch (1..$nch-1)
  {
    print "&Mixer::ip_mix$in_nch$out_nch, ";
  }
  print "&Mixer::ip_mix$in_nch$nch },\n";
}
print "};\n";
print "\n";

###########################################################
# mix functions

foreach $in_nch (1..$nch)
{
  foreach $out_nch (1..$nch)
  {
    print "void Mixer::ip_mix$in_nch$out_nch(samples_t samples, size_t nsamples)\n";
    print "{\n";
    print "  sample_t buf[$out_nch];\n";
    print "  for (size_t s = 0; s < nsamples; s++)\n";
    print "  {\n";

    for ($out_ch = 0; $out_ch < $out_nch; $out_ch++)
    {
      print "    buf[$out_ch]  = samples[0][s] * m[0][$out_ch];\n";
      for ($in_ch = 1; $in_ch < $in_nch; $in_ch++)
      {
        print "    buf[$out_ch] += samples[$in_ch][s] * m[$in_ch][$out_ch];\n";
      }
    }

    for ($out_ch = 0; $out_ch < $out_nch; $out_ch++)
    {
      print "    samples[$out_ch][s] = buf[$out_ch];\n";
    }
    print "  }\n";
    print "}\n\n";
  }
}
