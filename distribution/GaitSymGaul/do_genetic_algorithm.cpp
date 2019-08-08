// function to perform a genetic algorithm search

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#include "PGDMath.h"
#include "XMLConverter.h"
#include "Simulation.h"

#include "gaul.h"

#include "utilities.h"
#include "callbacks.h"

#include "do_genetic_algorithm.h"

#define THROWIFZERO(a) if ((a) == 0) throw __LINE__
#define THROWIF(a) if ((a) != 0) throw __LINE__

extern double g_mutation_sd;
extern XMLConverter g_XMLConverter;
extern Simulation *gSimulation;

void do_genetic_algorithm(DataFile *config_file)
{
    int population_size;
    int generations;
    int chromosome_size;
    double crossover;
    double mutation;
    double migration;
    char model_config_file[512];
    char starting_population_file[512] = "";
    int gaul_log_level = 3; // 1 fatal, 2, warnings, 3 normal, 4 verbose

    try
    {
        THROWIF(config_file->RetrieveQuotedStringParameter("model_config_file", model_config_file, 512));
        THROWIF(config_file->RetrieveParameter("population_size", &population_size));
        THROWIF(config_file->RetrieveParameter("generations", &generations));
        THROWIF(config_file->RetrieveParameter("chromosome_size", &chromosome_size));
        THROWIF(config_file->RetrieveParameter("mutation_sd", &g_mutation_sd));
        THROWIF(config_file->RetrieveParameter("crossover", &crossover));
        THROWIF(config_file->RetrieveParameter("mutation", &mutation));
        THROWIF(config_file->RetrieveParameter("migration", &migration));

        config_file->RetrieveParameter("starting_population_file", starting_population_file, 512);
        config_file->RetrieveParameter("gaul_log_level", &gaul_log_level);
    }

    catch (int e)
    {
        dief("Error on line %d\nMissing parameter in config_file", e);
    }

    random_seed(config_file);

    if (g_XMLConverter.LoadBaseXMLFile(model_config_file)) dief("LoadBaseXMLFile \"%s\"error", model_config_file);

    // turn on logging
    char log_filename[512];
#if HAVE_MPI == 1
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    sprintf(log_filename, "GeneticAlgorithmLog%06d.txt", rank);
#else
    strcpy(log_filename,"GeneticAlgorithmLog.txt");
#endif
    log_init((log_level_type)gaul_log_level, log_filename, 0, 1);


    char dir_name[256], output_file[256];
    create_output_folder(dir_name, sizeof(dir_name));

    population          *pop = 0;                   /* Population of solutions. */
    entity              *solution = 0;              /* Optimised solution. */

    // set up the initial population
    pop = ga_genesis_double(
            population_size,                    /* const int                population_size */
            1,                                  /* const int                num_chromo */
            chromosome_size,                    /* const int                len_chromo */
            callbacks::generation_callback,                /* GAgeneration_hook        generation_hook */
            callbacks::iteration_callback,                 /* GAiteration_hook         iteration_hook */
            NULL,                               /* GAdata_destructor        data_destructor */
            NULL,                               /* GAdata_ref_incrementor   data_ref_incrementor */
            callbacks::score,                              /* GAevaluate               evaluate */
            callbacks::seed,                               /* GAseed                   seed */
            NULL,                               /* GAadapt                  adapt */
            ga_select_one_bestof2,              /* GAselect_one             select_one */
            ga_select_two_bestof2,              /* GAselect_two             select_two */
            callbacks::ga_mutate_double_multipoint_usersd, /* GAmutate                 mutate */
            ga_crossover_double_singlepoints,   /* GAcrossover              crossover */
            NULL,                               /* GAreplace                replace */
            NULL                                /* vpointer                 User data */
            );



    ga_population_set_parameters(
            pop,                                /* population               *pop */
            GA_SCHEME_DARWIN,                   /* const ga_scheme_type     scheme */
            GA_ELITISM_PARENTS_SURVIVE,         /* const ga_elitism_type    elitism */
            crossover,                          /* double                   crossover */
            mutation,                           /* double                   mutation */
            migration                           /* double                   migration */
            );

    // allele_mutation_prob and allele ranges needed for multiple point mutation (does not have to be the same value as mutation)
    ga_population_set_allele_mutation_prob(pop, mutation);
    ga_population_set_allele_min_double(pop, 0.0);
    ga_population_set_allele_max_double(pop, 1.0);

    ga_population_seed(pop);

    // optionally load up a user population
    if (starting_population_file[0]) load_population(pop, starting_population_file);

    if (generations > 0)
    {
        ga_evolution(
            pop,                                /* population               *pop */
            generations                         /* const int                max_generations */
            );
    }
    plog(LOG_NORMAL, "RESULT %d GA Fitness = %g", 0, pop->entity_iarray[0]->fitness);

    // output the best solution
    solution = ga_get_entity_from_rank(pop, 0);

    double *array = new double[pop->len_chromosomes];
    for (int i = 0; i < pop->len_chromosomes; i++)
        array[i] = ((double *)solution->chromosome[0])[i];
    g_XMLConverter.ApplyGenome(pop->len_chromosomes, array);
    int docTxtLen;
    char *xml = g_XMLConverter.GetFormattedXML(&docTxtLen);

    sprintf(output_file, "%s/BestGenome.xml", dir_name);
    plog(LOG_VERBOSE, "Creating %s", output_file);
    std::ofstream best_genome_xml(output_file);
    best_genome_xml << xml;
    best_genome_xml.close();

    sprintf(output_file, "%s/BestGenome.txt", dir_name);
    plog(LOG_VERBOSE, "Creating %s", output_file);
    std::ofstream best_genome_text(output_file);
    best_genome_text.precision(17);
    best_genome_text.setf(std::ios::scientific, std:: ios::floatfield);
    best_genome_text << "-2\n" << pop->len_chromosomes << "\n";
    for (int i = 0; i < pop->len_chromosomes; i++)
        best_genome_text << array[i] << " 0.0 1.0 0.1 0\n";
    best_genome_text << solution->fitness << " 0 0 0 0\n";
    best_genome_text.close();
    delete [] array;

    ga_extinction(pop);

}

