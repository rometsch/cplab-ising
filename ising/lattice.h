/*
 * lattice.h
 *
 *  Created on: Jan 6, 2016
 *      Author: thomas
 */

#ifndef LATTICE_H_
#define LATTICE_H_

#include <iostream>
#include <vector>
#include "Rng.h"
#include <cmath>


class lattice {
public:
	int L;									// Size of lattice.
	int K;									// Size of the array to store the lattice. K=L+2 to account for periodic boundaries.

	int seed;								// Seed for rng.
	std::vector<std::vector<int> > lat;		// Spin LxL lattice.
	Rng* rng;								// Random number generator.

	int H;			// Energy.
	int M;			// Magnetization.


	lattice(int L, double beta);
	virtual ~lattice();

	void thermalize();			// Thermalize the lattice.
	void update_boundaries();	// Update the boundaries to account for periodic boundary condition.
	void update_boundaries_site(int i, int j); // Update the boundary after one single spin flip.

	bool single_spinflip(int i, int j, double beta);		// Try to update spin at (i,j) and return whether flip was successful.
	double sweep_multihit(double beta, unsigned int Ntry);		// Try to flip spins N times and return the success rate.
	void sweep_heatbath(double beta, double h);		// Sweep with heatbath algorithm.

	void print_lattice();		// Print the lattice.
	void calc_system_vars();	// Calculate inner energy H and magnetization M.

	void set_beta(double beta);	// Set the inverse temperature parameter.
	void set_h(double h);		// Set external field.
	void set_configuration(unsigned int conf);		// Set the configuration of the lattice using the bits of the integer

	double get_energy_density();	// Return the inner energy density.
	double get_magnetization_density();	// Return the magnetization density.

	void set_seed(int seed);		// Seed the rng.
};

#endif /* LATTICE_H_ */
