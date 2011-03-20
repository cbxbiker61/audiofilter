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
    print "void io_mix$in_nch$out_nch(samples_t input, samples_t output, size_t nsamples);\n";
  }
}
print "\n";

###########################################################
# lookup table

print "typedef void (Mixer::*io_mixfunc_t)(samples_t, samples_t, size_t); // input-output mixing\n\n";

print "static const io_mixfunc_t io_mix_tbl[NCHANNELS][NCHANNELS] = {\n";

foreach $in_nch (1..$nch)
{
  print "  { ";
  foreach $out_nch (1..$nch-1)
  {
    print "&Mixer::io_mix$in_nch$out_nch, ";
  }
  print "&Mixer::io_mix$in_nch$nch },\n";
}
print "};\n";
print "\n";

###########################################################
# mix functions

foreach $in_nch (1..$nch)
{
  foreach $out_nch (1..$nch)
  {
    print "void Mixer::io_mix$in_nch$out_nch(samples_t input, samples_t output, size_t nsamples)\n";
    print "{\n";
    print "  sample_t buf[$out_nch];\n";
    print "  for (size_t s = 0; s < nsamples; s++)\n";
    print "  {\n";

    for ($out_ch = 0; $out_ch < $out_nch; $out_ch++)
    {
      print "    buf[$out_ch]  = input[0][s] * m[0][$out_ch];\n";
      for ($in_ch = 1; $in_ch < $in_nch; $in_ch++)
      {
        print "    buf[$out_ch] += input[$in_ch][s] * m[$in_ch][$out_ch];\n";
      }
    }

    for ($out_ch = 0; $out_ch < $out_nch; $out_ch++)
    {
      print "    output[$out_ch][s] = buf[$out_ch];\n";
    }
    print "  }\n";
    print "}\n\n";
  }
}
