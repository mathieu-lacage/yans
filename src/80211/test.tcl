#
#
# Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr> 
#         for the Planete project (http://www-sop.inria.fr/planete)
#


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

# set all nodes in QoS station mode except for
# the ap which is set to QoS ap mode.
#   - the first argument must be the address of
#     the tentative ap.
#   - the second argument must be the name of
#     the array which holds all the nodes.
proc set-qbss-mode {ap arrayName} {
    upvar $arrayName nodes
    for {set i 0} {$i < [array size nodes]} {incr i} {
	set node $nodes($i)
	for {set j 0} {$j < [$node set nifs_]} {incr j} {
	    set mac [$node set mac_($j)]
	    if {$i == $ap} {
		$mac mode qaccess-point
	    } else {
		$mac mode qstation $ap
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

proc addts-request {node tspec granted_callback refused_callback} {
    for {set j 0} {$j < [$node set nifs_]} {incr j} {
	set mac [$node set mac_($j)]
	$mac addts-request $tspec $granted_callback $refused_callback
    }    
}


#################################################################
#################################################################


#################################################################
# Multicast simulation.
# No way this is ever going to work.
#set ns [new Simulator -multicast on]
#set group [Node allocaddr]
#set mproto DM          
#set mrthandle [$ns mrtproto $mproto] 

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

set tspec [new TSPEC]
$tspec set minimumServiceInterval 0.0
$tspec set maximumServiceInterval 0.0
# ms
$tspec set delayBound 0.125
# bytes
$tspec set nominalMSDUSize 72
# bytes per second
$tspec set meanDataRate 18000
$tspec set peakDataRate 18000 
proc addts-granted-callback0 {tspec tsid} {
    global ::ns
    global ::nodes
    set udp0 [new Agent/UDP];         # A UDP agent
    $ns attach-agent $nodes(0) $udp0; # on node $n0
    set cbr0 [new Application/Traffic/CBR]; # A CBR traffic generator agent
    $cbr0 attach-agent $udp0; # attached to the UDP agent
    $cbr0 set packetSize_ 72
    $cbr0 set interval_ 0.004
    $udp0 set class_ 0; # actually, the default, but. . .
    $udp0 set prio_ $tsid
    
    set null0 [new Agent/Null]; # Its sink
    $ns attach-agent $nodes(2) $null0; # on node  

    $ns connect $udp0 $null0
    $ns at 3.0 "$cbr0 start"
#    $ns at 3.0 "$cbr0 stop"

    puts "tspec granted for tsid $tsid";
}
proc addts-refused-callback0 {tspec tsid} {
    puts "tspec refused for tsid $tsid";
}
#addts-request $nodes(0) $tspec addts-granted-callback0 addts-refused-callback0


set tspec [new TSPEC]
$tspec set minimumServiceInterval 0.0
$tspec set maximumServiceInterval 0.0
# ms
$tspec set delayBound 0.125
# bytes
$tspec set nominalMSDUSize 72
# bytes per second
$tspec set meanDataRate 18000
$tspec set peakDataRate 18000 
proc addts-granted-callback1 {tspec tsid} {
    global ::ns
    global ::nodes
    set udp1 [new Agent/UDP];         # A UDP agent
    $ns attach-agent $nodes(1) $udp1; # on node $n0
    set cbr1 [new Application/Traffic/CBR]; # A CBR traffic generator agent
    $cbr1 attach-agent $udp1; # attached to the UDP agent
    $cbr1 set packetSize_ 72
    $cbr1 set interval_ 0.004
    $udp1 set class_ 0; # actually, the default, but. . .
    $udp1 set prio_ $tsid
    
    set null1 [new Agent/Null]; # Its sink
    $ns attach-agent $nodes(2) $null1; # on node  

    $ns connect $udp1 $null1
    $ns at 3.0 "$cbr1 start"
#    $ns at 3.0 "$cbr0 stop"

    puts "tspec granted for tsid $tsid";
}
proc addts-refused-callback1 {tspec tsid} {
    puts "tspec refused for tsid $tsid";
}
#addts-request $nodes(1) $tspec addts-granted-callback1 addts-refused-callback1
#########################################################



#########################################################
#    Multicast testing.
#    No way this is ever going to work.
# setup udp multicast receivers.
#set rcvr1 [new Agent/LossMonitor]
#set rcvr2 [new Agent/LossMonitor]
#$ns at 1.0 "$nodes(1) join-group $rcvr1 $group" 
#$ns at 1.0 "$nodes(2) join-group $rcvr2 $group" 


# setup udp multicast source.
#set n0 $nodes(0)
#set udp [new Agent/UDP]    
#$ns attach-agent $n0 $udp 
#set src [new Application/Traffic/CBR]        
#$src attach-agent $udp
#$udp set dst_addr_ $group
#$udp set dst_port_ 0
#$ns at 3.0 "$src start"
#
#########################################################


#########################################################
#   Normal tcp testing for 802.11
# setup tcp receiver
set sink [new Agent/TCPSink]
$ns attach-agent $nodes(1) $sink
# setup tcp source.
set tcp [new Agent/TCP]
$tcp set class_ 2
$ns attach-agent $nodes(0) $tcp
set ftp [new Application/FTP]
$ftp attach-agent $tcp
# connect tcp source to receiver
$ns connect $tcp $sink
# start source.
#$ns at 3.0 "$ftp start" 
#########################################################

$ns at 300 "puts \"End of simulation.\"; $ns halt"
puts "Starting Simulation..."
$ns run

