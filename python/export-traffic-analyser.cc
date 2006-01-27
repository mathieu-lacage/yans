/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <boost/python.hpp>
#include "traffic-analyser.h"
#include "export-callback.tcc"
#include "callback.tcc"
#include "packet.h"

using namespace boost::python;
using namespace yans;

class TrafficAnalyser_receive_callback : public Callback<void (Packet *)> {
public:
	TrafficAnalyser_receive_callback (TrafficAnalyser *self)
		: m_self (self) {}

	virtual void operator() (Packet *a1) {
		m_self->receive (a1);
	}
private:
	TrafficAnalyser *m_self;
};
#include <iostream>
template<typename TAG, typename R, typename T, typename T1>
struct CallbackGenerator {
	typedef R (T::*member_type) (T1);
	static Callback<R (T1)> *
	make_callback (T *self){
		std::cout << "create callback" << std::endl;
		return yans::make_callback (CallbackGenerator::m_member, self);
	}
	static member_type m_member;
};
template<typename TAG,typename R,typename T,typename T1>
R (T::* CallbackGenerator<TAG,R,T,T1>::m_member ) (T1);

template <typename TAG, typename R, typename T, typename T1>
void
export_method_as_callback (R (T::*member) (T1), char const *name)
{
	CallbackGenerator<TAG, R, T, T1>::m_member = member;
	def (name, &CallbackGenerator<TAG,R,T,T1>::make_callback, return_value_policy<manage_new_object> ());
	
}


void export_traffic_analyser (void)
{
	class_<TrafficAnalyser, boost::noncopyable> analyser ("TrafficAnalyser");
	analyser.def ("print_stats", &TrafficAnalyser::print_stats);
	analyser.def ("receive", &TrafficAnalyser::receive);

	export_method_as_callback<struct traffic_analyser_receive_callback> (&TrafficAnalyser::receive, 
									     "make_TrafficAnalyser_receive_callback");
}
