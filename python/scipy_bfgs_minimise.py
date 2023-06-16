#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import argparse
import re
import scipy
import scipy.optimize
import json
import GaitSym2019

# this is a parallel version of scipy.optimise.minimize(method=’L-BFGS-B’)
#from optimparallel import minimize_parallel
import optimparallel

# this line means that all the math functions do not require the math. prefix
from math import *

def scipy_bfgs_minimise():

    parser = argparse.ArgumentParser(description="Use various scipy.optimize routines on a gaitsym problem (minimise score)")
    parser.add_argument("-i", "--input_xml_file", required=True, help="GaitSym config file with substitution indicators")
    parser.add_argument("-o", "--output_xml_file", required=True, help="GaitSym config file with best solution substituted")
    parser.add_argument("-r", "--ranges_file", required=True, help="Ranges file")
    parser.add_argument("-l", "--log_file", default="", help="If set log to the specified file")
    parser.add_argument("-m", "--max_workers", type=int, help="Override the default number of worker threads")
    parser.add_argument("-j", "--json_string", default="", help="Use this JSON string to set the arguments required or read from this file")
    parser.add_argument("-n", "--negate_score", action="store_true", help="Negate the score")
    parser.add_argument("-p", "--parallel", action="store_true", help="Use the parallel version of the optimiser")
    parser.add_argument("-f", "--force", action="store_true", help="Force overwrite of existing files")
    parser.add_argument("-v", "--verbose", action="store_true", help="Write out more information whilst processing")
    args = parser.parse_args()

    if len(args.log_file) > 0:
        preflight_write_file(args.log_file, args.force, args.verbose)
        # enable some logging (extra argument turns off buffering)
        so = se = open(args.log_file, 'wb', 0)
        # redirect stdout and stderr to the log file opened above
        os.dup2(so.fileno(), sys.stdout.fileno())
        os.dup2(se.fileno(), sys.stderr.fileno())

    if args.verbose:
        pretty_print_sys_argv(sys.argv)
        pretty_print_argparse_args(args)
    
    # preflight
    preflight_read_file(args.input_xml_file, args.verbose)
    preflight_read_file(args.ranges_file, args.verbose)
    preflight_write_file(args.output_xml_file, args.force, args.verbose)
        
    if os.path.isfile(args.json_string): # if json_string is a valid file then read its contents
        with open(args.json_string, 'r', encoding='utf8') as f_in:
            data = f_in.read()
        args.json_string = data
    
    if args.json_string and args.json_string.strip():
        try:
            json_object = json.loads(args.json_string)
        except ValueError as e:
            print('"%s" is neither a valid file nor a valid JSON string' % (args.json_string))
            sys.exit(1)
        if not isinstance(json_object, dict):
            print('Error interpreting JSON string "%s" as dict' % (args.json_string))
            sys.exit(1)

    if args.verbose: print('Reading "%s"' % (args.ranges_file))
    ranges_dict = read_ranges(args.ranges_file)
    
    # read the config file
    if args.verbose: print('Reading "%s"' % (args.input_xml_file))
    with open(args.input_xml_file, 'r', encoding='utf8') as f_in:
        data = f_in.read()
    (smart_substitution_text_components, smart_substitution_parser_text) = create_smart_substitution(data)
    if args.verbose: print(f'{len(smart_substitution_text_components) = }')
    if args.verbose: print(f'{smart_substitution_parser_text = }')
    
    extra_args = []
    extra_args.append(smart_substitution_text_components)
    extra_args.append(smart_substitution_parser_text)
    extra_args.append(args.verbose)
    extra_args.append(args.negate_score)
    extra_args.append("")

    x0 = do_l_bgfs_b(ranges_dict, args.json_string, extra_args, args.verbose, args.parallel, args.max_workers)
    
    if len(args.output_xml_file) > 0 and len(x0) > 0:
        if args.verbose: print('Writing "%s"' % (args.output_xml_file))
        extra_args = []
        extra_args.append(smart_substitution_text_components)
        extra_args.append(smart_substitution_parser_text)
        extra_args.append(args.verbose)
        extra_args.append(args.negate_score)
        extra_args.append(args.output_xml_file)
        evaluate_fitness(x0, extra_args)

def do_l_bgfs_b(ranges_dict, json_string, extra_args, verbose, parallel, max_workers):

    x0_list = []
    bounds_list = []
    for i in range(0, len(ranges_dict['estimate'])):
        x0_list.append(ranges_dict['estimate'][i])
        bounds_list.append((ranges_dict['low'][i], ranges_dict['high'][i]))
    print(bounds_list)
    # note: null in a JSON string gets mapped to python None
    default_json = '{"disp": null, "maxcor": 10, "ftol": 2.220446049250313e-09, "gtol": 1e-05, "eps": 1e-08, "maxfun": 15000, "maxiter": 15000, "iprint": -1, "maxls": 20, "finite_diff_rel_step": null}'
    if not json_string or not json_string.strip(): json_string = default_json
    parameters_dict = json.loads(json_string)
    if not isinstance(parameters_dict, dict):
        print('Error interpreting JSON string "%s" as dict' % (json_string))
        sys.exit(1)
    check_dict(parameters_dict, default_json)
    
    if verbose:
        parameters_dict['disp'] = True

    if parallel:
        if max_workers:
            res = optimparallel.minimize_parallel(fun=evaluate_fitness_parallel, x0=x0_list, args=extra_args, bounds=bounds_list, parallel={"max_workers": max_workers})
        else:
            res = optimparallel.minimize_parallel(fun=evaluate_fitness_parallel, x0=x0_list, args=extra_args, bounds=bounds_list)
    else:
        res = scipy.optimize.minimize(fun=evaluate_fitness, x0=x0_list, args=extra_args, bounds=bounds_list, method='L-BFGS-B')

    if verbose: print(res)
    return res.x
    
def evaluate_fitness_parallel(x, arg0, arg1, arg2, arg3, arg4):
    args = (arg0, arg1, arg2, arg3, arg4)
    return evaluate_fitness(x, args)

def evaluate_fitness(x, args):
    # create the file to assess
    smart_substitution_text_components = args[0]
    smart_substitution_parser_text = args[1]
    verbose = args[2]
    negate_score = args[3]
    output_filename = args[4]
    data_list = [smart_substitution_text_components[0]]
    for i in range(0, len(smart_substitution_parser_text)):
        if verbose: print(f'{smart_substitution_parser_text[i] = }')
        v = parse_insert(smart_substitution_parser_text[i], x, verbose)
        # v = eval(smart_substitution_parser_text[i], global_dict)
        data_list.append('%.17e' % (v))
        data_list.append(smart_substitution_text_components[i + 1])
    file_contents = ''.join(data_list)
    if output_filename:
        with open(output_filename, 'w') as f_out:
            f_out.write(file_contents)
    
    gaitsym = GaitSym2019.GaitSym2019()
    err = gaitsym.SetXML(file_contents)
    if err:
        print('Error setting XML file')
        sys.exit(1)
    err = gaitsym.Run()
    if err:
        print('Error running GaitSym2019')
        if verbose: print(file_contents)
        sys.exit(1)
    fitness = gaitsym.GetFitness()
    if negate_score:
        fitness = -fitness
    if verbose: print('fitness = ', fitness)

    return fitness

def check_dict(input_dict, example_json):
    example_dict = json.loads(example_json)
    for key in example_dict.keys():
        if not key in input_dict.keys():
            print('Key "%s" missing from JSON string. Typical example might be\n\'%s\'' % (key, example_json))
            sys.exit(1)
    for key in input_dict.keys():
        if not key in example_dict.keys():
            print('Warning key "%s" not used' % (key))
    return

def parse_insert(insert_string, genes, verbose):    
    # this routine is a little fragile because exprtk and python do not use quite the same syntax
    # we try to fix things up in a rather dumb way that won't work in all cases
    # most of the math functions come from the "from math import *" line so that the
    # python eval statement can do all the work
    # but I could probabl;y do something cleverer by spcifying globals and locals to the 
    # eval statement (but the globals() would be needed for all the math functions)

    # first step is to substitute the genes
    match = re.search(r'g *\(', insert_string)
    if match:
        closing_bracket = find_unmatched_close_bracket(insert_string[match.end():])
        if closing_bracket == -1:
            print('Error parsing genes in %s' % (insert_string))
            sys.exit(1)
        index = int(0.5 + parse_insert(insert_string[match.end(): match.end() + closing_bracket], genes, verbose))
        prefix = insert_string[:match.start()]
        if match.end() + closing_bracket + 1 < len(insert_string):
            suffix = insert_string[match.end() + closing_bracket + 1:]
        else:
            suffix = ''
        return parse_insert('%s%.18e%s' % (prefix, genes[index], suffix), genes, verbose)

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
        test_value = parse_insert(arguments[0], genes, verbose)
        if test_value == 0:
            result = arguments[2]
        else:
            result = arguments[1]
        prefix = insert_string[:match.start()]
        if match.end() + closing_bracket + 1 < len(insert_string):
            suffix = insert_string[match.end() + closing_bracket + 1:]
        else:
            suffix = ''
        return s('%s%s%s'% (prefix, result, suffix), genes, verbose)

    # exprtk uses ^ when python uses **
    insert_string = insert_string.replace('^', '**')
    
    # we could now handle the other operators in order of precidence unaries, **^, */%, +-, booleans 
    # but there are a few gotchas (e.g. identifying unary operators) and python can now handle this
    if verbose: print('eval("%s")' % (insert_string))
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

def create_smart_substitution(data):
    smart_substitution_text_components = []
    smart_substitution_parser_text = []
    ptr1 = 0
    ptr2 = data.find("[[")
    if ptr2 == -1:
        print("Error: could not find any [[\n")
        sys.exit(1)
    while True:
        # print(ptr1, ptr2)
        smart_substitution_text_components.append(data[ptr1: ptr2])
        ptr2 += 2
        ptr1 = data[ptr2:].find("]]")
        if ptr1 == -1:
            print("Error: could not find matching ]]\n")
            sys.exit(1)
        ptr1 += ptr2
        smart_substitution_parser_text.append(data[ptr2: ptr1])
        ptr1 += 2
        ptr2 = data[ptr1:].find("[[")
        if ptr2 == -1:
            break
        ptr2 += ptr1
    smart_substitution_text_components.append(data[ptr1:])
    for i in range(0, len(smart_substitution_parser_text)): # convert g[] to g()
        smart_substitution_parser_text[i] = re.sub(r'g\[(\d+)\]', r'g(\1)', smart_substitution_parser_text[i])
    return (smart_substitution_text_components, smart_substitution_parser_text)

def read_ranges(file_name):
    ranges = {}
    with open(file_name, 'r', encoding='utf8') as f_in:
        lines = f_in.read().splitlines()
    if len(lines) == 0:
        print('Error: could not parse "%s"' % (file_name))
        sys.exit(1)
    column_names = lines[0].split()
    if len(column_names) == 0:
        print('Error: could not parse column_names in "%s"' % (file_name))
        sys.exit(1)
    for column_name in column_names:
        ranges[column_name] = []
    
    for i in range(1, len(lines)):
        tokens = lines[i].split()
        if len(tokens) == 0: # skip blank lines
            continue
        if len(tokens) != len(column_names):
            print('Error: could not parse data in "%s"' % (file_name))
            sys.exit(1)
        for j in range(0, len(tokens)):
            ranges[column_names[j]].append(float(tokens[j]))
    return ranges
    

def preflight_read_file(filename, verbose):
    if verbose: print('preflight_read_file: "%s"' % (filename))
    if not os.path.exists(filename):
        print("Error: \"%s\" not found" % (filename))
        sys.exit(1)
    if not os.path.isfile(filename):
        print("Error: \"%s\" not a file" % (filename))
        sys.exit(1)

def preflight_write_file(filename, force, verbose):
    if verbose: print('preflight_write_file: "%s"' % (filename))
    if os.path.exists(filename) and not os.path.isfile(filename):
        print("Error: \"%s\" exists and is not a file" % (filename))
        sys.exit(1)
    if os.path.exists(filename) and not force:
        print("Error: \"%s\" exists. Use --force to overwrite" % (filename))
        sys.exit(1)

def preflight_read_folder(folder, verbose):
    if verbose: print('preflight_read_folder: "%s"' % (folder))
    if not os.path.exists(folder):
        print("Error: \"%s\" not found" % (folder))
        sys.exit(1)
    if not os.path.isdir(folder):
        print("Error: \"%s\" not a folder" % (folder))
        sys.exit(1)

def preflight_write_folder(folder, verbose):
    if verbose: print('preflight_write_folder: "%s"' % (folder))
    if os.path.exists(folder):
        if not os.path.isdir(folder):
            print("Error: \"%s\" exists and is not a folder" % (folder))
            sys.exit(1)
    else:
        try:
            os.makedirs(folder, exist_ok = True)
        except OSError as error:
            print('Directory "%s" can not be created' % folder)
            sys.exit(1)

def is_a_number(string):
    """checks to see whether a string is a valid number"""
    if re.match(r'^([+-]?)(?=\d|\.\d)\d*(\.\d*)?([Ee]([+-]?\d+))?$', string.strip()) == None:
        return False
    return True

def pretty_print_sys_argv(sys_argv):
    quoted_sys_argv = quoted_if_necessary(sys_argv)
    print((" ".join(quoted_sys_argv)))

def pretty_print_argparse_args(argparse_args):
    for arg in vars(argparse_args):
        print(("%s: %s" % (arg, getattr(argparse_args, arg))))

def quoted_if_necessary(input_list):
    output_list = []
    for item in input_list:
        if re.search(r"[^a-zA-Z0-9_.-]", item): # note inside [] backslash quoting does not work so a minus sign to match must occur last
            item = "\"" + item + "\""
        output_list.append(item)
    return output_list

# program starts here

if __name__ == '__main__':
    scipy_bfgs_minimise()