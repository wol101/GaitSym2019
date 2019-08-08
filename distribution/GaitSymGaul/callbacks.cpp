// these are the callback functions that GAUL needs for various operations during optimisation

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
#include "GSUtil.h"

#include "gaul.h"

#include "utilities.h"
#include "callbacks.h"

double callbacks::m_mutation_sd = 1.0;
XMLConverter *callbacks::m_xml_converter = 0;
Simulation *callbacks::m_simulation = 0;
int callbacks::m_sd_half_life = 0;

// this routine converts the chromosome representation to an array of doubles as
// required by the simplex algorithm
boolean callbacks::to_double(population *pop, entity *entity, double *array)
{
    if (!pop) die("Null pointer to population structure passed.");
    if (!entity) die("Null pointer to entity structure passed.");

    for (int i = 0; i < pop->len_chromosomes; i++)
    {
        array[i] = ((double *)entity->chromosome[0])[i];
    }

    return TRUE;
}

// this routine converts from the array of doubles required by the simplex algorithm
// back to the chromosome representation
boolean callbacks::from_double(population *pop, entity *entity, double *array)
{
    if (!pop) die("Null pointer to population structure passed.");
    if (!entity) die("Null pointer to entity structure passed.");
    if (!entity->chromosome) die("Entity has no chromsomes.");
    for (int i = 0; i < pop->len_chromosomes; i++)
    {
        ((double *)entity->chromosome[0])[i] = array[i];
    }

    return TRUE;
}

Simulation *callbacks::simulation()
{
    return m_simulation;
}

void callbacks::set_simulation(Simulation *simulation)
{
    m_simulation = simulation;
}

double callbacks::mutation_sd()
{
    return m_mutation_sd;
}

void callbacks::set_mutation_sd(double mutation_sd)
{
    m_mutation_sd = mutation_sd;
}

XMLConverter *callbacks::xml_converter()
{
    return m_xml_converter;
}

void callbacks::set_xml_converter(XMLConverter *xml_converter)
{
    m_xml_converter = xml_converter;
}

int callbacks::sd_half_life()
{
    return m_sd_half_life;
}

void callbacks::set_sd_half_life(int sd_half_life)
{
    m_sd_half_life = sd_half_life;
}


// calculate the fitness score of the chromosome
boolean callbacks::score(population *pop, entity *entity)
{

    // it may not be necessary to copy the entity but it seems to be an opaque datatype.
    double *array = new double[pop->len_chromosomes];
    for (int i = 0; i < pop->len_chromosomes; i++)
    {
        array[i] = ((double *)entity->chromosome[0])[i];
    }

    m_xml_converter->ApplyGenome(pop->len_chromosomes, array);
    int docTxtLen;
    char *xml = m_xml_converter->GetFormattedXML(&docTxtLen);

    // create the simulation object
    m_simulation = new Simulation();

    double fitness = 0;
    std::string *err = m_simulation->LoadModel(xml, size_t(docTxtLen));

    if (err == nullptr)
    {
        while (m_simulation->ShouldQuit() == false)
        {
            m_simulation->UpdateSimulation();

            if (m_simulation->TestForCatastrophy()) break;
        }
        fitness = m_simulation->CalculateInstantaneousFitness();
    }

    delete m_simulation;
    m_simulation = nullptr;
    entity->fitness = fitness;
    plog(LOG_DEBUG, "New fitness = %.17g", entity->fitness);

    delete [] array;
    return TRUE;
}


// called once per iteration so useful for logging the activity of the
// algorithm
boolean callbacks::iteration_callback(int iteration, entity *solution)
{
    plog( LOG_DEBUG, "Iteration: %d Fitness = %g", iteration, solution->fitness);

    return TRUE;
}

// called once per generation so useful for logging the activity of the
// algorithm
boolean callbacks::generation_callback(int generation, population *pop)
{
    int i;
    plog( LOG_DEBUG, "Generation: %d Fitness = %g", generation, pop->entity_iarray[0]->fitness);

    // I want to implement random migrants coming into the population.
    // I can do that by re-seeding the worst migration ratio fraction of
    // the population

    int migration_threshold = (int)((double)pop->size * (1.0 - pop->migration_ratio));
    for (i = migration_threshold; i < pop->size; i++)
    {
        seed(pop, pop->entity_iarray[i]);
    }

    if (m_sd_half_life)
    {
        // I also want the SD to decrease as I get more generations
        // this version reduces sd by a factor of 2 every m_sd_half_life generations
        if (generation % m_sd_half_life == 0 && generation > 0)
        {
            m_mutation_sd = m_mutation_sd / 2;
            plog( LOG_DEBUG, "m_mutation_sd =  %g", m_mutation_sd);
        }
    }

    // output the best solution
    entity *solution = ga_get_entity_from_rank(pop, 0);

    double *array = new double[pop->len_chromosomes];
    for (int i = 0; i < pop->len_chromosomes; i++)
        array[i] = ((double *)solution->chromosome[0])[i];
    m_xml_converter->ApplyGenome(pop->len_chromosomes, array);
    int docTxtLen;
    char *xml = m_xml_converter->GetFormattedXML(&docTxtLen);

    char filename[64];
    sprintf(filename, "BestGenome%05d.xml", generation);
    std::ofstream best_genome_xml(filename);
    best_genome_xml << xml;
    best_genome_xml.close();

    sprintf(filename, "BestGenome%05d.txt", generation);
    std::ofstream best_genome_text(filename);
    best_genome_text.precision(16);
    for (int i = 0; i < pop->len_chromosomes; i++)
        best_genome_text << array[i] << "\n";
    best_genome_text.close();
    delete [] array;

    return TRUE;
}

// fill the chromosome with initial (probably random) data
boolean callbacks::seed(population *pop, entity *adam)
{
    /* Checks. */
    if (!pop) die("Null pointer to population structure passed.");
    if (!adam) die("Null pointer to entity structure passed.");

    /* Seeding. */

    for (int i = 0; i < pop->len_chromosomes; i++)
    {
        ((double *)adam->chromosome[0])[i] = random_double_range(pop->allele_min_double, pop->allele_max_double);
    }

    return TRUE;
}

// wis - this routine altered to reduce the SD of the mutation by using
// a global value: mutation_sd
/**********************************************************************
  ga_mutate_double_multipoint()
  synopsis:	Cause a number of mutation events.  This is equivalent
                to the more common 'bit-drift' mutation.
                (Unit Gaussian distribution.)
  parameters:
  return:
  last updated: 17 Feb 2005
 **********************************************************************/

void callbacks::ga_mutate_double_multipoint_usersd(population *pop, entity *father, entity *son)
{
    int		i;		/* Loop variable over all chromosomes */
    int		chromo;		/* Index of chromosome to mutate */
    int		point;		/* Index of allele to mutate */

    /* Checks */
    if (!father || !son) die("Null pointer to entity structure passed");

    /* Copy chromosomes of parent to offspring. */
    for (i=0; i<pop->num_chromosomes; i++)
    {
        memcpy(son->chromosome[i], father->chromosome[i], pop->len_chromosomes*sizeof(double));
    }

    /*
     * Mutate by tweaking alleles.
     */
    for (chromo=0; chromo<pop->num_chromosomes; chromo++)
    {
        for (point=0; point<pop->len_chromosomes; point++)
        {
            if (random_boolean_prob(pop->allele_mutation_prob))
            {
                ((double *)son->chromosome[chromo])[point] += (random_unit_gaussian() * m_mutation_sd);

                if (((double *)son->chromosome[chromo])[point] > pop->allele_max_double)
                    ((double *)son->chromosome[chromo])[point] -= (pop->allele_max_double-pop->allele_min_double);
                if (((double *)son->chromosome[chromo])[point] < pop->allele_min_double)
                    ((double *)son->chromosome[chromo])[point] += (pop->allele_max_double-pop->allele_min_double);
            }
        }
    }

    return;
}

// this is the mutate allele function required by the hill climbing functions
// is is basically the same as the normal mutator but is specifies exactly which allele is interesting
boolean callbacks::mutate_allele(population *pop, entity *father, entity *son, const int chromo_id, const int allele_id)
{
    /* Checks */
    if (!father || !son) die("Null pointer to entity structure passed");

    /* Copy chromosomes of parent to offspring. */
    memcpy(son->chromosome[chromo_id], father->chromosome[chromo_id], pop->len_chromosomes*sizeof(double));

    /*
     * Mutate by tweaking alleles.
     */
    ((double *)son->chromosome[chromo_id])[allele_id] += (random_unit_gaussian() * m_mutation_sd);

    if (((double *)son->chromosome[chromo_id])[allele_id] > pop->allele_max_double)
        ((double *)son->chromosome[chromo_id])[allele_id] -= (pop->allele_max_double-pop->allele_min_double);
    if (((double *)son->chromosome[chromo_id])[allele_id] < pop->allele_min_double)
        ((double *)son->chromosome[chromo_id])[allele_id] += (pop->allele_max_double-pop->allele_min_double);

    return TRUE;
}


void  callbacks::log_callback(const enum log_level_type level,
                                    const char *func_name,
                                    const char *file_name,
                                    const int line_num,
                                    const char *message)
{
    // FIX ME - do something clever here
}


