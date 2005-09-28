/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef TEST_H
#define TEST_H

#include <list>
#include <string>
#include <utility>
#include <ostream>

class TestManager;

#define RUN_SELF_TESTS 1

class Test {
public:
	Test (TestManager *manager);
	virtual ~Test ();

	virtual bool run_tests (void) = 0;

	std::ostream &failure (void);
private:
	TestManager *m_manager;
};

class TestManager {
public:
	TestManager ();
	~TestManager ();

	void enable_verbose (void);
	bool run_tests (void);

	std::ostream &failure (void);
private:
	typedef std::list<std::pair<Test *,std::string *> > Tests;
	typedef std::list<std::pair<Test *,std::string *> >::iterator TestsI;
	typedef std::list<std::pair<Test *,std::string *> >::const_iterator TestsCI;

	Tests m_tests;
	bool m_verbose;
	std::string *m_test_name;
};

#endif /* TEST_H */
