
import os.path
import shutil

class Ns3Module:
	def __init__ (self, name, dir):
		self.sources = []
		self.inst_headers = []
		self.headers = []
		self.deps = []
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
	def add_dep (self, dep):
		self.deps.append (dep)
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

def InstallHeader (target, source, env):
	shutil.copy (source[0].path, target[0].path)

class Ns3BuildVariant:
	def __init__ (self):
		self.static = False
		self.opti = False
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
	def __get_static_output_file (self, module):
		if module.executable:
			return os.path.join ('bin', module.get_static_output_file ())
		else:
			return os.path.join ('lib', module.get_static_output_file ())
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
			if variant.opti:
				cxx_flags = '-O3'
				c_flags = '-O3'
			if variant.static:
				obj_builder = env.StaticObject (target = tgt, source = src,
								CPPFLAGS=cpp_flags,
								CXXFLAGS=cxx_flags,
								CFLAGS=c_flags)
			else:
				obj_builder = env.SharedObject (target = tgt, source = src,
								CPPFLAGS=cpp_flags,
								CXXFLAGS=cxx_flags,
								CFLAGS=c_flags)
			objects.append (obj_builder)
		return objects
	def gen_mod_dep (self, env, variant):
		build_root = variant.build_root
		include_dir = os.path.join (build_root, 'include', 'yans')
		lib_path = os.path.join (build_root, 'lib')
		cpp_flags = '-I' + os.path.join (build_root, 'include')
		module_builders = []
		for module in self.__modules:
			objects = self.get_obj_builders (env, variant, module)
			if module.executable:
				filename = os.path.join (build_root, 'bin', module.name)
				libs = ''
				for dep_name in module.deps:
					libs = libs + ' ' + dep_name
				module_builder = env.Program (target = filename, source = objects,
							      LIBPATH=lib_path, LIBS=libs)
			else:
				filename = os.path.join (build_root, 'lib', 'lib' + module.name )
				if variant.static:
					filename = filename + '.a'
					module_builder = env.StaticLibrary (target = filename, source = objects)
				else:
					filename = filename + '.so'
					module_builder = env.SharedLibrary (target = filename, source = objects)
					
			for dep_name in module.deps:
				dep = self.__get_module (dep_name)
				env.Depends (module_builder, self.get_mod_output (dep, variant))
					
			for header in module.inst_headers:
				tgt = os.path.join (include_dir, header)
				src = os.path.join (module.dir, header)
				#builder = env.Install (target = tgt, source = src)
				header_builder = env.HeaderBuilder (target = tgt, source = src)
				env.Depends (module_builder, header_builder)
				
			module_builders.append (module_builder)
		return module_builders
		
	def generate_dependencies (self):
		flags = '-g3 -Wall -Werror'
		env = Environment (CFLAGS=flags,CXXFLAGS=flags)
		header_builder = Builder (action = Action (InstallHeader))
		env.Append (BUILDERS = {'HeaderBuilder':header_builder})
		variant = Ns3BuildVariant ()
		builders = []
		
		variant.opti = True
		variant.static = True
		variant.build_root = os.path.join (self.build_dir, 'opt', 'static')
		builders = self.gen_mod_dep (env, variant)
		for builder in builders:
			env.Alias ('opt-static', builder)

		variant.opti = True
		variant.static = False
		variant.build_root = os.path.join (self.build_dir, 'opt', 'shared')
		builders = self.gen_mod_dep (env, variant)
		for builder in builders:
			env.Alias ('opt-shared', builder)

		variant.opti = False
		variant.static = True
		variant.build_root = os.path.join (self.build_dir, 'dbg', 'static')
		builders = self.gen_mod_dep (env, variant)
		for builder in builders:
			env.Alias ('dbg-static', builder)

		variant.opti = False
		variant.static = False
		variant.build_root = os.path.join (self.build_dir, 'dbg', 'shared')
		builders = self.gen_mod_dep (env, variant)
		for builder in builders:
			env.Alias ('dbg-shared', builder)

		env.Alias ('dbg', 'dbg-shared')
		env.Alias ('opt', 'opt-shared')
		env.Default ('dbg')


ns3 = Ns3 ()
ns3.build_dir = 'build'

core = Ns3Module ('core', 'src/core')
ns3.add (core)
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



