#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import time
import GaitSym2019

if len(sys.argv) < 2:
    print('Must define the input XML file')
    sys.exit(1)

print('Test reading the file in C++')
t1 = time.perf_counter();

g = GaitSym2019.GaitSym2019()
err = g.SetArguments(sys.argv[0: -1])
if err:
    print('Error setting arguments')
    sys.exit(1)
err = g.ReadModel(sys.argv[-1])
if err:
    print('Error reading file')
    sys.exit(1)
err = g.Run()
if err:
    print('Error running GaitSym2019')
    sys.exit(1)
fitness = g.GetFitness()
print('fitness = ', fitness)

t2 = time.perf_counter();
print('Time elapsed = ', t2 - t1);

print('Test reading the file in python')
t1 = time.perf_counter();

with open(sys.argv[-1], 'r') as file:
    data = file.read()

g = GaitSym2019.GaitSym2019()
err = g.SetArguments(sys.argv[0: -1])
if err:
    print('Error setting arguments')
    sys.exit(1)
err = g.SetXML(data)
if err:
    print('Error setting XML file')
    sys.exit(1)
err = g.Run()
if err:
    print('Error running GaitSym2019')
    sys.exit(1)
fitness = g.GetFitness()
print('fitness = ', fitness)

t2 = time.perf_counter();
print('Time elapsed = ', t2 - t1);
