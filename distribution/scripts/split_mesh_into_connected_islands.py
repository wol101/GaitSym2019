#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import argparse
import re
import trimesh

def split_mesh_into_connected_islands():

    parser = argparse.ArgumentParser(description="Convert a mesh file (.obj .ply .stl should all work) a list of connected island objects")
    parser.add_argument("-i", "--input_obj_file", required=True, help="the input OBJ file")
    parser.add_argument("-o", "--output_folder", required=True, help="the folder to output all the files to")
    parser.add_argument("-c", "--output_csv", default="", help="the csv file to write calculated values to []")
    parser.add_argument("-e", "--output_extension", default=".ply", help="the output file extension [.ply]")
    parser.add_argument("-f", "--force", action="store_true", help="force overwrite of destination file")
    parser.add_argument("-p", "--preflight", action="store_true", help="report action but do nothing")
    parser.add_argument("-v", "--verbose", action="store_true", help="write out more information whilst processing")
    args = parser.parse_args()

    if args.preflight:
        args.verbose = True

    if args.verbose:
        pretty_print_sys_argv(sys.argv)
        pretty_print_argparse_args(args)

    # preflight
    preflight_read_file(args.input_obj_file)
    preflight_read_folder(args.output_folder)
    if args.output_csv: preflight_write_file(args.output_csv, args.force)
    
    if args.verbose: print('Reading "%s"' % (args.input_obj_file))

    # attach to logger so trimesh messages will be printed to console
    trimesh.util.attach_to_log()
    
    # if process=True Trimesh will do a light processing, which will
    # remove any NaN values and merge vertices that share position
    mesh = trimesh.load(args.input_obj_file, process=True)
    
    # lets output some information
    if args.verbose:
        if mesh.is_watertight: print('Mesh is watertight')
        else: print('Mesh is not watertight')
        if mesh.is_winding_consistent: print('Mesh has consistent winding')
        else: print('Mesh has inconsistent winding')
        print('Mesh euler number = %f' % (mesh.euler_number))
        print('Mesh bounding box = ', mesh.bounding_box)
    
    cvs_out = 0
    if not args.preflight and args.output_csv:
        cvs_out = open(args.output_csv, 'w')
    
    # split the mesh into connected components of face adjacency
    # splitting sometimes produces non- watertight meshes
    # though the splitter will try to repair single quad and
    # single triangle holes, in our case here we are going to be
    # taking convex hulls anyway so there is no reason to discard
    # the non- watertight bodies
    mesh_list = mesh.split(only_watertight=False)
    
    for mesh_i in range(0, len(mesh_list)):
        current_mesh = mesh_list[mesh_i]
        file_name = 'island_%07d%s' % (mesh_i, args.output_extension)
        path = os.path.join(args.output_folder, file_name)
        preflight_write_file(path, args.force)
        if args.verbose: print('Writing "%s"' % (path))
        if not args.preflight:
            current_mesh.export(path)
        if current_mesh.is_watertight:
            line = '%s\t%g' % (file_name, current_mesh.volume)
            if args.verbose: print(line)
            if cvs_out: cvs_out.write(line + '\n')
            
    if cvs_out: cvs_out.close()

    
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

def preflight_read_folder(folder):
    if not os.path.exists(folder):
        print("Error: \"%s\" not found" % (folder))
        sys.exit(1)
    if not os.path.isdir(folder):
        print("Error: \"%s\" not a folder" % (folder))
        sys.exit(1)
        
def pretty_print_sys_argv(sys_argv):
    quoted_sys_argv = quoted_if_necessary(sys_argv)
    print((' '.join(quoted_sys_argv)))

def pretty_print_argparse_args(argparse_args):
    for arg in vars(argparse_args):
        print(('%s: %s' % (arg, getattr(argparse_args, arg))))

def quoted_if_necessary(input_list):
    output_list = []
    for item in input_list:
        if re.search('[^a-zA-Z0-9_.-]', item):
            item = '"' + item + '"'
        output_list.append(item)
    return output_list

        # program starts here
if __name__ == "__main__":
    split_mesh_into_connected_islands()
