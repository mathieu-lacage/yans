source ./node-common/utils.tcl

set ns [new Simulator]
$ns use-scheduler Heap

set nodeConstructor [new NodeConstructor];
set interfaceConstructor [new TclNetInterfaceConstructor80211];
set channel [new TclFreeSpaceBroadcastChannel];

set nodes(0) [$nodeConstructor create-node];
set nodes(1) [$nodeConstructor create-node];
set nodes(2) [$nodeConstructor create-node];

$nodes(0) set-position 0.0 0.0 0.0
$nodes(1) set-position 200.0 0.0 0.0
$nodes(2) set-position 0.0 200.0 0.0

$interfaceConstructor set-qap;
set interfaces(0) [$interfaceConstructor create-interface];
$interfaceConstructor set-qsta 1;
set interfaces(1) [$interfaceConstructor create-interface];
set interfaces(2) [$interfaceConstructor create-interface];

$nodes(0) add-interface $interfaces(0) $channel
$nodes(1) add-interface $interfaces(1) $channel
$nodes(2) add-interface $interfaces(2) $channel



#########################################################
#      802.11e testing. Setup 2 TSs.

#
# This is the canonical PCM coding for telephony networks:
# sampling frequency is 8kHz, sample size is 1 byte, total
# bandwidth required is thus 64kb/s and the required end-to
# end delay is 125ms.
# Here, I arbitrarily decided that each 8-bit voice sample
# is stored in a 32-sample super-packet so this cbr source
# is 32 bytes every 4ms
#
# This means that the size of each packet is:
#  32 (payload) + 40 (ip+udp+rtp) + 34 (MAC header) = 104 bytes
# MSDU = 32+40 = 72 bytes.
# Thus, the Data Rate at the MAC level is:
#   72bytes/4ms = 18000bytes/s
#

set tspec [new TclTspec]
$tspec set-minimum-service-interval 0.0
$tspec set-maximum-service-interval 0.0
# ms
$tspec set-delay-bound 0.125
# bytes
$tspec set-nominal-msdu-size 72
# bytes per second
$tspec set-mean-data-rate 18000
$tspec set-peak-data-rate 18000 
proc addts-granted-callback0 {tspec tsid} {
    global ::ns
    global ::nodes
    set source [new Agent/UDP];
    $nodes(1) attach-agent $source;
    set cbr [new Application/Traffic/CBR];
    $cbr attach-agent $source;
    $cbr set packetSize_ 72
    $cbr set interval_ 0.004
    $source set class_ 0;
    $source set prio_ $tsid
    
    set sink [new Agent/Null];
    $nodes(2) attach-agent $sink;

    ip-connect $source $sink
    $ns at 3.0 "$cbr start"

    puts "tspec granted for tsid $tsid";
}
proc addts-refused-callback0 {tspec tsid} {
    puts "tspec refused for tsid $tsid";
}
$interfaces(1) addts $tspec addts-granted-callback0 addts-refused-callback0


set tspec [new TclTspec]
$tspec set-minimum-service-interval 0.0
$tspec set-maximum-service-interval 0.0
# ms
$tspec set-delay-bound 0.125
# bytes
$tspec set-nominal-msdu-size 72
# bytes per second
$tspec set-mean-data-rate 18000
$tspec set-peak-data-rate 18000 
proc addts-granted-callback1 {tspec tsid} {
    global ::ns
    global ::nodes
    set source [new Agent/UDP];
    $nodes(2) attach-agent $source;
    set cbr [new Application/Traffic/CBR];
    $cbr attach-agent $source;
    $cbr set packetSize_ 72
    $cbr set interval_ 0.004
    $source set class_ 0;
    $source set prio_ $tsid
    
    set sink [new Agent/Null];
    $nodes(1) attach-agent $sink;

    ip-connect $source $sink
    $ns at 3.0 "$cbr start"

    puts "tspec granted for tsid $tsid";
}
proc addts-refused-callback1 {tspec tsid} {
    puts "tspec refused for tsid $tsid";
}
$interfaces(2) addts $tspec addts-granted-callback1 addts-refused-callback1
#########################################################


$ns at 300 "puts \"End of simulation.\"; $ns halt"
puts "Starting Simulation..."
$ns run

