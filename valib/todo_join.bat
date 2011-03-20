@rem = '--*-Perl-*--
@echo off
if "%OS%" == "Windows_NT" goto WinNT
perl -x -S "%0" %1 %2 %3 %4 %5 %6 %7 %8 %9
goto endofperl
:WinNT
perl -x -S %0 %*
if NOT "%COMSPEC%" == "%SystemRoot%\system32\cmd.exe" goto endofperl
if %errorlevel% == 9009 echo You do not have Perl in your PATH.
if errorlevel 1 goto script_failed_so_exit_with_non_zero_val 2>nul
goto endofperl
@rem ';
#!perl
#line 15
# scan all subdirs and join all todo.txt files

use strict;
use File::Find;

my %todo = ();
my $tasks = 0;
my $done  = 0;
my $declined = 0;
my $dubious = 0;

finddepth sub
{
  $todo{$File::Find::dir} = $file::Find::name
    if ($_ eq 'todo.txt');
}, ".";

foreach my $dir (sort keys %todo)
{
  print "-------------------------------\n";
  print "$dir\n";
  print "-------------------------------\n";
  open (F, "< $dir\\todo.txt");
  while (<F>)
  {
    $tasks++    if (substr($_, 0, 1) eq '*');
    $done++     if (substr($_, 0, 1) eq '+');
    $dubious++  if (substr($_, 0, 1) eq '?');
    $declined++ if (substr($_, 0, 1) eq '-');

    print $_;
  }
  close(F);
  print "\n\n";
}

print "----------------------------------------\n";
print "Total tasks: ", $tasks + $done + $dubious + $declined, "\n";
print "Done: $done\n";
print "Dubious: $dubious\n";
print "Declined: $declined\n";
printf "Comlete: %.0f%%\n", $done * 100 / ($tasks + $done);

__END__
:endofperl
