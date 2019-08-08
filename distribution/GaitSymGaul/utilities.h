#ifndef UTILITIES_H
#define UTILITIES_H

#include "DataFile.h"

#include "gaul.h"

boolean load_population(population *pop, const char *starting_population_file);
void random_seed(DataFile *config_file);
bool create_output_folder(char *dir_name, int dir_name_size, int max_attempts = 1000);

#endif // UTILITIES_H
