#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import argparse
import re
import xml.etree.ElementTree
import trimesh
import math

def check_mass_properties():

    parser = argparse.ArgumentParser(description="Check the mass properties in a GaitSym file by recalculating from the meshes")
    
    parser.add_argument("-i", "--input_file", required=True, help="The GaitSym input file to process")
    parser.add_argument("-o", "--output_file", help="Optional output file for recalculated mass properties")
    parser.add_argument("-m", "--mesh_source", default="GraphicFile1", help="Mesh attrib to use for mass properties [GraphicFile1]")
    parser.add_argument("-p", "--mesh_path", help="If set then overwrite mesh path")
    parser.add_argument("-t", "--tolerance", type=float, default=1e-7, help="Tolerance for the match [1e-7]")
    parser.add_argument("-f", "--force", action="store_true", help="force overwrite of destination file")
    parser.add_argument("-v", "--verbose", action="store_true", help="write out more information whilst processing")
    args = parser.parse_args()

    if args.verbose:
        pretty_print_sys_argv(sys.argv)
        pretty_print_argparse_args(args)
        
    preflight_read_file(args.input_file, args.verbose)
    if args.output_file:
        preflight_write_file(args.output_file, args.force, args.verbose)
    
    # read the input XML file
    if args.verbose: print('Reading "%s"' % (args.input_file))
    input_tree = xml.etree.ElementTree.parse(args.input_file)
    input_root = input_tree.getroot()
    
    original_total_mass = 0.0
    new_total_mass = 0.0
    mesh_path = ['.']
    if args.mesh_path:
        mesh_path = decode_search_path(args.mesh_path)
    for child in input_root:
        if child.tag == "GLOBAL" and not args.mesh_path:
            mesh_path = decode_search_path(child.attrib["MeshSearchPath"])
            
        if child.tag == "BODY":
            if args.verbose: print('Processing BODY ID="%s"' % (child.attrib['ID']))
            mass_properties = calculate_mass_properties(child, os.path.split(args.input_file)[0], mesh_path, args.mesh_source, args.verbose)
            mass = float(child.attrib['Mass'])
            moments_of_inertia = [float(x) for x in child.attrib['MOI'].split()]
            if isclose(mass, mass_properties['mass'], args.tolerance):
                if args.verbose: print('Mass match is OK')
            else: print('Processing BODY ID="%s" Mass match fails %.17g != %.17g' % (child.attrib['ID'], mass, mass_properties['mass']))
            if isclose(moments_of_inertia[0], mass_properties['inertial_tensor'][0][0], args.tolerance):
                if args.verbose: print('Ixx match is OK')
            else: print('Processing BODY ID="%s" Ixx match fails %.17g != %.17g' % (child.attrib['ID'], moments_of_inertia[0], mass_properties['inertial_tensor'][0][0]))
            if isclose(moments_of_inertia[1], mass_properties['inertial_tensor'][1][1], args.tolerance):
                if args.verbose: print('Iyy match is OK')
            else: print('Processing BODY ID="%s" Iyy match fails %.17g != %.17g' % (child.attrib['ID'], moments_of_inertia[1], mass_properties['inertial_tensor'][1][1]))
            if isclose(moments_of_inertia[2], mass_properties['inertial_tensor'][2][2], args.tolerance):
                if args.verbose: print('Izz match is OK')
            else: print('Processing BODY ID="%s" Izz match fails %.17g != %.17g' % (child.attrib['ID'], moments_of_inertia[2], mass_properties['inertial_tensor'][2][2]))
            if isclose(moments_of_inertia[3], mass_properties['inertial_tensor'][0][1], args.tolerance):
                if args.verbose: print('Ixy match is OK')
            else: print('Processing BODY ID="%s" Ixy match fails %.17g != %.17g' % (child.attrib['ID'], moments_of_inertia[3], mass_properties['inertial_tensor'][0][1]))
            if isclose(moments_of_inertia[4], mass_properties['inertial_tensor'][0][2], args.tolerance):
                if args.verbose: print('Ixz match is OK')
            else: print('Processing BODY ID="%s" Ixz match fails %.17g != %.17g' % (child.attrib['ID'], moments_of_inertia[4], mass_properties['inertial_tensor'][0][2]))
            if isclose(moments_of_inertia[5], mass_properties['inertial_tensor'][1][2], args.tolerance):
                if args.verbose: print('Iyz match is OK')
            else: print('Processing BODY ID="%s" Iyz match fails %.17g != %.17g' % (child.attrib['ID'], moments_of_inertia[5], mass_properties['inertial_tensor'][1][2]))
            original_total_mass = original_total_mass + mass
            new_total_mass = new_total_mass + mass_properties['mass']
            
            # set the calculated values even though we might not use them
            child.attrib['Mass'] = format(mass_properties['mass'], '.18g')
            moi = [mass_properties['inertial_tensor'][0][0], mass_properties['inertial_tensor'][1][1], mass_properties['inertial_tensor'][2][2],
                   mass_properties['inertial_tensor'][0][1], mass_properties['inertial_tensor'][0][2], mass_properties['inertial_tensor'][1][2]]
            child.attrib["MOI"] = ' '.join(format(x, '.18g') for x in moi)
            
    if args.verbose: print('Original Total Mass = %.17g' % (original_total_mass))
    if args.verbose: print('Recalculated Total Mass = %.17g' % (new_total_mass))
    
    if args.output_file:
        # create an empty GAITSYM tree
        new_tree = xml.etree.ElementTree.Element("GAITSYM2019")
        new_tree.text = "\n"
        new_tree.tail = "\n"
        
        for child in input_root:
            new_tree.append(child)
            
        if args.verbose: print('Writing "%s"' % (args.output_file))
        with open(args.output_file, "wb") as out_file:
            out_file.write(xml.etree.ElementTree.tostring(new_tree, encoding="utf-8", method="xml"))
 
def isclose(a, b, tol):
    multiplier = a / b
    if math.fabs(multiplier - 1.0) > tol:
        return False
    return True

def calculate_mass_properties(body, mesh_root, mesh_path, mesh_source, verbose):
    if verbose: print('Processing "%s"' % (body.attrib['ID']))
    mass_properties = {}
    mesh_filename = body.attrib[mesh_source]
    for path in mesh_path:
        test_file = os.path.join(mesh_root, path, mesh_filename)
        if os.path.isfile(test_file):
            mesh_filename = test_file
            break
    preflight_read_file(mesh_filename, verbose)
    
    # if process=True Trimesh will do a light processing, which will
    # remove any NaN values and merge vertices that share position
    try:
        mesh = trimesh.load(mesh_filename, process=True)
    except:
        print('Error reading "%s"' % (mesh_filename))
        sys.exit(1)
    
    # lets output some information
    if verbose:
        if mesh.is_watertight: print('Mesh is watertight')
        else: print('Mesh is not watertight')
        if mesh.is_winding_consistent: print('Mesh has consistent winding')
        else: print('Mesh has inconsistent winding')
        print('Mesh euler number = %f' % (mesh.euler_number))
        print('Mesh bounding box = ',)
        print(mesh.bounding_box.extents)

    if not mesh.is_watertight:
        print('Error in "%s": mesh not watertight' % (mesh_filename))
    if not mesh.is_winding_consistent:
        print('Error in "%s": mesh not winding consistent' % (mesh_filename))
    
    density = float(body.attrib['ConstructionDensity'])
    if verbose: print('Density = %.17g' % (density))
    mesh.density = density
    
    # since the mesh is watertight, it means there is a
    # volumetric center of mass which we can set as the origin for our mesh
    mesh.vertices -= mesh.center_mass
    
    # mass properties
    if verbose: print(mesh.mass)
    if verbose: print(mesh.moment_inertia)

    mass_properties['mass'] = mesh.mass
    mass_properties['inertial_tensor'] = mesh.moment_inertia
    return mass_properties       
    

def decode_search_path(path_string):
    parts = path_string.split(':')
    for i in range(0, len(parts)):
        parts[i] = percent_decode(parts[i])
    return parts

def percent_decode(inp):
    out = ''
    i = 0
    while i < len(inp):
        if inp[i] != '%':
            out = out + inp[i]
            i = i + 1
            continue
        hex_str = inp[i + 1: i + 3]
        i = i + 3
        out = out + chr(int(hex_str, 16))
    return out

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
    check_mass_properties()
    
