#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import argparse
import glob
import re

# this line means that all the math functions do not require the math. prefix
from math import *

# doing this makes args a global which is not the worst thing in the world
parser = argparse.ArgumentParser(description='Apply a genome to a GaitSym XML config file')
parser.add_argument('-g', '--genome_file', default='', help='the genome file to use (defaults to last BestGenome*.txt)')
parser.add_argument('-i', '--input_xml_file', default='workingConfig.xml', help='the input GaitSym XML config file (defaults to workingConfig.xml)')
parser.add_argument('-o', '--output_xml_file', default='', help='the output GaitSym XML config file (defaults to genome_file name with .xml)')
parser.add_argument('-l', '--recursion_limit', type=int, default=10000, help='set the python recursion limit (defaults to 10000)')
parser.add_argument('-f', '--force', action='store_true', help='force overwrite of destination file')
parser.add_argument('-v', '--verbose', action='store_true', help='write out more information whilst processing')
args = parser.parse_args()

def apply_genome():

    # start by creating any missing arguments
    if not args.genome_file:
        files = sorted(glob.glob('BestGenome*.txt'))
        if not files:
            print("genome_file not found: glob('BestGenome*.txt') returned nothing")
            sys.exit(1)
        args.genome_file = files[-1]
    if not args.output_xml_file:
        prefix, suffix = os.path.splitext(args.genome_file)
        args.output_xml_file = prefix + '.xml'

    if args.verbose:
        pretty_print_sys_argv(sys.argv)
        pretty_print_argparse_args(args)

    if args.verbose:
        print('Checking files')
    if os.path.exists(args.genome_file) == False:
        print('genome_file "%s" not found' % (args.genome_file))
        sys.exit(1)
    if os.path.exists(args.input_xml_file) == False:
        print('input_xml_file "%s" not found' % (args.input_xml_file))
        sys.exit(1)
    if os.path.exists(args.output_xml_file) == True:
        if os.path.isfile(args.output_xml_file) == False:
            print('output_xml_file "%s" exists and is not a file' % (args.output_xml_file))
            sys.exit(1)
        if args.force == False:
            print('output_xml_file "%s" exists, use --force to overwrite' % (args.output_xml_file))
            sys.exit(1)

    if args.verbose:
        print('Reading gemome file "%s"' % (args.genome_file))
    f = open(args.genome_file, 'r')
    lines = f.readlines()
    f.close()
    genome_type = int(lines[0])
    if args.verbose:
        print('genome_type = %d' % (genome_type))
    num_genes = int(lines[1])
    if args.verbose:
        print('num_genes = %d' % (num_genes))
    genes = []
    for g in range(0, num_genes):
        l = g + 2
        if l > len(lines):
            print('Not enough genes in "%s"' % (args.genome_file))
            sys.exit(1)
        tokens = lines[l].split()
        if len(tokens) < 1:
            print('Not enough tokens in "%s"' % (args.genome_file))
            sys.exit(1)
        tokens = lines[l].split()
        genes.append(float(tokens[0]))

    if args.verbose:
        print('Reading input XML file "%s"' % (args.input_xml_file))
    f = open(args.input_xml_file, 'r')
    contents = f.read()
    f.close()

    if args.verbose:
        print('Parsing input XML file "%s"' % (args.input_xml_file))

    sys.setrecursionlimit(args.recursion_limit)
    new_contents = process_insert(contents, genes, args)

    if args.verbose:
        print('Writing output XML file "%s"' % (args.output_xml_file))
    f = open(args.output_xml_file, 'w')
    f.write(new_contents)
    f.close()


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
    # this routine is a little fragile because exprtk and python do not use quite the same syntax
    # we try to fix things up in a rather dumb way that won't work in all cases
    # most of the math functions come from the "from math import *" line so that the
    # python eval statement can do all the work

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

    # exprtk uses ^ when python uses **
    insert_string = insert_string.replace('^', '**')
    
    # we could now handle the other operators in order of precidence unaries, **^, */%, +-, booleans 
    # but there are a few gotchas (e.g. identifying unary operators) and python can now handle this
    if args.verbose: print('eval("%s")' % (insert_string))
    ret_val = eval(insert_string)
    return ret_val

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
    apply_genome()
