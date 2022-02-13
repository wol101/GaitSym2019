#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import argparse
import re
import xml.etree.ElementTree

def conditional_attrib_edit():

    parser = argparse.ArgumentParser(description="Conditional XML attribute editor (as a side effect it sorts attributes)")
    parser.add_argument("-i", "--input_xml_file", required=True, help="the input old format GaitSym XML config file")
    parser.add_argument("-o", "--output_xml_file", required=True, help="the output GaitSym2019 XML config file")
    parser.add_argument("-ct", "--condition_tag", required=True, help="the tag to match to allow change (regex search)")
    parser.add_argument("-ca", "--condition_attrib", required=True, help="the attribute to match to allow change (regex search)")
    parser.add_argument("-cav", "--condition_attrib_value", required=True, help="the attribute value to match to allow change (regex search)")
    parser.add_argument("-ac", "--attrib_to_change", required=True, help="the attribute to change or create (exact)")
    parser.add_argument("-av", "--attrib_new_value", required=True, help="the attribute new value (exact)")
    parser.add_argument("-f", "--force", action="store_true", help="force overwrite of destination file")
    parser.add_argument("-v", "--verbose", action="store_true", help="write out more information whilst processing")
    args = parser.parse_args()

    if args.verbose:
        pretty_print_sys_argv(sys.argv)
        pretty_print_argparse_args(args)

    # preflight
    if not os.path.exists(args.input_xml_file):
        print("Error: \"%s\" missing" % (args.input_xml_file))
        sys.exit(1)
    if os.path.exists(args.output_xml_file) and not args.force:
        print("Error: \"%s\" exists. Use --force to overwrite" % (args.output_xml_file))
        sys.exit(1)

    # read the input XML file
    input_tree = xml.etree.ElementTree.parse(args.input_xml_file)
    input_root = input_tree.getroot()

    process_children(input_root, args)

    out_file = open(args.output_xml_file, "wb")
    out_file.write(xml.etree.ElementTree.tostring(input_root, encoding="utf-8", method="xml"))
    out_file.close()


def process_children(node, args):

    if args.verbose:
        print('Processing "%s"' % (node.tag))
    if re.search(args.condition_tag, node.tag):
        if args.verbose:
            print('Tag "%s" matches "%s"' % (node.tag, args.condition_tag))
        attrib_keys = list(node.attrib.keys()) # this should take a copy of the keys
        for attrib in attrib_keys:
            if re.search(args.condition_attrib, attrib):
                if args.verbose:
                    print('"%s" found' % (args.condition_attrib))
                if re.search(args.condition_attrib_value, node.attrib[args.condition_attrib]):
                    if args.verbose:
                        print('Attrib "%s" matches "%s"' % (node.attrib[args.condition_attrib], args.condition_attrib_value))
                        print('Changing attrib "%s" to "%s"' % (args.attrib_to_change, args.attrib_new_value))
                    node.attrib[args.attrib_to_change] = args.attrib_new_value

    for child in node:
        process_children(child, args)

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
    conditional_attrib_edit()
