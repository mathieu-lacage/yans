from _simulator import *;


class __MyEvent__(Event):
    def set_callback (self, function, *args):
        self.__function_ = function;
        self.__args_ = args;
    def notify (self):
        self.__function_ (*self.__args_);
        
def make_event(function, *args):
    ev = __MyEvent__ ();
    ev.set_callback (function, *args);
    return ev

