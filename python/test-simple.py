#!/usr/bin/env python

import yans;

def my_nothing0 ():
    print "my model now=%f \n" % yans.now_s ();
def my_nothing1 (v1):
    print "my model now=%f v1=%d\n" % (yans.now_s (), v1);
def my_nothing2 (v1, v2):
    print "my model now=%f v1=%d,v2=%d\n" % (yans.now_s (), v1, v2);

class MyEvent(yans.Event):
    def set_callback (self, function, *args):
        self.__function_ = function;
        self.__args_ = args;
    def notify (self):
        self.__function_ (*self.__args_);
        
def make_event(function, *args):
    ev = MyEvent ();
    ev.set_callback (function, *args);
    return ev

ev = make_event (my_nothing0);
ev1 = make_event (my_nothing1, 99);
ev2 = make_event (my_nothing2, 98, 97);

yans.insert_in_s (1.0, ev);
yans.insert_in_s (2.0, ev1);
yans.insert_in_s (3.0, ev2);

yans.run ();

yans.destroy ();


