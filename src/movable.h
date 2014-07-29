
/*
 *
 * movable.h
 *
 * 15th January 2005: Created movable.h
 *
 * Part of the OpenJazz project
 *
 *
 * Copyright (c) 2005-2009 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#ifndef _MOVABLE_H
#define _MOVABLE_H


#include "OpenJazz.h"


// Class

class Movable {

	protected:
		fixed x, y, dx, dy;

		fixed getDrawX (int change);
		fixed getDrawY (int change);

	public:
		fixed getX ();
		fixed getY ();

};


#endif

