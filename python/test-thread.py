#!/usr/bin/env python

import yans;
from yans import *;

class MyThread(yans.Thread):
#class MyThread:
    def run(self):
        print "before sleep";
#        sleep_s (1.0);
#        print "after sleep %f" % simulator.now_s ();
        print "after sleep %f" % time_s ();

#thread = MyThread ();
#simulator.insert_at_s (1.0, simulator.make_event (MyThread.run, thread));
thread = MyThread ("test");

simulator.run ()

simulator.destroy ();
