/*
 * lattice.cpp
 *
 *  Created on: Jan 6, 2016
 *      Author: thomas
 */

#include "lattice.h"

lattice::lattice(int L, double beta) {
	this->L = L;		// Save Lattice size.
	this->K = L+2;		// Add boundaries.

	// Initialize random number generator with seed 0.
	this->seed = 0;
	this->rng = new Rng(seed);

	// Initialize the spin lattice.
	for (int i=0; i<K; i++) {
		this->lat.push_back(std::vector<int>());
		for (int j=0; j<K; j++) {
			this->lat[i].push_back(this->rng->draw_spin());
		}
	}
	// Initialize the outer corners to 0.
	this->lat[0][0] = 0;
	this->lat[0][this->K-1] = 0;
	this->lat[this->K-1][0] = 0;
	this->lat[this->K-1][this->K-1] = 0;

	// Update lattice boundaries.
	this->update_boundaries();

	// Calculate energy and magnetization.
	this->H = 0; this->M = 0;
	this->calc_system_vars();
}

lattice::~lattice() {
	// Delete dynamically created objects.
	delete this->rng;
}

double lattice::sweep_multihit(double beta, unsigned int Ntry) {
	// Try to flip spins N times and return the acceptance rate.
	unsigned int flipped_cnt = 0;
	for (unsigned int i=1; i<K-1; i++) { 	// Loop rows
		for (unsigned int j=1; j<K-1; j++) { // Loop colums
			for (unsigned int t=0; t<Ntry; t++) {
				if (single_spinflip(i,j,beta)) {		// Try a spinflip.
					flipped_cnt++;				// Add number of tries to count.
				}
			}
		}
	}
	double acc_rate = (double) flipped_cnt/L/L/Ntry;
	return acc_rate;
}


void lattice::sweep_heatbath(double beta, double h) {
	/* Sweep over lattice using heatbath algorithm. */
	for (unsigned int i=1; i<K-1; i++) { 	// Loop rows
		for (unsigned int j=1; j<K-1; j++) { // Loop colums
			int up = this->lat[i-1][j];
			int left = this->lat[i][j-1];
			int down = this->lat[i+1][j];
			int right = this->lat[i][j+1];
			double delta = up + down + left + right;

			double k = beta*(delta + h);
			double z = 2*std::cosh(k);
			double q = std::exp(-k)/z;
			double r = this->rng->draw();
			if (r<q) {
				this->lat[i][j] = -1;
			} else {
				this->lat[i][j] = 1;
			}
			this->update_boundaries_site(i,j);
		}
	}
}


bool lattice::single_spinflip(int i, int j, double beta) {
	// Update a random spin and return whether flip was successful to calculate acceptance rates.
	// Get a random site.

	int s = this->lat[i][j];
	// Propose a spin flip. Don't actually change the spin yet. Change it later if accepted.
	int up = this->lat[i-1][j];
	int left = this->lat[i][j-1];
	int down = this->lat[i+1][j];
	int right = this->lat[i][j+1];
	// Calculate the difference in energy and magnetization.
	int delta_E = 2*s*(up+left+down+right);
	int delta_M = -2*s;

	// If the energy change is smaller than 0 accept the change.
	// If not, accept it with a certain probability.
	if (delta_E > 0) {
		double p = std::exp(-beta*delta_E);
		double r = this->rng->draw();
		// Accept the flip if r < p.
		// If not, don't accept the change and leave the function by returning false.
//		std::cout << "p = " << p << std::endl;
		if (p<1 && r>p) {
			return false;
		}
	}
	// If function reaches this line, the spin flip is accepted.
	// Flip the spin.
	this->lat[i][j] = -s;
	// Update energy and magnetization.
	this->H += delta_E;
	this->M += delta_M;
	// Update boundaries.
	this->update_boundaries_site(i,j);

	// Return true because spin flip was accepted.
	return true;
}

void lattice::update_boundaries() {
	// Update the boundaries to account for periodic boundary condition.
	// Update upper boundary.
	for (int j=1; j<this->K-1; j++) this->lat[0][j] = this->lat[this->K-2][j];
	// Update lower boundary.
	for (int j=1; j<this->K-1; j++) this->lat[this->K-1][j] = this->lat[1][j];
	// Update left boundary.
	for (int i=1; i<this->K-1; i++) this->lat[i][0] = this->lat[i][this->K-2];
	// Update right boundary.
	for (int i=1; i<this->K-1; i++) this->lat[i][this->K-1] = this->lat[i][1];
}

void lattice::update_boundaries_site(int i, int j) {
	// Update the boundary after one single spin flip.
	// Upper boundary.
	if (i==1) this->lat[this->K-1][j] = this->lat[1][j];
	// Lower boundary.
	if (i==this->K-2) this->lat[0][j] = this->lat[this->K-2][j];
	// Left boundary.
	if (j==1) this->lat[i][this->K-1] = this->lat[i][1];
	// Right boundary.
	if (j==this->K-2) this->lat[i][0] = this->lat[i][this->K-2];
}

void lattice::print_lattice() {
	// Print the spin lattice.
	std::cout << this->L << "x" << this->L << " lattice" << std::endl;
	std::cout << "epsilon = " << double (this->H)/this->L << " , m = " << double (this->M)/this->L << std::endl;
	for (int i=0; i<this->K; i++) {
		for (int j=0; j<this->K; j++) {
			std::cout << this->lat[i][j] << "\t";
		}
		std::cout << std::endl;
	}
}

void lattice::set_configuration(unsigned int conf) {
	/* Set the configuration of the lattice using the bits of the integer */
	// Set the lattice configuration bit by bit.
	for (int i=0; i<L*L; i++) {
		int line =  i/L + 1;
		int col = i%L + 1;
		int bit = (conf >> i) & 1;
		bit = (bit == 1) ? 1 : -1;
		lat[line][col] = bit;
		update_boundaries();
	}
}


void lattice::calc_system_vars() {
	// Calculate inner energy H and magnetization M.
	this->H = 0;
	this->M = 0;
	// Iterate over the whole lattice.
	for (int i=1; i<this->K-1; i++) {
		for (int j=1; j<this->K-1; j++) {
			// Only consider right and lower neighbor for energy calculation.
			int s = this->lat[i][j];
			this->H -= s*this->lat[i][j+1] + s*this->lat[i+1][j];
			// Sum magnetization.
			this->M += s;
		}
	}
}


double lattice::get_energy_density() {
	// Return the inner energy density.
	double Lsq = L*L;
	return this->H/Lsq;
}
double lattice::get_magnetization_density() {
	// Return the magnetization density.
	double Lsq = L*L;
	return this->M/Lsq;
}

void lattice::set_seed(int seed) {
	/* Seed rng */
	this->rng->reseed(seed);
}
