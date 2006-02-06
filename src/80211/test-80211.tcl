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

# Set various PHY models
# the default is ET. See the README for
# a description of these models.
#$interfaceConstructor set-phy-model-snrt
#$interfaceConstructor set-phy-model-ber
#$interfaceConstructor set-phy-model-et


# set ARF rate control
#$interfaceConstructor set-arf
# set AARF rate control
#$interfaceConstructor set-aarf
# constant rate: 2 for data frames and 0 for control and management frames
#$interfaceConstructor set-cr 2 0


# for adhoc mode, use set-adhoc
#$interfaceConstructor set-adhoc;

$interfaceConstructor set-nqap;
$nodes(0) add-interface [$interfaceConstructor create-interface] $channel
$interfaceConstructor set-nqsta 1;
$nodes(1) add-interface [$interfaceConstructor create-interface] $channel
$nodes(2) add-interface [$interfaceConstructor create-interface] $channel



#########################################################
#   Normal tcp testing for 802.11
# setup tcp receiver
set sink [new Agent/TCPSink]
$nodes(1) attach-agent $sink
# setup tcp source.
set source [new Agent/TCP]
$source set class_ 2
$nodes(2) attach-agent $source
set ftp [new Application/FTP]
$ftp attach-agent $source
# connect tcp source to receiver
ip-connect $source $sink
# start source.
$ns at 3.0 "$ftp start" 
#########################################################

proc end-simulation {} {
    global ns
    puts "End of simulation."; 
    $ns halt;
}

$ns at 300 end-simulation 
puts "Starting Simulation..."
$ns run

