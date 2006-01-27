import _yans
import inspect
from simulator import *
from _yans import PeriodicGenerator
from _yans import TrafficAnalyser
from _yans import Packet


class MyPyFunction0Holder:
    def set_function(self, function):
        self.m_function = function
    def call (self):
        self.m_function ()

class MyPyFunctionNHolder:
    def set_function(self, function, *args):
        self.m_function = function
        self.m_args = args
    def call (self):
        self.m_function (*self.m_args)

def make_callback(function, *args):
    if inspect.ismethod (function):
        name = "_yans.make_" + function.im_class.__name__ + "_" + function.__name__ + "_callback"
        command = name + '(args[0])';
    else:
        name = method.__name__ + "_callback"
        command = name + '()'
    print "eval " + command
    holder = eval (command)
    try:
        if inspect.ismethod (function):
            name = "yans.make_" + function.im_class.__name__ + "_" + function.__name__ + "_callback"
            command = name + '(args[0])';
        else:
            name = "yans.make_" + method.__name__ + "_callback"
            command = name + '()'
        print "eval " + command
        holder = eval (command)
    except:
        if len(args) == 0:
            holder = MyPyFunctionHolder (function)
        elif len(args) > 1:
            holder = MyPyFunctionHolder (function, *args)
        else:
            holder = _yans.FunctionHolder (function, args[0])
    return holder
