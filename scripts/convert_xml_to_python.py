#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import argparse
import re
import xml.etree.ElementTree

def convert_xml_to_python():

    parser = argparse.ArgumentParser(description="Reads a GaitSym XML file (and maybe other types) and generates a python program that can write that file")
    
    parser.add_argument("-i", "--input_file", required=True, help="GaitSym input file to process")
    parser.add_argument("-o", "--output_file", required=True, help="Python output file")
    parser.add_argument("-f", "--force", action="store_true", help="force overwrite of destination file")
    parser.add_argument("-v", "--verbose", action="store_true", help="write out more information whilst processing")
    args = parser.parse_args()

    if args.verbose:
        pretty_print_sys_argv(sys.argv)
        pretty_print_argparse_args(args)
        
    preflight_read_file(args.input_file, args.verbose)
    preflight_write_file(args.output_file, args.force, args.verbose)
    
    # read the input XML file
    if args.verbose: print('Reading "%s"' % (args.input_file))
    input_tree = xml.etree.ElementTree.parse(args.input_file)
    input_root = input_tree.getroot()
    
    if args.verbose: print('Writing "%s"' % (args.output_file))
    out_f = open(args.output_file, 'w', newline = '\n')
    out_f.write('#!/usr/bin/env python3\n# -*- coding: utf-8 -*-\nimport sys\nimport os\nimport xml.etree.ElementTree\n')
    out_f.write('new_tree = xml.etree.ElementTree.Element("%s")\n' % (input_root.tag))
    out_f.write('new_tree.text = "\\n"\n')
    out_f.write('new_tree.tail = "\\n"\n')
    for child in input_root:
        if args.verbose: print('Processing "%s"' % (child.tag))
        out_f.write('new_child = xml.etree.ElementTree.Element("%s")\n' % (child.tag))
        out_f.write('new_child.tail = "\\n"\n')
        keys = sorted(list(child.attrib.keys()))
        for attrib in keys:
            out_f.write('new_child.attrib["%s"] = "%s"\n' % (attrib, child.attrib[attrib]))
        out_f.write('new_tree.append(new_child)\n')
        
    out_f.write('with open("%s_generated_file.xml", "wb") as out_file:\n' % (os.path.splitext(os.path.split(args.output_file)[1])[0]))
    out_f.write('    out_file.write(xml.etree.ElementTree.tostring(new_tree, encoding="utf-8", method="xml"))\n')
    out_f.close()

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
    convert_xml_to_python()
    
