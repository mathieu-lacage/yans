/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef RUNNABLE_H
#define RUNNABLE_H

class Host;

class Runnable {
public:
	Runnable ();
	virtual ~Runnable () = 0;
	Host *get_host (void);

private:
	friend class Fiber;
	virtual void run (void) = 0;
};


#endif /* RUNNABLE_H */
