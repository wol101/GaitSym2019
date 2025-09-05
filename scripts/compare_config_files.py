#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import argparse
import re
import xml.etree.ElementTree
import copy

def compare_config_files():

    parser = argparse.ArgumentParser(description="Compare two GaitSym files")
    parser.add_argument("-i1", "--input_xml_file1", required=True, help="First input GaitSym XML config file")
    parser.add_argument("-i2", "--input_xml_file2", required=True, help="Second input GaitSym XML config file")
    parser.add_argument("-mt", "--match_tag", default=r'.*', help="Only compare tags that match this regex default='.*'")
    parser.add_argument("-ma", "--match_attrib", default=r'.*', help="Only compare attributes that match this regex default='.*'")
    parser.add_argument("-d", "--debug", action="store_true", help="Write out debug information whilst processing")
    parser.add_argument("-v", "--verbose", action="store_true", help="Write out more information whilst processing")
    args = parser.parse_args()

    if args.debug:
        args.verbose = True
    if args.verbose:
        pretty_print_sys_argv(sys.argv)
        pretty_print_argparse_args(args)

    preflight_read_file(args.input_xml_file1, args.verbose)
    preflight_read_file(args.input_xml_file2, args.verbose)

    match_tag_regex = compile_regex(args.match_tag)
    match_attrib_regex = compile_regex(args.match_attrib)

    # read the input XML file
    if args.verbose: print('Reading "%s"' % (args.input_xml_file1))
    input_tree1 = xml.etree.ElementTree.parse(args.input_xml_file1)
    input_root1 = input_tree1.getroot()
    if args.verbose: print('Reading "%s"' % (args.input_xml_file2))
    input_tree2 = xml.etree.ElementTree.parse(args.input_xml_file2)
    input_root2 = input_tree2.getroot()

    # loop first tree and get all the tags with IDs
    tag_dict1 = {}
    if args.verbose: print('Iterating input_root1')
    for child in input_root1:
        if not 'ID' in child.attrib:
            if args.debug: print(f'input_xml_file1: tag found with no ID {child=}')
            continue
        ID = child.attrib['ID']
        if args.debug: print(f'input_xml_file1: tag found {ID=} {child=}')
        if ID in tag_dict1:
            print(f'input_xml_file1: ID is not unique {ID=}')
            sys.exit(1)
        tag_dict1[ID] = copy.deepcopy(child)

    # loop through second tree and append all new IDs
    if args.verbose: print('Iterating input_root2')
    tag_dict2 = {}
    for child in input_root2:
        if not 'ID' in child.attrib:
            if args.debug: print(f'input_xml_file2: tag found with no ID {child=}')
            continue
        ID = child.attrib['ID']
        if args.debug: print(f'input_xml_file2: tag found {ID=} {child=}')
        if ID in tag_dict2:
            print(f'input_xml_file2: ID is not unique {ID=}')
            sys.exit(1)
        tag_dict2[ID] = copy.deepcopy(child)

    # first check we have matching tags
    if args.verbose: print('Checking tag IDs')
    for ID in tag_dict1:
        if args.debug: print(f'{tag_dict1[ID]=} {ID=}')
        if match_tag_regex.search(tag_dict1[ID].tag) is None:
            continue
        if not ID in tag_dict2:
            print(f'{ID=} missing from input_xml_file2')
    for ID in tag_dict2:
        if args.debug: print(f'{tag_dict2[ID]=} {ID=}')
        if match_tag_regex.search(tag_dict2[ID].tag) is None:
            continue
        if not ID in tag_dict1:
            print(f'{ID=} missing from input_xml_file1')

    # now check for missing attributes
    if args.verbose: print('Checking missing attributes')
    for ID in tag_dict1:
        if args.debug: print(f'{tag_dict1[ID]=} {ID=}')
        if match_tag_regex.search(tag_dict1[ID].tag) is None:
            continue
        if not ID in tag_dict2:
            continue
        for attrib in tag_dict1[ID].attrib:
            if args.debug: print(f'{attrib=} {tag_dict1[ID].attrib[attrib]=}')
            if match_attrib_regex.search(attrib) is None:
                continue
            if not attrib in tag_dict2[ID].attrib:
                print(f' {ID=} {attrib=} missing from input_xml_file2')
    for ID in tag_dict2:
        if args.debug: print(f'{tag_dict2[ID]=} {ID=}')
        if match_tag_regex.search(tag_dict2[ID].tag) is None:
            continue
        if not ID in tag_dict1:
            continue
        for attrib in tag_dict2[ID].attrib:
            if args.debug: print(f'{attrib=} {tag_dict2[ID].attrib[attrib]=}')
            if match_attrib_regex.search(attrib) is None:
                continue
            if not attrib in tag_dict1[ID].attrib:
                print(f' {ID=} {attrib=} missing from input_xml_file1')

    # check values of attributes
    if args.verbose: print('Checking attribute values')
    for ID in tag_dict1:
        if args.debug: print(f'{tag_dict1[ID]=} {ID=}')
        if match_tag_regex.search(tag_dict1[ID].tag) is None:
            continue
        if not ID in tag_dict2:
            continue
        for attrib in tag_dict1[ID].attrib:
            if args.debug: print(f'{attrib=} {tag_dict1[ID].attrib[attrib]=}')
            if match_attrib_regex.search(attrib) is None:
                continue
            if not attrib in tag_dict2[ID].attrib:
                continue
            attrib1 = tag_dict1[ID].attrib[attrib]
            attrib2 = tag_dict2[ID].attrib[attrib]
            if attrib1 != attrib2:
                print(f' {ID=} {attrib=} {attrib1=} {attrib2=}')

def compile_regex(pattern):
    try:
        compiled_pattern = re.compile(pattern)
    except re.error as e:
        print(f"Regex Compilation Error: {e}")
        sys.exit(1)
    except TypeError as type_error:
        print(f"Type Error: {type_error}")
        sys.exit(1)
    return compiled_pattern

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
            print(error)
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
    compare_config_files()
