#!/usr/bin/env python

from yans import *;

class TestGc:
    __count_ = 0
    def test_one (self):
        self.__count_ += 1;
        if self.__count_ < 1000000:
            simulator.insert_in_s (1.0, simulator.make_event (TestGc.test_one, self))

simulator.insert_in_s (10.0, simulator.make_event (TestGc.test_one, TestGc ()))

simulator.run ();
