
import os
import os.path
import shutil

class YansModule:
	def __init__ (self, name, dir):
		self.sources = []
		self.inst_headers = []
		self.headers = []
		self.extra_dist = []
		self.deps = []
		self.external_deps = []
		self.config = []
		self.name = name
		self.dir = dir
		self.executable = False
		self.library = True
	def set_library (self):
		self.library = True
		self.executable = False
	def set_executable (self):
		self.library = False
		self.executable = True
	def add_config (self, config_fn):
		self.config.append (config_fn)
	def add_extra_dist (self, dist):
		self.extra_dist.append (dist)
	def add_external_dep (self, dep):
		self.external_deps.append (dep)
	def add_dep (self, dep):
		self.deps.append (dep)
	def add_deps (self, deps):
		self.deps.extend (deps)
	def add_source (self, source):
		self.sources.append (source)
	def add_sources (self, sources):
		self.sources.extend (sources)
	def add_header (self, header):
		self.headers.append (header)
	def add_headers (self, headers):
		self.headers.extend (headers)
	def add_inst_header (self, header):
		self.inst_headers.append (header)
	def add_inst_headers (self, headers):
		self.inst_headers.extend (headers)

def MyCopyAction (target, source, env):
	try:
		if len (target) == len (source):
			for i in range (len(target)):
				shutil.copy (source[i].path, target[i].path)
			return 0
		else:
			return 'invalid target/source match'
	except:
		print
		return 'exception'
def MyCopyActionPrint (target, source, env):
	if len (target) == len (source):
		output = ''
		for i in range (len(target)):
			output = output + 'copy \'' + source[i].path + '\' to \'' + target[i].path  + '\''
			if i < len (target) - 1:
				output = output + '\n'
		return output
	else:
		return 'error in copy'
def GcxxEmitter (target, source, env):
	if os.path.exists (source[0].path):
		return [target, source]
	else:
		return [[], []]
def MyRmTree (target, source, env):
	shutil.rmtree (env['RM_DIR'])
	return 0
def MyRmTreePrint (target, source, env):
	return ''
def print_cmd_line(s, target, src, env):
	print 'Building ' + (' and '.join([str(x) for x in target])) + '...'

class YansBuildVariant:
	def __init__ (self):
		self.static = False
		self.gcxx_deps = False
		self.gcxx_root = ''
		self.build_root = ''
		self.env = None

class Yans:
	def __init__ (self):
		self.__modules = []
		self.extra_dist = []
		self.build_dir = 'build'
		self.version = '0.0.1'
		self.name = 'noname'
	def add (self, module):
		self.__modules.append (module)
	def add_extra_dist (self, dist):
		self.extra_dist.append (dist)
	def __get_module (self, name):
		for module in self.__modules:
			if module.name == name:
				return module
		return None
	def get_mod_output (self, module, variant):
		if module.executable:
			suffix = variant.env.subst (variant.env['PROGSUFFIX'])
			filename = os.path.join (variant.build_root, 'bin',
						 module.name + suffix)
		else:
			if variant.static:
				prefix = variant.env['LIBPREFIX']
				suffix = variant.env['LIBSUFFIX']
			else:
				prefix = variant.env['SHLIBPREFIX']
				suffix = variant.env['SHLIBSUFFIX']
			prefix = variant.env.subst (prefix)
			suffix = variant.env.subst (suffix)
			filename = os.path.join (variant.build_root, 'lib',
						 prefix + module.name + suffix)
		return filename				
	def get_obj_builders (self, variant, module):
		env = variant.env
		objects = []
		if len (module.config) > 0:
			src_config_file = os.path.join (self.build_dir, 'config', module.name + '-config.h')
			tgt_config_file = os.path.join (variant.build_root, 'include',
							'yans', module.name + '-config.h')
		
		for source in module.sources:
			obj_file = os.path.splitext (source)[0] + '.o'
			tgt = os.path.join (variant.build_root, module.dir, obj_file)
			src = os.path.join (module.dir, source)
			if variant.static:
				obj_builder = env.StaticObject (target = tgt, source = src)
			else:
				obj_builder = env.SharedObject (target = tgt, source = src)
			if len (module.config) > 0:
				config_file = env.MyCopyBuilder (target = [tgt_config_file],
								 source = [src_config_file])
				env.Depends (obj_builder, config_file)
			if variant.gcxx_deps:
				gcno_tgt = os.path.join (variant.build_root, module.dir, 
							 os.path.splitext (source)[0] + '.gcno')
				gcda_tgt = os.path.join (variant.build_root, module.dir,
							 os.path.splitext (source)[0] + '.gcda')
				gcda_src = os.path.join (variant.gcxx_root, module.dir, 
							 os.path.splitext (source)[0] + '.gcda')
				gcno_src = os.path.join (variant.gcxx_root, module.dir, 
							 os.path.splitext (source)[0] + '.gcno')
				gcno_builder = env.CopyGcxxBuilder (target = gcno_tgt, source = gcno_src)
				gcda_builder = env.CopyGcxxBuilder (target = gcda_tgt, source = gcda_src)
				env.Depends (obj_builder, gcda_builder)
				env.Depends (obj_builder, gcno_builder)
			objects.append (obj_builder)
		return objects
	def get_internal_deps (self, module, hash):
		for dep_name in module.deps:
			dep = self.__get_module (dep_name)
			hash[dep_name] = dep
			self.get_internal_deps (dep, hash)
	def get_external_deps (self, module):
		hash = {}
		self.get_internal_deps (module, hash)
		ext_hash = {}
		for mod in hash.values ():
			for ext_dep in mod.external_deps:
				ext_hash[ext_dep] = 1
		return ext_hash.keys ()
	def get_sorted_deps (self, module):
		h = {}
		self.get_internal_deps (module, h)
		modules = []
		for dep in h.keys ():
			deps_copy = []
			mod = h[dep]
			deps_copy.extend (mod.deps)
			modules.append ([mod, deps_copy])
		sorted = []
		while len (modules) > 0:
			to_remove = []
			for item in modules:
				if len (item[1]) == 0:
					to_remove.append (item[0].name)
			for item in to_remove:
				for i in modules:
					if item in i[1]:
						i[1].remove (item)
			new_modules = []
			for mod in modules:
				found = False
				for i in to_remove:
					if i == mod[0].name:
						found = True
						break
				if not found:
					new_modules.append (mod)
			modules = new_modules
			sorted.extend (to_remove)
		sorted.reverse ()
		# append external deps
		ext_deps = self.get_external_deps (module)
		for dep in ext_deps:
			sorted.append (dep)
		return sorted
			
	def gen_mod_dep (self, variant):
		build_root = variant.build_root
		cpp_path = os.path.join (variant.build_root, 'include')
		env = variant.env
		env.Append (CPPPATH=[cpp_path])
		header_dir = os.path.join (build_root, 'include', 'yans')
		lib_path = os.path.join (build_root, 'lib')
		module_builders = []
		for module in self.__modules:
			objects = self.get_obj_builders (variant, module)
			libs = self.get_sorted_deps (module)

			filename = self.get_mod_output (module, variant)
			if module.executable:
				module_builder = env.Program (target = filename, source = objects,
							      LIBPATH=lib_path, LIBS=libs, RPATH=lib_path)
			else:
				if variant.static:
					module_builder = env.StaticLibrary (target = filename, source = objects)
				else:
					module_builder = env.SharedLibrary (target = filename, source = objects,
									    LIBPATH=lib_path, LIBS=libs)
					
			for dep_name in module.deps:
				dep = self.__get_module (dep_name)
				env.Depends (module_builder, self.get_mod_output (dep, variant))
					
			for header in module.inst_headers:
				tgt = os.path.join (header_dir, header)
				src = os.path.join (module.dir, header)
				#builder = env.Install (target = tgt, source = src)
				header_builder = env.MyCopyBuilder (target = tgt, source = src)
				env.Depends (module_builder, header_builder)
				
			module_builders.append (module_builder)
		return module_builders
	def gen_mod_config (self, env):
		config_dir = os.path.join (self.build_dir, 'config')
		for module in self.__modules:
			if len (module.config) > 0:
				config_file = os.path.join (config_dir, module.name + '-config.h')
				config_file_guard = module.name + '_CONFIG_H'
				config_file_guard.upper ()
				if not os.path.isfile (config_file):
					if not os.path.isdir (config_dir):
						os.makedirs (config_dir)
					outfile = open (config_file, 'w')
					outfile.write ('#ifndef ' + config_file_guard + '\n')
					outfile.write ('#define ' + config_file_guard + '\n')
					config = env.Configure ()
					for fn in module.config:
						output = fn (env, config)
						for o in output:
							outfile.write (o)
							outfile.write ('\n')
					outfile.write ('#endif /*' + config_file_guard + '*/\n')
					config.Finish ()
	def generate_dependencies (self):
		env = Environment()
		self.gen_mod_config (env)
		cc = env['CC']
		cxx = env.subst (env['CXX'])
		if cc == 'cl' and cxx == 'cl':
			env = Environment (tools = ['mingw'])
			cc = env['CC']
			cxx = env.subst (env['CXX'])
		if cc == 'gcc' and cxx == 'g++':
			common_flags = ['-g3', '-Wall', '-Werror']
			debug_flags = []
			opti_flags = ['-O3']
		elif cc == 'cl' and cxx == 'cl':
			env = Environment (ENV = os.environ)
			common_flags = []
			debug_flags = ['-W1', '-GX', '-EHsc', '-D_DEBUG', '/MDd']
			opti_flags = ['-O2', '-EHsc', '-DNDEBUG', '/MD']
		env.Append (CCFLAGS=common_flags,
			    CPPDEFINES=['RUN_SELF_TESTS'],
			    TARFLAGS='-c -z')
		verbose = ARGUMENTS.get('verbose', 'n')
		if verbose == 'n':
			env['PRINT_CMD_LINE_FUNC'] = print_cmd_line
		header_builder = Builder (action = Action (MyCopyAction, strfunction = MyCopyActionPrint))
		env.Append (BUILDERS = {'MyCopyBuilder':header_builder})
		gcxx_builder = Builder (action = Action (MyCopyAction, strfunction = MyCopyActionPrint),
					emitter = GcxxEmitter)
		env.Append (BUILDERS = {'CopyGcxxBuilder':gcxx_builder})
		variant = YansBuildVariant ()
		builders = []
		

		gcov_env = env.Copy ()
		gcov_env.Append (CFLAGS=['-fprofile-arcs', '-ftest-coverage'],
				 CXXFLAGS=['-fprofile-arcs', '-ftest-coverage'],
				 LINKFLAGS=['-fprofile-arcs'])
		# code coverage analysis
		variant.static = False
		variant.env = gcov_env
		variant.build_root = os.path.join (self.build_dir, 'gcov')
		builders = self.gen_mod_dep (variant)
		for builder in builders:
			gcov_env.Alias ('gcov', builder)


		opt_env = env.Copy ()
		opt_env.Append (CFLAGS=opti_flags,
				CXXFLAGS=opti_flags,
				CPPDEFINES=['NDEBUG'])
		# optimized static support
		variant.static = True
		variant.env = opt_env
		variant.build_root = os.path.join (self.build_dir, 'opt-static')
		builders = self.gen_mod_dep (variant)
		for builder in builders:
			opt_env.Alias ('opt-static', builder)


		opt_env = env.Copy ()
		opt_env.Append (CFLAGS=opti_flags,
				CXXFLAGS=opti_flags,
				CPPDEFINES=['NDEBUG'])
		# optimized shared support
		variant.static = False
		variant.env = opt_env
		variant.build_root = os.path.join (self.build_dir, 'opt-shared')
		builders = self.gen_mod_dep (variant)
		for builder in builders:
			opt_env.Alias ('opt-shared', builder)


		arc_env = env.Copy ()
		arc_env.Append (CFLAGS=opti_flags,
				CXXFLAGS=opti_flags,
				CPPDEFINES=['NDEBUG'])
		arc_env.Append (CFLAGS=['-frandom-seed=0','-fprofile-generate'],
				CXXFLAGS=['-frandom-seed=0','-fprofile-generate'],
				LINKFLAGS=['-frandom-seed=0', '-fprofile-generate'])
		# arc profiling
		variant.static = False
		variant.env = arc_env
		variant.build_root = os.path.join (self.build_dir, 'opt-arc')
		builders = self.gen_mod_dep (variant)
		for builder in builders:
			arc_env.Alias ('opt-arc', builder)


		arcrebuild_env = env.Copy ()
		arcrebuild_env.Append (CFLAGS=opti_flags,
				       CXXFLAGS=opti_flags,
				       CPPDEFINES=['NDEBUG'])
		arcrebuild_env.Append (CFLAGS=['-frandom-seed=0', '-fprofile-use'],
				       CXXFLAGS=['-frandom-seed=0', '-fprofile-use'],
				       LINKFLAGS=['-frandom-seed=0','-fprofile-use'])
		# arc rebuild
		variant.static = False
		variant.env = arcrebuild_env
		variant.gcxx_deps = True
		variant.gcxx_root = os.path.join (self.build_dir, 'opt-arc')
		variant.build_root = os.path.join (self.build_dir, 'opt-arc-rebuild')
		builders = self.gen_mod_dep (variant)
		for builder in builders:
			arcrebuild_env.Alias ('opt-arc-rebuild', builder)
		variant.gcxx_deps = False




		dbg_env = env.Copy ()
		env.Append (CFLAGS=debug_flags,
			    CXXFLAGS=debug_flags,)
		# debug static support
		variant.static = True
		variant.env = dbg_env
		variant.build_root = os.path.join (self.build_dir, 'dbg-static')
		builders = self.gen_mod_dep (variant)
		for builder in builders:
			dbg_env.Alias ('dbg-static', builder)

		dbg_env = env.Copy ()
		env.Append (CFLAGS=debug_flags,
			    CXXFLAGS=debug_flags,)
		# debug shared support
		variant.static = False
		variant.env = dbg_env
		variant.build_root = os.path.join (self.build_dir, 'dbg-shared')
		builders = self.gen_mod_dep (variant)
		for builder in builders:
			dbg_env.Alias ('dbg-shared', builder)

		env.Alias ('dbg', 'dbg-shared')
		env.Alias ('opt', 'opt-shared')
		env.Default ('dbg')
		env.Alias ('all', ['dbg-shared', 'dbg-static', 'opt-shared', 'opt-static'])


		# dist support
		dist_env = env.Copy ()
		if dist_env['PLATFORM'] == 'posix':
			dist_list = []
			for module in self.__modules:
				for f in module.sources:
					dist_list.append (os.path.join (module.dir, f))
				for f in module.headers:
					dist_list.append (os.path.join (module.dir, f))
				for f in module.inst_headers:
					dist_list.append (os.path.join (module.dir, f))
				for f in module.extra_dist:
					dist_list.append (os.path.join (module.dir, f))
			for f in self.extra_dist:
				dist_list.append (tag, f)
			dist_list.append ('SConstruct')

			targets = []
			basename = self.name + '-' + self.version
			for src in dist_list:
				tgt = os.path.join (basename, src)
				targets.append (dist_env.MyCopyBuilder (target = tgt, source = src))
			tar = basename + '.tar.gz'
			zip = basename + '.zip'
			tmp_tar = os.path.join (self.build_dir, tar)
			tmp_zip = os.path.join (self.build_dir, zip)
			dist_tgt = [tar, zip]
			dist_src = [tmp_tar, tmp_zip]
			dist_env.Tar (tmp_tar, targets)
			dist_env.Zip (tmp_zip, targets)
			dist_builder = dist_env.MyCopyBuilder (target = dist_tgt, source = dist_src)
			dist_env.Alias ('dist', dist_builder)
			dist_env.Append (RM_DIR=basename)
			dist_env.AddPostAction (dist_builder, dist_env.Action (MyRmTree,
									       strfunction = MyRmTreePrint))
			dist_builder = dist_env.MyCopyBuilder (target = dist_tgt, source = dist_src)
			dist_env.Alias ('fastdist', dist_tgt)

			# distcheck
			distcheck_list = []
			for src in dist_list:
				tgt = os.path.join (self.build_dir, basename, src)
				distcheck_list.append (tgt)
			untar = env.Command (distcheck_list, tar,
					     ['cd ' + self.build_dir + ' && tar -zxf ../' + tar])
			scons_dir = os.path.join (self.build_dir, basename)
			distcheck_builder = env.Command ('x',distcheck_list,
							 ['cd ' + scons_dir + ' && scons'])
			env.AlwaysBuild (distcheck_builder)
			env.Alias ('distcheck', distcheck_builder)


yans = Yans ()
yans.build_dir = 'build-dir'
yans.version = '0.0.1'
yans.name = 'yans'


#
# The Core module
#
core = YansModule ('core', 'src/core')
yans.add (core)
core.add_sources ([
        'reference-list-test.cc',
        'callback-test.cc',
        'test.cc'
	])
env = Environment ()
if env['PLATFORM'] == 'posix':
	core.add_external_dep ('pthread')
	core.add_sources ([
		'unix-system-semaphore.cc',
		'unix-system-thread.cc',
		'unix-system-mutex.cc',
		'unix-exec-commands.cc',
		'unix-wall-clock-ms.cc',
		'unix-system-file.cc'
		])
elif env['PLATFORM'] == 'win32':
	core.add_sources ([
		'win32-system-semaphore.cc',
		'win32-system-thread.cc',
		'win32-system-mutex.cc',
		'win32-wall-clock-ms.cc',
		'win32-system-file.cc'
		])
core.add_inst_headers ([
	'system-semaphore.h',
        'system-thread.h',
        'system-mutex.h',
	'system-file.h',
        'exec-commands.h',
        'wall-clock-ms.h',
        'reference-list.h',
        'callback.h',
        'test.h'
	])


#
# The Simu module
#
simu = YansModule ('simulator', 'src/simulator')
yans.add (simu)
simu.add_dep ('core')
simu.add_sources ([
	'scheduler.cc', 
	'scheduler-list.cc',
        'scheduler-heap.cc',
        'scheduler-map.cc',
        'event-impl.cc',
        'event-tcc.cc',
        'event-tcc-test.cc',
        'simulator.cc',
	])
simu.add_headers ([
	'scheduler.h',
	'scheduler-heap.h',
	'scheduler-map.h',
	'scheduler-list.h'
	])
simu.add_inst_headers ([
	'event.h',
	'event-impl.h',
	'simulator.h',
	'event.tcc'
	])

#
# The Common module
#
common = YansModule ('common', 'src/common')
common.add_deps (['core', 'simulator'])
yans.add (common)
common.add_sources ([
	'buffer.cc',
	'mac-address-factory.cc',
	'static-position.cc',
	'chunk.cc',
	'mac-network-interface.cc',
	'static-speed-position.cc',
	'chunk-constant-data.cc',
	'packet.cc',
	'tags.cc',
	'chunk-llc-snap.cc',
	'packet-logger.cc',
	'chunk-utils.cc',
	'pcap-writer.cc',
	'trace-container.cc',
	'population-analysis.cc',
	'traced-variable-test.cc',
	'ipv4-address.cc',
	'position.cc',
	'trace-stream-test.cc',
	'ipv4-network-interface.cc',
	'random-uniform-mrg32k3a.cc',
	'utils.cc',
	'llc-snap-encapsulation.cc',
	'rng-mrg32k3a.cc',
	'mac-address.cc',
	'timeout.cc',
	'seed-generator-mrg32k3a.cc'
	])
common.add_inst_headers ([
	'ipv4-address.h',
	'buffer.h',
	'chunk.h',
	'tags.h',
	'packet.h',
	'ipv4-network-interface.h',
	'count-ptr-holder.tcc',
	'ui-traced-variable.tcc',
	'si-traced-variable.tcc',
	'f-traced-variable.tcc',
	'callback-logger.h',
	'trace-container.h',
	'packet-logger.h',
	'chunk-constant-data.h',
	'mac-address.h',
	'chunk-utils.h',
	'sgi-hashmap.h',
	'llc-snap-encapsulation.h',
	'mac-network-interface.h',
	'population-analysis.h',
	'position.h',
	'random-uniform.h',
	'timeout.h',
	'trace-stream.h',
	'pcap-writer.h',
	'mac-address-factory.h',
	'static-position.h',
	'utils.h'
	])
common.add_headers ([
	'chunk-llc-snap.h',
	'ref-ptr.h',
	'rng-mrg32k3a.h',
	'seed-generator.h',
	'static-speed-position.h'
	])

ipv4 = YansModule ('ipv4', 'src/ipv4')
yans.add (ipv4)
ipv4.add_deps (['core', 'common', 'simulator'])
ipv4.add_sources ([
	'chunk-icmp.cc',
	'chunk-ipv4.cc',
	'chunk-tcp.cc',
	'chunk-udp.cc',
	'defrag-state.cc',
	'ipv4.cc',
	'ipv4-end-point.cc',
	'ipv4-end-points.cc',
	'ipv4-route.cc',
	'tcp-buffer.cc',
	'tcp.cc',
	'tcp-connection.cc',
	'tcp-connection-listener.cc',
	'udp.cc'
	])
ipv4.add_inst_headers ([
	'tcp.h',
	'udp.h',
	'ipv4.h',
	'ipv4-route.h',
	'tcp-connection.h',
	'tcp-connection-listener.h',
	'tag-ipv4.h',
	'ipv4-end-point.h'
	])
ipv4.add_headers ([
	'chunk-icmp.h',
	'chunk-ipv4.h',
	'chunk-tcp.h',
	'chunk-udp.h',
	'defrag-state.h',
	'ipv4-end-points.h',
	'tcp-buffer.h'
	])

arp = YansModule ('arp', 'src/arp')
yans.add (arp)
arp.add_dep ('common')
arp.add_sources ([
	'arp-cache-entry.cc',
	'arp-ipv4-network-interface.cc',
	'chunk-arp.cc'
	])
arp.add_inst_header ('arp-ipv4-network-interface.h')
arp.add_headers ([
	'arp-cache-entry.h',
	'chunk-arp.h'
	])

host = YansModule ('host', 'src/host')
yans.add (host)
host.add_deps (['ipv4', 'arp'])
host.add_sources ([
	'host.cc',
	'loopback-ipv4.cc'
	])
host.add_inst_headers ([
	'host.h'
	])
host.add_headers ([
	'loopback-ipv4.h'
	])

apps = YansModule ('apps', 'src/apps')
yans.add (apps)
apps.add_deps (['ipv4', 'host'])
apps.add_sources ([
	'periodic-generator.cc',
	'tcp-sink.cc',
	'tcp-source.cc',
	'throughput-printer.cc',
	'traffic-analyser.cc',
	'udp-sink.cc',
	'udp-source.cc'
	])
apps.add_inst_headers ([
	'periodic-generator.h',
	'tcp-sink.h',
	'tcp-source.h',
	'throughput-printer.h',
	'traffic-analyser.h',
	'udp-sink.h',
	'udp-source.h'
	])

wifi = YansModule ('80211', 'src/80211')
yans.add (wifi)
wifi.add_deps (['core', 'common', 'simulator'])
wifi.add_sources ([
	'chunk-mac-80211-hdr.cc',
	'chunk-mac-80211-fcs.cc',
	'mac-stations.cc',
	'mac-station.cc',
	'arf-mac-stations.cc',
	'aarf-mac-stations.cc',
	'cr-mac-stations.cc',
	'ideal-mac-stations.cc',
	'propagation-model.cc',
	'base-channel-80211.cc',
	'channel-80211.cc',
	'transmission-mode.cc',
	'bpsk-mode.cc',
	'qam-mode.cc',
	'phy-80211.cc',
	'network-interface-80211-simple.cc',
	'network-interface-80211-simple-factory.cc',
	'mac-simple.cc',
	'mac-low.cc',
	'mac-parameters.cc',
	'dcf.cc',
	'mac-tx-middle.cc',
	'mac-rx-middle.cc',
	'dca-txop.cc',
	'mac-queue-80211e.cc',
	'mac-high-adhoc.cc',
	'ssid.cc',
	'supported-rates.cc',
	'capability-information.cc',
	'status-code.cc',
	'chunk-mgt.cc',
	'mac-high-nqsta.cc',
	'mac-high-nqap.cc',
	'network-interface-80211.cc',
	'network-interface-80211-factory.cc',
	])
wifi.add_inst_headers ([
	'network-interface-80211-simple.h',
	'network-interface-80211-simple-factory.h',
	'network-interface-80211.h',
	'network-interface-80211-factory.h',
	'base-channel-80211.h',
	'channel-80211.h',
	'propagation-model.h',
	'ssid.h',
	'chunk-mac-80211-hdr.h'
])
wifi.add_headers ([
	'chunk-mac-80211-fcs.h',
	'mac-stations.h',
	'mac-station.h',
	'arf-mac-stations.h',
	'aarf-mac-stations.h',
	'cr-mac-stations.h',
	'ideal-mac-stations.h',
	'transmission-mode.h',
	'bpsk-mode.h',
	'qam-mode.h',
	'phy-80211.h',
	'mac-simple.h',
	'mac-low.h',
	'mac-parameters.h',
	'dcf.h',
	'mac-tx-middle.h',
	'mac-rx-middle.h',
	'mac-queue-80211e.h',
	'dca-txop.h',
	'mac-high-adhoc.h',
	'supported-rates.h',
	'capability-information.h',
	'status-code.h',
	'chunk-mgt.h',
	'mac-high-nqsta.h',
	'mac-high-nqap.h'
	])

eth = YansModule ('ethernet', 'src/ethernet')
yans.add (eth)
eth.add_deps (['core', 'common', 'simulator'])
eth.add_sources ([
	'cable.cc',
	'chunk-mac-crc.cc',
	'chunk-mac-eth.cc',
	'ethernet-network-interface.cc'
	])
eth.add_headers ([
	'chunk-mac-crc.h',
	'chunk-mac-eth.h',	
	])
eth.add_inst_headers ([
	'cable.h',
	'ethernet-network-interface.h'
	])


# utils
run_tests = YansModule ('run-tests', 'utils')
yans.add (run_tests)
run_tests.set_executable ()
run_tests.add_deps (['core', 'simulator', 'common', 'ipv4', 'arp', 'ethernet', '80211'])
run_tests.add_source ('run-tests.cc')

bench_packets = YansModule ('bench-packets', 'utils')
#yans.add (bench_packets)
bench_packets.set_executable ()
bench_packets.add_dep ('core')
bench_packets.add_source ('bench-packets.cc')

bench_simu = YansModule ('bench-simulator', 'utils')
yans.add (bench_simu)
bench_simu.set_executable ()
bench_simu.add_dep ('simulator')
bench_simu.add_source ('bench-simulator.cc')


# samples
main_callback = YansModule ('main-callback', 'samples')
main_callback.set_executable ()
yans.add (main_callback)
main_callback.add_dep ('core')
main_callback.add_source ('main-callback.cc')

main_event = YansModule ('main-event', 'samples')
main_event.set_executable ()
yans.add (main_event)
main_event.add_dep ('simulator')
main_event.add_source ('main-event.cc')

main_trace = YansModule ('main-trace', 'samples')
yans.add (main_trace)
main_trace.add_dep ('common')
main_trace.set_executable ()
main_trace.add_source ('main-trace.cc')

main_simu = YansModule ('main-simulator', 'samples')
yans.add (main_simu)
main_simu.set_executable ()
main_simu.add_dep ('simulator')
main_simu.add_source ('main-simulator.cc')

main_packet = YansModule ('main-packet', 'samples')
yans.add (main_packet)
main_packet.set_executable ()
main_packet.add_dep ('common')
main_packet.add_source ('main-packet.cc')

main_simple = YansModule('main-simple', 'samples')
yans.add (main_simple)
main_simple.set_executable ()
main_simple.add_deps (['ipv4', 'apps', 'common', 'ethernet'])
main_simple.add_source ('main-simple.cc')

main_router = YansModule('main-router', 'samples')
yans.add (main_router)
main_router.set_executable ()
main_router.add_deps (['ipv4', 'apps', 'common', 'ethernet'])
main_router.add_source ('main-router.cc')

mainwifi_simple = YansModule ('main-80211-simple', 'samples')
yans.add (mainwifi_simple)
mainwifi_simple.set_executable ()
mainwifi_simple.add_deps (['ipv4', 'apps', 'common', '80211'])
mainwifi_simple.add_source ('main-80211-simple.cc')

mainwifi_adhoc = YansModule ('main-80211-adhoc', 'samples')
yans.add (mainwifi_adhoc)
mainwifi_adhoc.set_executable ()
mainwifi_adhoc.add_deps (['ipv4', 'apps', 'common', '80211'])
mainwifi_adhoc.add_source ('main-80211-adhoc.cc')

mainwifi_ap = YansModule ('main-80211-ap', 'samples')
yans.add (mainwifi_ap)
mainwifi_ap.set_executable ()
mainwifi_ap.add_deps (['ipv4', 'apps', 'common', '80211'])
mainwifi_ap.add_source ('main-80211-ap.cc')



yans.generate_dependencies ()



