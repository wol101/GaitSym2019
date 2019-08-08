#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#include "utilities.h"

#ifdef _MSC_VER
#include <process.h>
#include <direct.h>
#endif

// load the population with one stored in a standard GaitSym population file
// returns true on success
boolean load_population(population *pop, const char *starting_population_file)
{
    entity *ent;
    int len_pop = 0;
    int gene_type = 0;
    double len_gene, gene_val, gene_min, gene_max, gene_sd, fitness;
    int gene_circ, parent1_gen_num, parent1_rank, parent2_gen_num, parent2_rank;

    plog(LOG_DEBUG, "Loading starting_population_file =  %s", starting_population_file);

    std::ifstream population_stream(starting_population_file);
    if (population_stream.is_open() == false) die("Unable to open starting_population_file");
    population_stream >> len_pop;
    for (int i_pop = 0; i_pop < len_pop && i_pop < pop->size; i_pop++)
    {
        ent = ga_get_entity_from_rank(pop, i_pop);
        population_stream >> gene_type;
        switch (gene_type)
        {
        case -1:
            population_stream >> len_gene;
            if (len_gene != pop->len_chromosomes) die("Genome length does not match model population.");
            for (int i = 0; i < len_gene; i++)
            {
                population_stream >> gene_val >> gene_min >> gene_max >> gene_sd;
                ((double *)ent->chromosome[0])[i] = gene_val;
            }
            population_stream >> fitness >> parent1_gen_num >> parent1_rank >> parent2_gen_num >> parent2_rank;
            ent->fitness = fitness;
            break;

        case -2:
            population_stream >> len_gene;
            if (len_gene != pop->len_chromosomes) die("Genome length does not match model population.");
            for (int i = 0; i < len_gene; i++)
            {
                population_stream >> gene_val >> gene_min >> gene_max >> gene_sd >> gene_circ;
                ((double *)ent->chromosome[0])[i] = gene_val;
            }
            population_stream >> fitness >> parent1_gen_num >> parent1_rank >> parent2_gen_num >> parent2_rank;
            ent->fitness = fitness;
            break;

        default:
            die("Unrecognised population type");
        }
    }
    population_stream.close();
    return true;
}

// handle the random seed based either on system clock or in file
void random_seed(DataFile *config_file)
{
    int random_seed_value;
    if (config_file->RetrieveParameter("random_seed_value", &random_seed_value))
    {
        // seed the random number generator based on the system clock
        random_init();
#ifndef _MSC_VER
        random_seed(time(0) * getpid());
#else
        random_seed(time(0) * _getpid());
#endif
        // random_tseed();
    }
    else
    {
        random_init();
        random_seed(random_seed_value);
    }
}

// create a unique output folder based on the current time
// returns true on success
bool create_output_folder(char *dir_name, int dir_name_size, int max_attempts)
{
    // create the output folder
    int err;
    time_t the_time;
    struct tm *the_local_time;
    int attempts = 0;
    int required_length;
    int dir_count = 0;
    do
    {
        the_time = time(0);
        the_local_time = localtime(&the_time);
        required_length = snprintf(dir_name, dir_name_size, "%04d_Run_%04d-%02d-%02d_%02d.%02d.%02d", dir_count, the_local_time->tm_year + 1900, the_local_time->tm_mon + 1,
                the_local_time->tm_mday, the_local_time->tm_hour, the_local_time->tm_min, the_local_time->tm_sec);
        if (required_length > (dir_name_size - 1)) return false;
        plog(LOG_NORMAL, "Creating folder  %s", dir_name);
#if !defined(_MSC_VER)
#if defined(_WIN32) || defined(__WIN32__)
        err = mkdir(dir_name);
#else
        err = mkdir(dir_name, 0777);
#endif
#else
        err = _mkdir(dir_name);
#endif
        if (err == 0) return true;
#ifndef _MSC_VER
        sleep(1);
#else
        _sleep(1);
#endif
        attempts++;
    }
    while (attempts < max_attempts || max_attempts == 0);

    return false;
}


