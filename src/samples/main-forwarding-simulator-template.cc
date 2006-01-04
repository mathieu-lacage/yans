/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
#include "event.h"
#include "event.tcc"
#include "simulator.h"
#include <iostream>

using namespace yans;

class MyModel {
public:
	void start (void);
private:
	void deal_with_event (double event_value);
};

void 
MyModel::start (void)
{
	Simulator::insert_at_s (10.0, make_event (&MyModel::deal_with_event, 
						  this, Simulator::now_s ()));
}
void
MyModel::deal_with_event (double value)
{
	std::cout << "Received event at " << Simulator::now_s () << " started at " << value << std::endl;
}


int main (int argc, char *argv[])
{
	MyModel model;

	model.start ();

	Simulator::run ();

	Simulator::destroy ();
}
