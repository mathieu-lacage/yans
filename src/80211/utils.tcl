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
