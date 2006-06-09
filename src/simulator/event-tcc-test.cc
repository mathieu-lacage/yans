/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2006 INRIA
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
#include "event.tcc"

#ifdef RUN_SELF_TESTS

namespace {
void null_cb (void)
{}
void one_cb (int)
{}
void two_cb (int,int)
{}
void three_cb (int,int,int)
{}
void four_cb (int,int,int,int)
{}
void five_cb (int,int,int,int,int)
{}

void test (void)
{
  yans::make_event (&null_cb);
  yans::make_event (&one_cb, 1);
  yans::make_event (&two_cb, 1, 2);
  yans::make_event (&three_cb, 1, 2, 3);
  yans::make_event (&four_cb, 1, 2, 3, 4);
  yans::make_event (&five_cb, 1, 2, 3, 4, 5);
  
}
};

#endif /* RUN_SELF_TESTS */
