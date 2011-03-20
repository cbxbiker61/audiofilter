use strict;

my ($min,$hour,$mday,$mon,$year) = (localtime)[1, 2, 3, 4, 5];
print "-------------------------------------------------------------------------------\n";
printf "Date:\t%04i/%02i/%02i %02i:%02i\n", $year+1900, $mon, $mday, $hour, $min;

my @system = `systeminfo`;
my $type;
my $value;

foreach (@system)
{
  $type = substr($_, 0, 27) if substr($_, 0, 1) ne ' ';
  $value = substr($_, 27);

  print "Host:\t$value" if $type eq "Host Name:                 ";
  print "OS:\t$value"   if $type eq "OS Name:                   ";
  print "CPU:\t$value"  if $type eq "Processor(s):              ";
  print "Mem:\t$value"  if $type eq "Total Physical Memory:     ";

#  print "$type $value";
}

# compilation info

print "\n";
$_ = <>;
while ($_)
{
  $_ = <>;
  chomp;
  print "$_\n" if $_;
}

# find all speed tests

while (<>) 
{
  print m/\* .+/gi, "\n" if (/speed/);
  print m/\* .+/gi, "\n" if (/Total time/);
}
print "\n";
