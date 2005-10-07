/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef EVENT_HEAP_H
#define EVENT_HEAP_H

#include <list>
#include <utility>

class Event;

class EventHeap {
 public:
	EventHeap ();

	/* the insert operations might be veeery slow
	 * but peek_next and remove_next should be
	 * really fast.
	 */

	void insert_at_us (Event *event, uint64_t time);
	void insert_at_s (Event *event, double time);

	Event   *peek_next (void);
	uint64_t peek_next_time_us (void);
	void     remove_next (void);

	void clear (void);

	/* blocking call until there is a non-zero number of events. */
	void wait (void);
 private:
	typedef std::list<std::pair<Event *, uint64_t> > Events;
	typedef std::list<std::pair<Event *, uint64_t> >::iterator EventsI;
	Events m_events;
};


#endif /* EVENT_HEAP_H */
