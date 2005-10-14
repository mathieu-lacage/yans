/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "host.h"
#include "tag-manager.h"

int main (int argc, char *argv[])
{
	Host *hclient, *hserver;
	hclient = new Host ("client");
	hserver = new Host ("server");
	

	delete hclient;
	delete hserver;

	TagManager::instance ()->destroy ();
	return 0;
}
