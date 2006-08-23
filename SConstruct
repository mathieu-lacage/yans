
import os.path

source_files = [
	'src/simulator/scheduler.cc', 
	'src/simulator/scheduler-list.cc',
        'src/simulator/scheduler-heap.cc',
        'src/simulator/scheduler-map.cc',
        'src/simulator/event-impl.cc',
        'src/simulator/event-tcc.cc',
        'src/simulator/event-tcc-test.cc',
        'src/simulator/simulator.cc',
        'src/simulator/unix-system-semaphore.cc',
        'src/thread/semaphore.cc',
        'src/thread/fiber.cc',
        'src/thread/fiber-scheduler.cc',
        'src/thread/thread.cc',
        'src/common/unix-system-thread.cc',
        'src/common/unix-system-mutex.cc',
        'src/common/unix-exec-commands.cc',
        'src/common/wall-clock-ms-unix.cc',
        'src/common/chunk.cc',
        'src/common/chunk-utils.cc',
        'src/common/packet.cc',
        'src/common/buffer.cc',
        'src/common/chunk-constant-data.cc',
        'src/common/ipv4-address.cc',
        'src/common/mac-address.cc',
        'src/common/mac-address-factory.cc',
        'src/common/mac-network-interface.cc',
        'src/common/tags.cc',
        'src/common/utils.cc',
        'src/common/population-analysis.cc',
        'src/common/data-writer.cc',
        'src/common/callback-test.cc',
        'src/common/reference-list-test.cc',
        'src/common/traced-variable-test.cc',
        'src/common/trace-stream-test.cc',
        'src/common/packet-logger.cc',
        'src/common/trace-container.cc',
        'src/common/pcap-writer.cc',
        'src/common/seed-generator-mrg32k3a.cc',
        'src/common/random-uniform-mrg32k3a.cc',
        'src/common/rng-mrg32k3a.cc',
        'src/common/timeout.cc',
        'src/common/chunk-llc-snap.cc',
        'src/common/llc-snap-encapsulation.cc',
        'src/common/ipv4-network-interface.cc',
        'src/common/position.cc',
        'src/common/static-position.cc',
        'src/common/static-speed-position.cc',
        'src/ipv4/ipv4-end-point.cc',
        'src/ipv4/ipv4-end-points.cc',
        'src/ipv4/ipv4-route.cc',
        'src/ipv4/ipv4.cc',
        'src/ipv4/defrag-state.cc',
        'src/ipv4/chunk-ipv4.cc',
        'src/ipv4/udp.cc',
        'src/ipv4/chunk-udp.cc',
        'src/ipv4/chunk-icmp.cc',
        'src/ipv4/chunk-tcp.cc',
        'src/ipv4/tcp.cc',
        'src/ipv4/tcp-connection-listener.cc',
        'src/ipv4/tcp-connection.cc',
        'src/ipv4/tcp-buffer.cc',
        'src/arp/arp-ipv4-network-interface.cc',
        'src/arp/chunk-arp.cc',
        'src/arp/arp-cache-entry.cc',
        'src/host/loopback-ipv4.cc',
        'src/host/host.cc',
        'src/ethernet/cable.cc',
        'src/ethernet/chunk-mac-crc.cc',
        'src/ethernet/chunk-mac-eth.cc',
        'src/ethernet/ethernet-network-interface.cc',
        'src/apps/udp-source.cc',
        'src/apps/udp-sink.cc',
        'src/apps/tcp-source.cc',
        'src/apps/tcp-sink.cc',
        'src/apps/periodic-generator.cc',
        'src/apps/traffic-analyser.cc',
        'src/apps/throughput-printer.cc',
        'src/80211/chunk-mac-80211-hdr.cc',
        'src/80211/chunk-mac-80211-fcs.cc',
        'src/80211/mac-stations.cc',
        'src/80211/mac-station.cc',
        'src/80211/arf-mac-stations.cc',
        'src/80211/aarf-mac-stations.cc',
        'src/80211/cr-mac-stations.cc',
        'src/80211/ideal-mac-stations.cc',
        'src/80211/propagation-model.cc',
        'src/80211/base-channel-80211.cc',
        'src/80211/channel-80211.cc',
        'src/80211/transmission-mode.cc',
        'src/80211/bpsk-mode.cc',
        'src/80211/qam-mode.cc',
        'src/80211/phy-80211.cc',
        'src/80211/network-interface-80211-simple.cc',
        'src/80211/network-interface-80211-simple-factory.cc',
        'src/80211/mac-simple.cc',
        'src/80211/mac-low.cc',
        'src/80211/mac-parameters.cc',
        'src/80211/dcf.cc',
        'src/80211/mac-tx-middle.cc',
        'src/80211/mac-rx-middle.cc',
        'src/80211/dca-txop.cc',
        'src/80211/mac-queue-80211e.cc',
        'src/80211/mac-high-adhoc.cc',
        'src/80211/ssid.cc',
        'src/80211/supported-rates.cc',
        'src/80211/capability-information.cc',
        'src/80211/status-code.cc',
        'src/80211/chunk-mgt.cc',
        'src/80211/mac-high-nqsta.cc',
        'src/80211/mac-high-nqap.cc',
        'src/80211/network-interface-80211.cc',
        'src/80211/network-interface-80211-factory.cc',
        'src/test/test.cc'
]

class Ns3BuildConfig:
	def __init__(self):
		self.opti = False
		self.static = False
		self.build_dir = 'build'
	def get_obj (self, source):
		obj_file = os.path.splitext (os.path.join (self.__get_build_dir (), source))[0] + '.o'
		return obj_file
	def __get_build_dir (self):
		base_output_dir = 'build'
		if self.static:
			static_output_dir = '-static'
		else:
			static_output_dir = '-shared'
		if self.opti:
			opti_output_dir = 'opt'
		else:
			opti_output_dir = 'dbg'
		output_dir = os.path.join (base_output_dir + static_output_dir, opti_output_dir)
		return output_dir
	def __get_lib (self, name):
		if self.static:
			full_name = 'lib' + name + '.a'
		else:
			full_name = 'lib' + name + '.so'
		return os.path.join (self.__get_build_dir (), full_name)
	def generate_library (self, env, name, source_files):
		object_files = []
		if self.static:
			for src in source_files:
				object_files.append (env.StaticObject (target = ns3.get_obj (src), source = src))
			library = env.StaticLibrary (target = self.__get_lib (name), source = object_files)
		else:
			for src in source_files:
				object_files.append (env.SharedObject (target = ns3.get_obj (src), source = src))
			library = env.SharedLibrary (target = self.__get_lib (name), source = object_files)
		return library

	

env = Environment ()

ns3 = Ns3BuildConfig ()

ns3.opti = False
ns3.static = True
library = ns3.generate_library (env, 'yans', source_files)
env.Alias ('dbg-static', library)

ns3.opti = False
ns3.static = False
library = ns3.generate_library (env, 'yans', source_files)
env.Alias ('dbg-shared', library)
env.Alias ('dbg', library)
env.Default (library)

ns3.opti = True
ns3.static = True
library = ns3.generate_library (env, 'yans', source_files)
env.Alias ('opti-static', library)

ns3.opti = True
ns3.static = False
library = ns3.generate_library (env, 'yans', source_files)
env.Alias ('opti-shared', library)
env.Alias ('opti', library)



