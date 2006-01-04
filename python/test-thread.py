#!/usr/bin/env python

import yans;
from yans import *;

class MyThread(yans.Thread):
    def run(self):
        print "before sleep";
        self.sleep_s (1.0);
        print "after sleep %f" % self.time_s ();

thread = MyThread ("test");

simulator.run ()

simulator.destroy ();
