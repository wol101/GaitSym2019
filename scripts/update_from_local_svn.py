#!/usr/bin/env python3
# -*- coding: utf-8 -*-


import sys
import os
import argparse
import re
import subprocess

def update_from_local_svn():

    parser = argparse.ArgumentParser(description="After a svn checkout and sync, this command uses status to list the changes and applies them to the svn server\n" +
                                     "Needs to be run from within the directory that svn status would be run from.\n")
    parser.add_argument("-u", "--username", default="", help="svn username")
    parser.add_argument("-w", "--password", default="", help="svn password")
    parser.add_argument("-c", "--commit", action="store_true", help="Do the commit stage")
    parser.add_argument("-p", "--preflight", action="store_true", help="Write what will be done but do nothing")
    parser.add_argument("-v", "--verbose", action="store_true", help="Write out more information whilst processing")
    args = parser.parse_args()

    if args.preflight:
        args.verbose = True

    command = ['svn', 'status']
    if args.verbose: print(f"{command=}")
    completed_process = subprocess.run(command, capture_output = True)
    if completed_process.returncode:
        print('"%s" returned an error' % (' '.join(command)))
        sys.exit(1)
    status_string = completed_process.stdout.decode('utf-8')
    lines = status_string.splitlines()
    modified = []
    deleted = []
    added = []
    modified_tag = 'M       '
    deleted_tag = '!       '
    added_tag = '?       '
    for line in lines:
        if line.startswith(modified_tag):
            modified.append(line[len(modified_tag):])
        if line.startswith(deleted_tag):
            deleted.append(line[len(deleted_tag):])
        if line.startswith(added_tag):
            added.append(line[len(added_tag):])
    
    if args.verbose:
        print(f"{modified=}")
        print(f"{deleted=}")
        print(f"{added=}")
    

    for item in deleted:
        command = ['svn', 'delete']
        if args.username: command.extend(['--username', args.username])
        if args.password: command.extend(['--password', args.password])
        command.extend([item])
        if args.verbose: print(f"{command=}")
        if not args.preflight:
            completed_process = subprocess.run(command)
            if completed_process.returncode:
                print('"%s" returned an error' % (' '.join(command)))
                sys.exit(1)

    for item in added:
        command = ['svn', 'add']
        if args.username: command.extend(['--username', args.username])
        if args.password: command.extend(['--password', args.password])
        command.extend([item])
        if args.verbose: print(f"{command=}")
        if not args.preflight:
            completed_process = subprocess.run(command)
            if completed_process.returncode:
                print('"%s" returned an error' % (' '.join(command)))
                sys.exit(1)

    if args.commit:
        command = ['svn', 'commit']
        if args.username: command.extend(['--username', args.username])
        if args.password: command.extend(['--password', args.password])
        command.extend(['-m', 'commiting changes for release'])
        if args.verbose: print(f"{command=}")
        if not args.preflight:
            completed_process = subprocess.run(command)
            if completed_process.returncode:
                print('"%s" returned an error' % (' '.join(command)))
                sys.exit(1)


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
    update_from_local_svn()
