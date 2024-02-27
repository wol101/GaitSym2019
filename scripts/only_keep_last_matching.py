#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import argparse
import re

def only_keep_last_matching():
    parser = argparse.ArgumentParser(description="Only keep the last matching file")
    parser.add_argument("folder_list", nargs="+", help="The folders to look in")
    parser.add_argument("-r", "--regular_expression", default="Population_[0-9]+.txt", help="The regular expression to match [Population_[0-9]+.txt]")
    parser.add_argument("-n", "--number_to_keep", type=int, default=1, help="The number of files to keep [1]")
    parser.add_argument("-g", "--genome", action="store_true", help="Replace regular expression with 'BestGenome_[0-9]+.txt'")                     
    parser.add_argument("-p", "--preflight", action="store_true", help="Print what will be done but do nothing")
    parser.add_argument("-v", "--verbose", action="store_true", help="Write out more information whilst processing")
    args = parser.parse_args()

    if args.preflight:
        args.verbose = True

    if args.verbose:
        pretty_print_sys_argv(sys.argv)
        pretty_print_argparse_args(args)
    
    for folder in args.folder_list:
        preflight_read_folder(folder)
    
    if args.genome:
        args.regular_expression = 'BestGenome_[0-9]+.txt'
    
    for folder in args.folder_list:
        if args.verbose: print('Reading "%s"' % os.path.abspath(folder))
        items = os.listdir(folder)
        matching_items = []
        for item in items:
            item_path = os.path.join(folder, item)
            if not os.path.isfile(item_path):
                if args.verbose: print('Skipping "%s": not a file' % item_path)
                continue
            if re.match(args.regular_expression + "$", item):
                if args.verbose: print('Adding "%s" to list' % item_path)
                matching_items.append(item)
            else:
                if args.verbose: print('Skipping "%s": does not match' % item_path)
    
        if len(matching_items) < args.number_to_keep + 1:
            print("len(matching_items) < %d: nothing to do" % (args.number_to_keep + 1))
            continue
            
        matching_items.sort()
        for i in range(0, len(matching_items) - args.number_to_keep):
            item_path = os.path.join(folder, matching_items[i])
            if args.verbose: print('Deleting "%s"' % item_path)
            if not args.preflight:
                os.remove(item_path)
		
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

def preflight_read_folder(folder):
    if not os.path.exists(folder):
        print("Error: \"%s\" not found" % (folder))
        sys.exit(1)
    if not os.path.isdir(folder):
        print("Error: \"%s\" not a folder" % (folder))
        sys.exit(1)

def preflight_write_folder(folder):
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
    only_keep_last_matching()