#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import argparse
import re
import trimesh

def simplify_obj_files():

    parser = argparse.ArgumentParser(description="Takes a list of OBJ files and simplifies them by removing materials, normals, UVs and triangulating faces")
    parser.add_argument("input_obj_files", nargs='+', help="input OBJ files")
    parser.add_argument("-o", "--output_folder", required=True, help="output folder for OBJ files")
    parser.add_argument("-d", "--decimal_places", type=int, default=6, help="number of decimal places to write out [6]")
    parser.add_argument("-r", "--repair", action="store_true", help="try to repair the mesh if faults are found")
    parser.add_argument("-f", "--force", action="store_true", help="force overwrite of destination file")
    parser.add_argument("-v", "--verbose", action="store_true", help="write out more information whilst processing")
    args = parser.parse_args()

    if args.verbose:
        pretty_print_sys_argv(sys.argv)
        pretty_print_argparse_args(args)
    
    preflight_write_folder(args.output_folder, args.verbose)
    for input_obj_file in args.input_obj_files:
        preflight_read_file(input_obj_file, args.verbose)
        preflight_write_file(os.path.join(args.output_folder, os.path.split(input_obj_file)[1]), args.force, args.verbose)
    
    for input_obj_file in args.input_obj_files:
        output_obj_file = os.path.join(args.output_folder, os.path.split(input_obj_file)[1])
        simplify_obj_file(input_obj_file, output_obj_file, args.decimal_places, args.repair, args.verbose)
        
def simplify_obj_file(input_obj_file, output_obj_file, decimal_places, repair, verbose):
    if verbose: print('Reading "%s"' % input_obj_file)
    
    # if process=True Trimesh will do a light processing, which will
    # remove any NaN values and merge vertices that share position
    # any quads or ngons are converted to triangles and the mesh contains any UVs, normals, and materials
    try:
        mesh = trimesh.load(input_obj_file, process=True, force='mesh')            
        # mesh.faces = trimesh.geometry.triangulate_quads(mesh.faces) # not needed - it looks like triangulation happens at load
    except:
        print('Error reading "%s"' % (input_obj_file))
        sys.exit(1)
    
    # lets output some information
    if verbose:
        if mesh.is_watertight: print('Mesh is watertight')
        else: print('Mesh is not watertight')
        if mesh.is_winding_consistent: print('Mesh has consistent winding')
        else: print('Mesh has inconsistent winding')
        print('Mesh Euler number = %f' % (mesh.euler_number))
        print('Mesh bounding box = ',)
        print(mesh.bounding_box.extents)

    if repair:
        if not mesh.is_watertight:
            print('Warning in "%s": mesh not watertight so hole filling' % (input_obj_file))
            trimesh.repair.fill_holes(mesh)
        # there is no test for normals so just fix them (which sorts out winding too
        trimesh.repair.fix_normals(mesh)
        if not mesh.is_winding_consistent:
            print('Error in "%s": mesh has inconsistent winding' % (input_obj_file))
            sys.exit(1)
    
    # we now have a mesh and we need to create a simple OBJ files from it
    if verbose: print('Writing "%s"' % output_obj_file)
    vertex_output_format = 'v %%.%dg %%.%dg %%.%dg\n' % (decimal_places, decimal_places, decimal_places)
    face_output_format = 'f %d %d %d\n'
    with open(output_obj_file, 'w') as f:
        f.write('# created by simplify_obj_files.py\n')
        for vertex in mesh.vertices:
            f.write(vertex_output_format % (vertex[0], vertex[1], vertex[2]))
        for face in mesh.faces:
            if len(face) != 3: # this should always be 3 since the loader is supposed to triangulate polygons
                print('Only triangular faces supported')
                sys.exit(1)
            f.write(face_output_format % (face[0] + 1, face[1] + 1, face[2] + 1)) # +1 needed because OBJ files index from 1
    
    return                


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
    simplify_obj_files()
    
