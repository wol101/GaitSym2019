#ifndef OBJECTIVEMAIN_H
#define OBJECTIVEMAIN_H

#include <string>

int ReadModel(void *userData = 0);
int WriteModel();
void ParseArguments(int argc, char ** argv);
bool GetOption(char ** begin, char ** end, const std::string &option, char **ptr);
bool GetOption(char ** begin, char ** end, const std::string &option, double *ptr);
bool GetOption(char ** begin, char ** end, const std::string &option, int *ptr);
bool GetOption(char** begin, char** end, const std::string &option);


#endif // OBJECTIVEMAIN_H
