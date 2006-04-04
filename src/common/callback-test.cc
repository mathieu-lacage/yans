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
#include "callback.h"

namespace yans {

static bool g_test5 = false;

void test5 (void)
{
	g_test5 = true;
}

class CallbackTest : public yans::Test {
private:
	bool m_test1;
	bool m_test2;
	bool m_test3;
	bool m_test4;
public:
	CallbackTest ();
	virtual bool run_tests (void);
	void test1 (void);
	int test2 (void);
	void test3 (double a);
	int test4 (double a, int b);
};

CallbackTest::CallbackTest ()
	: yans::Test ("Callback"),
	  m_test1 (false),
	  m_test2 (false),
	  m_test3 (false),
	  m_test4 (false)
{}

void 
CallbackTest::test1 (void)
{
	m_test1 = true;
}
int 
CallbackTest::test2 (void)
{
	m_test2 = true;
	return 2;
}
void 
CallbackTest::test3 (double a)
{
	m_test3 = true;
}
int 
CallbackTest::test4 (double a, int b)
{
	m_test4 = true;
	return 4;
}

  
bool 
CallbackTest::run_tests (void)
{
	typedef yans::Callback<void> A;
	typedef yans::Callback<int> B;
	typedef yans::Callback<void, double> C;
	typedef yans::Callback<int, double, int> D;
	typedef yans::Callback<void> E;
	
	//A a = A (this, &CallbackTest::test1);
	A a = yans::make_callback (&CallbackTest::test1, this);
	B b = B (this, &CallbackTest::test2);
	//B b = yans::make_callback (&CallbackTest::test2, this);
	C c = C (this, &CallbackTest::test3);// = yans::make_callback (&CallbackTest::test3, this);
	D d = D (this, &CallbackTest::test4);// = yans::make_callback (&CallbackTest::test4, this);
	E e = E (&test5);
	
	a ();

	b ();

	c (0.0);
	
	d (0.0, 1);

	e ();

	if (m_test1 &&
	    m_test2 &&
	    m_test3 &&
	    m_test4 &&
	    g_test5) {
		return true;
	}
	return false;
}

static CallbackTest g_callback_test;

}; // namespace

