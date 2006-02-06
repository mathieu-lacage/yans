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

#ifndef TSPEC_REQUEST_H
#define TSPEC_REQUEST_H

class TSpec;

class TSpecRequest 
{
public:
	TSpecRequest (TSpec *tspec);
	virtual ~TSpecRequest ();

	TSpec *getTSpec (void);

	void notifyGranted (void);
	void notifyRefused (void);

private:
	virtual void notifyRequestGranted (void) = 0;
	virtual void notifyRequestRefused (void) = 0;

	TSpec *m_tspec;
};

class TclTSpecRequest : public TSpecRequest
{
public:
	TclTSpecRequest (TSpec *tspec, 
			 char const *tclTSpec,
			 char const *grantedCallback,
			 char const *refusedCallback);
	virtual ~TclTSpecRequest ();

private:
	virtual void notifyRequestGranted (void);
	virtual void notifyRequestRefused (void);

	char const *m_tclTspec;
	char const *m_grantedCallback;
	char const *m_refusedCallback;
};

#endif /* TSPEC_REQUEST_H */
