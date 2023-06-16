#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import re
import fnmatch
import math
import xml.etree.ElementTree
import argparse

def generate_rom_drivers():
    
    parser = argparse.ArgumentParser(description="Create a version of a GaitSym config file suitable for ROM testing\n"
                                                 "Needs a posed starting file as an input\n"
                                                 "Note angles are in degrees by default but can be specified in radians if they end in an r")
    
    parser.add_argument("-l", "--left_side_string", default='Left', help="prefix string identifying left side muscles etc [Left]")
    parser.add_argument("-r", "--right_side_string", default='Right', help="prefix string identifying right side muscles etc [Right]")
    parser.add_argument("-i", "--input_xml_file", required=True, help="file name for input XML config file")
    parser.add_argument("-o", "--output_xml_file", required=True, help="file name for output XML config file")
    parser.add_argument("-is", "--integration_step_size", type=float, help="value to use for the new integration_step_size")
    parser.add_argument("-al", "--activation_level", default=1.0, type=float, help="value to use for driver activation level [1]")
    parser.add_argument("-cd", "--cycle_duration", default=1.0, type=float, help="duration of each activation step [1]")
    parser.add_argument("-rd", "--rom_delta", nargs=3, required=False, help="apply a range of motion delta to glob matching joint name (name low high)")
    parser.add_argument("-rs", "--rom_set", nargs=3, required=False, help="set the range of motion values to glob matching joint name (name low high)")    
    parser.add_argument("-re", "--regex_match", action="store_true", help="fname match using regex instead of globbing")
    parser.add_argument("-ze", "--zero_pose", action="store_true", help="reset the pose to the construction pose")
    parser.add_argument("-f", "--force", action="store_true", help="force overwrite of destination file")
    parser.add_argument("-v", "--verbose", action="store_true", help="write out more information whilst processing")
    args = parser.parse_args()

    if args.verbose:
        pretty_print_sys_argv(sys.argv)
        pretty_print_argparse_args(args)
        
    rom_delta_map = {}
    if args.rom_delta:
        name = args.rom_delta[0]
        low = degrees(args.rom_delta[1])
        high = degrees(args.rom_delta[2])
        rom_delta_map[name] = (low, high)
        
    rom_set_map = {}
    if args.rom_set:
        name = args.rom_set[0]
        low = degrees(args.rom_set[1])
        high = degrees(args.rom_set[2])
        rom_delta_map[name] = (low, high)
    
    preflight_read_file(args.input_xml_file, args.verbose)
    preflight_write_file(args.output_xml_file, args.force, args.verbose)
    
    if args.verbose: print('Reading "%s"' % (args.input_xml_file))
    tree = xml.etree.ElementTree.parse(args.input_xml_file)
    root = tree.getroot()
    
    muscle_list = []
    
    for child in root:      
        # get a list of the muscles
        if child.tag == "MUSCLE":
            muscle_id = child.attrib['ID']
            if (muscle_id.startswith(args.left_side_string)):
                muscle_id_root = muscle_id[len(args.left_side_string):]
                if args.verbose: print('MUSCLE muscle_id_root="%s"' % (muscle_id_root))
                muscle_list.append(muscle_id_root)
        
        # alter the joint ranges of motion
        if child.tag == "JOINT":
            if child.attrib['Type'] == 'Hinge':
                joint_id = child.attrib['ID']
                for name in list(rom_delta_map.keys()):
                    if (args.regex_match):
                        regex_match_str = name
                    else:
                        regex_match_str = fnmatch.translate(name)
                    if re.match(regex_match_str, joint_id):
                        new_low_stop = '%gd' % (degrees(child.attrib['LowStop']) + rom_delta_map[name][0])
                        new_high_stop = '%gd' % (degrees(child.attrib['HighStop']) + rom_delta_map[name][1])
                        if args.verbose: print('Changing JOINT %s LowStop from %gd to %gd' % (joint_id, degrees(child.attrib['LowStop']), degrees(new_low_stop)))
                        if args.verbose: print('Changing JOINT %s HighStop from %gd to %gd' % (joint_id, degrees(child.attrib['HighStop']), degrees(new_high_stop)))
                        child.attrib['LowStop'] = new_low_stop
                        child.attrib['HighStop'] = new_high_stop
                        continue
                for name in list(rom_set_map.keys()):
                    if (args.regex_match):
                        regex_match_str = name
                    else:
                        regex_match_str = fnmatch.translate(name)
                    if re.match(regex_match_str, joint_id):
                        new_low_stop = '%gd' % (rom_set_map[name][0])
                        new_high_stop = '%gd' % (rom_set_map[name][1])
                        if args.verbose: print('Changing JOINT %s LowStop from %gd to %gd' % (joint_id, degrees(child.attrib['LowStop']), degrees(new_low_stop)))
                        if args.verbose: print('Changing JOINT %s HighStop from %gd to %gd' % (joint_id, degrees(child.attrib['HighStop']), degrees(new_high_stop)))
                        child.attrib['LowStop'] = new_low_stop
                        child.attrib['HighStop'] = new_high_stop
                        continue
                    
        # zero the pose to the construction pose
        if args.zero_pose and child.tag == "BODY":
            child.attrib['Position'] = child.attrib['ConstructionPosition']
            child.attrib['Quaternion'] = '1 0 0 0'                    
            child.attrib['LinearVelocity'] = '0 0 0'                    
            child.attrib['AngularVelocity'] = '0 0 0'                    
    
    # copy to new tree
    # deleting old driver lines and creating new ones
    
    new_tree = xml.etree.ElementTree.Element(root.tag)
    new_tree.text = '\n'
    new_driver_tree = xml.etree.ElementTree.Element(root.tag)
    new_driver_tree.text = '\n'
    
    for child in root:
        if child.tag != "DRIVER" and child.tag != "ENVIRONMENT":
            new_tree.append(child)
    
    # left_side_string side first
    for imuscle in range(0, len(muscle_list)):
        muscle = muscle_list[imuscle]
        new_driver = xml.etree.ElementTree.Element('DRIVER')
        new_driver.attrib['Type'] = "Cyclic"
        new_driver.attrib['ID'] = args.left_side_string + muscle + 'Driver'
        new_driver.attrib['TargetIDList'] = args.left_side_string + muscle
        durations = ''
        values = ''
        for i in range(0, len(muscle_list)):
            durations = durations + ('%g ' % (args.cycle_duration))
            if (i != imuscle):
                values = values + ('0 ')
            else:
                values = values + ('%g ' % (args.activation_level))
        new_driver.attrib['Durations'] = durations[:-1]
        new_driver.attrib['Values'] = values[:-1]
        new_driver.attrib['PhaseDelay'] = '0'
        new_driver.tail = '\n'
        new_tree.append(new_driver)
        if args.verbose: print('DRIVER ID="%s"' % new_driver.attrib['ID'])
        

    # now right_side_string
    for imuscle in range(0, len(muscle_list)):
        muscle = muscle_list[imuscle]
        new_driver = xml.etree.ElementTree.Element('DRIVER')
        new_driver.attrib['Type'] = "Cyclic"
        new_driver.attrib['ID'] = args.right_side_string + muscle + 'Driver'
        new_driver.attrib['TargetIDList'] = args.right_side_string + muscle
        durations = ''
        values = ''
        for i in range(0, len(muscle_list)):
            durations = durations + ('%g ' % (args.cycle_duration))
            if (i != imuscle):
                values = values + ('0 ')
            else:
                values = values + ('%g ' % (args.activation_level))
        new_driver.attrib['Durations'] = durations[:-1]
        new_driver.attrib['Values'] = values[:-1]
        new_driver.attrib['PhaseDelay'] = '0'
        new_driver.tail = '\n'
        new_tree.append(new_driver)
        if args.verbose: print('DRIVER ID="%s"' % new_driver.attrib['ID'])

    for child in new_tree:
        if child.tag == "GLOBAL":
            child.attrib['TimeLimit'] = str(len(muscle_list) * args.cycle_duration)
            child.attrib['MechanicalEnergyLimit'] = '0'
            child.attrib['MetabolicEnergyLimit'] = '0'
            child.attrib['GravityVector'] = '0.0 0.0 0.0'
            if args.integration_step_size:
                child.attrib['IntegrationStepSize'] = str(args.integration_step_size)

        if child.tag == "BODY":
            child.attrib['PositionLowBound'] = '-9999 -9999 -9999'
            child.attrib['PositionHighBound'] = '9999 9999 9999'

    if args.verbose: print('Writing "%s"' % (args.output_xml_file))
    with open(args.output_xml_file, 'wb') as out:
        xml_text = xml.etree.ElementTree.tostring(new_tree, encoding="utf-8", method="xml")
        out.write(xml_text)
    
    

def gene_string(n_genes, gene_start):
    gene_string = ''
    for i in range(0, n_genes):
        gene_string += '[[g(%d)]] ' % (gene_start + i)
    gene_string = gene_string[0:-1]
    return gene_string

def gene_string_phase(n_genes, gene_start, phase):
    gene_string = ''
    for i in range(0, n_genes):
        gene_string += '[[if(g(%d)+%g>1.0,g(%d)-%g,g(%d)+%g)]] ' % (gene_start + i, phase, gene_start + i, phase, gene_start + i, phase)
    gene_string = gene_string[0:-1]
    return gene_string

def degrees(str):
    if (str.endswith('d')):
        return float(str[0:-1])
    else:
        if (str.endswith('r')):
            return float(str[0:-1]) * 180.0 / math.pi            
    return float(str)

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
    generate_rom_drivers()
    
