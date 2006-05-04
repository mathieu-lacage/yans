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
