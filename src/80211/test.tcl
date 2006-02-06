#
#
# Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr> 
#         for the Planete project (http://www-sop.inria.fr/planete)
#

set ns [new Simulator]
$ns use-scheduler Heap

$ns node-config \
    -adhocRouting  DumbAgent \
    -llType LL \
    -macType Mac/Mac80211 \
    -ifqType Queue/DropTail \
    -ifqLen 50 \
    -antType  Antenna/OmniAntenna \
    -propType Propagation/TwoRayGround \
    -phyType Phy/Phy80211 \
    -channel [new Channel/Channel80211] \
    -topoInstance [new Topography] \
    -agentTrace OFF \
    -routerTrace OFF \
    -macTrace ON \
    -movementTrace OFF 

create-god 2

$ns trace-all [open test.tr w]

set node(0) [$ns node]
set node(1) [$ns node]
set node(2) [$ns node]
set node(3) [$ns node]

#$node(0) addenergymodel [new EnergyModel $node(0) 200.0 100.0 200.0] 
#$node(1) addenergymodel [new EnergyModel  $node(1) 200.0 100.0 100.0]
#$node(0) addenergymodel [new EnergyModel]
#$node(1) addenergymodel [new NullEnergyModel]

$node(0) set X_ 0.0
$node(0) set Y_ 0.0
$node(0) set Z_ 0.0

$node(1) set X_ 300.0
$node(1) set Y_ 0.0
$node(1) set Z_ 0.0

$node(2) set X_ 0.0
$node(2) set Y_ 300.0
$node(2) set Z_ 0.0

$node(3) set X_ 300.0
$node(3) set Y_ 300.0
$node(3) set Z_ 0.0


set tcp [new Agent/TCP]
$tcp set class_ 2
set sink [new Agent/TCPSink]
$ns attach-agent $node(0) $tcp
$ns attach-agent $node(1) $sink
$ns connect $tcp $sink
set ftp [new Application/FTP]
$ftp attach-agent $tcp
$ns at 3.0 "$ftp start" 

set tcp [new Agent/TCP]
$tcp set class_ 2
set sink [new Agent/TCPSink]
$ns attach-agent $node(2) $tcp
$ns attach-agent $node(3) $sink
$ns connect $tcp $sink
set ftp [new Application/FTP]
$ftp attach-agent $tcp
$ns at 4.0 "$ftp start" 


$ns at 300 "puts \"End of simulation.\"; $ns halt"
puts "Starting Simulation..."
$ns run

