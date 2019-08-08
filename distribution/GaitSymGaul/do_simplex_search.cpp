// function to perform a simplex search

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

#include "do_simplex_search.h"

#define THROWIFZERO(a) if ((a) == 0) throw __LINE__
#define THROWIF(a) if ((a) != 0) throw __LINE__

void do_simplex_search(DataFile *config_file)
{
    int population_size;
    int chromosome_size;
    char model_config_file[512];
    int simplex_max_iterations;
    double simplex_initial_stepsize;
    double allele_min;
    double allele_max;
    XMLConverter xml_converter;

    int gaul_log_level = 4; // 0 none, 1 fatal, 2, warnings, 3 normal, 4 verbose, 5 fixme, 6 debug
    char starting_population_file[512];
    bool starting_population_sorted = false;
    starting_population_file[0] = 0;

    try
    {
        THROWIF(config_file->RetrieveQuotedStringParameter("model_config_file", model_config_file, 512));
        THROWIF(config_file->RetrieveParameter("population_size", &population_size));
        THROWIF(config_file->RetrieveParameter("chromosome_size", &chromosome_size));
        THROWIF(config_file->RetrieveParameter("simplex_max_iterations", &simplex_max_iterations));
        THROWIF(config_file->RetrieveParameter("simplex_initial_stepsize", &simplex_initial_stepsize));
        THROWIF(config_file->RetrieveParameter("allele_min", &allele_min));
        THROWIF(config_file->RetrieveParameter("allele_max", &allele_max));

        config_file->RetrieveParameter("starting_population_file", starting_population_file, 512);
        config_file->RetrieveParameter("starting_population_sorted", &starting_population_sorted);
        config_file->RetrieveParameter("gaul_log_level", &gaul_log_level);
    }

    catch (int e)
    {
        dief("Error on line %d\nMissing parameter in config_file", e);
    }

    random_seed(config_file);

    if (xml_converter.LoadBaseXMLFile(model_config_file)) dief("LoadBaseXMLFile \"%s\"error", model_config_file);

    // turn on logging
    char log_filename[512];
    strcpy(log_filename,"SimplexLog.txt");
    log_init(static_cast<log_level_type>(gaul_log_level), log_filename, 0, 1);

    population          *pop = 0;                   /* Population of solutions. */
    entity              *solution = 0;              /* Optimised solution. */

    char dir_name[256], output_file[256];
    create_output_folder(dir_name, sizeof(dir_name));

    callbacks::set_xml_converter(&xml_converter);

    // set up the initial population
    pop = ga_genesis_double(
            population_size,                    /* const int                population_size */
            1,                                  /* const int                num_chromo */
            chromosome_size,                    /* const int                len_chromo */
            NULL,                               /* GAgeneration_hook        generation_hook */
            callbacks::iteration_callback,                 /* GAiteration_hook         iteration_hook */
            NULL,                               /* GAdata_destructor        data_destructor */
            NULL,                               /* GAdata_ref_incrementor   data_ref_incrementor */
            callbacks::score,                              /* GAevaluate               evaluate */
            callbacks::seed,                               /* GAseed                   seed */
            NULL,                               /* GAadapt                  adapt */
            NULL,                               /* GAselect_one             select_one */
            NULL,                               /* GAselect_two             select_two */
            NULL,                               /* GAmutate                 mutate */
            NULL,                               /* GAcrossover              crossover */
            NULL,                               /* GAreplace                replace */
            NULL                                /* vpointer                 User data */
            );

    ga_population_set_simplex_parameters(
               pop,                                /* population               *pop */
               chromosome_size,                    /* const int                num_dimensions */
               simplex_initial_stepsize,           /* const double             Initial step size. */
               callbacks::to_double,                          /* const GAto_double        to_double */
               callbacks::from_double                         /* const GAfrom_double      from_double */
               );

    // allele ranges needed for setting the search limits
    ga_population_set_allele_min_double(pop, allele_min);
    ga_population_set_allele_max_double(pop, allele_max);

    ga_population_seed(pop);

    // optionally load up a user population
    if (starting_population_file[0])
    {
        load_population(pop, starting_population_file);
        if (starting_population_sorted == false)
        {
            /* Evaluate and sort the initial population members (i.e. select best of the random solutions. */
            ga_population_score_and_sort(pop);
        }
        else
        {
            plog(LOG_VERBOSE, "Best stored fitness =  %g", pop->entity_iarray[0]->fitness);
            pop->evaluate(pop, pop->entity_iarray[0]);
//            double *array = new double[pop->len_chromosomes];
//            for (int i = 0; i < pop->len_chromosomes; i++)
//                array[i] = ((double *)pop->entity_iarray[0]->chromosome[0])[i];
//            xml_converter.ApplyGenome(pop->len_chromosomes, array);
//            int docTxtLen;
//            char *xml = xml_converter.GetFormattedXML(&docTxtLen);
//            std::ofstream test_genome_xml("test.xml");
//            test_genome_xml << xml;
//            test_genome_xml.close();
//            exit(1);
        }
    }
    else
    {
        /* Evaluate and sort the initial population members (i.e. select best of the random solutions. */
        ga_population_score_and_sort(pop);
    }
    plog(LOG_NORMAL, "Best starting fitness =  %g", pop->entity_iarray[0]->fitness);

    int i_ent = 0; // use the best rank for the initial solution
    solution = ga_get_entity_from_rank(pop, i_ent);

    ga_simplex(
                pop,                                /* population               *pop */
                solution,                           /* entity                   *solution */
                simplex_max_iterations              /* const int                max_iterations */
                );

    plog(LOG_NORMAL, "RESULT %d Simplex Fitness = %g", i_ent, solution->fitness);

    // output the best solution
    double *array = new double[pop->len_chromosomes];
    for (int i = 0; i < pop->len_chromosomes; i++)
        array[i] = ((double *)solution->chromosome[0])[i];
    xml_converter.ApplyGenome(pop->len_chromosomes, array);
    int docTxtLen;
    char *xml = xml_converter.GetFormattedXML(&docTxtLen);

    sprintf(output_file, "%s/%05dBestGenome.xml", dir_name, i_ent);
    plog(LOG_VERBOSE, "Creating %s", output_file);
    std::ofstream best_genome_xml(output_file);
    best_genome_xml << xml;
    best_genome_xml.close();

    sprintf(output_file, "%s/%05dBestGenome.txt", dir_name, i_ent);
    plog(LOG_VERBOSE, "Creating %s", output_file);
    std::ofstream best_genome_text(output_file);
    best_genome_text.precision(17);
    best_genome_text.setf(std::ios::scientific, std::ios::floatfield);
    best_genome_text << "-2\n" << pop->len_chromosomes << "\n";
    for (int i = 0; i < pop->len_chromosomes; i++)
        best_genome_text << array[i] << " 0.0 1.0 0.1 0\n";
    best_genome_text << solution->fitness << " 0 0 0 0\n";
    best_genome_text.close();
    delete [] array;

    ga_extinction(pop);

}

