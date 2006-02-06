source 80211/utils.tcl

#################################################################
# Normal Unicast simulation.
set ns [new Simulator]
$ns use-scheduler Heap

$ns node-config \
    -adhocRouting DumbAgent \
    -llType LL \
    -macType Mac/Mac80211 \
    -ifqType Queue/DropTail \
    -ifqLen 50 \
    -antType  Antenna/OmniAntenna \
    -propType Propagation/TwoRayGround \
    -phyType Phy/80211/Ber \
    -channel [new Channel/Channel80211] \
    -topoInstance [new Topography] \
    -agentTrace OFF \
    -routerTrace OFF \
    -macTrace OFF \
    -movementTrace OFF 

create-god 2

$ns trace-all [open test.tr w]

set nodes(0) [$ns node]
set nodes(1) [$ns node]
set nodes(2) [$ns node]
$nodes(0) set X_ 0.0
$nodes(0) set Y_ 0.0
$nodes(0) set Z_ 0.0

$nodes(1) set X_ 200.0
$nodes(1) set Y_ 0.0
$nodes(1) set Z_ 0.0

$nodes(2) set X_ 0.0
$nodes(2) set Y_ 200.0
$nodes(2) set Z_ 0.0


set-qbss-mode 2 nodes
#set-bss-mode 2 nodes
#set-adhoc-mode nodes

#########################################################
#   Normal tcp testing for 802.11
# setup tcp receiver
set sink [new Agent/TCPSink]
$ns attach-agent $nodes(1) $sink
# setup tcp source.
set tcp [new Agent/TCP]
$tcp set class_ 2
$ns attach-agent $nodes(2) $tcp
set ftp [new Application/FTP]
$ftp attach-agent $tcp
# connect tcp source to receiver
$ns connect $tcp $sink
# start source.
$ns at 3.0 "$ftp start" 
#########################################################

$ns at 300 "puts \"End of simulation.\"; $ns halt"
puts "Starting Simulation..."
$ns run

