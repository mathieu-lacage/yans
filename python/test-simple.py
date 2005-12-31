#!/usr/bin/env python

import yans;

class MyEvent(yans.Event):
    def notify (self):
        print "my event %f\n" % yans.simulator_now_s ();

ev = MyEvent ();
yans.simulator_insert_in_s (1.0, ev);

yans.simulator_run ();

yans.simulator_destroy ();


