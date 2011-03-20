use strict;  
$| = 0;

my $i;
my $n;
my $cnt = 0;
my @prime = (3, 5, 7, 11, 13, 17, 19, 23);

printf "static const int prime[] = {\n";
printf "     1,    2,    3,    5,    7,   11,   13,   17,   19,   23, \n  ";

$n = 29;
while ($n < 4096)
{
  $i = 0; 
  $i++ while ( ($i < $#prime) && ($n % $prime[$i]) );
  if ($i >= $#prime)
  {
    push(@prime, $n);
    printf("%4i, ", $n);
    if ($cnt++ >= 9)
    {
      printf("\n  ");
      $cnt = 0;
    }
  };
  $n = $n + 2;
}

printf "\n}\n";
