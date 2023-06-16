/*
 *  ObjectiveMainMPI.h
 *  GaitSym2019
 *
 *  Created by Bill Sellers on 24/12/2019.
 *  Copyright 2019 Bill Sellers. All rights reserved.
 *
 */

#ifndef OBJECTIVEMAINMPI_H
#define OBJECTIVEMAINMPI_H

int GaitSymMain(int argc, char *argv[]);
void ParseArguments(int argc, char ** argv);
int ReadModel(void *userData = nullptr);
int WriteModel();

#endif // OBJECTIVEMAINMPI_H
