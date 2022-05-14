#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import argparse
import re
import subprocess
import threading
import shutil

def run_multiple_clients():

    parser = argparse.ArgumentParser(description='Run multiple client files')
    parser.add_argument('-x', '--executable_to_run', default='gaitsym_2019_asio_async', help='the client executable to run [gaitsym_2019_asio_async]')
    parser.add_argument('-a', '--arguments', default='', help='Command line arguments for executable e.g. -a"-ho localhost:8086" with no space after the -a if the argument starts with - []')
    parser.add_argument('-n', '--number_of_clients', type=int, default=1, help='Number of clients to run [1]')
    parser.add_argument('-s', '--search_folder', default='.', help='Recursively search for the execultable from this folder [.]')
    parser.add_argument('-v', '--verbose', action='store_true', help='Write out more information whilst processing')
    args = parser.parse_args()

    if args.verbose:
        pretty_print_sys_argv(sys.argv)
        pretty_print_argparse_args(args)
    
    if args.search_folder:
        preflight_read_folder(args.search_folder)
        filename = os.path.split(args.executable_to_run)[1]
        executable_path = find_file(args.search_folder, filename)
        if not executable_path:
            print('Error: "%s" not found in "%s"' % (filename, args.search_folder))
            sys.exit(1)
    else:
        executable_path = shutil.which(args.executable_to_run)
        if not executable_path:
            print('Error: "%s" not found' % (args.executable_to_run))
            sys.exit(1)
    
    command = [executable_path]
    arguments = args.arguments.split()
    if arguments:
        command.extend(arguments)

    thread_list = []
    for i in range(0, args.number_of_clients):
        print(command)
        if args.verbose: print('Creating thread %d %s' % (i, pretty_print_sys_argv(command)))
        thread_list.append(threading.Thread(target=subprocess.run, args=[command]))
    
    for i in range(0, args.number_of_clients):
        if args.verbose: print('Starting thread %d' % (i))
        thread_list[i].start()

    for i in range(0, args.number_of_clients):
        if args.verbose: print('Joining thread %d' % (i))
        thread_list[i].join()
    
    if args.verbose: print('Finished')
    return

def find_file(folder, filename):
    for dirpath, dirnames, files in os.walk(folder):
        for name in files:
            if name == filename:
                return(os.path.join(dirpath, name))
    return ''
    
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
    try:
        run_multiple_clients()
    except KeyboardInterrupt:
        print('Interrupted')
        try:
            sys.exit(1)
        except SystemExit:
            os._exit(1)

