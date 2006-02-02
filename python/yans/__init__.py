import _yans
import inspect
from simulator import *
from _yans import PeriodicGenerator
from _yans import TrafficAnalyser
from _yans import Packet
from _yans import Ipv4Address
from _yans import Ipv4Mask
from _yans import MacAddress
from _yans import Host
from _yans import NetworkInterface
from _yans import EthernetNetworkInterface
from _yans import Cable
from _yans import UdpSource
from _yans import UdpSink
from _yans import PcapWriter
from _yans import TraceContainer
from _yans import TcpSource
from _yans import TcpSink



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
    try:
        if inspect.ismethod (function):
            name = "_yans." + function.im_class.__name__ + "_" + function.__name__ + "_callback"
            command = name + '(args[0])';
        else:
            name = "_yans." + method.__name__ + "_callback"
            command = name + '()'
        holder = eval (command)
    except:
        if len(args) == 0:
            holder = MyPyFunctionHolder (function)
        elif len(args) > 1:
            holder = MyPyFunctionHolder (function, *args)
        else:
            holder = _yans.FunctionHolder (function, args[0])
    return holder
