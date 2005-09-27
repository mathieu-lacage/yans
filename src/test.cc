/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "test.h"
#include <iostream>
#include "buffer.h"

#ifdef RUN_SELF_TESTS

#define ADD_TEST(klass, name) 	m_tests.push_back (std::make_pair (new klass (this), new std::string (name)));

TestManager::TestManager ()
	: m_verbose (false)
{
	ADD_TEST (BufferTest, "Buffer");
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
void 
TestManager::report_error (std::string *str)
{
	std::cerr << "Test failed: \"" << *m_test_name << "\" " << str << std::endl;
}
void 
TestManager::report_ok (std::string *str)
{
	if (m_verbose) {
		std::cerr << "Test ok: \"" << *m_test_name << "\" " << str << std::endl;
	}
}
bool 
TestManager::run_tests (void)
{
	bool is_success = true;
	for (TestsCI i = m_tests.begin (); i != m_tests.end (); i++) {
		m_test_name = (*i).second;
		if (!(*i).first->run_tests ()) {
			is_success = false;
			std::cerr << "Test \"" << *m_test_name << "\" failed." << std::endl;
		}
		if (m_verbose) {
			std::cerr << "Test \"" << *m_test_name << "\" success." << std::endl;
		}
	}
	return is_success;
}

Test::Test (TestManager *manager)
	: m_manager (manager)
{}
Test::~Test ()
{}

void 
Test::report_error (std::string *str)
{
	m_manager->report_error (str);
}
void 
Test::report_ok (std::string *str)
{
	m_manager->report_ok (str);
}

#endif /* RUN_SELF_TESTS */
