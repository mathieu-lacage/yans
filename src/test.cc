/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "test.h"

#ifdef RUN_SELF_TESTS
#include <iostream>
#include "buffer.h"
#include "fiber-stack.h"
#include "fiber-scheduler.h"


#define ADD_TEST(klass, name) 	m_tests.push_back (std::make_pair (new klass (this), new std::string (name)));

TestManager::TestManager ()
	: m_verbose (false)
{
	ADD_TEST (BufferTest, "Buffer");
	ADD_TEST (FiberStackTest, "FiberStack");
	ADD_TEST (TestFiberScheduler, "FiberScheduler");
}

TestManager::~TestManager ()
{
	TestsI i = m_tests.begin ();
	while (i != m_tests.end ()) {
		delete (*i).first;
		delete (*i).second;
		i = m_tests.erase (i);
	}
}
void
TestManager::enable_verbose (void)
{
	m_verbose = true;
}
std::ostream &
TestManager::failure (void)
{
	return std::cerr;
}
bool 
TestManager::run_tests (void)
{
	bool is_success = true;
	for (TestsCI i = m_tests.begin (); i != m_tests.end (); i++) {
		m_test_name = (*i).second;
		if (!(*i).first->run_tests ()) {
			is_success = false;
		} else {
			if (m_verbose) {
				std::cerr << "Test \"" << *m_test_name << "\" success." << std::endl;
			}
		}
	}
	return is_success;
}

Test::Test (TestManager *manager)
	: m_manager (manager)
{}
Test::~Test ()
{}

std::ostream &
Test::failure (void)
{
	return m_manager->failure ();
}

#endif /* RUN_SELF_TESTS */
