#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import argparse
import re
import xml.etree.ElementTree

def extract_muscles_by_joint():

    parser = argparse.ArgumentParser(description="Produce a list of muscles grouped by the joints they act over")
    parser.add_argument("-i", "--input_xml_file", required=True, help="the input OBJ file")
    parser.add_argument("-x", "--output_xml_file", required=False, default='', help="the output GaitSym2019 XML config file")
    parser.add_argument("-o", "--output_txt_file", required=False, default='', help="the output text config file")
    parser.add_argument("-j", "--joint_list", required=True, nargs='+', help="the joint list used to identify the required muscles")
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
    if args.verbose:
        print('Reading "%s"' % (args.input_xml_file))
    input_tree = xml.etree.ElementTree.parse(args.input_xml_file)
    input_root = input_tree.getroot()

    # loop and parse
    body_list = {}
    joint_list = {}
    marker_list = {}
    muscle_list = {}
    muscle_strap_map = {}
    strap_list = {}
    for child in input_root:
        if child.tag == "BODY":
            if args.verbose: print('Body "%s" found' % (child.attrib['ID']))
            body_list[child.attrib['ID']] = False

        if child.tag == "MARKER":
            if args.verbose: print('Marker "%s" found' % (child.attrib['ID']))
            marker_list[child.attrib['ID']] = child.attrib['BodyID']

        if child.tag == "JOINT":
            if args.verbose: print('Joint "%s" found' % (child.attrib['ID']))
            joint_list[child.attrib['ID']] = [marker_list[child.attrib['Body1MarkerID']], marker_list[child.attrib['Body2MarkerID']]]

        if child.tag == "MUSCLE":
            if args.verbose: print('Muscle "%s" found' % (child.attrib['ID']))
            muscle_list[child.attrib['ID']] = child.attrib
            muscle_strap_map[child.attrib['StrapID']] = child.attrib['ID']

        if child.tag == "STRAP":
            if args.verbose: print('Strap "%s" found' % (child.attrib['ID']))
            dependent_bodies = []
            for attrib in child.attrib:
                if attrib.endswith('MarkerID'):
                    dependent_bodies.append(marker_list[child.attrib[attrib]])
            strap_list[child.attrib['ID']] = dependent_bodies

    if args.verbose:
        print('Found %d joints, %d markers, %d muscles and %d straps' % (len(joint_list), len(marker_list), len(muscle_list), len(strap_list)))

    joint_strap_map = {}
    for strap in strap_list:
        dependent_bodies = strap_list[strap]
        for i in range(0, len(dependent_bodies)):
            for j in range(0, i):
                if dependent_bodies[i] != dependent_bodies[j]:
                    possible_paths = find_paths(dependent_bodies[i], dependent_bodies[j], joint_list, body_list)
                    if args.verbose: print(strap, dependent_bodies[i], dependent_bodies[j], possible_paths)
                    for path_options in possible_paths:
                        for path in path_options:
                            joint_name = path[0]
                            if joint_name in joint_strap_map:
                                joint_strap_map[joint_name].append(strap)
                            else:
                                joint_strap_map[joint_name] = [strap]

    if args.verbose: print(joint_strap_map)

    if args.output_xml_file:
        # create an empty GAITSYM tree
        new_tree = xml.etree.ElementTree.Element("GAITSYM2019")
        new_tree.text = "\n"
        new_tree.tail = "\n"
        for joint in args.joint_list:
            straps_found = joint_strap_map[joint]
            for strap in straps_found:
                muscle = muscle_strap_map[strap]
                if args.verbose: print('Creating "%s"' % (muscle))
                new_muscle = xml.etree.ElementTree.Element("MUSCLE")
                new_muscle.attrib = muscle_list[muscle]
                new_muscle.tail = '\n'
                new_tree.append(new_muscle)

        with open(args.output_xml_file, "wb") as out_file:
            if args.verbose:
                print('Writing "%s"' % (args.output_xml_file))
            out_file.write(xml.etree.ElementTree.tostring(new_tree, encoding="utf-8", method="xml"))

    if args.output_txt_file:
        lines = []
        for joint in args.joint_list:
            lines.append(joint + '\t')
            straps_found = joint_strap_map[joint]
            for strap in straps_found:
                muscle = muscle_strap_map[strap]
                lines.append('\t' + muscle)

        with open(args.output_txt_file, "w") as out_file:
            if args.verbose:
                print('Writing "%s"' % (args.output_txt_file))
            out_file.write('\n'.join(lines))

def find_paths(b1, b2, joint_list, body_list):
    list_of_paths = []
    current_path = []
    tested_bodies = dict(body_list)
    [tested_bodies, list_of_paths] = next_path(b1, b2, joint_list, tested_bodies, list_of_paths, current_path)
    return list_of_paths

def next_path(current_body, target_body, joint_list, tested_bodies, list_of_paths, current_path):
    tested_bodies[current_body] = True
    for body in tested_bodies:
        if tested_bodies[body]:
            continue
        for joint_name in joint_list:
            new_path = list(current_path)
            joint = joint_list[joint_name]
            if (current_body in joint) and (body in joint):
                new_path.append((joint_name, joint))
                if body == target_body:                    
                    list_of_paths.append(new_path)
                    continue
                else:
                    [tested_bodies, list_of_paths] = next_path(body, target_body, joint_list, tested_bodies, list_of_paths, new_path)
                    continue
    return [tested_bodies, list_of_paths]
            
    

def next_path2(current_body, untested_joints, target_body, sample_path):
    for joint_name in untested_joints:
        bodies = untested_joints[joint_name]
        # look for downstream connections
        if bodies[0] == current_body:
            del untested_joints[joint_name]
            sample_path.append(joint_name)
            if bodies[1] == target_body:
                return [sample_path, untested_joints]
            return next_path(bodies[1], untested_joints, target_body, sample_path)
        # look for upstream connections
        if bodies[1] == current_body:
            del untested_joints[joint_name]
            sample_path.append(joint_name)
            if bodies[0] == target_body:
                return [sample_path, untested_joints]
            return next_path(bodies[0], untested_joints, target_body, sample_path)
    return  ['', untested_joints]



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
    extract_muscles_by_joint()
