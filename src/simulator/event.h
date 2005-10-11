/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef EVENT_H
#define EVENT_H

class Event {
public:
	Event ();
	virtual ~Event ();

	virtual void notify (void) = 0;
	virtual void notify_canceled (void);
};


#endif /* EVENT_H */
