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

#include "callback.tcc"
#include "callback-test.h"
#include <stdio.h>

namespace yans {

CallbackTest::CallbackTest ()
	: m_test1 (false),
	  m_test2 (false),
	  m_test3 (false),
	  m_test4 (false)
{}
  
bool 
CallbackTest::run_tests (void)
{
	typedef yans::Callback<void (void)> A;
	typedef yans::Callback<int (void)> B;
	typedef yans::Callback<void (double)> C;
	typedef yans::Callback<int (double, int)> D;
	
	A * a = yans::make_callback (&CallbackTest::test1, this);
	B * b = yans::make_callback (&CallbackTest::test2, this);
	C * c = yans::make_callback (&CallbackTest::test3, this);
	D * d = yans::make_callback (&CallbackTest::test4, this);
	
	(*a) ();

	(*b) ();

	(*c) (0.0);

	(*d) (0.0, 1);

	delete a;
	delete b;
	delete c;
	delete d;

	if (m_test1 &&
	    m_test2 &&
	    m_test3 &&
	    m_test4) {
		return true;
	}
	return false;
}

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

}; // namespace yans

