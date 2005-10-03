#!/usr/bin/env perl

for ($size = 2; $size < 200; $size+= 2) {
    system ("echo \"#ifndef OBJET_SIZE\" > object-size.h");
    system ("echo \"#define OBJECT_SIZE $size\" >> object-size.h");
    system ("echo \"#endif\" >> object-size.h");
    system ("make clean >compil.out 2>compil.err && make >>compil.out 2>>compil.err");
    system ("./test > /dev/null");
    $output = system ("./test > output");
    open (OUTPUT, "output");
    my $indirect;
    my $direct;
    my $sizeof;
    while ($line = <OUTPUT>) {
	if ($line =~ /indirect: ([0-9]+)ms/) {
	    $indirect = $1;
	} elsif ($line =~ /direct: ([0-9]+)ms/) {
	    $direct = $1;
	} elsif ($line =~ /sizeof: ([0-9]+)/) {
	    $sizeof = $1;
	}
    }
    print "$size $sizeof $indirect $direct\n";
}

