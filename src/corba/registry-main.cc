/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <stdint.h>
#include <string>

#include "yans/system-thread.h"
#include "yans/exec-commands.h"

#include "registry_impl.h"

using namespace yans;

class StartServers : public SystemThread {
public:
  StartServers (char const *name);
  virtual ~StartServers ();
private:
  virtual void real_run (void);
  void null_cb (char const *buffer, uint32_t size);
  std::string m_name;
};

StartServers::StartServers (char const *name)
  : m_name (name)
{}
StartServers::~StartServers ()
{}

void
StartServers::null_cb (char const *data, uint32_t size)
{}

void
StartServers::real_run (void)
{
	std::string registry_str = std::string ("--registry=");
	registry_str.append (m_name);
  	ExecCommands commands = ExecCommands (2);
	Command a;
	Command b;
	a.append ("./build-dir/bin/echo-server");
	a.append ("--name=a");
	a.append (registry_str);
	commands.add (a, make_callback (&StartServers::null_cb, this));
	b.append ("./build-dir/bin/echo-server");
	b.append ("--name=b");
	b.append (registry_str);
	commands.add (b, make_callback (&StartServers::null_cb, this));
	commands.start_and_wait ();
}


int
main (int argc, char *argv[])
{
  CORBA::ORB_var orb = CORBA::ORB_init (argc, argv, "mico-local-orb");
  CORBA::Object_var poa_obj = orb->resolve_initial_references ("RootPOA");
  PortableServer::POA_var poa = PortableServer::POA::_narrow (poa_obj);
  PortableServer::POAManager_var manager = poa->the_POAManager ();

  Registry_impl *servant = new Registry_impl ();

  PortableServer::ObjectId_var object_id = poa->activate_object (servant);
 
  manager->activate ();

  CORBA::String_var ref = PortableServer::ObjectId_to_string (object_id);
  StartServers *servers = new StartServers (ref);

  //std::cout << "servant="<< orb->object_to_string () << std::endl;

  orb->run ();

  poa->destroy (TRUE, TRUE);
  delete servant;
  delete servers; // XXX
  
  return 0;
}
