/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef PROCESS_H
#define PROCESS_H

#include "thread.h"

class Host;

class Process : public Thread {
public:
	Process (char const *name);
	Process (int argc, char const *argv[]);
	Process (Host *host, char const *name);
	Process (Host *host, int argc, char const *argv[]);
	virtual ~Process ();

	int get_retval (void);

private:
	/* for the children to override. */
	virtual int run (int argc, char const *const argv[]) = 0;

	virtual void run (void);

	int m_argc;
	char **m_argv;
	int m_retval;
};


#endif /* PROCESS_H */
