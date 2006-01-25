#!/usr/bin/env python

from yans import *


class RxPacket:
    def rx (self, packet):
        print "rx packet at %f" % simulator.now_s () ;


generator = PeriodicGenerator ()


rxPacket = RxPacket ()
generator.set_send_callback (make_callback (RxPacket.rx, rxPacket))

generator.start_at (0.0)
generator.set_packet_interval (1.0);
generator.set_packet_size (100);
generator.stop_at (10.0);

simulator.run ()

simulator.destroy ()
