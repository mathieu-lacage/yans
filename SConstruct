
import os.path

class Ns3Module:
	def __init__ (self, name, dir):
		self.sources = []
		self.inst_headers = []
		self.headers = []
		self.deps = []
		self.name = name
		self.dir = dir
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
	def get_static_output_file (self):
		full_name = 'lib' + self.name + '.a'
		return full_name
	def get_shared_output_file (self):
		full_name = 'lib' + self.name + '.so'
		return full_name

class Ns3App:
	def __init__ (self, name, dir):
		self.sources = []
		self.headers = []
		self.deps = []
		self.name = name
		self.dir = dir
	def add_dep (self, dep):
		self.deps.append (dep)
	def add_source (self, source):
		self.sources.append (source)
	def add_sources (self, sources):
		self.sources.append (sources)
	def add_header (self, header):
		self.headers.append (header)
	def get_shared_output_file (self):
		return self.name
	def get_static_output_file (self):
		return self.name

class Ns3:
	def __init__ (self):
		self.__modules = []
		self.build_dir = 'build'
	def add (self, module):
		self.__modules.append (module)
	# return order in which modules must be built
	def sort_modules (self):
		modules = []
		for module in self.__modules:
			copy = module.get_deps ()
			modules.append ([module, copy])
		sorted = []
		while len (modules) > 0:
			to_remove = []
			# identify which modules to remove
			for module in modules:
				if len (module[1]) == 0:
					to_remove.append (module)
			# remove identified modules
			for to_rem in to_remove:
				modules.remove (to_rem)
			# update modules left
			for module in modules:
				for to_rem in to_remove:
					if to_rem[0].name in module[1]:
						module[1].remove (to_rem[0].name)
			sorted.extend (to_remove)
		retval = []
		for i in sorted:
			retval.append (i[0])
		return retval
	def __get_module (self, name):
		for module in self.__modules:
			if module.name == name:
				return module
		return None
	def generate_dependencies (self):
		flags = '-g3 -Wall -Werror'
		env = Environment (CFLAGS=flags,CXXFLAGS=flags)
		build_dir = os.path.join (self.build_dir, 'opt', 'static')
		for module in self.__modules:
			objects = []
			include_list = ''
			for dep_name in module.deps:
				dep = self.__get_module (dep_name)
				include_list = include_list + '-I' + dep.dir + ' '
			for source in module.sources:
				obj_file = os.path.splitext (source)[0] + '.o'
				tgt = os.path.join (build_dir, module.dir, obj_file)
				src = os.path.join (module.dir, source)
				obj = env.StaticObject (target = tgt, source = src, CPPFLAGS=include_list)
				objects.append (obj)
			filename = os.path.join (build_dir, module.dir, module.get_static_output_file ())
			library = env.StaticLibrary (target = filename, source = objects)
			for dep_name in module.deps:
				dep = self.__get_module (dep_name)
				filename = os.path.join (build_dir, dep.dir, dep.get_static_output_file ())
				env.Depends (library, filename)
			env.Alias ('opt-static', library)
		return

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
        'reference-list.tcc',
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


main_callback = Ns3App ('main-callback', 'samples')
ns3.add (main_callback)
main_callback.add_dep ('core')
main_callback.add_source ('main-callback.cc')

main_event = Ns3App ('main-event', 'samples')
ns3.add (main_event)
main_event.add_dep ('simulator')
main_event.add_source ('main-event.cc')


ns3.generate_dependencies ()



