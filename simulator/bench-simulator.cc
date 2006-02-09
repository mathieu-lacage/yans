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

#include "simulator.h"
#include "event.h"
#include "event.tcc"
#include <iostream>
#include <fstream>
#include <vector>

using namespace yans;

#include "sys/time.h"

class Time {
public:
	void start (void);
	double end (void);
private:
	struct timeval m_start;
};

void Time::start (void)
{
	gettimeofday (&m_start, 0);
}
double Time::end (void)
{
	uint64_t total_us;
	struct timeval end;
	gettimeofday (&end, 0);
	total_us = (end.tv_sec - m_start.tv_sec) * 1000000;
	total_us += end.tv_usec - m_start.tv_usec;
	double total_s = ((double)total_us)/1000000;
	return total_s;
}


class Bench {
public:
	void read_distribution (std::istream &istream);
	void set_total (uint32_t total);
	void bench (void);
private:
	void cb (void);
	std::vector<uint64_t> m_distribution;
	std::vector<uint64_t>::const_iterator m_current;
	uint32_t m_n;
	uint32_t m_total;
};

void 
Bench::set_total (uint32_t total)
{
	m_total = total;
}

void
Bench::read_distribution (std::istream &input)
{
	double data;
	while (!input.eof ()) {
		if (input >> data) {
			uint64_t us = (uint64_t) (data * 1000000);
			m_distribution.push_back (us);
		} else {
			input.clear ();
			std::string line;
			input >> line;
		}
	}
}

void
Bench::bench (void) 
{
	Time time;
	double init, simu;
	time.start ();
	for (std::vector<uint64_t>::const_iterator i = m_distribution.begin ();
	     i != m_distribution.end (); i++) {
		Simulator::insert_in_us (*i, make_event (&Bench::cb, this));
	}
	init = time.end ();

	m_current = m_distribution.begin ();

	time.start ();
	Simulator::run ();
	simu = time.end ();

	std::cout <<
		"init n=" << m_distribution.size () << ", time=" << init << "s" << std::endl <<
		"simu n=" << m_n << ", time=" <<simu << "s" << std::endl <<
		"init " << ((double)m_distribution.size ()) / init << " insert/s, avg insert=" <<
		init / ((double)m_distribution.size ())<< "s" << std::endl <<
		"simu " << ((double)m_n) / simu<< " hold/s, avg hold=" << 
		simu / ((double)m_n) << "s" << std::endl
		;
}

void
Bench::cb (void)
{
	if (m_n > m_total) {
		return;
	}
	if (m_current == m_distribution.end ()) {
		m_current = m_distribution.begin ();
	}
	Simulator::insert_in_us (*m_current, make_event (&Bench::cb, this));
	m_current++;
	m_n++;
}

int main (int argc, char *argv[])
{
	char const *filename = argv[1];
	std::istream *input;
	if (strcmp (filename, "-") == 0) {
		input = &std::cin;
	} else {
		input = new std::ifstream (filename);
	}
	Bench *bench = new Bench ();
	bench->read_distribution (*input);
	bench->set_total (20000);
	bench->bench ();

	return 0;
}
