/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef OBJECT_H
#define OBJECT_H

#include "object-size.h"

class Object {
 public:
	void set (char c, unsigned int index);
	char get (unsigned int index);
 private:
	char m_data[OBJECT_SIZE];
};

#endif /* OBJECT_H */
