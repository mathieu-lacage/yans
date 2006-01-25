import _yans;

from _yans import run;
from _yans import stop;
from _yans import destroy;
from _yans import insert_in_s;
from _yans import insert_in_us;
from _yans import insert_at_s;
from _yans import insert_at_us;
from _yans import now_s;
from _yans import now_us;
from _yans import insert_later;

class MyFunctionHolder:
    def set_event (self, function, *args):
        self.m_function = function
        self.m_args = args
    def call (self):
        self.m_function (*(self.m_args))

def make_event(function, *args):
    holder = MyFunctionHolder ()
    holder.set_event (function, *args)
    ev = _yans.FunctionHolder (MyFunctionHolder.call, holder)
    return ev

