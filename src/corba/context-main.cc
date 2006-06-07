#include "context_impl.h"
#include "registry.h"

int main (int argc, char *argv[])
{
  char *name = 0;
  char *registry_str = 0;
  for (int i = 0; i < argc; i++) {
    if (strncmp (argv[i], "--name=", strlen ("--name=")) == 0) {
      name = strdup (argv[i] + strlen ("--name="));
    } else if (strncmp (argv[i], "--registry=", strlen ("--registry=")) == 0) {
      registry_str = strdup (argv[i] + strlen ("--registry="));
    }
  }

  CORBA::ORB_var orb = CORBA::ORB_init (argc, argv, "mico-local-orb");
  CORBA::Object_var poa_obj = orb->resolve_initial_references ("RootPOA");
  PortableServer::POA_var poa = PortableServer::POA::_narrow (poa_obj);
  PortableServer::POAManager_var manager = poa->the_POAManager ();

  ComputingContext_impl *servant = new ComputingContext_impl (orb);

  PortableServer::ObjectId_var object_id = poa->activate_object (servant);

  manager->activate ();

  CORBA::Object_var registry_obj = orb->string_to_object (registry_str);
  Registry_var registry = Registry::_narrow (registry_obj);
  registry->record (servant->_this (), name);

  std::cout << "servant="<< orb->object_to_string (servant->_this ()) << std::endl;

  orb->run ();

  poa->destroy (TRUE, TRUE);
  delete servant;
  
  return 0;
}