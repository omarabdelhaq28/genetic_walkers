// We will implement a genetic algorithm to teach an walker how to walk
// in a 2D environment. The walker will have 2 legs, a head, and 4 joints in 
// total. The 2 upper joints will fix the legs to the head, and the 2 lower
// joints will be basically knees (connects "upper" and "lower" halves of each
// leg).

// The walker will be represented by a chromosome, which is a vector of
// 4 floating point numbers, each representing the motor speed each joint
// attempts to reach

// Define the function main that does the following:
// 1. Start with a population of 100 chromosomes (walkers), each with a
//    random motor speed for each joint.
// 2. For each chromosome, create a walker with the given motor speed values.
// 3. Run the simulation for 10 iterations:
//    a. Set the speed values to the joint motors.
//    b. Get the new position of the head.
//    c. Calculate the fitness of the chromosome based on the new position
//       of the head.  
// 4. Sort the chromosomes by fitness.
// 5. Select the top 10% of chromosomes (i.e. the fittest 10%).
// 6. Create a new population of 100 chromosomes by randomly selecting
//    chromosomes from the top 10% and applying crossover and mutation
//    to them.
// 7. Repeat steps 2-6 for 10 iterations.
// 8. Print the chromosome with the highest fitness.

// The fitness of a chromosome is calculated as follows:
// 1. The fitness is the x displacement of the head from the starting
//    position.

// The crossover function should randomly select a crossover point
// between 0 and 3 (inclusive) and swap the values of the two chromosomes
// at that point and all subsequent points.

// The mutation function should randomly select a mutation point
// between 0 and 3 (inclusive) and randomly select a new value for that
// point.

#include <algorithm>
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include "walker.h"
#include <omp.h>

// #define N_BEST 1

std::chrono::high_resolution_clock::time_point program_start;
std::chrono::high_resolution_clock::time_point program_end;
std::chrono::high_resolution_clock::time_point start;
std::chrono::high_resolution_clock::time_point end;
std::chrono::high_resolution_clock::time_point start_initial_generation;
std::chrono::high_resolution_clock::time_point end_initial_generation;

// define a timer for each the creation, simulation, and fitness selection
// of the walkers
double create_time;
double simulate_time;
double fitness_selection_time;


// Define the chromosome type, which is a vector of 4 floating point numbers.
typedef std::vector<float> Chromosome;

// Define a function to initialize a chromosome with random values.
Chromosome initialize_chromosome() 
{
    Chromosome chromosome;
    std::random_device rd;
    std::mt19937 gen(rd());
    float stdev = (MAX_MOTOR_SPEED - MIN_MOTOR_SPEED) / 2;
    std::normal_distribution<float> dis(0, stdev);
    for (int i = 0; i < 4; i++) {
        chromosome.push_back(dis(gen));
    }
    return chromosome;
}

// Define a function to create a walker with a given chromosome.
// the walker starts in the same position as the passed parent
Walker* create_walker(Walker* parent, Chromosome chromosome) 
{
    Walker* walky;
    if (parent) {
        walky = new Walker(parent->states);
    } else {
        walky = new Walker();
    }
    walky->SetMotorSpeeds(chromosome[0], chromosome[1], chromosome[2], 
                            chromosome[3]);
    return walky;
}

// Define a function to calculate the fitness of an walker assuming that
// the walker's step had already been simulated.
float calculate_fitness(Walker* walker) 
{
    // Get the distance walked by the walker and return it as the fitness.
    return walker->GetPositionX();

    //return ABS(walker->GetVelocityX());
}

// Define a function to perform crossover on two chromosomes.
Chromosome crossover(Chromosome chromosome1, Chromosome chromosome2) 
{
    // for each index in the chromosome, randomly select the allele from one 
    // of the parents with probability 0.5
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    Chromosome child;
    for (int i = 0; i < 4; i++) {
        if (dis(gen) < 0.5f) {
            child.push_back(chromosome1[i]);
        } else {
            child.push_back(chromosome2[i]);
        }
    }
    return child;
}

// Define a function to perform mutation on a chromosome.
Chromosome mutate(Chromosome chromosome) 
{
    // for each index in the chromosome, randomly sample from a normal 
    // distribution with mean 0 and standard deviation 0.04, with prob. 0.1

    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<float> dis(0.0f, MUTATE_SIZE);
    Chromosome mutated;

    for (int i = 0; i < 4; i++) {
        float chrom = chromosome[i], mutation = dis(gen);

        if (chrom + mutation <= MAX_MOTOR_SPEED &&
            chrom + mutation >= MIN_MOTOR_SPEED) {
            chrom += mutation;
        }
        mutated.push_back(chrom);
    }
    return mutated;
}

// Define a function to select the fittest walkers after the speeds have 
// been applied, and it returns the top 10% of the walkers
std::vector<Walker*> select_fittest(std::vector<Walker*> walkers, float ratio) 
{
    // print the walkers' speeds in the new population and their fitness
    // std::cout << "select_fittest called" << std::endl;
    // for (int i = 0; i < 10; i++) {
    //     std::cout << "Walker " << i << ": ";
    //     Chromosome chromosome = walkers[i]->GetMotorSpeeds();
    //     for (int j = 0; j < chromosome.size(); j++) {
    //         std::cout << chromosome[j] << " ";
    //     }
    //     std::cout << "Fitness: " << walkers[i]->GetDistanceWalked();
    //     std::cout << std::endl;
    // }

    if (ratio > 1.0f) ratio = 0.1f;

    // sort the walkers by fitness

    std::sort(walkers.begin(), walkers.end(), [](Walker* w1, Walker* w2) 
    {
        return calculate_fitness(w1) < calculate_fitness(w2);
    });
	
    // return the top `ratio` portion of the walkers using assign
    std::vector<Walker*> fittest;
    int cutoff_i = walkers.size() * ratio;
    for (int i = 0; i < cutoff_i; i++) {
        fittest.push_back(walkers[i]);
    }

    // free memory used by the remaining (dead) walkers
    for (int i = cutoff_i; i < (int)walkers.size(); i++) {
        delete walkers[i];
    }

    return fittest;
}

// Define a function to create a new population of walkers given the fittest 
// walkers from the previous generation and their chromosomes, and the number
// of walkers to create.
std::vector<Walker*> create_new_population(
    std::vector<Walker*> fittest_walkers,
    int num_walkers) 
{
    // std::vector<Walker*> new_population;
    // define a new_population vector of size num_walkers
    // for parallel
    std::vector<Walker*> new_population(num_walkers);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

#pragma omp parallel for num_threads(4)
    for (int i = 0; i < num_walkers; i++) {
        // randomly select two walkers from the fittest walkers
        int index1 = dis(gen) * fittest_walkers.size();
        int index2 = dis(gen) * fittest_walkers.size();

        // get the chromosomes of the walkers
        Chromosome chromosome1 = fittest_walkers[index1]->GetMotorSpeeds();
        Chromosome chromosome2 = fittest_walkers[index2]->GetMotorSpeeds();

        // perform crossover with probability 0.8
        if (dis(gen) < CROSSOVER_PROBABILITY) {
            chromosome1 = crossover(chromosome1, chromosome2);
        }

        // perform mutation with probability 0.1
        if (dis(gen) < MUTATION_PROBABILITY) {
            chromosome1 = mutate(chromosome1);
        }

        // [TODO] for now, just use the first parent to initialize the child's
        // position
        // new_population.push_back(create_walker(fittest_walkers[index1],
        //                             chromosome1));
        // for parallel
        new_population[i] = create_walker(fittest_walkers[index1], chromosome1);
    }

#pragma omp parallel for num_threads(4)
// because if multiple threads try to delete the same
                         // walker, it will cause a segfault, or undefined
                         // behavior
	for (Walker* walky : fittest_walkers) {
		delete walky;
    }

    return new_population;
}

// Define a function to run the genetic algorithm and return the best walkers.
std::vector<Walker*> run_genetic_algorithm( int num_walkers, 
                                            int num_iterations,
                                            float fit_ratio) 
{
    // initialize the population of walkers
    // std::vector<Walker*> walkers;
    // for parallel
    std::vector<Walker*> walkers(num_walkers);

    //start = std::chrono::high_resolution_clock::now();

    start_initial_generation = std::chrono::high_resolution_clock::now();

#pragma omp parallel for num_threads(4)
    // thread-local list of walkers
    for (int i = 0; i < num_walkers; i++) {
        Walker* walk0 = create_walker(nullptr, initialize_chromosome());
        walk0->Simulate();
        // walkers.push_back(walk0);
        // for parallel
        walkers[i] = walk0;
    }

    end_initial_generation = std::chrono::high_resolution_clock::now();
    std::cout << "(run_genetic_algorithm): Time to create initial population: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(
                     end_initial_generation - start_initial_generation)
                     .count()
              << "ms" << std::endl;



//     start = std::chrono::high_resolution_clock::now();

//     // for parallel
// #pragma omp parallel for num_threads(4)
//     for (int i = 0; i < num_walkers; i++) {
//         Walker* walk0 = create_walker(nullptr, initialize_chromosome());
//     }

//     end = std::chrono::high_resolution_clock::now();
//     std::cout	<< "(run_genetic_algorithm): Time to create new "
// 					<< " population, iteration 0: " 
//     				<< std::chrono::duration_cast<std::chrono::milliseconds>(
//     				end - start).count() << "ms" << std::endl;
    
//     // add end-start to create_time
//     create_time += std::chrono::duration_cast<std::chrono::milliseconds>(
//     				end - start).count();

//     start = std::chrono::high_resolution_clock::now();

//     // simulate the walkers
// #pragma omp parallel for num_threads(4)
//     for (int i = 0; i < num_walkers; i++) {
//         walkers[i]->Simulate();
//     }

//     end = std::chrono::high_resolution_clock::now();
//     std::cout	<< "(run_genetic_algorithm): Time to simulate new "
//                     << " population, iteration 0: " 
//     				<< std::chrono::duration_cast<std::chrono::milliseconds>(
//     				end - start).count() << "ms" << std::endl;

//     // add end-start to simulate_time
//     simulate_time += std::chrono::duration_cast<std::chrono::milliseconds>(
//     				end - start).count();

//     start = std::chrono::high_resolution_clock::now();

    

    // run the genetic algorithm for a number of iterations

    // select the first walkers to generate offspring
    walkers = select_fittest(walkers, fit_ratio);

    for (int i = 1; i < num_iterations; i++) {

        // print the iteration number
        // std::cout << "Iteration " << i << std::endl;
        // apply the currently set motor speeds to the walkers

        // create a new population of walkers
        
        start = std::chrono::high_resolution_clock::now();

        walkers = create_new_population(walkers, num_walkers);

        end = std::chrono::high_resolution_clock::now();
        std::cout	<< "(run_genetic_algorithm): Time to create new "
						<< " population, iteration " << i << ": " 
						<< std::chrono::duration_cast
							<std::chrono::milliseconds>(end - start).count() 
						<< "ms" << std::endl;
        // start = end;

        // add end-start to create_time
        create_time += std::chrono::duration_cast
                            <std::chrono::milliseconds>(end - start).count();
                            

        start = std::chrono::high_resolution_clock::now();

// #pragma omp parallel for num_threads(4)
// add parallel and collapse
#pragma omp parallel for num_threads(4)
        for (int j = 0; j < num_walkers; j++) {
        	walkers[j]->Simulate();
        }

        end = std::chrono::high_resolution_clock::now();
        std::cout	<< "(run_genetic_algorithm): Time to simulate walkers,"
							" iteration " << i << ": " 
						<< std::chrono::duration_cast
							<std::chrono::milliseconds>(end - start).count() 
						<< "ms" << std::endl;
        // add end-start to simulate_time
        simulate_time += std::chrono::duration_cast
                            <std::chrono::milliseconds>(end - start).count();


        // start = end; // wrong

        // select the fittest walkers

        start = std::chrono::high_resolution_clock::now();

        walkers = select_fittest(walkers, fit_ratio);

        // end = std::chrono::high_resolution_clock::now();
        // std::cout	<< "(run_genetic_algorithm): Time to select fittest "
		// 					"walkers, iteration " << i << ": " 
		// 				<< std::chrono::duration_cast
		// 					<std::chrono::milliseconds>(end - start).count() 
		// 				<< "ms" << std::endl;
        // start = end;
    }

    return walkers;
}

// [USAGE] ./main (# of walkers) (# generations) (# survival fraction)
int main(int argc, char *argv[]) 
{
    int n_walkers = NUM_WALKERS, n_iter = NUM_ITERATIONS;
    float fit_r = FITTEST_RATIO; //, total_time;

    if (argc > 1) {
        n_walkers = atoi(argv[1]);
        if (argc > 2) {
            // total_time = atof(argv[2]);
            // n_iter = total_time / SIM_DT;
            n_iter = atoi(argv[2]);
            if (argc > 3) {
                fit_r = atof(argv[3]);
            }
        }
    }

    // print number of max threads
    std::cout << "Max threads: " << omp_get_max_threads() << std::endl;

    program_start = std::chrono::high_resolution_clock::now();

    // std::cout   << "#Walkers = " << n_walkers << "\nTime = " << total_time
    //             << " (" << n_iter << " Iterations)\nFittest = " << fit_r 
    //             << " (Top " << n_walkers * fit_r << ")" << std::endl;

    std::cout   << "# Walkers = " << n_walkers << "\n# Iterations = " << n_iter 
                << "\n# Fittest = " << n_walkers * fit_r << " (Top " 
                <<  100 * fit_r << "%)" << std::endl;

	// run the genetic algorithm
    std::vector<Walker*> walkers = run_genetic_algorithm(n_walkers, n_iter,
                                                            fit_r);

    // print the best walker
    std::cout	<< "Best walker: " 
				<< calculate_fitness(walkers[0]) << std::endl;

    // print the size of the best walker in bytes
    std::cout	<< "Size of best walker:   "
                << sizeof(*walkers[0]) << std::endl;

    // print simulate_time average (we don't count the first iteration)
    std::cout	<< "Average simulate_time: "
                << simulate_time / (n_iter - 1) << "ms" << std::endl;
            
    // print create_time average (we don't count the first iteration)
    std::cout	<< "Average create_time:   "
                << create_time / (n_iter - 1) << "ms" << std::endl;


    // // print the best walker's chromosome
    // std::cout << "Best walker's chromosome: ";
    // Chromosome chromosome = walkers[0]->GetMotorSpeeds();
    // for (int i = 0; i < chromosome.size(); i++) {
    //     std::cout << chromosome[i] << " ";
    // }
    // std::cout << std::endl;

    // // print the best walker's states
    // std::cout << "Best walker's states: " << std::endl;
    // for (int i = 0; i < walkers[0]->states.size(); i++) {
    //     std::cout << "State " << i << ": ";
    //     walkers[0]->states[i].Print();
    // }




    // walkers[0]->states[0].Print();


    /*
    // print the best `N_BEST` walkers and their fitness
    std::cout << "\nBest " << N_BEST << " walkers: " << std::endl;
    for (int i = 0; i < N_BEST; i++) {
        std::cout << "Walker " << i << ": ";

        Chromosome chromosome = walkers[i]->GetMotorSpeeds();
        for (int j = 0; j < chromosome.size(); j++) {
            std::cout << chromosome[j] << " ";
        }

        std::cout << "Fitness: " << walkers[i]->GetDistanceWalked();
        std::cout << std::endl;

        walkers[i]->Dump();
    }
    */
   walkers[0]->Dump(true);

	for (Walker* walky : walkers) {
		delete walky;
	}

    program_end = std::chrono::high_resolution_clock::now();
    std::cout	<< "Time to run genetic algorithm: "
					<< std::chrono::duration_cast<std::chrono::milliseconds>
						( program_end - program_start).count()
					<< "ms" << std::endl;

    return 0;
}