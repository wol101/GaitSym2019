#ifndef CALLBACKS_H
#define CALLBACKS_H

// these are the callback functions that GAUL needs for various operations during optimisation

#include "gaul.h"

class Simulation;
class XMLConverter;

class callbacks
{
public:
    static boolean score(population *pop, entity *entity);
    static boolean iteration_callback(int iteration, entity *solution);
    static boolean generation_callback(int generation, population *pop);
    static boolean seed(population *pop, entity *adam);
    static void ga_mutate_double_multipoint_usersd(population *pop, entity *father, entity *son);
    static boolean mutate_allele(population *pop, entity *father, entity *son, const int chromo_id, const int allele_id);
    static boolean to_double(population *pop, entity *entity, double *array);
    static boolean from_double(population *pop, entity *entity, double *array);
    static void log_callback(const enum log_level_type level, const char *func_name, const char *file_name, const int line_num, const char *message);

    static Simulation *simulation();
    static void set_simulation(Simulation *simulation);

    static double mutation_sd();
    static void set_mutation_sd(double mutation_sd);

    static XMLConverter *xml_converter();
    static void set_xml_converter(XMLConverter *xml_converter);

    static int sd_half_life();
    static void set_sd_half_life(int sd_half_life);

private:
    static Simulation *m_simulation;
    static double m_mutation_sd;
    static XMLConverter *m_xml_converter;
    static int m_sd_half_life;
};

#endif // CALLBACKS_H

