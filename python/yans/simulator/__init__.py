from _simulator import *;

class MyFunctionHolder:
    def set_event (self, function, *args):
        self.m_function = function
        self.m_args = args
    def call (self):
        self.m_function (*(self.m_args))

def make_event(function, *args):
    holder = MyFunctionHolder ()
    holder.set_event (function, *args)
    ev = FunctionHolder (MyFunctionHolder.call, holder)
    return ev

