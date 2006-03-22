/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
#include "yans/event.h"
#include "yans/simulator.h"
#include <iostream>

using namespace yans;


class MyModel {
public:
	void start (void);
	void deal_with_event (double event_value);
};
class MyEvent : public Event {
public:
        MyEvent (MyModel *model, double event_value);
	virtual void notify (void);
private:
	MyModel *m_model;
	double m_value;
};


MyEvent::MyEvent (MyModel *model, double event_value)
	: m_model (model),
	  m_value (event_value)
{}
void
MyEvent::notify (void)
{
	m_model->deal_with_event (m_value);
	delete this;
}

void 
MyModel::start (void)
{
	Simulator::insert_at_s (10.0, new MyEvent (this, Simulator::now_s ()));
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
