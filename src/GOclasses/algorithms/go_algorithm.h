/*****************************************************************************
 *   Copyright (C) 2004-2009 The PaGMO development team,                     *
 *   Advanced Concepts Team (ACT), European Space Agency (ESA)               *
 *   http://apps.sourceforge.net/mediawiki/pagmo                             *
 *   http://apps.sourceforge.net/mediawiki/pagmo/index.php?title=Developers  *
 *   http://apps.sourceforge.net/mediawiki/pagmo/index.php?title=Credits     *
 *   act@esa.int                                                             *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation; either version 2 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *   GNU General Public License for more details.                            *
 *                                                                           *
 *   You should have received a copy of the GNU General Public License       *
 *   along with this program; if not, write to the                           *
 *   Free Software Foundation, Inc.,                                         *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.               *
 *****************************************************************************/

#ifndef PAGMO_GO_ALGORITHM_H
#define PAGMO_GO_ALGORITHM_H

#include <iostream>
#include <string>
#include <typeinfo>

#include "../../../config.h"
#include "../../Functions/rng/rng.h"
#include "../basic/population.h"

class __PAGMO_VISIBLE go_algorithm
{
		friend std::ostream __PAGMO_VISIBLE_FUNC &operator<<(std::ostream &, const go_algorithm &);
	public:
		go_algorithm();
		go_algorithm(const go_algorithm &);
		go_algorithm &operator=(const go_algorithm &);
		virtual Population evolve(const Population &) const = 0;
		virtual go_algorithm *clone() const = 0;
        virtual ~go_algorithm() {}
		std::string id_name() const {return typeid(*this).name();}
		/// Get the name identyfing the object (<b>not</b> the class).
		/** Exposed to Python. The string should identify the object, so that instanciations of the same class with different parameters are distinguishable. */
		virtual std::string id_object() const = 0;
	protected:
		///Virtual log method. Is called by the overloaded operator << of go_algorithm that can thus behave differently for each derived class
		virtual void log(std::ostream& s) const {s << "You need to implement the virtual method log() for this derived class of go_algorithm" << std::endl;}
		mutable rng_double drng;
};

std::ostream __PAGMO_VISIBLE_FUNC &operator<<(std::ostream &, const go_algorithm &);

#endif
