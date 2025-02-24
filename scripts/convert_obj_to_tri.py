#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import argparse
import re

def convert_obj_to_tri():

    parser = argparse.ArgumentParser(description="Try to convert an OBJ file into a triangle file (basically a list of vertices)")
    parser.add_argument("-i", "--input_obj_file", required=True, help="the input OBJ file")
    parser.add_argument("-t", "--output_tri_file", required=False, default="", help="the output triangle file if wanted")
    parser.add_argument("-o", "--output_obj_file", required=False, default="", help="the output obj file if wanted")
    parser.add_argument("-m", "--min_distance", type=float, default=0.0001, help="the minimum distance between vertices (0 ignores) [0.0001]")
    parser.add_argument("-iu", "--ignore_uv", action="store_true", help="ignores the texture coordinates in the OBJ file")
    parser.add_argument("-in", "--ignore_normal", action="store_true", help="ignores the normal vector in the OBJ file")
    parser.add_argument("-n", "--no_mesh_checks", action="store_true", help="do not perform any the mesh checks")
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
    if os.path.exists(args.output_tri_file) and not args.force:
        print("Error: \"%s\" exists. Use --force to overwrite" % (args.output_tri_file))
        sys.exit(1)

    (vertices, normals, texture_coords, triangles, triangle_normals, triangle_textures, triangle_colours, bounds) = \
        read_obj_file(args.input_obj_file, args.verbose, args.ignore_uv, args.ignore_normal)
    if len(triangles) == 0:
        print("Error: \"%s\" has no triangles" % (args.input_obj_file))
        sys.exit(1)
#    write_obj_file('debug0.obj', vertices, triangles, verbose = True)
    if args.min_distance > 0:
        check_vertices(vertices, args.min_distance)
    if not args.no_mesh_checks:
        check_mesh(vertices, triangles)

    if args.output_tri_file:
        if len(triangle_normals) == len(triangles) and len(triangle_textures) == len(triangles):
            write_xyzxnynznuv_tri(bounds, vertices, normals, texture_coords, triangles, triangle_normals, triangle_textures, args.output_tri_file, args.verbose)
            
        elif len(triangle_normals) != len(triangles) and len(triangle_textures) == len(triangles):
            write_xyzuv_tri(bounds, vertices, texture_coords, triangles, triangle_textures, args.output_tri_file, args.verbose)
    
        elif len(triangle_normals) == len(triangles) and len(triangle_textures) != len(triangles) and len(triangle_colours) == len(triangles):
            write_xyzxnynznrgb_tri(bounds, vertices, normals, triangles, triangle_normals, triangle_colours, args.output_tri_file, args.verbose)
                 
        elif len(triangle_normals) != len(triangles) and len(triangle_textures) != len(triangles) and len(triangle_colours) == len(triangles):
            write_xyzrgb_tri(bounds, vertices, triangles, triangle_colours, args.output_tri_file, args.verbose)
            
        elif len(triangle_normals) == len(triangles) and len(triangle_textures) != len(triangles) and len(triangle_colours) != len(triangles):
            write_xyzxnynzn_tri(bounds, vertices, normals, triangles, triangle_normals, args.output_tri_file, args.verbose)
                 
        elif len(triangle_normals) != len(triangles) and len(triangle_textures) != len(triangles) and len(triangle_colours) != len(triangles):
            write_xyz_tri(bounds, vertices, triangles, args.output_tri_file, args.verbose)
    
    if args.output_obj_file:
        write_obj_file(args.output_obj_file, vertices, triangles, args.verbose)
 
def write_xyzxnynznuv_tri(bounds, vertices, normals, texture_coords, triangles, triangle_normals, triangle_textures, output_tri_file, verbose):
    if verbose:
        print('Writing "%s"\nxyzxnynznuv %d\n' % (output_tri_file, len(triangles)))
    with open(output_tri_file, 'w', newline = '\n') as f_out:
        f_out.write('xyzxnynznuv %d ' % (len(triangles)))
        f_out.write('%f %f %f %f %f %f\n' % (bounds[0][0], bounds[0][1], bounds[0][2], bounds[1][0], bounds[1][1], bounds[1][2]))
        coordinates_written = 0
        for i in range(0, len(triangles)):
            for j in range(0, 3):
                f_out.write('%f %f %f ' % (vertices[triangles[i][j]][0], vertices[triangles[i][j]][1], vertices[triangles[i][j]][2]))
                coordinates_written += 3
                f_out.write('%f %f %f ' % (normals[triangle_normals[i][j]][0], normals[triangle_normals[i][j]][1], normals[triangle_normals[i][j]][2]))
                coordinates_written += 3
                f_out.write('%f %f\n' % (texture_coords[triangle_textures[i][j]][0], texture_coords[triangle_textures[i][j]][1]))
                coordinates_written += 2
    if verbose:
        print('%d coordinates written\n' % (coordinates_written))

def write_xyzuv_tri(bounds, vertices, texture_coords, triangles, triangle_textures, output_tri_file, verbose):
    if verbose:
        print('Writing "%s"\nxyzuv %d\n' % (output_tri_file, len(triangles)))
    with open(output_tri_file, 'w', newline = '\n') as f_out:
        f_out.write('xyzuv %d ' % (len(triangles)))
        f_out.write('%f %f %f %f %f %f\n' % (bounds[0][0], bounds[0][1], bounds[0][2], bounds[1][0], bounds[1][1], bounds[1][2]))
        coordinates_written = 0
        for i in range(0, len(triangles)):
            for j in range(0, 3):
                f_out.write('%f %f %f ' % (vertices[triangles[i][j]][0], vertices[triangles[i][j]][1], vertices[triangles[i][j]][2]))
                coordinates_written += 3
                f_out.write('%f %f\n' % (texture_coords[triangle_textures[i][j]][0], texture_coords[triangle_textures[i][j]][1]))
                coordinates_written += 2
    if verbose:
        print('%d coordinates written\n' % (coordinates_written))

def write_xyzxnynznrgb_tri(bounds, vertices, normals, triangles, triangle_normals, triangle_colours, output_tri_file, verbose):
    if verbose:
        print('Writing "%s"\nxyzxnynznrgb %d\n' % (output_tri_file, len(triangles)))
    with open(output_tri_file, 'w', newline = '\n') as f_out:
        f_out.write('xyzxnynznrgb %d ' % (len(triangles)))
        f_out.write('%f %f %f %f %f %f\n' % (bounds[0][0], bounds[0][1], bounds[0][2], bounds[1][0], bounds[1][1], bounds[1][2]))
        coordinates_written = 0
        for i in range(0, len(triangles)):
            for j in range(0, 3):
                f_out.write('%f %f %f ' % (vertices[triangles[i][j]][0], vertices[triangles[i][j]][1], vertices[triangles[i][j]][2]))
                coordinates_written += 3
                f_out.write('%f %f %f ' % (normals[triangle_normals[i][j]][0], normals[triangle_normals[i][j]][1], normals[triangle_normals[i][j]][2]))
                coordinates_written += 3
                f_out.write('%f %f %f\n' % (triangle_colours[i][0], triangle_colours[i][1], triangle_colours[i][2]))
                coordinates_written += 3
    if verbose:
        print('%d coordinates written\n' % (coordinates_written))

def write_xyzrgb_tri(bounds, vertices, triangles, triangle_colours, output_tri_file, verbose):
    if verbose:
        print('Writing "%s"\nxyzrgb %d\n' % (output_tri_file, len(triangles)))
    with open(output_tri_file, 'w', newline = '\n') as f_out:
        f_out.write('xyzrgb %d ' % (len(triangles)))
        f_out.write('%f %f %f %f %f %f\n' % (bounds[0][0], bounds[0][1], bounds[0][2], bounds[1][0], bounds[1][1], bounds[1][2]))
        coordinates_written = 0
        for i in range(0, len(triangles)):
            for j in range(0, 3):
                f_out.write('%f %f %f ' % (vertices[triangles[i][j]][0], vertices[triangles[i][j]][1], vertices[triangles[i][j]][2]))
                coordinates_written += 3
                f_out.write('%f %f %f\n' % (triangle_colours[i][0], triangle_colours[i][1], triangle_colours[i][2]))
                coordinates_written += 3
    if verbose:
        print('%d coordinates written\n' % (coordinates_written))

def write_xyzxnynzn_tri(bounds, vertices, normals, triangles, triangle_normals, output_tri_file, verbose):
    if verbose:
        print('Writing "%s"\nxyzxnynzn %d\n' % (output_tri_file, len(triangles)))
    with open(output_tri_file, 'w', newline = '\n') as f_out:
        f_out.write('xyzxnynzn %d ' % (len(triangles)))
        f_out.write('%f %f %f %f %f %f\n' % (bounds[0][0], bounds[0][1], bounds[0][2], bounds[1][0], bounds[1][1], bounds[1][2]))
        coordinates_written = 0
        for i in range(0, len(triangles)):
            for j in range(0, 3):
                f_out.write('%f %f %f ' % (vertices[triangles[i][j]][0], vertices[triangles[i][j]][1], vertices[triangles[i][j]][2]))
                coordinates_written += 3
                f_out.write('%f %f %f ' % (normals[triangle_normals[i][j]][0], normals[triangle_normals[i][j]][1], normals[triangle_normals[i][j]][2]))
                coordinates_written += 3
    if verbose:
        print('%d coordinates written\n' % (coordinates_written))


def write_xyz_tri(bounds, vertices, triangles, output_tri_file, verbose):
    if verbose:
        print('Writing "%s"\nxyz %d\n' % (output_tri_file, len(triangles)))
    with open(output_tri_file, 'w', newline = '\n') as f_out:
        f_out.write('xyz %d ' % (len(triangles)))
        f_out.write('%f %f %f %f %f %f\n' % (bounds[0][0], bounds[0][1], bounds[0][2], bounds[1][0], bounds[1][1], bounds[1][2]))
        coordinates_written = 0
        for i in range(0, len(triangles)):
            for j in range(0, 3):
                f_out.write('%f %f %f ' % (vertices[triangles[i][j]][0], vertices[triangles[i][j]][1], vertices[triangles[i][j]][2]))
                coordinates_written += 3
    if verbose:
        print('%d coordinates written\n' % (coordinates_written))

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
    with open(filename, 'w', newline = '\n') as f_out:
        f_out.write('# Generated by convert_obj_to_tri.py\n')
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

def read_obj_file(filename, verbose, ignore_uv, ignore_normal):
    vertices = []
    normals = []
    texture_coords = []
    triangles = []
    triangle_normals = []
    triangle_textures = []
    triangle_colours = []
    materials = {}
    current_material = {}
    low_bounds = [sys.float_info.max, sys.float_info.max, sys.float_info.max]
    high_bounds = [-sys.float_info.max, -sys.float_info.max, -sys.float_info.max]
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
        if tokens[0] == 'mtllib':
            if len(tokens) != 2:
                print('Error in read_obj_file: mtllib need 1 filename, no spaces')
                sys.exit(1)
            materials = read_obj_materials_file(os.path.join(os.path.dirname(filename), tokens[1]), verbose)
            continue
        if tokens[0] == 'usemtl':
            if len(tokens) != 2:
                print('Error in read_obj_file: usemtl need 1 name, no spaces')
                sys.exit(1)
            if not tokens[1] in materials:
                print('Error in read_obj_file: material "%s" not found' % (tokens[1]))
                sys.exit(1)
            current_material = materials[tokens[1]]
            continue
        if tokens[0] == 'v':
            if len(tokens) != 4:
                print('Error in read_obj_file: vertices need 3 coordinates')
                sys.exit(1)
            vertex = [float(i) for i in tokens[1:4]]
            vertices.append(vertex)
            low_bounds[0] = min(low_bounds[0], vertex[0])
            low_bounds[1] = min(low_bounds[1], vertex[1])
            low_bounds[2] = min(low_bounds[2], vertex[2])
            high_bounds[0] = max(high_bounds[0], vertex[0])
            high_bounds[1] = max(high_bounds[1], vertex[1])
            high_bounds[2] = max(high_bounds[2], vertex[2])
            continue
        if tokens[0] == 'vn' and not ignore_normal:
            if len(tokens) != 4:
                print('Error in read_obj_file: normals need 3 coordinates')
                sys.exit(1)
            normals.append([float(i) for i in tokens[1:4]])
            continue
        if tokens[0] == 'vt' and not ignore_uv:
            if len(tokens) < 3:
                print('Error in read_obj_file: texture coordinates need 2+ coordinates')
                sys.exit(1)
            texture_coords.append([float(i) for i in tokens[1:3]])
            continue
        if tokens[0] == 'f':
            if len(tokens) != 4:
                print('Error in read_obj_file: only triangular faces supported')
                sys.exit(1)
            test_list = tokens[1].split('/')
            if test_list[0]:
                triangles.append([int(i.split('/')[0]) - 1 for i in tokens[1:4]])
                if 'Kd' in current_material:
                    triangle_colours.append(current_material['Kd'])
            if len(test_list) > 1 and test_list[1] and not ignore_uv:
                triangle_textures.append([int(i.split('/')[1]) - 1 for i in tokens[1:4]])
            if len(test_list) > 2 and test_list[2] and not ignore_normal:
                triangle_normals.append([int(i.split('/')[2]) - 1 for i in tokens[1:4]])

    if verbose:
        print('vertices %d normals %d texture_coords %d triangles %d triangle_normals %d triangle_textures %d' % 
              (len(vertices), len(normals), len(texture_coords), len(triangles), len(triangle_normals), len(triangle_textures)))
    bounds = [low_bounds, high_bounds]
    return (vertices, normals, texture_coords, triangles, triangle_normals, triangle_textures, triangle_colours, bounds)

def read_obj_materials_file(filename, verbose):
    materials = {}
    current_material = {}
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
        if tokens[0] == 'newmtl':
            if len(tokens) != 2:
                print('Error in read_obj_file: newmtl needs 1 name, no spaces')
                sys.exit(1)
            current_material = {}
            current_material['name'] = tokens[1]
            materials[current_material['name']] = current_material
            continue
        if tokens[0] == 'Ns':
            if len(tokens) != 2:
                print('Error in read_obj_file: Ns needs 1 value')
                sys.exit(1)
            current_material['Ns'] = float(tokens[1])
            materials[current_material['name']] = current_material
            continue
        if tokens[0] == 'Ka':
            if len(tokens) != 4:
                print('Error in read_obj_file: Ka needs 3 values')
                sys.exit(1)
            current_material['Ka'] = [float(tokens[1]), float(tokens[2]), float(tokens[3])]
            materials[current_material['name']] = current_material
            continue
        if tokens[0] == 'Kd':
            if len(tokens) != 4:
                print('Error in read_obj_file: Kd needs 3 values')
                sys.exit(1)
            current_material['Kd'] = [float(tokens[1]), float(tokens[2]), float(tokens[3])]
            materials[current_material['name']] = current_material
            continue
        if tokens[0] == 'Ks':
            if len(tokens) != 4:
                print('Error in read_obj_file: Ks needs 3 values')
                sys.exit(1)
            current_material['Ks'] = [float(tokens[1]), float(tokens[2]), float(tokens[3])]
            materials[current_material['name']] = current_material
            continue
        if tokens[0] == 'Ke':
            if len(tokens) != 4:
                print('Error in read_obj_file: Ke needs 3 values')
                sys.exit(1)
            current_material['Ke'] = [float(tokens[1]), float(tokens[2]), float(tokens[3])]
            materials[current_material['name']] = current_material
            continue
        if tokens[0] == 'Ni':
            if len(tokens) != 2:
                print('Error in read_obj_file: Ni needs 1 value')
                sys.exit(1)
            current_material['Ni'] = float(tokens[1])
            materials[current_material['name']] = current_material
            continue
        if tokens[0] == 'd':
            if len(tokens) != 2:
                print('Error in read_obj_file: d needs 1 value')
                sys.exit(1)
            current_material['d'] = float(tokens[1])
            materials[current_material['name']] = current_material
            continue
        if tokens[0] == 'illum':
            if len(tokens) != 2:
                print('Error in read_obj_file: illum needs 1 value')
                sys.exit(1)
            current_material['illum'] = int(tokens[1])
            materials[current_material['name']] = current_material
            continue
    return materials
               
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
        if re.search("[^a-zA-Z0-9_\-.]", item):
            item = "\"" + item + "\""
        output_list.append(item)
    return output_list

        # program starts here
if __name__ == "__main__":
    convert_obj_to_tri()
