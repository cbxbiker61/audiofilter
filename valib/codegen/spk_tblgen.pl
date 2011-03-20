# Generates tables to:
# * convert channel mask to number of channels (mask_nch_tbl[] at spk.cpp)
# * convert channel mask to channel order (mask_order_tbl[][] at spk.cpp)

use strict;

my @ch_names = ('CH_L', 'CH_C', 'CH_R', 'CH_SL', 'CH_SR', 'CH_LFE');

sub mask2nch
{
  my $n = shift;
  my $bits = 0;
  while ($n)
  {
    $bits += ($n & 1);
    $n >>= 1;
  }
  return $bits
}

sub mask2order
{
  my $n = shift;
  my @ch = ();

  for (my $i = 0; $i < 6; $i++)
  {
    push(@ch, $ch_names[$i]) if ((1 << $i) & $n);
  }
  return "  { ".join(", ", @ch)." }";
}


print "extern const int mask_nch_tbl[64] = \n";
print "{\n";
for (my $i = 0; $i < 8; $i++)
{
  print "  ";
  for (my $j = 0; $j < 8; $j++)
  {
    print mask2nch($i * 8 + $j), ", ";
  }
  print "\n";
}
print "};\n\n";


print "extern const int mask_order_tbl[64][6] =\n";
print "{\n";
for (my $i = 0; $i < 64; $i++)
{
  print mask2order($i), ",\n";
}
print "};\n";
