#!/usr/bin/env python

from yans import *

generator = PeriodicGenerator ()
generator.start_at (0.0)
generator.set_packet_size (100)
generator.stop_at (10.0)

analyser = TrafficAnalyser ()
generator.set_send_callback (make_callback (TrafficAnalyser.receive, analyser))
generator.set_packet_interval (0.00001)

simulator.run ()
analyser.print_stats ()
simulator.destroy ()


class RxPacket:
    def rx (self, packet):
        print "rx packet at %f" % simulator.now_s ()

rxPacket = RxPacket ()
generator.set_send_callback (make_callback (RxPacket.rx, rxPacket))
generator.set_packet_interval (1.0)
generator.start_at (0.0)
generator.stop_at (10.0)

simulator.run ()
simulator.destroy ()
