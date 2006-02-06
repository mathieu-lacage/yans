proc ip-connect-to {source dest} {
    $source set dst_addr_ [$dest set agent_addr_]
    $source set dst_port_ [$dest set agent_port_]
}

set ns [new Simulator]
$ns use-scheduler Heap

set nodeConstructor [new NodeConstructor];

set nodea [$nodeConstructor create-node];

set nodeb [$nodeConstructor create-node];

$nodea set-var x 0.0;
$nodeb set-var y 0.0;

# dest sink
set sink [new Agent/TCPSink]
$nodea attach-agent $sink
# source 
set source [new Agent/TCP]
$source set class_ 2
$nodeb attach-agent $source
# connect source to dest
ip-connect-to $source $sink;
# 
set ftp [new Application/FTP]
$ftp attach-agent $source
# start source at 3.0
$ns at 3.0 "$ftp start" 

