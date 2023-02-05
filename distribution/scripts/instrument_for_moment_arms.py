#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import argparse
import re
import xml.etree.ElementTree
import copy

def instrument_for_moment_arms():

    parser = argparse.ArgumentParser(description="Check the mass properties in a GaitSym file by recalculating from the meshes")
    
    parser.add_argument("-i", "--input_file", required=True, help="The GaitSym input file to process")
    parser.add_argument("-o", "--output_file", required=True, help="Output file containing the required torque reporters")
    parser.add_argument("-r", "--recursion_limit", default=1000, type=int, help="The recursion limit for the parsing code [1000]")
    parser.add_argument("-f", "--force", action="store_true", help="force overwrite of destination file")
    parser.add_argument("-v", "--verbose", action="store_true", help="write out more information whilst processing")
    args = parser.parse_args()

    if args.verbose:
        pretty_print_sys_argv(sys.argv)
        pretty_print_argparse_args(args)
    
    sys.setrecursionlimit(args.recursion_limit)
        
    preflight_read_file(args.input_file, args.verbose)
    preflight_write_file(args.output_file, args.force, args.verbose)
    
    # read the input XML file
    if args.verbose: print('Reading "%s"' % (args.input_file))
    input_tree = xml.etree.ElementTree.parse(args.input_file)
    input_root = input_tree.getroot()

    # cycle through the input file
    # collecting list of the required elements to generate the torque reporters
    body_dict = {}
    marker_dict = {}
    joint_dict = {}    
    for child in input_root:
        if child.tag == "BODY":
            body_dict[child.attrib['ID']] = copy.deepcopy(child)
            continue  
        if child.tag == "MARKER":
            marker_dict[child.attrib['ID']] = copy.deepcopy(child)
            continue
        if child.tag == "JOINT":
            joint_dict[child.attrib['ID']] = copy.deepcopy(child)
            continue
    
    # create an empty GAITSYM tree
    new_tree = xml.etree.ElementTree.Element("GAITSYM2019")
    new_tree.text = "\n"
    new_tree.tail = "\n"
    
    # copy the old tree to the new tree and process the straps
    for child in input_root:
        if child.tag != "STRAP":
            new_tree.append(copy.deepcopy(child))
            continue
        
        if args.verbose: print('Processing MUSCLE ID="%s"' % (child.attrib['ID']))
        new_child = process_muscle(child, body_dict, marker_dict, joint_dict)
        if args.verbose: print('TorqueMarkerIDList="%s"' % (new_child.attrib['TorqueMarkerIDList']))
        new_tree.append(new_child)
    
    # write out the new tree
    if args.verbose: print('Writing "%s"' % (args.output_file))
    with open(args.output_file, "wb") as out_file:
        out_file.write(xml.etree.ElementTree.tostring(new_tree, encoding="utf-8", method="xml"))
        
def process_muscle(child, body_dict, marker_dict, joint_dict):
    strap = copy.deepcopy(child)
    strap.attrib['TorqueMarkerIDList'] = ''
    origin_marker = marker_dict[strap.attrib['OriginMarkerID']]
    origin_body = body_dict[origin_marker.attrib['BodyID']]
    insertion_marker = marker_dict[strap.attrib['InsertionMarkerID']]
    insertion_body = body_dict[insertion_marker.attrib['BodyID']]
    start_body = origin_body.attrib['ID']
    end_body = insertion_body.attrib['ID']
    # (body_chain, joint_chain) = get_joint_links(start_body, end_body, marker_dict, joint_dict, body_chain, joint_chain)
    # joint_chain = get_joint_links(start_body, end_body, marker_dict, joint_dict)
    joint_chain = []
    body_chain = [start_body]
    solution_list = []
    get_joint_links(end_body, marker_dict, joint_dict, joint_chain, body_chain, solution_list)
    if not solution_list:
        return strap
    joint_chain = []
    min_length = len(solution_list[0][0])
    min_length_index = 0
    for i in range(1, len(solution_list)):
        if len(solution_list[i][0]) < min_length:
            min_length_index = i
    for joint in solution_list[min_length_index][1]:
        joint_chain.append(joint)
    torque_marker_list = []
    for joint_id in joint_chain:
        body1_marker_id = joint_dict[joint_id].attrib['Body1MarkerID']
        body1_id = marker_dict[body1_marker_id].attrib['BodyID']
        body2_marker_id = joint_dict[joint_id].attrib['Body2MarkerID']
        body2_id = marker_dict[body2_marker_id].attrib['BodyID']
        while True:
            if body1_id == start_body or body1_id == end_body:
                torque_marker_list.append(joint_dict[joint_id].attrib['Body1MarkerID'])
                break
            if body2_id == start_body or body2_id == end_body:
                torque_marker_list.append(joint_dict[joint_id].attrib['Body2MarkerID'])
                break
            success = False
            for attrib in strap.attrib:
                # print(attrib)
                if attrib.endswith('MarkerID'):
                    body_id = marker_dict[strap.attrib[attrib]].attrib['BodyID']
                    if body1_id == body_id:
                        torque_marker_list.append(joint_dict[joint_id].attrib['Body1MarkerID'])
                        success = True
                        break
                    if body2_id == body_id:
                        torque_marker_list.append(joint_dict[joint_id].attrib['Body2MarkerID'])
                        success = True
                        break
                if attrib.endswith('MarkerIDList'):
                    for marker_id in strap.attrib[attrib].split():
                        body_id = marker_dict[marker_id].attrib['BodyID']
                        if body1_id == body_id:
                            torque_marker_list.append(joint_dict[joint_id].attrib['Body1MarkerID'])
                            success = True
                            break
                        if body2_id == body_id:
                            torque_marker_list.append(joint_dict[joint_id].attrib['Body2MarkerID'])
                            success = True
                            break
            if success:
                break
            print('Warning "%s": unable calculate moment arm around "%s" or "%s"' % (strap.attrib['ID'], joint_dict[joint_id].attrib['Body1MarkerID'], joint_dict[joint_id].attrib['Body2MarkerID']))
            break
    strap.attrib['TorqueMarkerIDList'] = ' '.join(torque_marker_list)
    return strap

def get_joint_links(end_body, marker_dict, joint_dict, joint_chain, body_chain, solution_list):
    for joint in joint_dict:
        if joint in joint_chain:
            continue
        connected_body = test_connected(joint_dict[joint], body_chain[-1], marker_dict)
        if not connected_body:
            continue
        body_chain.append(connected_body)
        joint_chain.append(joint)
        if body_chain[-1] == end_body:
            solution_list.append(copy.deepcopy((body_chain, joint_chain)))
        else:
            get_joint_links(end_body, marker_dict, joint_dict, joint_chain, body_chain, solution_list)
        body_chain.pop(-1)
        joint_chain.pop(-1)
    return False

def permute(input_list): # uses yield rather than return so it returns a generator object that can be iterated rather than a list
    length = len(input_list)
    if length <= 1:
        yield input_list
    else:
        for n in range(0,length):
             for end in permute( input_list[:n] + input_list[n+1:] ):
                 yield [ input_list[n] ] + end
                 
def get_joint_links_permute(start_body, end_body, marker_dict, joint_dict): # this probably works but it very slow
    # get a indexed list of the joints
    joint_list = []
    joint_names = list(joint_dict.keys())
    for joint in joint_names:
        joint_list.append(joint_dict[joint])
    # create an empty solution list
    solution_list = []
    # get all the permutations of joint indices
    indices = list(range(0, len(joint_list)))
    permutations = permute(indices)
    for permutation in permutations:
        print(permutation)
        next_body = start_body
        current_path = [next_body]        
        for i in range(0, len(permutation)):
            maybe_next_body = test_connected(joint_list[permutation[i]], next_body, marker_dict)
            if not next_body:
                break
            current_path.append(maybe_next_body)
            next_body = maybe_next_body
            if next_body == end_body:
                solution_list.append(current_path)
                break
        if len(current_path) > 1 and current_path[-1] == end_body:
            solution_list.append(current_path)
    if len(solution_list) == 0: return []
    joint_chain = []
    min_length = len(solution_list[0])
    min_length_index = 0
    for i in range(1, len(solution_list[0])):
        if len(solution_list[i]) < min_length:
            min_length_index = i
    for i in solution_list[min_length_index]:
        joint_chain.append(joint_names[i])
    return solution_list  
                        
                        
def test_connected(joint, body_id, marker_dict):
    if marker_dict[joint.attrib['Body1MarkerID']].attrib['BodyID'] == body_id:
        return marker_dict[joint.attrib['Body2MarkerID']].attrib['BodyID']
    if marker_dict[joint.attrib['Body2MarkerID']].attrib['BodyID'] == body_id: 
        return marker_dict[joint.attrib['Body1MarkerID']].attrib['BodyID']
    return ''

def create_marker(marker_id, body_id, position, quaternion):
    new_marker = xml.etree.ElementTree.Element("MARKER")
    new_marker.tail = "\n"
    new_marker.attrib["ID"] = str(marker_id)
    new_marker.attrib["BodyID"] = str(body_id)
    new_marker.attrib["Position"] = convert_to_string(position)
    new_marker.attrib["Quaternion"] = convert_to_string(quaternion)
    return new_marker

def convert_to_string(input_object):
    if type(input_object) is str:
        return input_object
    
    if type(input_object) is list:
        l = []
        for o in input_object:
            l.append(convert_to_string(o))
        return ' '.join(l)
    
    if type(input_object) is int:
        return str(input_object)
    
    if type(input_object) is float:
        return format(input_object, ".18g")
    
    print('Warning: %s type not handled explicitly in convert_to_string' % (str(type(input_object))))
    return str(input_object)

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
    instrument_for_moment_arms()
    
