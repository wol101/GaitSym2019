#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import argparse
import re
import subprocess
import fnmatch

def fix_iccp_profiles():

    parser = argparse.ArgumentParser(description="Fix PNG ICCP profile errors")
    parser.add_argument("item_list", nargs="+", help="a list of files or folders")
    parser.add_argument("-c", "--pngcrush", default="pngcrush -n -q", help="pngcrush command including arguments [pngcrush -n -q]")
    parser.add_argument("-m", "--mogrify", default="mogrify", help="the mogrify command from imagemagick [mogrify]")
    parser.add_argument("-g", "--glob_png", default="*.png", help="the glob to match image files in folders [*.png]")
    parser.add_argument("-r", "--invalid_regexp", default="iCCP", help="the regex to seach for to identify a file that needs fixing [iCCP]")
    parser.add_argument("-p", "--preflight", action="store_true", help="do not do any processing")
    parser.add_argument("-v", "--verbose", action="store_true", help="write out more information whilst processing") 
    parser.add_argument("-d", "--debug", action="store_true", help="write out lots more information whilst processing") 
    args = parser.parse_args()

    if args.preflight:
        args.verbose = True
    if args.debug:
        pretty_print_sys_argv(sys.argv)
        pretty_print_argparse_args(args)

    # preflight
    for item in args.item_list:
        if not os.path.exists(item):
            print("Error: \"%s\" missing" % (item))
            sys.exit(1)

    for item in args.item_list:
        if os.path.isdir(item):
            process_folder(item, args)
        if os.path.isfile(item):
            process_file(item, args)
            

def process_folder(item, args):
    for root, dirs, files in os.walk(item):
        for name in files:
            if fnmatch.fnmatch(name, args.glob_png):
                process_file(os.path.join(root, name), args)
        # for name in dirs:
        # print(os.path.join(root, name))

def process_file(item, args):
    commands = args.pngcrush.split()
    commands.append(item)
    if args.debug:
        pretty_print_sys_argv(commands)
    result = subprocess.run(commands, capture_output=True)
    output = result.stdout.decode("utf-8")  + '\n' + result.stderr.decode("utf-8")
    if args.debug:
        print(output)
    if re.search(args.invalid_regexp, output):
        commands = args.mogrify.split()
        commands.append(item)
        if args.verbose:
            pretty_print_sys_argv(commands)
        if not args.preflight:
            result = subprocess.run(commands, capture_output=True)

def pretty_print_sys_argv(sys_argv):
    quoted_sys_argv = quoted_if_necessary(sys_argv)
    print((" ".join(quoted_sys_argv)))

def pretty_print_argparse_args(argparse_args):
    for arg in vars(argparse_args):
        print(("%s: %s" % (arg, getattr(argparse_args, arg))))

def quoted_if_necessary(input_list):
    output_list = []
    for item in input_list:
        if re.search("[^a-zA-Z0-9_\.-]", item):
            item = "\"" + item + "\""
        output_list.append(item)
    return output_list

        # program starts here
if __name__ == "__main__":
    fix_iccp_profiles()
