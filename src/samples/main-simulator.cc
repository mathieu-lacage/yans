#include "event.h"
#include "simulator.h"
#include <iostream>

using namespace yans;

class MyEvent : public Event {
public:
        MyEvent ();
	virtual void notify (void);
};

MyEvent::MyEvent () 
{}

void
MyEvent::notify (void)
{
        std::cout << "notified at " << Simulator::now_s () << "s" << std::endl;
	delete this;
}

int main (int argc, char *argv[])
{
        Simulator::insert_at_s (10.0, new MyEvent ());

	Simulator::run ();

	Simulator::destroy ();
}
