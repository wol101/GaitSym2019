#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import argparse
import re
import xml.etree.ElementTree

def inject_tag_by_id():

    parser = argparse.ArgumentParser(description="Inject a tag from one XML file into another")
    parser.add_argument("-src", "--input_tag_source", required=True, help="input GaitSym XML config file")
    parser.add_argument("-dst", "--input_tag_destination", required=True, help="input GaitSym XML config file")
    parser.add_argument("-o", "--output_xml_file", required=True, help="output GaitSym XML config file")
    parser.add_argument("-tag", "--tag_to_inject", required=True, help="tag to delete (e.g. BODY)")
    parser.add_argument("-id", "--tag_to_inject_id", required=True, help="ID to delete (e.g. thigh)")
    parser.add_argument("-r", "--regex", action="store_true", help="use regex for tag and tag id")
    parser.add_argument("-d", "--debug", action="store_true", help="write out more information whilst processing")
    parser.add_argument("-f", "--force", action="store_true", help="force overwrite of destination file")
    parser.add_argument("-v", "--verbose", action="store_true", help="write out more information whilst processing")
    args = parser.parse_args()

    if args.debug: args.verbose = True

    if args.verbose:
        pretty_print_sys_argv(sys.argv)
        pretty_print_argparse_args(args)

    preflight_read_file(args.input_tag_source, args.verbose)
    preflight_read_file(args.input_tag_destination, args.verbose)
    preflight_write_file(args.output_xml_file, args.force, args.verbose)

    # read the tag source XML file
    tree = xml.etree.ElementTree.parse(args.input_tag_source)
    root = tree.getroot()
    tags_to_inject = []
    for child in root:
        if args.regex:
            if re.search(args.tag_to_inject, child.tag) and re.search(args.tag_to_inject_id, child.attrib['ID']):
                if args.verbose: print('%s ID="%s" found' % (child.tag, child.attrib['ID']))
                tags_to_inject.append(child)
        else:
            if child.tag == args.tag_to_inject and child.attrib['ID'] == args.tag_to_inject_id:
                if args.verbose: print('%s ID="%s" found' % (child.tag, child.attrib['ID']))
                tags_to_inject.append(child)

    # now read the destination tree
    tree = xml.etree.ElementTree.parse(args.input_tag_destination)
    root = tree.getroot()
    for tag in tags_to_inject:
        root.append(tag)

    tree.write(args.output_xml_file, encoding='utf-8', xml_declaration=True)

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
    inject_tag_by_id()
