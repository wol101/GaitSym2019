#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import argparse
import re
import shutil
import difflib

def sync_shared_files():

    parser = argparse.ArgumentParser(description="Two way sync pairs of files depending on which is newer")
    parser.add_argument("file_list", nargs='+', help="List of file pairs to sync: f1a f1b f2a f2b ...")
    parser.add_argument("-d", "--diff", action="store_true", help="Do not copy files but perform a diff instead")
    parser.add_argument("-b", "--backup", action="store_true", help="Create backup file")
    parser.add_argument("-p", "--preflight", action="store_true", help="Write what will happen but do nothing")
    parser.add_argument("-q", "--quiet", action="store_true", help="Write out less information whilst processing")
    parser.add_argument("-v", "--verbose", action="store_true", help="Write out more information whilst processing")
    args = parser.parse_args()
    
    if args.preflight:
        args.verbose = True

    if args.verbose:
        pretty_print_sys_argv(sys.argv)
        pretty_print_argparse_args(args)
        
    if len(args.file_list) % 2 != 0:
        print('Error: file_list must contain an even number of files')
        sys.exit(1)

    if args.verbose: print('Checking files exist')
    for file in args.file_list:
        preflight_read_file(file)
    
    if args.diff:
        if args.verbose: print('Performing diff')
        for i in range(0, len(args.file_list), 2):
            diff_pair(args.file_list[i], args.file_list[i + 1], args)
    else:
        if args.verbose: print('Performing sync')
        for i in range(0, len(args.file_list), 2):
            sync_pair(args.file_list[i], args.file_list[i + 1], args)

def diff_pair(file1, file2, args):
    filepath1 = os.path.abspath(file1)
    filepath2 = os.path.abspath(file2)
    if args.verbose: print('Diffing "%s" and "%s"' % (filepath1, filepath2))
    with open(filepath1) as f1:
        f1_text = f1.read().splitlines()
    with open(filepath2) as f2:
        f2_text = f2.read().splitlines()
    # Find and print the diff:
    for line in difflib.unified_diff(f1_text, f2_text, fromfile=filepath1, tofile=filepath2):
        print(line)

def sync_pair(file1, file2, args):
    # start with some basic name checks
    filepath1 = os.path.abspath(file1)
    filepath2 = os.path.abspath(file2)
    if args.verbose: print('Checking "%s" and "%s"' % (filepath1, filepath2))
    if filepath1 == filepath2:
        print('Error: "%s" and "%s" refer to the same file' % (file1, file2))
        sys.exit(1)
    modtime1 = os.path.getmtime(filepath1)
    modtime2 = os.path.getmtime(filepath2)
    filesize1 = os.path.getsize(filepath1)
    filesize2 = os.path.getsize(filepath2)
    if modtime1 == modtime2 and filesize1 == filesize2:
        if args.verbose: print('"%s" and "%s" have same modification time and size so are probably identical' % (filepath1, filepath2))
    if modtime1 == modtime2 and filesize1 != filesize2:
        print('Error: "%s" and "%s" have same modification time but different sizes' % (filepath1, filepath2))
        sys.exit(1)
    if modtime1 > modtime2:
        if args.verbose: print('"%s" is newer than "%s"' % (filepath1, filepath2))
        if args.backup:
            backup_path = filepath2 + '.sync_backup'
            if args.verbose: print('Moving "%s" to "%s"' % (filepath2, backup_path))
            if not args.preflight: shutil.move(filepath2, backup_path)
        if not args.quiet: print('Copying "%s" to "%s"' % (filepath1, filepath2))
        if not args.preflight: shutil.copy2(filepath1, filepath2)
    if modtime2 > modtime1:
        if args.verbose: print('"%s" is newer than "%s"' % (filepath2, filepath1))
        if args.backup:
            backup_path = filepath1 + '.sync_backup'
            if args.verbose: print('Moving "%s" to "%s"' % (filepath1, backup_path))
            if not args.preflight: shutil.move(filepath1, backup_path)
        if not args.quiet: print('Copying "%s" to "%s"' % (filepath2, filepath1))
        if not args.preflight:
            shutil.copy2(filepath2, filepath1)
    


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
    sync_shared_files()
