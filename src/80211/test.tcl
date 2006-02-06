#
#
# Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr> 
#         for the Planete project (http://www-sop.inria.fr/planete)
#

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
    -phyType Phy/Phy80211 \
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
set nodes(3) [$ns node]

# set all nodes in station mode except for
# the ap which is set to ap mode.
#   - the first argument must be the address of
#     the tentative ap.
#   - the second argument must be the name of
#     the array which holds all the nodes.
proc set-bss-mode {ap arrayName} {
    upvar $arrayName nodes
    for {set i 0} {$i < [array size nodes]} {incr i} {
	set node $nodes($i)
	for {set j 0} {$j < [$node set nifs_]} {incr j} {
	    set mac [$node set mac_($j)]
	    if {$i == $ap} {
		$mac mode access-point
	    } else {
		$mac mode station $ap
	    }
	}
    }
}

# set all nodes in adhoc mode.
#   - the first and only argument must be 
#     the name of the array which holds
#     all the nodes.
proc set-adhoc-mode {arrayName} {
    upvar $arrayName nodes
    for {set i 0} {$i < [array size nodes]} {incr i} {
	set node $nodes($i)
	for {set j 0} {$j < [$node set nifs_]} {incr j} {
	    set mac [$node set mac_($j)]
	    $mac mode adhoc
	}
    }
}

#set-bss-mode 1 nodes

set-adhoc-mode nodes

$nodes(0) set X_ 0.0
$nodes(0) set Y_ 0.0
$nodes(0) set Z_ 0.0

$nodes(1) set X_ 300.0
$nodes(1) set Y_ 0.0
$nodes(1) set Z_ 0.0

$nodes(2) set X_ 0.0
$nodes(2) set Y_ 300.0
$nodes(2) set Z_ 0.0

$nodes(3) set X_ 300.0
$nodes(3) set Y_ 300.0
$nodes(3) set Z_ 0.0



set tcp [new Agent/TCP]
$tcp set class_ 2
set sink [new Agent/TCPSink]
$ns attach-agent $nodes(0) $tcp
$ns attach-agent $nodes(1) $sink
$ns connect $tcp $sink
set ftp [new Application/FTP]
$ftp attach-agent $tcp
$ns at 3.0 "$ftp start" 


$ns at 300 "puts \"End of simulation.\"; $ns halt"
puts "Starting Simulation..."
$ns run

