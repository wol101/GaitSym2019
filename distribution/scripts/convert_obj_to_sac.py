#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import argparse
import re
import xml.etree.ElementTree
import math
import copy

# globals
debug_flag = False

class position(object):
    __slots__ = ['x', 'y', 'z']

class moi(object):
    __slots__ = ['xx', 'yy', 'zz', 'xy', 'xz', 'yz']

def convert_obj_to_sac():

    parser = argparse.ArgumentParser(description="Try to convert an OBJ file into a FluidSac definition")
    parser.add_argument("-i", "--input_obj_file", required=True, help="the input OBJ file")
    parser.add_argument("-o", "--output_xml_file", required=True, help="the output GaitSym2019 XML config file")
    parser.add_argument("-t", "--type", required=True, help="type of fluid sac IdealGas or Incompressible")
    parser.add_argument("-md", "--min_distance", type=float, default=0.0001, help="the minimum distance between vertices [0.0001]")
    parser.add_argument("-bm", "--body_mass", type=float, default=0.001, help="the body mass to use for the created bodies [0.001]")
    parser.add_argument("-ts", "--time_step", type=float, default=0.0001, help="the integration time step [0.0001]")
    parser.add_argument("-mf", "--mesh_folder", default="meshes", help="the folder to use for the created meshes [meshes]")
    parser.add_argument("-b", "--create_bodies", action="store_true", help="create a body for each vertex")
    parser.add_argument("-g", "--create_global", action="store_true", help="create a global so the file is loadable")
    parser.add_argument("-s", "--create_springs", action="store_true", help="create the edge springs")
    parser.add_argument("-d", "--debug", action="store_true", help="turn on some debugging options")
    parser.add_argument("-f", "--force", action="store_true", help="force overwrite of destination file")
    parser.add_argument("-v", "--verbose", action="store_true", help="write out more information whilst processing")
    args = parser.parse_args()

    if args.verbose:
        pretty_print_sys_argv(sys.argv)
        pretty_print_argparse_args(args)

    global debug_flag    # Needed to modify global copy of debug_flag
    debug_flag = args.debug

    # preflight
    preflight_read_file(args.input_obj_file, args.verbose)
    preflight_write_file(args.output_xml_file, args.force, args.verbose)
    if args.create_bodies:
        preflight_write_folder(args.mesh_folder, args.verbose)

    permissable_types = ['IdealGas', 'Incompressible']
    if not args.type in permissable_types:
        print('Error: --types argument must be one of: %s' % (' '.join(permissable_types)))
        sys.exit(1)

    if args.verbose: print('Reading "%s"' % (args.input_obj_file))
    (vertices, triangles) = read_obj_file(args.input_obj_file, args.verbose)
    if args.verbose: print('Checking vertices')
    check_vertices(vertices, args.min_distance)
    if args.verbose: print('Checking mesh')
    edge_list = check_mesh(vertices, triangles)
    pruned_edge_list = prune_edge_list(edge_list)

    # create an empty GAITSYM tree
    new_tree = xml.etree.ElementTree.Element("GAITSYM2019")
    new_tree.text = "\n"
    new_tree.tail = "\n"

    if args.create_global:
        global_tag = create_global(args.mesh_folder, args.time_step)
        if args.verbose: print('Creating %s "%s"' % (global_tag.tag, global_tag.attrib["ID"]))
        new_tree.append(global_tag)

    # create the markers (and bodies if necessary)
    marker_names = []
    body_names = []
    if not args.create_bodies:
        for i in range(0, len(vertices)):
            vertex = vertices[i]
            # create the vertex markers
            marker_names.append("FluidSac_Marker_%d" % (i))
            marker = create_marker(marker_names[-1], vertex, "1 0 0 0", "World")
            if args.verbose: print('Creating %s "%s"' % (marker.tag, marker.attrib["ID"]))
            new_tree.append(marker)
    else:
        for i in range(0, len(vertices)):
            vertex = vertices[i]
            # create the vertex bodies
            body_names.append("FluidSac_Body_%d" % (i))
            body = create_body(body_names[-1], vertex, "1 0 0 0", Mass = args.body_mass, mesh_folder = args.mesh_folder)
            if args.verbose: print('Creating %s "%s"' % (body.tag, body.attrib["ID"]))
            new_tree.append(body)
            # create the vertex markers
            marker_names.append("FluidSac_Marker_%d" % (i))
            marker = create_marker(marker_names[-1], "0 0 0", "1 0 0 0", body_names[-1])
            if args.verbose: print('Creating %s "%s"' % (marker.tag, marker.attrib["ID"]))
            new_tree.append(marker)

    if args.create_springs:
        for edge in pruned_edge_list:
            name = "FluidSac_Edge_%d_%d" % (edge[0], edge[1])
            OriginMarkerID = "FluidSac_Marker_%d" % (edge[0])
            InsertionMarkerID = "FluidSac_Marker_%d" % (edge[1])
            edge_vector = Sub3x1(vertices[edge[1]], vertices[edge[0]])
            UnloadedLength = Magnitude3x1(edge_vector) * 0.5 # this means the edge can shorten
            if args.verbose: print(f'{UnloadedLength=}')
            SpringConstant = 1e5
            Damping = 1e1
            Area = 0.01
            (muscle, strap, driver) = create_spring(name, OriginMarkerID, InsertionMarkerID, UnloadedLength, SpringConstant, Damping, Area)
            if args.verbose: print('Creating %s "%s"' % (strap.tag, strap.attrib["ID"]))
            new_tree.append(strap)
            if args.verbose: print('Creating %s "%s"' % (muscle.tag, muscle.attrib["ID"]))
            new_tree.append(muscle)
            if args.verbose: print('Creating %s "%s"' % (driver.tag, driver.attrib["ID"]))
            new_tree.append(driver)

    # create the FluidSac
    fluid_sac = xml.etree.ElementTree.Element("FLUIDSAC")
    fluid_sac.tail = "\n"
    fluid_sac.attrib["ID"] = "FluidSac1"
    fluid_sac.attrib["NumMarkers"] = str(len(marker_names))
    fluid_sac.attrib["MarkerIDList"] = ' '.join(marker_names)
    fluid_sac.attrib["NumTriangles"] = str(len(triangles))
    if args.verbose:
        print(f'{fluid_sac.attrib["ID"]=}')
        print(f'{fluid_sac.attrib["NumMarkers"]=}')
        print(f'{fluid_sac.attrib["NumTriangles"]=}')
    flat_triangles = [item for sublist in triangles for item in sublist]
    fluid_sac.attrib["TriangleIndexList"] = ' '.join([str(x) for x in flat_triangles])
    (mass, cm, inertia) = calculate_mass_properties(vertices, flat_triangles, density = 1.0) # density of 1 so mass is the same as volume
    volume = mass
    if args.type == "IdealGas":
        fluid_sac.attrib["Type"] = "IdealGas"
        fluid_sac.attrib["AmountOfSubstance"] = format((101.325 * 1000 * volume) / (8.314 * 273.15 + 20), '.18g') # n = (P V) / (R T)
        fluid_sac.attrib["ExternalPressure"] = str(101.325 * 1000) # 101.325 kPa (NTP)
        fluid_sac.attrib["Temperature"] = str(273.15 + 20) # 20 degrees C in Kelvin (NTP)
        if args.verbose:
            print(f'{fluid_sac.attrib["Type"]=}')
            print(f'{fluid_sac.attrib["AmountOfSubstance"]=}')
            print(f'{fluid_sac.attrib["ExternalPressure"]=}')
            print(f'{fluid_sac.attrib["Temperature"]=}')
    if args.type == "Incompressible":
        fluid_sac.attrib["Type"] = "Incompressible"

        fluid_sac.attrib["FluidVolume"] = format(volume, '.18g') # the calculated volume
        fluid_sac.attrib["BulkModulus"] = str(2.2 * 1e9) # 2.2 GPa
        fluid_sac.attrib["BulkModulusDamping"] = str(2.2 * 1e9 * args.time_step * 0.01)  # needs to be tweaked to get a stable system
        fluid_sac.attrib["StartingPressure"] = "0.0" # zero probably makes more sense than NTP [str(101.325 * 1000) # 101.325 kPa (NTP)]
        if args.verbose:
            print(f'{fluid_sac.attrib["Type"]=}')
            print(f'{fluid_sac.attrib["FluidVolume"]=}')
            print(f'{fluid_sac.attrib["BulkModulus"]=}')
            print(f'{fluid_sac.attrib["BulkModulusDamping"]=}')
            print(f'{fluid_sac.attrib["StartingPressure"]=}')
    # decorations
    fluid_sac.attrib["Colour1"] = "128 128 128 255"
    fluid_sac.attrib["Colour2"] = "96 96 96 255"
    fluid_sac.attrib["Colour3"] = "64 64 64 255"
    fluid_sac.attrib["Size1"] = "0.01"
    fluid_sac.attrib["Size2"] = "0.01"
    fluid_sac.attrib["Size3"] = "0.01"
    new_tree.append(fluid_sac)

    if args.verbose: print('Writing "%s"' % (args.output_xml_file))
    with open(args.output_xml_file, "wb") as out_file:
        out_file.write(xml.etree.ElementTree.tostring(new_tree, encoding="utf-8", method="xml"))

def create_global(mesh_folder = "meshes", time_step = 0.0001):
    new_child = xml.etree.ElementTree.Element("GLOBAL")
    new_child.tail = "\n"
    new_child.attrib["AllowConnectedCollisions"] = "false"
    new_child.attrib["AllowInternalCollisions"] = "false"
    new_child.attrib["AngularDamping"] = "0"
    new_child.attrib["BMR"] = "0"
    new_child.attrib["CFM"] = "1.00000000000000004e-10"
    new_child.attrib["Colour1"] = "155 48 255 255"
    new_child.attrib["Colour2"] = "0 0 0 255"
    new_child.attrib["Colour3"] = "0 0 0 255"
    new_child.attrib["ContactMaxCorrectingVel"] = "100"
    new_child.attrib["ContactSurfaceLayer"] = "0.00100000000000000002"
    new_child.attrib["CurrentWarehouse"] = ""
    new_child.attrib["DistanceTravelledBodyID"] = "Trunk"
    new_child.attrib["ERP"] = "0.200000000000000011"
    new_child.attrib["FitnessType"] = "KinematicMatch"
    new_child.attrib["GravityVector"] = "0 0 0" # "0 0 -9.8100000000000005"
    new_child.attrib["Group"] = ""
    new_child.attrib["ID"] = "global"
    new_child.attrib["IntegrationStepSize"] = convert_to_string(time_step)
    new_child.attrib["LinearDamping"] = "0"
    new_child.attrib["MechanicalEnergyLimit"] = "0"
    new_child.attrib["MeshSearchPath"] = ".:%s" % (mesh_folder)
    new_child.attrib["MetabolicEnergyLimit"] = "0"
    new_child.attrib["NumericalErrorsScore"] = "0"
    new_child.attrib["PermittedNumericalErrors"] = "-1"
    new_child.attrib["Size1"] = "1"
    new_child.attrib["Size2"] = "0"
    new_child.attrib["Size3"] = "0"
    new_child.attrib["StepType"] = "World"
    new_child.attrib["TimeLimit"] = "10"
    new_child.attrib["WarehouseDecreaseThresholdFactor"] = "0.5"
    new_child.attrib["WarehouseFailDistanceAbort"] = "0.5"
    new_child.attrib["WarehouseUnitIncreaseDistanceThreshold"] = "0.5"
    return new_child

def create_spring(ID, OriginMarkerID, InsertionMarkerID, UnloadedLength, SpringConstant, Damping, Area):
    new_child = xml.etree.ElementTree.Element("MUSCLE")
    new_child.tail = "\n"
    new_child.attrib["ID"] = convert_to_string(ID)
    new_child.attrib["UnloadedLength"] = convert_to_string(UnloadedLength)
    new_child.attrib["SpringConstant"] = convert_to_string(SpringConstant)
    new_child.attrib["Damping"] = convert_to_string(Damping)
    new_child.attrib["Area"] = convert_to_string(Area)
    new_child.attrib["StrapID"] = convert_to_string(ID) + "_strap"
    new_child.attrib["Type"] = "DampedSpring"
    new_child.attrib["BreakingStrain"] = "-1"
    # decorations
    new_child.attrib["Colour1"] = "255 255 0 255"
    new_child.attrib["Colour2"] = "255 127 0 255"
    new_child.attrib["Colour3"] = "255 63 0 255"
    new_child.attrib["Size1"] = "0.01"
    new_child.attrib["Size2"] = "0.01"
    new_child.attrib["Size3"] = "0.01"

    new_child2 = xml.etree.ElementTree.Element("STRAP")
    new_child2.tail = "\n"
    new_child2.attrib["Group"] = ""
    new_child2.attrib["ID"] = new_child.attrib["StrapID"]
    new_child2.attrib["InsertionMarkerID"] = convert_to_string(InsertionMarkerID)
    new_child2.attrib["Length"] = convert_to_string(UnloadedLength)
    new_child2.attrib["OriginMarkerID"] = convert_to_string(OriginMarkerID)
    new_child2.attrib["TorqueMarkerIDList"] = ""
    new_child2.attrib["Type"] = "TwoPoint"
    # decorations
    new_child2.attrib["Colour1"] = "0 238 238 255"
    new_child2.attrib["Colour2"] = "0 205 205 255"
    new_child2.attrib["Colour3"] = "0 0 0 255"
    new_child2.attrib["Size1"] = "0.0100000000000000002"
    new_child2.attrib["Size2"] = "0.100000000000000006"
    new_child2.attrib["Size3"] = "0"

    new_child3 = xml.etree.ElementTree.Element("DRIVER")
    new_child3.tail = "\n"
    new_child3.attrib["DriverRange"] = "0 1"
    new_child3.attrib["Durations"] = "1 1 1 1 1"
    new_child3.attrib["ID"] = convert_to_string(ID) + "_driver"
    new_child3.attrib["LinearInterpolation"] = "false"
    new_child3.attrib["Steps"] = "6"
    new_child3.attrib["TargetIDList"] = convert_to_string(ID)
    new_child3.attrib["Type"] = "Step"
    new_child3.attrib["Values"] = "1 1 1 1 1"

    return (new_child, new_child2, new_child3)


def create_marker(ID, Position, Quaternion, BodyID):
    new_child = xml.etree.ElementTree.Element("MARKER")
    new_child.tail = "\n"
    new_child.attrib["ID"] = convert_to_string(ID)
    new_child.attrib["Position"] = '%s %s' % (convert_to_string(BodyID), convert_to_string(Position))
    new_child.attrib["Quaternion"] = '%s %s' % (convert_to_string(BodyID), convert_to_string(Quaternion))
    new_child.attrib["BodyID"] = convert_to_string(BodyID)
    # decorations
    new_child.attrib["Colour1"] = "255 255 0 255"
    new_child.attrib["Colour2"] = "255 127 0 255"
    new_child.attrib["Colour3"] = "255 63 0 255"
    new_child.attrib["Size1"] = "0.01"
    new_child.attrib["Size2"] = "0.01"
    new_child.attrib["Size3"] = "0.01"
    return new_child

def create_body(ID, Position, Quaternion, Mass = 1.0, Density = 1000.0, mesh_folder = "meshes"):
    new_child = xml.etree.ElementTree.Element("BODY")
    new_child.tail = "\n"
    # variable values
    new_child.attrib["ID"] = convert_to_string(ID)
    new_child.attrib["ConstructionPosition"] = convert_to_string(Position)
    new_child.attrib["Position"] = convert_to_string(Position)
    new_child.attrib["Quaternion"] = convert_to_string(Quaternion)
    new_child.attrib["ConstructionDensity"] = convert_to_string(Density)
    mesh = [ [] for _ in range(3) ]
    rx = 1; ry = 1; rz = 1; stacks = 16; slices = 16
    create_elipsoid(rx, ry, rz, stacks, slices, mesh)
    vertices = mesh[0]
    triangles = mesh[2]
    # write_obj_file('test.obj', vertices, triangles, False)
    flat_triangles = [item for sublist in triangles for item in sublist]
    (mass, cm, inertia) = calculate_mass_properties(vertices, flat_triangles, density = Density)
    # print(f'{Mass=} {mass=}')
    radius_multiplier = (Mass / mass) ** (1.0/3)
    # print(f'{radius_multiplier=}')
    new_vertices = []
    for i in range(0, len(vertices)):
        new_vertices.append([vertices[i][0] * radius_multiplier + Position[0], vertices[i][1] * radius_multiplier + Position[1], vertices[i][2] * radius_multiplier + Position[2]])
    (mass, cm, inertia) = calculate_mass_properties(new_vertices, flat_triangles, density = Density)
    # print(f'{Mass=} {mass=}')
    obj_file_name = convert_to_string(ID) + '_mesh.obj'
    new_child.attrib["GraphicFile1"] = obj_file_name
    write_obj_file(os.path.join(mesh_folder, obj_file_name), new_vertices, triangles, False)
    new_child.attrib["Mass"] = convert_to_string(mass)
    new_child.attrib["MOI"] = "%.18g %.18g %.18g %.18g %.18g %.18g" % (inertia.xx, inertia.yy, inertia.zz, inertia.xy, inertia.xz, inertia.yz)

    # fixed values
    new_child.attrib["AngularVelocity"] = "0 0 0"
    new_child.attrib["AngularVelocityHighBound"] = "1.79769313486231571e+308 1.79769313486231571e+308 1.79769313486231571e+308"
    new_child.attrib["AngularVelocityLowBound"] = "-1.79769313486231571e+308 -1.79769313486231571e+308 -1.79769313486231571e+308"
    new_child.attrib["ConstructionDensity"] = "1000"
    new_child.attrib["DragControl"] = "NoDrag"
    new_child.attrib["GraphicFile2"] = ""
    new_child.attrib["GraphicFile3"] = ""
    new_child.attrib["LinearVelocity"] = "0 0 0"
    new_child.attrib["LinearVelocityHighBound"] = "1.79769313486231571e+308 1.79769313486231571e+308 1.79769313486231571e+308"
    new_child.attrib["LinearVelocityLowBound"] = "-1.79769313486231571e+308 -1.79769313486231571e+308 -1.79769313486231571e+308"
    new_child.attrib["PositionHighBound"] = "1.79769313486231571e+308 1.79769313486231571e+308 1.79769313486231571e+308"
    new_child.attrib["PositionLowBound"] = "-1.79769313486231571e+308 -1.79769313486231571e+308 -1.79769313486231571e+308"
    # decorations
    new_child.attrib["Colour1"] = "0 255 0 255"
    new_child.attrib["Colour2"] = "0 238 0 255"
    new_child.attrib["Colour3"] = "0 205 0 255"
    new_child.attrib["Size1"] = "0.1"
    new_child.attrib["Size2"] = "0.1"
    new_child.attrib["Size3"] = "0.1"
    return new_child


def convert_to_string(input_object):
    if isinstance(input_object, str):
        return str(input_object)
    if isinstance(input_object, bool):
        return str(input_object)
    if isinstance(input_object, int):
        return str(input_object)
    if isinstance(input_object, float):
        return format(input_object, '.18g')
    if isinstance(input_object, list):
        output_list = []
        for element in input_object:
            output_list.append(convert_to_string(element))
        return ' '.join(output_list)
    print('Type "%s" cannot be converted' % (type(input_object)))
    sys.exit(1)
    return ""

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
            if debug_flag:
                new_triangles = [[0, 1, 2]]
                new_vertices = []
                new_vertices.append(vertices[triangles[edge[2]][0]])
                new_vertices.append(vertices[triangles[edge[2]][1]])
                new_vertices.append(vertices[triangles[edge[2]][2]])
                write_obj_file('debug1.obj', new_vertices, new_triangles, verbose = True)
                new_vertices = []
                new_vertices.append(vertices[triangles[match[2]][0]])
                new_vertices.append(vertices[triangles[match[2]][1]])
                new_vertices.append(vertices[triangles[match[2]][2]])
                write_obj_file('debug2.obj', new_vertices, new_triangles, verbose = True)
            sys.exit(1)
    return edgeList

def write_obj_file(filename, vertices, triangles, verbose):
    if verbose:
        print('Writing "%s"' % (filename))
    with open(filename, 'w', newline='\n') as f_out:
        f_out.write('# Generated by convert_obj_to_sac.py\n')
        f_out.write('# Vertices: %d\n' % (len(vertices)))
        f_out.write('# Faces: %d\n' % (len(triangles)))
        for i in range(0, len(vertices)):
            v = vertices[i]
            f_out.write('v %.10f %.10f %.10f\n' % (v[0], v[1], v[2]));
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

def prune_edge_list(edge_list):
    pruned_edge_list = []
    for edge in edge_list:
        match = False
        for pruned_edge in pruned_edge_list:
            if pruned_edge[1] == edge[0] and pruned_edge[0] == edge[1]:
                match = True
                break
            if pruned_edge[0] == edge[0] and pruned_edge[1] == edge[1]:
                match = True
                break
        if match == True:
            continue
        pruned_edge_list.append(edge)
    return pruned_edge_list

def calculate_mass_properties_sub(w0,w1,w2):
    temp0 = w0+w1; f1 = temp0+w2; temp1 = w0*w0; temp2 = temp1+w1*temp0
    f2 = temp2+w2*f1; f3 = w0*temp1+w1*temp2+w2*f2
    g0 = f2+w0*(f1+w0); g1 = f2+w1*(f1+w1); g2 = f2+w2*(f1+w2)
    return (f1,f2,f3,g0,g1,g2)

def calculate_mass_properties(p, index, density):
    mult = [1./6,1./24,1./24,1./24,1./60,1./60,1./60,1./120,1./120,1./120]
    intg = [0.,0.,0.,0.,0.,0.,0.,0.,0.,0.] # order: 1, x, y, z, x^2, y^2, z^2, xy, yz, zx
    if len(index) % 3 != 0:
        print('Error: triangle vertex list length must be a multiple of 3')
        sys.exit(1)
    tmax = int(len(index) / 3)
    for t in range(0, tmax):
        # get vertices of triangle t
        i0 = index[3*t]; i1 = index[3*t+1]; i2 = index[3*t+2];
        x0 = p[i0][0]; y0 = p[i0][1]; z0 = p[i0][2];
        x1 = p[i1][0]; y1 = p[i1][1]; z1 = p[i1][2];
        x2 = p[i2][0]; y2 = p[i2][1]; z2 = p[i2][2];
        # get edges and cross product of edges
        a1 = x1-x0; b1 = y1-y0; c1 = z1-z0; a2 = x2-x0; b2 = y2-y0; c2 = z2-z0;
        d0 = b1*c2-b2*c1; d1 = a2*c1-a1*c2; d2 = a1*b2-a2*b1;
        # compute integral terms
        (f1x,f2x,f3x,g0x,g1x,g2x) = calculate_mass_properties_sub(x0,x1,x2);
        (f1y,f2y,f3y,g0y,g1y,g2y) = calculate_mass_properties_sub(y0,y1,y2);
        (f1z,f2z,f3z,g0z,g1z,g2z) = calculate_mass_properties_sub(z0,z1,z2);
        # update integrals
        intg[0] += d0*f1x;
        intg[1] += d0*f2x; intg[2] += d1*f2y; intg[3] += d2*f2z;
        intg[4] += d0*f3x; intg[5] += d1*f3y; intg[6] += d2*f3z;
        intg[7] += d0*(y0*g0x+y1*g1x+y2*g2x);
        intg[8] += d1*(z0*g0y+z1*g1y+z2*g2y);
        intg[9] += d2*(x0*g0z+x1*g1z+x2*g2z);

    for i in range(0, 10):
        intg[i] *= mult[i];
    mass = intg[0];
    # center of mass
    cm = position()
    cm.x = intg[1]/mass;
    cm.y = intg[2]/mass;
    cm.z = intg[3]/mass;
    # inertia tensor relative to center of mass
    inertia = moi()
    inertia.xx = intg[5]+intg[6]-mass*(cm.y*cm.y+cm.z*cm.z);
    inertia.yy = intg[4]+intg[6]-mass*(cm.z*cm.z+cm.x*cm.x);
    inertia.zz = intg[4]+intg[5]-mass*(cm.x*cm.x+cm.y*cm.y);
    inertia.xy = -(intg[7]-mass*cm.x*cm.y);
    inertia.yz = -(intg[8]-mass*cm.y*cm.z);
    inertia.xz = -(intg[9]-mass*cm.z*cm.x);

    # to accommodate density D, then you need to multiply the output mass by D and the output inertia tensor by D
    mass *= density
    inertia.xx *= density
    inertia.yy *= density
    inertia.zz *= density
    inertia.xy *= density
    inertia.yz *= density
    inertia.xz *= density

    return (mass, cm, inertia)

def create_elipsoid(rx, ry, rz, stacks, slices, mesh):
    # print('elipsoid rx = %f ry = %f rz = %f' % (rx, ry, rz))
    for t in range(0, stacks): # stacks are ELEVATION so they count zenith
        theta1 = ( float(t)/stacks )*math.pi
        theta2 = ( float(t+1)/stacks )*math.pi

        for p in range(0, slices) :# slices are ORANGE SLICES so the count azimuth
            phi1 = ( float(p)/slices )*2*math.pi # azimuth goes around 0 .. 2*PI
            phi2 = ( float(p+1)/slices )*2*math.pi

            # phi2   phi1
            #  |      |
            #  2------1 -- theta1
            #  |\ _   |
            #  |    \ |
            #  3------4 -- theta2

            #vertex1 = vertex on a sphere of radius r at spherical coords theta1, phi1
            #vertex2 = vertex on a sphere of radius r at spherical coords theta1, phi2
            #vertex3 = vertex on a sphere of radius r at spherical coords theta2, phi2
            #vertex4 = vertex on a sphere of radius r at spherical coords theta2, phi1
            #
            # x = rx sin(zenith) cos(azimuth)
            # y = ry sin(zenith) sin(azimuth)
            # z = rz cos(zenith)

            vertex1 = [rx * math.sin(theta1) * math.cos(phi1), ry * math.sin(theta1) * math.sin(phi1), rz * math.cos(theta1)]
            vertex2 = [rx * math.sin(theta1) * math.cos(phi2), ry * math.sin(theta1) * math.sin(phi2), rz * math.cos(theta1)]
            vertex3 = [rx * math.sin(theta2) * math.cos(phi2), ry * math.sin(theta2) * math.sin(phi2), rz * math.cos(theta2)]
            vertex4 = [rx * math.sin(theta2) * math.cos(phi1), ry * math.sin(theta2) * math.sin(phi1), rz * math.cos(theta2)]

            norm1 = Normalise3x1(vertex1)
            norm2 = Normalise3x1(vertex2)
            norm3 = Normalise3x1(vertex3)
            norm4 = Normalise3x1(vertex4)

            # facing out
            if( t == 0 ): # top cap
                add_tri(vertex1, vertex3, vertex4, norm1, norm3, norm4, True, mesh) # t1p1, t2p2, t2p1
            else:
                if ( t + 1 == stacks ): #end cap
                    add_tri(vertex3, vertex1, vertex2, norm3, norm1, norm2, True, mesh)  #t2p2, t1p1, t1p2
                else:
                    # body, facing OUT:
                    add_tri(vertex1, vertex2, vertex4, norm1, norm2, norm4, True, mesh)
                    add_tri(vertex2, vertex3, vertex4, norm2, norm3, norm4, True, mesh)

def create_cylinder(l, r, sides, mesh):
    print('cylinder l = %f r = %f' % (l, r))
    vertexList = []
    vertex = [0, 0, -l/2.0]
    vertexList.append(copy.copy(vertex))
    vertex[2] = l + vertex[2]
    vertexList.append(copy.copy(vertex))

    theta = 2.0 * math.pi / float(sides)
    vertex[2] = -l/2.0;
    for i in range(0, sides):
        vertex[0] = r * math.cos(theta * float(i))
        vertex[1] = r * math.sin(theta * float(i))
        vertexList.append(copy.copy(vertex))
    vertex[2] = l/2.0;
    for i in range(0, sides):
        vertex[0] = vertexList[i + 2][0];
        vertex[1] = vertexList[i + 2][1]
        vertexList.append(copy.copy(vertex))

    # first end cap
    norm1 = [0.0, 0.0, -1.0]
    norm2 = norm1
    norm3 = norm1
    for i in range(0, sides):
        triangle = [0.0] * 9
        if (i < sides - 1):
            triangle[0] = vertexList[0][0];
            triangle[1] = vertexList[0][1];
            triangle[2] = vertexList[0][2];
            triangle[3] = vertexList[i + 2][0];
            triangle[4] = vertexList[i + 2][1];
            triangle[5] = vertexList[i + 2][2];
            triangle[6] = vertexList[i + 3][0];
            triangle[7] = vertexList[i + 3][1];
            triangle[8] = vertexList[i + 3][2];
        else:
            triangle[0] = vertexList[0][0];
            triangle[1] = vertexList[0][1];
            triangle[2] = vertexList[0][2];
            triangle[3] = vertexList[i + 2][0];
            triangle[4] = vertexList[i + 2][1];
            triangle[5] = vertexList[i + 2][2];
            triangle[6] = vertexList[2][0];
            triangle[7] = vertexList[2][1];
            triangle[8] = vertexList[2][2];
        add_tri(triangle[0:3], triangle[3:6], triangle[6:9], norm1, norm2, norm3, True, mesh)

    # sides
    for i in range(0, sides):
        if (i < sides - 1):
            triangle = [0.0] * 9
            triangle[0] = vertexList[i + 2][0];
            triangle[1] = vertexList[i + 2][1];
            triangle[2] = vertexList[i + 2][2];
            triangle[3] = vertexList[i + 2 + sides][0];
            triangle[4] = vertexList[i + 2 + sides][1];
            triangle[5] = vertexList[i + 2 + sides][2];
            triangle[6] = vertexList[i + 3 + sides][0];
            triangle[7] = vertexList[i + 3 + sides][1];
            triangle[8] = vertexList[i + 3 + sides][2];
            add_tri(triangle[0:3], triangle[3:6], triangle[6:9], Normalise3x1([triangle[0], triangle[1], 0]),
                    Normalise3x1([triangle[3], triangle[4], 0]), Normalise3x1([triangle[6], triangle[7], 0]), True, mesh)
            triangle = [0.0] * 9
            triangle[0] = vertexList[i + 3 + sides][0];
            triangle[1] = vertexList[i + 3 + sides][1];
            triangle[2] = vertexList[i + 3 + sides][2];
            triangle[3] = vertexList[i + 3][0];
            triangle[4] = vertexList[i + 3][1];
            triangle[5] = vertexList[i + 3][2];
            triangle[6] = vertexList[i + 2][0];
            triangle[7] = vertexList[i + 2][1];
            triangle[8] = vertexList[i + 2][2];
            add_tri(triangle[0:3], triangle[3:6], triangle[6:9], Normalise3x1([triangle[0], triangle[1], 0]),
                    Normalise3x1([triangle[3], triangle[4], 0]), Normalise3x1([triangle[6], triangle[7], 0]), True, mesh)
        else:
            triangle = [0.0] * 9
            triangle[0] = vertexList[i + 2][0];
            triangle[1] = vertexList[i + 2][1];
            triangle[2] = vertexList[i + 2][2];
            triangle[3] = vertexList[i + 2 + sides][0];
            triangle[4] = vertexList[i + 2 + sides][1];
            triangle[5] = vertexList[i + 2 + sides][2];
            triangle[6] = vertexList[2 + sides][0];
            triangle[7] = vertexList[2 + sides][1];
            triangle[8] = vertexList[2 + sides][2];
            add_tri(triangle[0:3], triangle[3:6], triangle[6:9], Normalise3x1([triangle[0], triangle[1], 0]),
                    Normalise3x1([triangle[3], triangle[4], 0]), Normalise3x1([triangle[6], triangle[7], 0]), True, mesh)
            triangle = [0.0] * 9
            triangle[0] = vertexList[2 + sides][0];
            triangle[1] = vertexList[2 + sides][1];
            triangle[2] = vertexList[2 + sides][2];
            triangle[3] = vertexList[2][0];
            triangle[4] = vertexList[2][1];
            triangle[5] = vertexList[2][2];
            triangle[6] = vertexList[i + 2][0];
            triangle[7] = vertexList[i + 2][1];
            triangle[8] = vertexList[i + 2][2];
            add_tri(triangle[0:3], triangle[3:6], triangle[6:9], Normalise3x1([triangle[0], triangle[1], 0]),
                    Normalise3x1([triangle[3], triangle[4], 0]), Normalise3x1([triangle[6], triangle[7], 0]), True, mesh)

    # final end cap
    norm1 = [0.0, 0.0, 1.0]
    norm2 = norm1
    norm3 = norm1
    for i in range(0, sides):
        triangle = [0.0] * 9
        if (i < sides - 1):
            triangle[0] = vertexList[1][0];
            triangle[1] = vertexList[1][1];
            triangle[2] = vertexList[1][2];
            triangle[3] = vertexList[i + 3 + sides][0];
            triangle[4] = vertexList[i + 3 + sides][1];
            triangle[5] = vertexList[i + 3 + sides][2];
            triangle[6] = vertexList[i + 2 + sides][0];
            triangle[7] = vertexList[i + 2 + sides][1];
            triangle[8] = vertexList[i + 2 + sides][2];
        else:
            triangle[0] = vertexList[1][0];
            triangle[1] = vertexList[1][1];
            triangle[2] = vertexList[1][2];
            triangle[3] = vertexList[2 + sides][0];
            triangle[4] = vertexList[2 + sides][1];
            triangle[5] = vertexList[2 + sides][2];
            triangle[6] = vertexList[i + 2 + sides][0];
            triangle[7] = vertexList[i + 2 + sides][1];
            triangle[8] = vertexList[i + 2 + sides][2];
        add_tri(triangle[0:3], triangle[3:6], triangle[6:9], norm1, norm2, norm3, True, mesh)
    return

def add_tri(v1, v2, v3, n1, n2, n3, reverse, mesh):
    start_index = len(mesh[0])
    mesh[0] = mesh[0] + [v1, v2, v3]
    mesh[1] = mesh[1] + [n1, n2, n3]
    if reverse:
        mesh[2].append([start_index + 2, start_index + 1, start_index])
    else:
        mesh[2].append([start_index, start_index + 1, start_index + 2])
    return

def rotate_mesh(xr, yr, zr, mesh):
    print('Rotate mesh xyz = %f, %f, %f' % (xr, yr, zr))
    xq = QuaternionFromAxisAngle([1,0,0], xr)
    yq = QuaternionFromAxisAngle([0,1,0], yr)
    zq = QuaternionFromAxisAngle([0,0,1], zr)
    for i_vertex in range(0, len(mesh[0])):
        mesh[0][i_vertex] = QuaternionVectorRotate(xq, mesh[0][i_vertex])
        mesh[0][i_vertex] = QuaternionVectorRotate(yq, mesh[0][i_vertex])
        mesh[0][i_vertex] = QuaternionVectorRotate(zq, mesh[0][i_vertex])
    for i_normal in range(0, len(mesh[1])):
        mesh[1][i_normal] = QuaternionVectorRotate(xq, mesh[1][i_normal])
        mesh[1][i_normal] = QuaternionVectorRotate(yq, mesh[1][i_normal])
        mesh[1][i_normal] = QuaternionVectorRotate(zq, mesh[1][i_normal])
    return

def translate_mesh(xd, yd, zd, mesh):
    print('Translate mesh xyz = %f, %f, %f' % (xd, yd, zd))
    for i_vertex in range(0, len(mesh[0])):
        mesh[0][i_vertex][0] += xd
        mesh[0][i_vertex][1] += yd
        mesh[0][i_vertex][2] += zd
    return

# add two vectors
def Add3x1(a, b):
    c = [0, 0, 0]
    c[0] = a[0] + b[0]
    c[1] = a[1] + b[1]
    c[2] = a[2] + b[2]
    return c

# subtract two vectors
def Sub3x1(a, b):
    c = [0, 0, 0]
    c[0] = a[0] - b[0]
    c[1] = a[1] - b[1]
    c[2] = a[2] - b[2]
    return c

# calculate cross product (vector product)
def CrossProduct3x1(a, b):
    c = [0, 0, 0]
    c[0] = a[1] * b[2] - a[2] * b[1]
    c[1] = a[2] * b[0] - a[0] * b[2]
    c[2] = a[0] * b[1] - a[1] * b[0]
    return c

# calculate dot product (scalar product)
def DotProduct3x1(a, b):

    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]

# calculate length of vector
def Magnitude3x1(a):

    return math.sqrt((a[0]*a[0]) + (a[1]*a[1]) + (a[2]*a[2]))

# reverse a vector
def Reverse3x1(a):

    return [-a[0], -a[1], -a[2]]

def Normalise3x1(a):
    s = Magnitude3x1(a)
    c = [0, 0, 0]
    c[0] = a[0] / s
    c[1] = a[1] / s
    c[2] = a[2] / s
    return c

# calculate length of vector
def Magnitude4x1(a):

    return math.sqrt((a[0]*a[0]) + (a[1]*a[1]) + (a[2]*a[2]) + (a[3]*a[3]))

def Normalise4x1(a):
    s = Magnitude4x1(a)
    c = [0, 0, 0, 0]
    c[0] = a[0] / s
    c[1] = a[1] / s
    c[2] = a[2] / s
    c[3] = a[3] / s
    return c

def QuaternionFromAxisAngle(axis, angle):

    v = Normalise3x1(axis)

    sin_a = math.sin( angle / 2 )
    cos_a = math.cos( angle / 2 )

    q = [0, 0, 0, 0]
    q[1]    = v[0] * sin_a
    q[2]    = v[1] * sin_a
    q[3]    = v[2] * sin_a
    q[0]    = cos_a

    return q

def QuaternionGetAngle(q):

    if (q[0] <= -1):
        return 0 # 2 * pi
    if (q[0] >= 1):
        return 0 # 2 * 0

    return (2*math.acos(q[0]))


def QuaternionGetAxis(q):

    v = [0, 0, 0]
    v[0] = q[1]
    v[1] = q[2]
    v[2] = q[3]
    s = Magnitude3x1(v)
    if (s <= 0): # special case for zero rotation - set arbitrary axis
        v = [1, 0, 0]
    else:
        v[0] = v[0] / s
        v[1] = v[1] / s
        v[2] = v[2] / s
    return v

def GetRotationTo(v0, v1):
    # Based on Stan Melax's article in Game Programming Gems
    #vector a = crossproduct(v1, v2)
    #q.xyz = a
    #q.w = sqrt((v1.Length ^ 2) * (v2.Length ^ 2)) + dotproduct(v1, v2)

    v0 = Normalise3x1(v0)
    v1 = Normalise3x1(v1)

    d = DotProduct3x1(v0, v1)
    # If dot == 1, vectors are the same
    if (d >= 1.0):
        return [1, 0, 0, 0] # identity quaternion

    if (d < (1e-7 - 1.0)):
        # Generate an axis
        axis = CrossProduct3x1([1, 0, 0], v0)
        if (Magnitude3x1(axis) < 1e-6 and Magnitude3x1(axis) > -1e-6): # pick another if colinear
            axis = CrossProduct3x1([0, 1, 0], v0)
        axis= Normalise3x1(axis)
        q = QuaternionFromAxisAngle(axis, math.pi)

    else:

        s = math.sqrt( (1+d)*2 )
        invs = 1 / s

        c = CrossProduct3x1(v0, v1)

        q = [0, 0, 0, 0]
        q[1] = c[0] * invs
        q[2] = c[1] * invs
        q[3] = c[2] * invs
        q[0] = s * 0.5
        q = Normalise4x1(q)

    return q

def QuaternionVectorRotate(q, v):

    QwQx = q[0] * q[1]
    QwQy = q[0] * q[2]
    QwQz = q[0] * q[3]
    QxQy = q[1] * q[2]
    QxQz = q[1] * q[3]
    QyQz = q[2] * q[3]

    rx = 2* (v[1] * (-QwQz + QxQy) + v[2] *( QwQy + QxQz))
    ry = 2* (v[0] * ( QwQz + QxQy) + v[2] *(-QwQx + QyQz))
    rz = 2* (v[0] * (-QwQy + QxQz) + v[1] *( QwQx + QyQz))

    QwQw = q[0] * q[0]
    QxQx = q[1] * q[1]
    QyQy = q[2] * q[2]
    QzQz = q[3] * q[3]

    rx += v[0] * (QwQw + QxQx - QyQy - QzQz)
    ry += v[1] * (QwQw - QxQx + QyQy - QzQz)
    rz += v[2] * (QwQw - QxQx - QyQy + QzQz)

    return [rx, ry, rz]

def QuaternionQuaternionMultiply(q1, q2):
    return  [ q1[0]*q2[0] - q1[1]*q2[1] - q1[2]*q2[2] - q1[3]*q2[3],
              q1[0]*q2[1] + q1[1]*q2[0] + q1[2]*q2[3] - q1[3]*q2[2],
              q1[0]*q2[2] + q1[2]*q2[0] + q1[3]*q2[1] - q1[1]*q2[3],
              q1[0]*q2[3] + q1[3]*q2[0] + q1[1]*q2[2] - q1[2]*q2[1]]

class matrix:
    def __init__(self):
        self.e11 = 0
        self.e12 = 0
        self.e13 = 0
        self.e21 = 0
        self.e22 = 0
        self.e23 = 0
        self.e31 = 0
        self.e32 = 0
        self.e33 = 0

def QuaternionFromMatrix(R):
    # quaternion is [w, x, y, z]
    # matrix is:
    # R.e11 R.e12 R.e13
    # R.e21 R.e22 R.e23
    # R.e31 R.e32 R.e33
    q = [0,0,0,0]
    tr = R.e11 + R.e22 + R.e33
    if (tr >= 0):
        s = math.sqrt (tr + 1)
        q[0] = 0.5 * s
        s = 0.5 * (1.0/s)
        q[1] = (R.e32 - R.e23) * s
        q[2] = (R.e13 - R.e31) * s
        q[3] = (R.e21 - R.e12) * s
        return q
    else:
        # find the largest diagonal element and jump to the appropriate case
        if (R.e22 > R.e11):
            if (R.e33 > R.e22):
                s = math.sqrt((R.e33 - (R.e11 + R.e22)) + 1)
                q[3] = 0.5 * s
                s = 0.5 * (1.0/s)
                q[1] = (R.e31 + R.e13) * s
                q[2] = (R.e23 + R.e32) * s
                q[0] = (R.e21 - R.e12) * s
                return q
            s = math.sqrt((R.e22 - (R.e33 + R.e11)) + 1)
            q[2] = 0.5 * s
            s = 0.5 * (1.0/s)
            q[3] = (R.e23 + R.e32) * s
            q[1] = (R.e12 + R.e21) * s
            q[0] = (R.e13 - R.e31) * s
            return q
        if (R.e33 > R.e11):
            s = math.sqrt((R.e33 - (R.e11 + R.e22)) + 1)
            q[3] = 0.5 * s
            s = 0.5 * (1.0/s)
            q[1] = (R.e31 + R.e13) * s
            q[2] = (R.e23 + R.e32) * s
            q[0] = (R.e21 - R.e12) * s
            return q
        s = math.sqrt((R.e11 - (R.e22 + R.e33)) + 1)
        q[1] = 0.5 * s
        s = 0.5 * (1.0/s)
        q[2] = (R.e12 + R.e21) * s
        q[3] = (R.e31 + R.e13) * s
        q[0] = (R.e32 - R.e23) * s
        return q

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
if __name__ == "__main__":
    convert_obj_to_sac()
