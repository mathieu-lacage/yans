/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
#include "yans/event.h"
#include "yans/event.tcc"
#include <iostream>

using namespace yans;

class MyModel {
public:
	void deal_with_event (double event_value);
};

void
MyModel::deal_with_event (double value)
{
	std::cout << "Member method received event." << std::endl;
}

static void 
random_function (void)
{
	std::cout << "Function received event." << std::endl;
}


int main (int argc, char *argv[])
{
	Event ev;
	ev = make_event (&random_function);
	ev ();
	MyModel model;
	ev = make_event (&MyModel::deal_with_event, &model, 10.0);
	ev ();
}
