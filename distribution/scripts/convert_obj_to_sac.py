#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import argparse
import re
import xml.etree.ElementTree

def convert_obj_to_sac():

    parser = argparse.ArgumentParser(description="Try to convert an OBJ file into a FluidSac definition")
    parser.add_argument("-i", "--input_obj_file", required=True, help="the input OBJ file")
    parser.add_argument("-o", "--output_xml_file", required=True, help="the output GaitSym2019 XML config file")
    parser.add_argument("-m", "--min_distance", type=float, default=0.0001, help="the minimum distance between vertices [0.0001]")
    parser.add_argument("-f", "--force", action="store_true", help="force overwrite of destination file")
    parser.add_argument("-v", "--verbose", action="store_true", help="write out more information whilst processing")
    args = parser.parse_args()

    if args.verbose:
        pretty_print_sys_argv(sys.argv)
        pretty_print_argparse_args(args)

    # preflight
    if not os.path.exists(args.input_obj_file):
        print("Error: \"%s\" missing" % (args.input_obj_file))
        sys.exit(1)
    if os.path.exists(args.output_xml_file) and not args.force:
        print("Error: \"%s\" exists. Use --force to overwrite" % (args.output_xml_file))
        sys.exit(1)

    (vertices, triangles) = read_obj_file(args.input_obj_file, args.verbose)
#    write_obj_file('debug0.obj', vertices, triangles, verbose = True)
    check_vertices(vertices, args.min_distance)
    check_mesh(vertices, triangles)

    # create an empty GAITSYM tree
    new_tree = xml.etree.ElementTree.Element("GAITSYM2019")
    new_tree.text = "\n"
    new_tree.tail = "\n"

    # create the markers
    marker_names = []
    for i in range(0, len(vertices)):
        vertex = vertices[i]
        # create the muscle markers
        marker = xml.etree.ElementTree.Element("MARKER")
        marker.tail = "\n"
        marker_names.append("FluidSac_Marker_%d" % (i))
        marker.attrib["ID"] = marker_names[-1]
        marker.attrib["BodyID"] = "World"
        marker.attrib["ConstructionPosition"] = ' '.join([str(x) for x in vertex])
        marker.attrib["ConstructionQuaternion"] = "1 0 0 0"
        new_tree.append(marker)

    # create the FluidSac

    fluid_sac= xml.etree.ElementTree.Element("FLUIDSAC")
    fluid_sac.tail = "\n"
    fluid_sac.attrib["ID"] = "FluidSac1"
    fluid_sac.attrib["Type"] = "IdealGas"
    fluid_sac.attrib["AmountOfSubstance"] = "1"
    fluid_sac.attrib["Temperature"] = "293.15"
    fluid_sac.attrib["NumMarkers"] = str(len(marker_names))
    fluid_sac.attrib["MarkerIDList"] = ' '.join(marker_names)
    fluid_sac.attrib["NumTriangles"] = str(len(triangles))
    flat_triangles = [item for sublist in triangles for item in sublist]
    fluid_sac.attrib["TriangleIndexList"] = ' '.join([str(x) for x in flat_triangles])
    new_tree.append(fluid_sac)

    with open(args.output_xml_file, "wb", ) as out_file:
        out_file.write(xml.etree.ElementTree.tostring(new_tree, encoding="utf-8", method="xml"))

def check_vertices(vertices, min_distance):
    min_distance_2 = min_distance ** 2
    for i in range(1, len(vertices)):
        v1 = vertices[i]
        for j in range(0, i):
            v2 = vertices[j]
            distance_2 = (v2[0] - v1[0])**2 + (v2[1] - v1[1])**2 + (v2[2] - v1[2])**2
            if distance_2 < min_distance_2:
                print('Vertex distance error: %d and %d are %g apart' % (j, i, distance_2 ** 0.5))
                sys.exit(1)
    return

def check_mesh(vertices, triangles):
    # for a water tight mesh edges should be in pairs and the direction should be reversed
    # first store the edges in a multimap using the start vertex as key
    edgeList = []
    for i in range(0, len(triangles)):
        triangle = triangles[i];
        edgeList.append((triangle[0], triangle[1], i))
        edgeList.append((triangle[1], triangle[2], i))
        edgeList.append((triangle[2], triangle[0], i))
    edgeList.sort()
#    print(edgeList)
    # then iterate through the edges
    for edge in edgeList:
#        print('edge',edge)
        matched = equal_range(edgeList, edge[1])
#        print('matched',matched)
        count = 0
        for match in matched:
            if match[1] == edge[0]:
                count = count + 1
        if count != 1:
            print('Edge error: each edge should be matched by exactly one reversed edge')
#            new_triangles = [[0, 1, 2]]
#            new_vertices = []
#            new_vertices.append(vertices[triangles[edge[2]][0]])
#            new_vertices.append(vertices[triangles[edge[2]][1]])
#            new_vertices.append(vertices[triangles[edge[2]][2]])
#            write_obj_file('debug1.obj', new_vertices, new_triangles, verbose = True)
#            new_vertices = []
#            new_vertices.append(vertices[triangles[match[2]][0]])
#            new_vertices.append(vertices[triangles[match[2]][1]])
#            new_vertices.append(vertices[triangles[match[2]][2]])
#            write_obj_file('debug2.obj', new_vertices, new_triangles, verbose = True)
            sys.exit(1)

def write_obj_file(filename, vertices, triangles, verbose):
    if verbose:
        print('Writing "%s"' % (filename))
    with open(filename, 'w') as f_out:
        f_out.write('# Generated by convert_obj_to_sac.py\n')
        f_out.write('# Vertices: %d\n' % (len(vertices)))
        f_out.write('# Faces: %d\n' % (len(triangles)))
        for i in range(0, len(vertices)):
            v = vertices[i]
            f_out.write('v %f %f %f\n' % (v[0], v[1], v[2]));
        for i in range(0, len(triangles)):
            t = triangles[i]
            f_out.write('f %d %d %d\n' % (t[0] + 1, t[1] + 1, t[2] + 1));
        f_out.write('# End of file\n')
        if verbose:
            print('Written %d vertices and %d triangles' % (len(vertices), len(triangles)))

def read_obj_file(filename, verbose):
    vertices = []
    triangles = []
    if verbose:
        print('Reading "%s"' % (filename))
    with open(filename) as f_in:
        lines = f_in.read().splitlines()
    for line in lines:
        line = line.strip()
        if line.startswith('#'):
            continue
        tokens = line.split()
        if len(tokens) == 0:
            continue
        if tokens[0] == 'v':
            if len(tokens) != 4:
                print('Error in read_obj_file: vertices need 3 coordinates')
                sys.exit(1)
            vertices.append([float(i) for i in tokens[1:4]])
            continue
        if tokens[0] == 'f':
            if len(tokens) != 4:
                print('Error in read_obj_file: only triangular faces supported')
                sys.exit(1)
            triangles.append([int(i.split('/')[0]) - 1 for i in tokens[1:4]])
    if verbose:
        print('Read %d vertices and %d triangles' % (len(vertices), len(triangles)))
    return (vertices, triangles)

def equal_range(sorted_list_of_pairs, key):
    left_i = 0
    right_i = 0
    for i in range(0, len(sorted_list_of_pairs)):
        if sorted_list_of_pairs[i][0] == key:
            left_i = i
            break
    for j in range(i, len(sorted_list_of_pairs)):
        if sorted_list_of_pairs[j][0] == key:
            right_i = j
        else:
            right_i = j - 1
            break
    return sorted_list_of_pairs[left_i: right_i + 1]

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
    convert_obj_to_sac()
