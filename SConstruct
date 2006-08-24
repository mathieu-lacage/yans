
import os.path
import shutil

class Ns3Module:
	def __init__ (self, name, dir):
		self.sources = []
		self.inst_headers = []
		self.headers = []
		self.deps = []
		self.external_deps = []
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
	shutil.copy (source[0].path, target[0].path)
def GcxxEmitter (target, source, env):
	if os.path.exists (source[0].path):
		return [target, source]
	else:
		return [[], []]
	

class Ns3BuildVariant:
	def __init__ (self):
		self.static = False
		self.opti = False
		self.gcxx_deps = False
		self.gcxx_root = ''
		self.build_root = ''

class Ns3:
	def __init__ (self):
		self.__modules = []
		self.build_dir = 'build'
	def add (self, module):
		self.__modules.append (module)
	def __get_module (self, name):
		for module in self.__modules:
			if module.name == name:
				return module
		return None
	def get_mod_output (self, module, variant):
		if module.executable:
			filename = os.path.join (variant.build_root, 'bin', module.name)
		else:
			filename = os.path.join (variant.build_root, 'lib', 'lib' + module.name )
			if variant.static:
				filename = filename + '.a'
			else:
				filename = filename + '.so'
		return filename
	def get_obj_builders (self, env, variant, module):
		cpp_flags = '-I' + os.path.join (variant.build_root, 'include')
		objects = []
		for source in module.sources:
			obj_file = os.path.splitext (source)[0] + '.o'
			tgt = os.path.join (variant.build_root, module.dir, obj_file)
			src = os.path.join (module.dir, source)
			cxx_flags = ''
			c_flags = ''
			if variant.static:
				obj_builder = env.StaticObject (target = tgt, source = src,
								CPPFLAGS=cpp_flags,
								CXXFLAGS=env['CXXFLAGS'] + ' ' + cxx_flags,
								CFLAGS=env['CFLAGS'] + ' ' + c_flags)
			else:
				obj_builder = env.SharedObject (target = tgt, source = src,
								CPPFLAGS=cpp_flags,
								CXXFLAGS=env['CXXFLAGS'] + ' ' + cxx_flags,
								CFLAGS=env['CFLAGS'] + ' ' + c_flags)
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
	def get_all_deps (self, module, hash):
		for dep_name in module.deps:
			hash[dep_name] = 1
			dep = self.__get_module (dep_name)
			self.get_all_deps (dep, hash)
		for dep_name in module.external_deps:
			hash[dep_name] = 1
	def gen_mod_dep (self, env, variant):
		build_root = variant.build_root
		include_dir = os.path.join (build_root, 'include', 'yans')
		lib_path = os.path.join (build_root, 'lib')
		cpp_flags = '-I' + os.path.join (build_root, 'include')
		module_builders = []
		for module in self.__modules:
			objects = self.get_obj_builders (env, variant, module)
			all_deps = {}
			self.get_all_deps (module, all_deps)
			libs = all_deps.keys ()

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
				tgt = os.path.join (include_dir, header)
				src = os.path.join (module.dir, header)
				#builder = env.Install (target = tgt, source = src)
				header_builder = env.MyCopyBuilder (target = tgt, source = src)
				env.Depends (module_builder, header_builder)
				
			module_builders.append (module_builder)
		return module_builders
		
	def generate_dependencies (self):
		flags = '-g3 -Wall -Werror'
		env = Environment (CFLAGS=flags,
				   CXXFLAGS=flags,
				   CPPDEFINES=['RUN_SELF_TESTS'])
		header_builder = Builder (action = Action (MyCopyAction))
		env.Append (BUILDERS = {'MyCopyBuilder':header_builder})
		gcxx_builder = Builder (action = Action (MyCopyAction), emitter = GcxxEmitter)
		env.Append (BUILDERS = {'CopyGcxxBuilder':gcxx_builder})
		variant = Ns3BuildVariant ()
		builders = []
		

		gcov_env = env.Copy ()
		gcov_env.Append (CFLAGS=' -fprofile-arcs -ftest-coverage',
				 CXXFLAGS=' -fprofile-arcs -ftest-coverage',
				 LINKFLAGS='-fprofile-arcs')
		# code coverage analysis
		variant.opti = False
		variant.static = False
		variant.build_root = os.path.join (self.build_dir, 'gcov')
		builders = self.gen_mod_dep (gcov_env, variant)
		for builder in builders:
			gcov_env.Alias ('gcov', builder)


		opt_env = env.Copy ()
		opt_env.Append (CFLAGS=' -O3',
				CXXFLAGS=' -O3',
				CPPDEFINES=['NDEBUG'])
		# optimized static support
		variant.opti = True
		variant.static = True
		variant.build_root = os.path.join (self.build_dir, 'opt', 'static')
		builders = self.gen_mod_dep (opt_env, variant)
		for builder in builders:
			opt_env.Alias ('opt-static', builder)


		# optimized shared support
		variant.opti = True
		variant.static = False
		variant.build_root = os.path.join (self.build_dir, 'opt', 'shared')
		builders = self.gen_mod_dep (opt_env, variant)
		for builder in builders:
			opt_env.Alias ('opt-shared', builder)


		arc_env = opt_env.Copy ()
		arc_env.Append (CFLAGS=' -frandom-seed=0 -fprofile-generate',
				CXXFLAGS=' -frandom-seed=0 -fprofile-generate',
				LINKFLAGS=' -frandom-seed=0 -fprofile-generate')
		# arc profiling
		variant.opti = True
		variant.static = False
		variant.build_root = os.path.join (self.build_dir, 'opt-arc')
		builders = self.gen_mod_dep (arc_env, variant)
		for builder in builders:
			arc_env.Alias ('opt-arc', builder)


		arcrebuild_env = opt_env.Copy ()
		arcrebuild_env.Append (CFLAGS=' -frandom-seed=0 -fprofile-use',
				       CXXFLAGS=' -frandom-seed=0 -fprofile-use',
				       LINKFLAGS=' -frandom-seed=0 -fprofile-use')
		# arc rebuild
		variant.opti = True
		variant.static = False
		variant.gcxx_deps = True
		variant.gcxx_root = os.path.join (self.build_dir, 'opt-arc')
		variant.build_root = os.path.join (self.build_dir, 'opt-arc-rebuild')
		builders = self.gen_mod_dep (arcrebuild_env, variant)
		for builder in builders:
			arcrebuild_env.Alias ('opt-arc-rebuild', builder)
		variant.gcxx_deps = False




		dbg_env = env.Copy ()
		# debug static support
		variant.opti = False
		variant.static = True
		variant.build_root = os.path.join (self.build_dir, 'dbg', 'static')
		builders = self.gen_mod_dep (dbg_env, variant)
		for builder in builders:
			dbg_env.Alias ('dbg-static', builder)

		# debug shared support
		variant.opti = False
		variant.static = False
		variant.build_root = os.path.join (self.build_dir, 'dbg', 'shared')
		builders = self.gen_mod_dep (dbg_env, variant)
		for builder in builders:
			dbg_env.Alias ('dbg-shared', builder)

		env.Alias ('dbg', 'dbg-shared')
		env.Alias ('opt', 'opt-shared')
		env.Default ('dbg')
		env.Alias ('all', ['dbg-shared', 'dbg-static', 'opt-shared', 'opt-static'])


ns3 = Ns3 ()
ns3.build_dir = 'build'

core = Ns3Module ('core', 'src/core')
ns3.add (core)
core.add_external_dep ('pthread')
core.add_sources ([
	'unix-system-semaphore.cc',
        'unix-system-thread.cc',
        'unix-system-mutex.cc',
        'unix-exec-commands.cc',
        'unix-wall-clock-ms.cc',
        'reference-list-test.cc',
        'callback-test.cc',
        'test.cc'
	])
core.add_inst_headers ([
	'system-semaphore.h',
        'system-thread.h',
        'system-mutex.h',
        'exec-commands.h',
        'wall-clock-ms.h',
        'reference-list.h',
        'callback.h',
        'test.h'
	])

simu = Ns3Module ('simulator', 'src/simulator')
ns3.add (simu)
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

run_tests = Ns3Module ('run-tests', 'utils')
ns3.add (run_tests)
run_tests.set_executable ()
run_tests.add_deps (['core', 'simulator'])
run_tests.add_source ('run-tests.cc')

main_callback = Ns3Module ('main-callback', 'samples')
main_callback.set_executable ()
ns3.add (main_callback)
main_callback.add_dep ('core')
main_callback.add_source ('main-callback.cc')

main_event = Ns3Module ('main-event', 'samples')
main_event.set_executable ()
ns3.add (main_event)
main_event.add_dep ('simulator')
main_event.add_source ('main-event.cc')


ns3.generate_dependencies ()



