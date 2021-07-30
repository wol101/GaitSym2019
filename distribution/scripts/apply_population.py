#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import argparse
import glob
import re

# this line means that all the math functions do not require the math. prefix
from math import *

def apply_population():

    parser = argparse.ArgumentParser(description='Apply a population to a GaitSym XML config file')
    parser.add_argument('-p', '--population_file', default='', help='the population file to use (defaults to Population*.txt)')
    parser.add_argument('-i', '--input_xml_file', default='workingConfig.xml', help='the input GaitSym XML config file (defaults to workingConfig.xml)')
    parser.add_argument('-o', '--output_xml_file', default='', help='the output GaitSym XML config file (defaults to population_file genome_number with .xml)')
    parser.add_argument('-n', '--genome_number', type=int, default=0, help='the required genome (defaults to 0)')
    parser.add_argument('-r', '--genome_range', nargs=2, type=int, help='the required genome range (defaults to empty)')
    parser.add_argument('-l', '--recursion_limit', type=int, default=10000, help='set the python recursion limit (defaults to 10000)')
    parser.add_argument('-q', '--query', action='store_true', help='only query the population file by outputting genome number and fitness')
    parser.add_argument('-f', '--force', action='store_true', help='force overwrite of destination file')
    parser.add_argument('-v', '--verbose', action='store_true', help='write out more information whilst processing')
    args = parser.parse_args()

    # start by creating any missing arguments
    if not args.population_file:
        files = sorted(glob.glob('Population*.txt'))
        if not files:
            print("population_file not found: glob('Population*.txt') returned nothing")
            sys.exit(1)
        args.population_file = files[-1]
    if not args.output_xml_file:
        prefix, suffix = os.path.splitext(args.population_file)
        args.output_xml_file = '%s_%d.xml' % (prefix, args.genome_number)

    if args.verbose:
        pretty_print_sys_argv(sys.argv)
        pretty_print_argparse_args(args)

    if args.verbose:
        print('Checking files')
    preflight_read_file(args.population_file)
    preflight_read_file(args.input_xml_file)
        
    
    if args.verbose:
        print('Reading population file "%s"' % (args.population_file))
    with open(args.population_file, 'r') as f:
        lines = f.read().splitlines()
    
    index = 0
    n_genomes = int(lines[index])
    index = index + 1
    if args.verbose:
        print('n_genomes = %d' % (n_genomes))
    if args.genome_number < 0 or args.genome_number >= n_genomes:
        print('args.genome_number out of range')
        sys.exit(1)
    
    if args.query:
        for i in range(0, n_genomes):
            (genes, fitness, index) = parse_genome(lines, index, args)
            print('Genome n = %d fitness = %g' % (i, fitness))
        return
    
    if not args.genome_range:
        for i in range(0, args.genome_number + 1):
            (genes, fitness, index) = parse_genome(lines, index, args)

        if args.verbose:
            print('Genome %d read, fitness = %g' % (args.genome_number, fitness))

        if args.verbose:
            print('Reading input XML file "%s"' % (args.input_xml_file))
        with open(args.input_xml_file, 'r') as f:
            contents = f.read()

        if args.verbose:
            print('Parsing input XML file "%s"' % (args.input_xml_file))

        sys.setrecursionlimit(args.recursion_limit)
        new_contents = process_insert(contents, genes, args)

        preflight_write_file(args.output_xml_file, args.force)
        if args.verbose:
            print('Writing output XML file "%s"' % (args.output_xml_file))
        with open(args.output_xml_file, 'w') as f:
            f.write(new_contents)
        return
    
    # skip to start genome
    if args.verbose:
        print('Skipping to genome %d' % (args.genome_range[0]))
    for i in range(0, args.genome_range[0]):
        (genes, fitness, index) = parse_genome(lines, index, args)
    
    # now process the range
    for i in range(args.genome_range[0], min(args.genome_range[1], n_genomes)):
        (genes, fitness, index) = parse_genome(lines, index, args)
        args.output_xml_file = '%s_%d.xml' % (prefix, i)
        
        if args.verbose:
            print('Genome %d read, fitness = %g' % (args.genome_number, fitness))

        if args.verbose:
            print('Reading input XML file "%s"' % (args.input_xml_file))
        with open(args.input_xml_file, 'r') as f:
            contents = f.read()

        if args.verbose:
            print('Parsing input XML file "%s"' % (args.input_xml_file))

        sys.setrecursionlimit(args.recursion_limit)
        new_contents = process_insert(contents, genes, args)

        preflight_write_file(args.output_xml_file, args.force)
        if args.verbose:
            print('Writing output XML file "%s"' % (args.output_xml_file))
        with open(args.output_xml_file, 'w') as f:
            f.write(new_contents)
    return

def parse_genome(lines, index, args):
    if index > len(lines) - 3:
        print('Not enough lines in "%s"' % (args.genome_file))
        sys.exit(1)
    genome_type = int(lines[index])
    index = index + 1
    if args.verbose:
        print('genome_type = %d' % (genome_type))
    num_genes = int(lines[index])
    index = index + 1
    if args.verbose:
        print('num_genes = %d' % (num_genes))
    genes = []
    for g in range(0, num_genes):
        if index > len(lines):
            print('Not enough genes in "%s"' % (args.genome_file))
            sys.exit(1)
        tokens = lines[index].split()
        index = index + 1
        if len(tokens) < 1:
            print('Not enough gene tokens in "%s"' % (args.genome_file))
            sys.exit(1)
        genes.append(float(tokens[0]))
    tokens = lines[index].split()
    index = index + 1
    if len(tokens) < 1:
        print('Not enough fitness tokens in "%s"' % (args.genome_file))
        sys.exit(1)
    fitness = float(tokens[0])
    return (genes, fitness, index)

def process_insert(contents, genes, args):
    next_index_start = contents.find('[[')
    if next_index_start == -1:
        return contents
    next_index_end = contents.find(']]', next_index_start + 2)
    if next_index_end == -1:
        print('Unmatched [[ found at offset %d' % (next_index_start))
        sys.exit(1)
    insert_string = contents[next_index_start + 2: next_index_end]
    if args.verbose:
        print('insert_string "%s" found' % (insert_string))
    # replacing g[int] with g(int) here because this causes problems later
    # this replacement does not allow the int to be a function
    insert_string = re.sub(r'g\[(\d+)\]', r'g(\1)', insert_string)
    value = parse_insert(insert_string, genes)
    if args.verbose:
        print('replaced with "%.18e"' % (value))
    prefix = contents[: next_index_start]
    if next_index_end + 2 < len(contents):
        suffix = contents[next_index_end + 2:]
    else:
        suffix = ''
    return process_insert('%s%.18e%s' % (prefix, value, suffix), genes, args)

def parse_insert(insert_string, genes):

    # first step is to substitute the genes
    match = re.search(r'g *\(', insert_string)
    if match:
        closing_bracket = find_unmatched_close_bracket(insert_string[match.end():])
        if closing_bracket == -1:
            print('Error parsing genes in %s' % (insert_string))
            sys.exit(1)
        index = int(0.5 + parse_insert(insert_string[match.end(): match.end() + closing_bracket], genes))
        prefix = insert_string[:match.start()]
        if match.end() + closing_bracket + 1 < len(insert_string):
            suffix = insert_string[match.end() + closing_bracket + 1:]
        else:
            suffix = ''
        return parse_insert('%s%.18e%s' % (prefix, genes[index], suffix), genes)

    # second step is to handle functions not understood by python
    # if(test,result_when_true,result_when_false)
    match = re.search(r'if *\(', insert_string)
    if match:
        closing_bracket = find_unmatched_close_bracket(insert_string[match.end():])
        if closing_bracket == -1:
            print('Error parsing genes in %s' % (insert_string))
            sys.exit(1)
        argument_string = insert_string[match.end(): match.end() + closing_bracket]
        arguments = split_on_unbracketed_command(argument_string)
        if len(arguments) != 3:
            print('if() requires 3 arguments %s' % (insert_string))
            sys.exit(1)
        test_value = parse_insert(arguments[0], genes)
        if test_value == 0:
            result = arguments[2]
        else:
            result = arguments[1]
        prefix = insert_string[:match.start()]
        if match.end() + closing_bracket + 1 < len(insert_string):
            suffix = insert_string[match.end() + closing_bracket + 1:]
        else:
            suffix = ''
        return parse_insert('%s%s%s'% (prefix, result, suffix), genes)

    # we could now handle the other operators in order of precidence unaries, **^, */%, +-, booleans but there are a few gotchas (e.g. identifying unary operators) and python can now handle this
    # print(insert_string)
    return eval(insert_string)

def find_unmatched_close_bracket(input_string):
    num_brackets = 0
    for i in range(0, len(input_string)):
        if input_string[i] == '(':
            num_brackets = num_brackets + 1
        if input_string[i] == ')':
            num_brackets = num_brackets - 1
        if num_brackets < 0:
            return i
    return -1

def split_on_unbracketed_command(input_string):
    num_brackets = 0
    split_string = []
    last_string_start = 0
    for i in range(0, len(input_string)):
        if input_string[i] == '(':
            num_brackets = num_brackets + 1
        if input_string[i] == ')':
            num_brackets = num_brackets - 1
        if num_brackets < 0:
            print('Unmatched bracket found in "%s"' % (input_string))
            sys.exit(1)
        if num_brackets == 0 and input_string[i] == ',':
            split_string.append(input_string[last_string_start: i])
            last_string_start = i + 1
    if last_string_start >= i:
        split_string.append('')
    else:
        split_string.append(input_string[last_string_start:])
    return split_string
    
def preflight_read_file(filename):
    if not os.path.exists(filename):
        print("Error: \"%s\" not found" % (filename))
        sys.exit(1)
    if not os.path.isfile(filename):
        print("Error: \"%s\" not a file" % (filename))
        sys.exit(1)

def preflight_write_file(filename, force):
    if os.path.exists(filename) and not os.path.isfile(filename):
        print("Error: \"%s\" exists and is not a file" % (filename))
        sys.exit(1)
    if os.path.exists(filename) and not force:
        print("Error: \"%s\" exists. Use --force to overwrite" % (filename))
        sys.exit(1)
        
def pretty_print_sys_argv(sys_argv):
    quoted_sys_argv = quoted_if_necessary(sys_argv)
    print((' '.join(quoted_sys_argv)))

def pretty_print_argparse_args(argparse_args):
    for arg in vars(argparse_args):
        print(('%s: %s' % (arg, getattr(argparse_args, arg))))

def quoted_if_necessary(input_list):
    output_list = []
    for item in input_list:
        if re.search('[^a-zA-Z0-9_.-]', item):
            item = '"' + item + '"'
        output_list.append(item)
    return output_list

# program starts here

if __name__ == '__main__':
    apply_population()
