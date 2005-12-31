#!/usr/bin/env python

import yans;

class MyEvent(yans.Event):
    def notify (self):
        print "my event " + yans.simulator_now_s () + "\n";

ev = MyEvent ();

yans.simulator_insert_in_s ((double)5.0, ev);


yans.run ();
