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

#include "test.h"

#ifdef RUN_SELF_TESTS
#include <iostream>
#include "thread.h"
#include "buffer.h"
#include "tcp-buffer.h"
#if 0
#include "utils.h"
#include "fiber-scheduler.h"
#include "callback-test.h"
#include "tcp-pieces.h"
#endif

namespace yans {

#define ADD_TEST(klass, name) 	m_tests.push_back (std::make_pair (new klass (), new std::string (name)));

TestManager::TestManager ()
	: m_verbose (false)
{
#if 0
	ADD_TEST (TcpPiecesTest, "TcpPieces");
	ADD_TEST (BufferTest, "Buffer");
	ADD_TEST (UtilsTest, "Utils");
#ifdef SIMULATOR_FIBER
	ADD_TEST (TestFiberScheduler, "FiberScheduler");
#endif
	ADD_TEST (CallbackTest, "Callback");
#endif
	ADD_TEST (ThreadTest, "Thread");
	ADD_TEST (BufferTest, "Buffer");
	ADD_TEST (TcpBufferTest, "TcpBuffer");
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
			if (m_verbose) {
				std::cerr << "Test \"" << *m_test_name << "\" fail." << std::endl;
			}
		} else {
			if (m_verbose) {
				std::cerr << "Test \"" << *m_test_name << "\" success." << std::endl;
			}
		}
	}
	if (!is_success) {
		std::cerr << "Some tests failed." << std::endl;
	}
	return is_success;
}

Test::~Test ()
{}

std::ostream &
Test::failure (void)
{
	return TestManager::failure ();
}

}; // namespace yans

#endif /* RUN_SELF_TESTS */
