/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <stdint.h>
#include <string>

#include "yans/system-thread.h"
#include "yans/system-semaphore.h"
#include "yans/exec-commands.h"

#include "registry_impl.h"

using namespace yans;

class StartServers : public SystemThread {
public:
	StartServers (char const *registry_ref, Registry_var registry, char const *filename);
	virtual ~StartServers ();
	void registered (void);
private:
	virtual void real_run (void);
	std::string m_name;
	SystemSemaphore m_wait_started;
	Registry_var m_registry;
};

StartServers::StartServers (char const *name, Registry_var registry, char const *filename)
	: m_name (name),
	  m_wait_started (0),
	  m_registry (registry)
{}
StartServers::~StartServers ()
{}

void
StartServers::registered (void)
{
	std::cout << "register"<<std::endl;
	m_wait_started.post ();
}

void
StartServers::real_run (void)
{
	std::string registry_str = std::string ("--registry=");
	registry_str.append (m_name);
  	ExecCommands commands = ExecCommands (1);
	Command a;
	Command b;
	a.append ("./build-dir/bin/context-server");
	a.append ("--name=a");
	a.append (registry_str);
	commands.add (a, "a");
	b.append ("./build-dir/bin/context-server");
	b.append ("--name=b");
	b.append (registry_str);
	commands.add (b, "b");

	commands.enable_log ("main.log");
	commands.start ();
	m_wait_started.wait (2);
	std::cout << "waited"<<std::endl;
#if 1
	try {
		::Remote::ComputingContext_var a_d, b_d;
		a_d = m_registry->lookup ("a");
		if (!CORBA::is_nil (a_d)) {
			//
		}
		b_d = m_registry->lookup ("b");
		if (!CORBA::is_nil (b_d)) {
			//
		}
		std::cout << "completed ping" << std::endl;
		m_registry->shutdown_recorded ();
		m_registry->shutdown ();
		std::cout << "completed shutdown" << std::endl;
	} catch (...) {
		std::cout << "exception" << std::endl;
	}
#endif
}


int
main (int argc, char *argv[])
{
	CORBA::ORB_var orb = CORBA::ORB_init (argc, argv, "mico-local-orb");
	CORBA::Object_var poa_obj = orb->resolve_initial_references ("RootPOA");
	PortableServer::POA_var poa = PortableServer::POA::_narrow (poa_obj);
	PortableServer::POAManager_var manager = poa->the_POAManager ();

	Registry_impl *servant = new Registry_impl (orb);

	PortableServer::ObjectId_var object_id = poa->activate_object (servant);
 
	manager->activate ();

	Registry_var registry = servant->_this ();
	CORBA::String_var ref = orb->object_to_string (registry);
	StartServers *servers = new StartServers (ref, registry, "filename.xml");
	servant->set_callback (make_callback (&StartServers::registered, servers));

	//std::cout << "servant="<< ref << std::endl;

	orb->run ();

	poa->destroy (TRUE, TRUE);
	delete servant;
	delete servers; // XXX
  
	return 0;
}
