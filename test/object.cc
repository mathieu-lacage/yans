/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "object.h"

void Object::set (char c, unsigned int index)
{
	m_data[index] = c;
}
char Object::get (unsigned int index)
{
	return m_data[index];
}
