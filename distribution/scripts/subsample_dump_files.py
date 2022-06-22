#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import argparse
import re


def subsample_dump_files():

    parser = argparse.ArgumentParser(description="Subsample Dump Files")
    
    parser.add_argument("input_files", nargs='+', help="The input files to process")
    parser.add_argument("-s", "--subsample", type=int, default=10, help="The subsample ratio [10]")
    parser.add_argument("-u", "--suffix", default="_subsampled_%03d", help="Suffix to add to the renamed files [_subsampled_%%03d]") # putting a % sign in the help test requires %%
    parser.add_argument("-p", "--in_place", action="store_true", help="Allow the subsampling to occur directly on the input file")
    parser.add_argument("-f", "--force", action="store_true", help="force overwrite of destination file")
    parser.add_argument("-v", "--verbose", action="store_true", help="write out more information whilst processing")
    args = parser.parse_args()

    if args.subsample < 2:
        parser.error('--subsample value must be 2 or greater')

    if args.verbose:
        pretty_print_sys_argv(sys.argv)
        pretty_print_argparse_args(args)
    
    for file in args.input_files:
        preflight_read_file(file, args.verbose)
        with open(file, 'r') as f:
            lines = f.read().splitlines()
        if not args.in_place:
            parts = os.path.splitext(file)
            if re.search('%[0-9]*d', args.suffix):
                suffix = args.suffix % (args.subsample)
            else:
                suffix = args.suffix
            output_file = parts[0] + suffix + parts[1]
            preflight_write_file(output_file, args.force, args.verbose)
        else:
            output_file = file       
    
        output_lines = [lines[0]]
        counter = 0
        for i in range(1, len(lines)):
            if counter % args.subsample == 0:
                output_lines.append(lines[i])
            counter = counter + 1
        
        if args.verbose: print('Writing "%s"' % (output_file))
        with open(output_file, 'w') as f:
            f.write('\n'.join(output_lines))
            f.write('\n')

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
    subsample_dump_files()
    
