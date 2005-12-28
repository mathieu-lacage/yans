/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2005 INRIA
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#ifndef TEST_H
#define TEST_H

#include <list>
#include <string>
#include <utility>
#include <ostream>

namespace yans {

class TestManager;

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
}; // namespace yans 

#endif /* TEST_H */
