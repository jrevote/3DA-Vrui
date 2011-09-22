#!/usr/bin/perl -w 

use strict;

sub read_arguments;
sub pre_process;

&pre_process();

sub pre_process {
   my @counts = (0, 0, 0);
   #my @min = (0.0, 0.0, 0.0);
   #my @max = (0.0, 0.0, 0.0);
   my $min;
   my $max;
   my @elements = ((), (), ());
   my @seen = ((), (), ());
   my @order = (0, 0, 0);
   my @scalars;
   my @columns;
   my $input_file = "";
   my $flag = 0;
   my $i;

   open( CLOUD_INPUT, "<$ARGV[0]" ) or die;
   while( <CLOUD_INPUT> ) {
      my $count;
      my ($line) = $_;
      my @container = ();
      my $magnitude;
      
      chomp($line);

      (@container) = split( ' ', $line );
      if( $flag == 1 ) {
         $magnitude = $container[5];
         $magnitude =~ s/\"//g;
         print $magnitude;
         if( $min > $magnitude ) { 
            $min = $magnitude;
         }
         elsif( $max < $magnitude ) {
            $max = $magnitude;
         }
      } 
      else {
         $min = $magnitude;
         $max = $magnitude;
         $flag = 1;
      }
   }
   print "min - ".$min." max - ".$max."\n";
}
