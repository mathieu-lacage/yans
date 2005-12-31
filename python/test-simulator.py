#!/usr/bin/env python

from yans import *;

class MyTestClass:
    def member_method(self):
        print "my member now=%f" % simulator.now_s ();

def my_nothing0 ():
    print "my model now=%f" % simulator.now_s ();
def my_nothing1 (v1):
    print "my model now=%f v1=%d" % (simulator.now_s (), v1);
def my_nothing2 (v1, v2):
    print "my model now=%f v1=%d,v2=%d" % (simulator.now_s (), v1, v2);

ev = simulator.make_event (my_nothing0);
ev1 = simulator.make_event (my_nothing1, 99);
ev2 = simulator.make_event (my_nothing2, 98, 97);
test = MyTestClass ();
ev3 = simulator.make_event (MyTestClass.member_method, test);

simulator.insert_in_s (1.0, ev);
simulator.insert_in_s (2.0, ev1);
simulator.insert_in_s (3.0, ev2);
simulator.insert_in_s (4.0, ev3);

class MyEventClass (simulator.Event):
    def notify (self):
        print "my event class now=%f" % simulator.now_s ();

ev4 = MyEventClass ();

simulator.insert_in_s (5.0, ev4);
simulator.insert_in_s (6.0, MyEventClass ());


simulator.run ();

simulator.destroy ();
