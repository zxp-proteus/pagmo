/*
 *  population.cpp
 *  SeGMO, a Sequential Global Multiobjective Optimiser
 *
 *  Created by Dario Izzo on 5/16/08.
 *  Copyright 2008 ¿dvanced Concepts Team (European Space Agency). All rights reserved.
 *
 */
#include "population.h"
#include <math.h>



	void Population::createRandomPopulation(std::vector<double> LB, std::vector<double> UB, int N, rng_type &rng){
		Individual x;
		pop.clear();

		for (int i=0; i < N; i++){
			x.createRandomIndividual(LB,UB, rng);
			pop.push_back(x);
		}//for
	};//createRandomPopulation

	void Population::resetVelocities(std::vector<double> LB, std::vector<double> UB, rng_type &rng){
		for (unsigned int j=0 ;j<pop.size();j++){
				pop[j].resetVelocity(LB,UB, rng);
		}
	}


	void Population::evaluatePopulation(GOProblem& problem){

		for (unsigned int i=0; i < pop.size(); i++)
			pop[i].evaluateFitness(problem);
	};//evaluatePopulation

	void Population::addIndividual(Individual x){
		pop.push_back(x);
	};

	void Population::substituteIndividual(const Individual x, const int n){
		pop[n].setDecisionVector(x.getDecisionVector());
		pop[n].setVelocity(x.getVelocity());
		pop[n].setFitness(x.getFitness());
	}

	unsigned int Population::size(){
		return pop.size();
	};

	Individual Population::extractBestIndividual(){

		double f = pop[0].getFitness();
		int index = 0;

		for (unsigned int i=1; i<pop.size();i++){
			if( pop[i].getFitness() < f ){
				index=i;
				f = pop[i].getFitness();
			}
		}
		return pop[index];
	}

	Individual Population::extractWorstIndividual(){

		double f = pop[0].getFitness();
		int index = 0;

		for (unsigned int i=1; i<pop.size();i++){
			if( pop[i].getFitness() > f ){
				index=i;
				f = pop[i].getFitness();
			}
		}
		return pop[index];
	}

	Population Population::extractRandomDeme(int N, std::vector<int> &picks, rng_type &rng){
		Population deme;
		std::vector<int> PossiblePicks;
		int Pick;

		for (unsigned int i=0; i < pop.size(); i++)
			PossiblePicks.push_back(i);


		for (int i=0; i < N; i++){
			//we pick a random position between 0 and popsize-1
			Pick = (int)(Pk::nextDouble(rng) * PossiblePicks.size());
			//and store it
			picks.push_back(PossiblePicks[Pick]);
			//we insert the corresponding individual in the deme
			deme.addIndividual(pop[PossiblePicks[Pick]]);
			//and erase it from the possible picks
			PossiblePicks.erase(PossiblePicks.begin() + Pick);
		}
		return deme;
	};

	void Population::insertDeme(Population deme, std::vector<int> picks){
		for (unsigned int i=0; i<picks.size(); i++){
			if ( deme[i].getFitness() < pop[picks[i]].getFitness() ){
				pop[picks[i]] = deme[i];
			}
		}
	}

	void Population::insertDemeForced(Population deme, std::vector<int> picks){
		for (unsigned int i=0; i<picks.size(); i++){
				pop[picks[i]] = deme[i];
		}
	}

	void Population::insertBestInDeme(Population deme, std::vector<int> picks){
		const int Ndeme = deme.size();

		int bestindeme =  0;
		int worstinpicks = 0;
		double best = deme[0].getFitness();
		double worst = pop[picks[0]].getFitness();

		for (int i=1; i<Ndeme; i++){
			if ( deme[i].getFitness() < best){
				bestindeme = i;
				best = deme[i].getFitness();
			}
			if ( pop[picks[i]].getFitness() > worst) {
				worstinpicks = i;
				worst = pop[picks[i]].getFitness();
			}
		}
		pop[picks[worstinpicks]] = deme[bestindeme];

	}

	double Population::evaluateMean(){
		double mean=0;
		int size = 0;
		size = pop.size();
		for (int i=0; i<size; i++){
			mean=mean+pop[i].getFitness();
		}
		mean = mean/(double)size;
		return mean;
	}

	double Population::evaluateStd(){
		double Std=0,mean=0;
		int size = 0;
		size = pop.size();
		mean = evaluateMean();

		for (int i=0; i<size; i++){
			Std=Std+pow((pop[i].getFitness()-mean),2.0);
		}
		Std = sqrt(Std/size);
		return Std;
	}

	Individual& Population::operator[](int index){
		return pop[index];
	};

	void Population::operator=(Population newpop){
		pop.clear();
		for (unsigned int i=0 ; i<newpop.size(); i++){
			pop.push_back(newpop[i]);
		}
	};

	void Population::operator=(Individual x){
		pop.clear();
		pop.push_back(x);
	};

	std::ostream& operator<<(std::ostream& s, Population& pop){
        for (unsigned int i=0;i<pop.size(); i++){
            s << "Individual #" << i << ": " << pop[i].getFitness() << " " << pop[i] << std::endl;
        }
        return s;
    }

