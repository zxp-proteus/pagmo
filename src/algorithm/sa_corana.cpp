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

#include "sa_corana.h"
#include "../problem/base.h"
#include "../population.h"
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_real.hpp>


namespace pagmo { namespace algorithm {

/// Constructor.
/**
 * Allows to specify in detail all the parameters of the algorithm.
 *
 * @param[in] niter number of total iterations.
 * @param[in] Ts starting temperature
 * @param[in] Tf final temperature
 * @param[in] niterT
 * @param[in] niterR
 * @param[in] range
 * @throws value_error niter is non positive, Ts is greater than Tf, Ts is non positive, Tf is non positive,
 * niterT or niterR are negative, range is not in the [0,1] interval
 */
sa_corana::sa_corana(int niter, const double &Ts, const double &Tf, const int niterT, const int niterR, const double range):
		base(),m_niter(niter),m_Ts(Ts),m_Tf(Tf),m_niterT(niterT),m_niterR(niterR),m_range(range)
{
	if (niter < 0) {
		pagmo_throw(value_error,"number of iterations must be nonnegative");
	}
	if (Ts <= 0 || Tf <= 0 || Ts <= Tf) {
		pagmo_throw(value_error,"temperatures must be positive and Ts must be greater than Tf");
	}
	if (niterT < 0) {
		pagmo_throw(value_error,"number of iteration before adjusting the temperature must be positive");
	}
	if (niterR < 0) {
		pagmo_throw(value_error,"number of iteration before adjusting the neighbourhood must be positive");
	}
	if (range < 0 || range >1) {
		pagmo_throw(value_error,"Initial range must be between 0 and 1");
	}
}
/// Clone method.
base_ptr sa_corana::clone() const
{
	return base_ptr(new sa_corana(*this));
}

/// Evolve implementation.
/**
 * Run the sa_corana algorithm for the number of iterations specified in the constructors.
 * At each accepted point velocity is also updated.
 *
 * @param[in,out] pop input/output pagmo::population to be evolved. The population champion is considered.
 */

void sa_corana::evolve(population &pop) const {

// Let's store some useful variables.
const problem::base &prob = pop.problem();
const problem::base::size_type D = prob.get_dimension(), prob_i_dimension = prob.get_i_dimension(), prob_c_dimension = prob.get_c_dimension(), prob_f_dimension = prob.get_f_dimension();
const decision_vector &lb = prob.get_lb(), &ub = prob.get_ub();
const population::size_type NP = pop.size();
const problem::base::size_type Dc = D - prob_i_dimension;


//We perform some checks to determine wether the problem/population are suitable for sa_corana
if ( Dc == 0 ) {
	pagmo_throw(value_error,"There is no continuous part in the problem decision vector for sa_corana to optimise");
}

if ( prob_c_dimension != 0 ) {
	pagmo_throw(value_error,"The problem is not box constrained and sa_corana is not suitable to solve it");
}

if ( prob_f_dimension != 1 ) {
	pagmo_throw(value_error,"The problem is not single objective and sa_corana is not suitable to solve it");
}

//Determines the number of iteration of the outer for loop
const size_t niterOuter = m_niter / (m_niterT * m_niterR * Dc);
if (niterOuter == 0) {
	pagmo_throw(value_error,"niterOuter is zero, increase niter");
}

//Starting point is the best individual
const int bestidx = pop.get_best_idx();
const decision_vector &x0 = pop.get_individual(bestidx).cur_x;
const fitness_vector &fit0 = pop.get_individual(bestidx).cur_f;
//Determines the coefficient to dcrease the temperature
const double Tcoeff = std::pow(m_Tf/m_Ts,1.0/(double)(niterOuter));
//Stores the current and new points
decision_vector xNEW = x0, xOLD = xNEW;
fitness_vector fNEW = fit0, fOLD = fNEW;
//Stores the adaptive steps of each component (integer part included but not used)
decision_vector Step(D,m_range);

//Stores the number of accepted points per compnent (integer part included but not used)
std::vector<int> acp(D,0) ;
double ratio = 0, currentT = m_Ts, probab = 0,  r = 0;

//Main SA loops
for (size_t jter = 0; jter < niterOuter; ++jter) {
	for (size_t mter = 0; mter < m_niterT; ++mter) {
		for (size_t kter = 0; kter < m_niterR; ++kter) {
			size_t nter =  (size_t)(m_drng() * Dc);
			for (size_t numb = 0; numb < Dc ; ++numb) {
				nter = (nter + 1) % Dc;
				//We modify the current point actsol by mutating its nter component within
				//a Step that we will later adapt
				r = 2.0 * m_drng() - 1.0; //random number in [-1,1]
				xNEW[nter] = xOLD[nter] + r * Step[nter] * ( ub[nter] - lb[nter] );

				// If new solution produced is infeasible ignore it
				if ((xNEW[nter] > ub[nter]) || (xNEW[nter] < lb[nter])) {
					xNEW[nter]=xOLD[nter];
					continue;
				}
				//And we valuate the objective function for the new point
				prob.objfun(fNEW,xNEW);

				// We decide wether to accept or discard the point
				if (prob.compare_fitness(fNEW,fOLD) ) {
					//accept
					xOLD[nter] = xNEW[nter];
					fOLD = fNEW;
					acp[nter]++;	//Increase the number of accepted values
				} else {
					//test it with Boltzmann to decide the acceptance
					probab = exp ( fabs(fOLD[0] - fNEW[0] )/ currentT );

					// we compare prob with a random probability.
					if (probab > m_drng()) {
						xOLD[nter] = xNEW[nter];
						fOLD = fNEW;
						acp[nter]++;	//Increase the number of accepted values
					} else {
						xNEW[nter] = xOLD[nter];
					}
				} // end if
			} // end for(nter = 0; ...
		} // end for(kter = 0; ...


		// adjust the step (adaptively)
		for (size_t iter = 0; iter < Dc; ++iter) {
			ratio = (double)acp[iter]/(double)m_niterR;
			acp[iter] = 0;  //reset the counter
			if (ratio > .6) {
				//too many acceptances, increase the step by a factor 3 maximum
				Step[iter] = Step [iter] * (1 + 2 *(ratio - .6)/.4);
			} else {
				if (ratio < .4) {
					//too few acceptance, decrease the step by a factor 3 maximum
					Step [iter]= Step [iter] / (1 + 2 * ((.4 - ratio)/.4));
				};
			};
			//And if it becomes too large, reset it to its initial value
			if ( Step[iter] > m_range ) {
				Step [iter] = m_range;
			};
		}
	}
	// Cooling schedule
	currentT *= Tcoeff;
}
if ( prob.compare_fitness(fOLD,fit0) ){
	pop.set_x(bestidx,xOLD); //new evaluation is possible here......
	std::transform(xOLD.begin(), xOLD.end(), pop.get_individual(bestidx).cur_x.begin(), xOLD.begin(),std::minus<double>());
	pop.set_v(bestidx,xOLD);
}
}




/// Extra human readable algorithm info.
/**
 * Will return a formatted string displaying the parameters of the algorithm.
 */
std::string sa_corana::human_readable_extra() const
{
	std::ostringstream s;
	s << "\tIteration:\t" << m_niter << '\n';
	return s.str();
}

}} //namespaces
