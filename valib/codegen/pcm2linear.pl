use strict;

my @chs      = (1..6);
my @formats  = qw(FORMAT_PCM16 FORMAT_PCM24 FORMAT_PCM32 FORMAT_PCM16_BE FORMAT_PCM24_BE FORMAT_PCM32_BE FORMAT_PCMFLOAT FORMAT_PCMDOUBLE FORMAT_LPCM20 FORMAT_LPCM24);
my @names    = qw(pcm16    pcm24    pcm32    pcm16_be pcm24_be pcm32_be pcmfloat pcmdouble lpcm20  lpcm24  );
my @types    = qw(int16_t  int24_t  int32_t  int16_t  int24_t  int32_t  float    double    int16_t int16_t );
my @sample_size = (2, 3, 4, 2, 3, 4, 4, 8, 5, 6);
my @pcm2lin  =
(
'*dst[$ch] = i2s(le2int16(src[$ch])); dst[$ch]++;',
'*dst[$ch] = i2s(le2int24(src[$ch])); dst[$ch]++;',
'*dst[$ch] = i2s(le2int32(src[$ch])); dst[$ch]++;',

'*dst[$ch] = i2s(be2int16(src[$ch])); dst[$ch]++;',
'*dst[$ch] = i2s(be2int24(src[$ch])); dst[$ch]++;',
'*dst[$ch] = i2s(be2int32(src[$ch])); dst[$ch]++;',

'*dst[$ch] = sample_t(src[$ch]); dst[$ch]++;',
'*dst[$ch] = sample_t(src[$ch]); dst[$ch]++;',

'dst[$ch][0] = i2s(int32_t(be2int16(src[$ch+$nch*0])) << 4); '.
'dst[$ch][1] = i2s(int32_t(be2int16(src[$ch+$nch*1])) << 4); '.
'dst[$ch]+=2;',

'dst[$ch][0] = i2s(int32_t(be2int16(src[$ch+$nch*0]) << 8) | rawdata[$nch*4+$ch+$nch*0]); '.
'dst[$ch][1] = i2s(int32_t(be2int16(src[$ch+$nch*1]) << 8) | rawdata[$nch*4+$ch+$nch*1]); '.
'dst[$ch]+=2;',
);


###############################################################################
# load template

my @templ;
open TEMPL, "<pcm2linear.template";
@templ = <TEMPL>;
close TEMPL;

###############################################################################
# array of functions                     &

print "typedef void (Converter::*convert_t)(uint8_t *rawdata, samples_t samples, size_t size);\n";
print "static const int pcm2linear_formats[] = { ".join(", ", @formats)." };\n\n";

print "static const convert_t pcm2linear_tbl[NCHANNELS][".($#formats+1)."] = {\n";
foreach my $nch (@chs)
{
  print " { ";
  print join ", ", map { "${_}_linear_${nch}ch" } @names;
  print " },\n";
}
print "};\n\n";

###############################################################################
# function implementation

foreach my $nch (@chs)
{
  for (my $i = 0; $i <= $#formats; $i++)
  {
    my $format = $formats[$i];
    my $name = $names[$i];
    my $type = $types[$i];
    my $sample_size = $sample_size[$i] * $nch;

    foreach my $templ (@templ)
    {
      if (substr($templ, 0, 1) eq '*')
      {
        for (my $ch = 0; $ch < $nch; $ch++)
        {
          my $convert = $pcm2lin[$i];
          $convert =~ s/(\$\w+)/$1/gee;
          $convert =~ s/(\${\w+})/$1/gee;
          my $ch_templ = substr($templ, 1);
          $ch_templ =~ s/(\$\w+)/$1/gee;
          $ch_templ =~ s/(\${\w+})/$1/gee;
          print $ch_templ;
        }
      }
      else
      {
        my $tmp_templ = $templ;
        $tmp_templ =~ s/(\$\w+)/$1/gee;
        $tmp_templ =~ s/(\${\w+})/$1/gee;
        print $tmp_templ;
      }
    }
  }
}
