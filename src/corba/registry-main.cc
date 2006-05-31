#include "registry_impl.h"

Registry *
registry_run (int argc, char *argv[])
{
  CORBA::ORB_var orb = CORBA::ORB_init (argc, argv, "mico-local-orb");
  CORBA::Object_var poa_obj = orb->resolve_initial_references ("RootPOA");
  PortableServer::POA_var poa = PortableServer::POA::_narrow (poa_obj);
  PortableServer::POAManager_var manager = poa->the_POAManager ();

  Registry_impl *servant = new Registry_impl ();

  PortableServer::ObjectId_var object_id = poa->activate_object (servant);

  manager->activate ();

  //std::cout << "servant="<< orb->object_to_string () << std::endl;

  orb->run ();

  poa->destroy (TRUE, TRUE);
  delete servant;
  
  return 0;
}
