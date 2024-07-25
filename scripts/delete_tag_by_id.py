#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import argparse
import re
import xml.etree.ElementTree

def delete_tag_by_id():

    parser = argparse.ArgumentParser(description="Transform the bodies in a GaitSym file")
    parser.add_argument("-i", "--input_xml_file", required=True, help="input GaitSym XML config file")
    parser.add_argument("-o", "--output_xml_file", required=True, help="output GaitSym XML config file")
    parser.add_argument("-t", "--delete_tag", required=True, help="tag to delete (e.g. BODY)")
    parser.add_argument("-d", "--delete_tag_id", required=True, help="ID to delete (e.g. thigh)")
    parser.add_argument("-r", "--regex", action="store_true", help="use regex for tag and tag id")
    parser.add_argument("-f", "--force", action="store_true", help="force overwrite of destination file")
    parser.add_argument("-v", "--verbose", action="store_true", help="write out more information whilst processing")
    args = parser.parse_args()

    if args.verbose:
        pretty_print_sys_argv(sys.argv)
        pretty_print_argparse_args(args)

    # read the input XML file
    input_tree = xml.etree.ElementTree.parse(args.input_xml_file)
    input_root = input_tree.getroot()

    # create a new empty tree
    new_tree = xml.etree.ElementTree.Element(input_root.tag)
    new_tree.text = "\n"
    new_tree.tail = "\n"

    if args.regex:
        for child in input_root:
            if re.search(args.delete_tag, child.tag) and re.search(args.delete_tag_id, child.attrib['ID']):
                if args.verbose:
                    print('%s ID="%s" not copied' % (child.tag, child.attrib['ID']))
                continue
            new_tree.append(child)

    else:
        for child in input_root:
            if child.tag == args.delete_tag and child.attrib['ID'] == args.delete_tag_id:
                if args.verbose:
                    print('%s ID="%s" not copied' % (child.tag, child.attrib['ID']))
                continue
            new_tree.append(child)

    with open(args.output_xml_file, "wb") as out_file:
        out_file.write(xml.etree.ElementTree.tostring(new_tree, encoding="utf-8", method="xml"))

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
    delete_tag_by_id()
