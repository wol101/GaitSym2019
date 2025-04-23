#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import argparse
import re
import xml.etree.ElementTree

def print_attrib():

    parser = argparse.ArgumentParser(description="Print the values of matching attributes")
    parser.add_argument("input_xml_files", nargs='+', help="A list of GaitSym XML config file")
    parser.add_argument("-ct", "--condition_tag", default=r'.*', help="Tags to print (regex search) [.*]")
    parser.add_argument("-ca", "--condition_attrib", default=r'.*', help="Attribute to check condition (regex search) [.*]")
    parser.add_argument("-cav", "--condition_attrib_value", default=r'.*', help="Attribute to to check value (regex search) [.*]")
    parser.add_argument("-pa", "--print_attrib", default=r'.*', help="Attribute to print (regex search) [.*]")
    parser.add_argument("-v", "--verbose", action="store_true", help="Write out more information whilst processing")
    args = parser.parse_args()

    if args.verbose:
        pretty_print_sys_argv(sys.argv)
        pretty_print_argparse_args(args)

    for input_xml_file in args.input_xml_files:
        preflight_read_file(input_xml_file, args.verbose)

        # read the input XML file
        input_tree = xml.etree.ElementTree.parse(input_xml_file)
        input_root = input_tree.getroot()

        process_children(input_root, args, input_xml_file)

def process_children(node, args, filename):

    if args.verbose:
        print('Processing "%s"' % (node.tag))
    if re.search(args.condition_tag, node.tag):
        if args.verbose: print('Tag "%s" matches "%s"' % (node.tag, args.condition_tag))
        attrib_keys = list(node.attrib.keys()) # this should take a copy of the keys
        attrib_keys_copy = attrib_keys.copy()
        for attrib in attrib_keys:
            if re.search(args.condition_attrib, attrib):
                if args.verbose: print('"%s" found' % (attrib))
                value = node.attrib[attrib]
                if re.search(args.condition_attrib_value, value):
                    if args.verbose: print('"%s" found' % (value))
                    for found_attrib in attrib_keys_copy:
                        if re.search(args.print_attrib, found_attrib):
                            if args.verbose:  print('"%s" found' % (found_attrib))
                            found_value = node.attrib[found_attrib]
                            if is_a_number(found_value):
                                print('"%s"\t"%s"\t"%s"\t"%s"\t"%s"\t%s' % (filename, node.tag, attrib, value, found_attrib, found_value))
                            else:
                                print('"%s"\t"%s"\t"%s"\t"%s"\t"%s"\t"%s"' % (filename, node.tag, attrib, value, found_attrib, found_value))

    for child in node:
        process_children(child, args, filename)

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
if __name__ == "__main__":
    print_attrib()
