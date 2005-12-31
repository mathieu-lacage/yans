#!/usr/bin/env python

from yans import *;

def my_nothing0 ():
    print "my model now=%f \n" % simulator.now_s ();
def my_nothing1 (v1):
    print "my model now=%f v1=%d\n" % (simulator.now_s (), v1);
def my_nothing2 (v1, v2):
    print "my model now=%f v1=%d,v2=%d\n" % (simulator.now_s (), v1, v2);

ev = simulator.make_event (my_nothing0);
ev1 = simulator.make_event (my_nothing1, 99);
ev2 = simulator.make_event (my_nothing2, 98, 97);

simulator.insert_in_s (1.0, ev);
simulator.insert_in_s (2.0, ev1);
simulator.insert_in_s (3.0, ev2);

simulator.run ();

simulator.destroy ();
