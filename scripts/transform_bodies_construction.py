#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import argparse
import re
import xml.etree.ElementTree
import math

def transform_bodies_construction():

    parser = argparse.ArgumentParser(description="Transform the bodies in a GaitSym file")
    parser.add_argument("-i", "--input_xml_file", required=True, help="input GaitSym XML config file")
    parser.add_argument("-o", "--output_xml_file", required=True, help="output GaitSym XML config file")
    parser.add_argument("-t", "--translation", nargs=3, type=float, default=[0.0, 0.0, 0.0], help="translation vector x y z [0, 0, 0])")
    parser.add_argument("-rc", "--rotation_centre", nargs=3, type=float, default=[0.0, 0.0, 0.0], help="rotation centre x y z [0, 0, 0])")
    parser.add_argument("-r1", "--rotation_angle_axis_1", nargs=4, type=float, default=[0.0, 1.0, 0.0, 0.0], help="rotation angle axis r x y z degrees [0, 1, 0, 0])")
    parser.add_argument("-r2", "--rotation_angle_axis_2", nargs=4, type=float, default=[0.0, 1.0, 0.0, 0.0], help="rotation angle axis r x y z degrees [0, 1, 0, 0])")
    parser.add_argument("-r3", "--rotation_angle_axis_3", nargs=4, type=float, default=[0.0, 1.0, 0.0, 0.0], help="rotation angle axis r x y z degrees [0, 1, 0, 0])")
    parser.add_argument("-x", "--set_x_position", type=float, help="Set the x position value to this")
    parser.add_argument("-y", "--set_y_position", type=float, help="Set the y position value to this")
    parser.add_argument("-z", "--set_z_position", type=float, help="Set the z position value to this")
    parser.add_argument("-b", "--bodies_list", nargs='*', type=str, default=[], help="only process these bodies if defined []")
    parser.add_argument("-ig", "--input_graphics_folder", default='.', help="location of the input OBJ files [.]")
    parser.add_argument("-og", "--output_graphics_folder", default='converted', help="location of the output OBJ files [converted]")
    parser.add_argument("-zs", "--zero_start_poses", action="store_true", help="zero the start poses of all bodies in the output XML")
    parser.add_argument("-rp", "--retain_graphics_path", action="store_true", help="keep the path information in GraphicFile* attributes")
    parser.add_argument("-f", "--force", action="store_true", help="force overwrite of destination file")
    parser.add_argument("-v", "--verbose", action="store_true", help="write out more information whilst processing")
    args = parser.parse_args()

    if args.verbose:
        pretty_print_sys_argv(sys.argv)
        pretty_print_argparse_args(args)
    
    preflight_read_file(args.input_xml_file)
    preflight_write_file(args.output_xml_file, args.force)
    preflight_read_folder(args.input_graphics_folder)
    preflight_write_folder(args.output_graphics_folder)

    # read the input XML file
    input_tree = xml.etree.ElementTree.parse(args.input_xml_file)
    input_root = input_tree.getroot()
    
    translation = args.translation
    rotation_centre = args.rotation_centre
    
    axis = args.rotation_angle_axis_1[1:4]
    angle = args.rotation_angle_axis_1[0] * math.pi / 180.0
    rotation1 = QuaternionFromAxisAngle(axis, angle)
    axis = args.rotation_angle_axis_2[1:4]
    angle = args.rotation_angle_axis_2[0] * math.pi / 180.0
    rotation2 = QuaternionFromAxisAngle(axis, angle)
    axis = args.rotation_angle_axis_3[1:4]
    angle = args.rotation_angle_axis_3[0] * math.pi / 180.0
    rotation3 = QuaternionFromAxisAngle(axis, angle)
    rotation21 = QuaternionQuaternionMultiply(rotation2, rotation1)
    rotation = QuaternionQuaternionMultiply(rotation3, rotation21)
    axis = QuaternionGetAxis(rotation)
    angle = QuaternionGetAngle(rotation) * 180.0 / math.pi
    if args.verbose:
        print("rotation %g %g %g %g" % (angle, axis[0], axis[1], axis[2]))
    
    # find the relative path from the output file to the output graphics
    output_file_path = os.path.dirname(os.path.abspath(args.output_xml_file))
    output_graphics_path = os.path.abspath(args.output_graphics_folder)
    relative_graphics_path = os.path.relpath(output_file_path, output_graphics_path)
    
    # loop through bodies
    body_translation = {}
    for child in input_root:
        if child.tag == "GLOBAL":
            if relative_graphics_path and relative_graphics_path != '.':
                child.attrib["MeshSearchPath"] = '.:' + percentEncode(relative_graphics_path, '%:')
            else:
                child.attrib["MeshSearchPath"] = '.'
    
        if child.tag == "BODY":
            if args.bodies_list and not child.attrib["ID"] in args.bodies_list:
                if args.verbose:
                    print('Not rotating : BODY ID="%s"' % (child.attrib["ID"]))
                if child.attrib["GraphicFile1"]:
                    if not args.retain_graphics_path: child.attrib["GraphicFile1"] = os.path.split(child.attrib["GraphicFile1"])[1]
                    transform_obj_files(child.attrib["GraphicFile1"], args.input_graphics_folder, args.output_graphics_folder,
                                        [0, 0, 0], [1, 0, 0, 0], [0, 0, 0], args.verbose, args.force)
                if child.attrib["GraphicFile2"]:
                    if not args.retain_graphics_path: child.attrib["GraphicFile2"] = os.path.split(child.attrib["GraphicFile2"])[1]
                    transform_obj_files(child.attrib["GraphicFile2"], args.input_graphics_folder, args.output_graphics_folder,
                                        [0, 0, 0], [1, 0, 0, 0], [0, 0, 0], args.verbose, args.force)
                if child.attrib["GraphicFile3"]:
                    if not args.retain_graphics_path: child.attrib["GraphicFile3"] = os.path.split(child.attrib["GraphicFile3"])[1]
                    transform_obj_files(child.attrib["GraphicFile3"], args.input_graphics_folder, args.output_graphics_folder,
                                        [0, 0, 0], [1, 0, 0, 0], [0, 0, 0], args.verbose, args.force)
            else:
                if args.verbose:
                    print('Old: BODY ID="%s" ConstructionPosition="%s"' % (child.attrib["ID"], child.attrib["ConstructionPosition"]))
                p1 = [float(i) for i in child.attrib["ConstructionPosition"].split()]
                p2 = Sub3x1(p1, rotation_centre)
                p3 = QuaternionVectorRotate(rotation, p2)
                p4 = Add3x1(p3, rotation_centre)
                body_translation[child.attrib["ID"]] = translation
                if type(args.set_x_position) == float: body_translation[child.attrib["ID"]] [0] = args.set_x_position - p4[0]
                if type(args.set_y_position) == float: body_translation[child.attrib["ID"]] [1] = args.set_y_position - p4[1]
                if type(args.set_z_position) == float: body_translation[child.attrib["ID"]] [2] = args.set_x_position - p4[2]
                p5 = Add3x1(p4, translation)
                child.attrib["ConstructionPosition"] = " ".join(format(x, ".18e") for x in p5)
                if args.verbose:
                    print('New: BODY ID="%s" ConstructionPosition="%s"' % (child.attrib["ID"], child.attrib["ConstructionPosition"]))
                if child.attrib["GraphicFile1"]:
                    if not args.retain_graphics_path: child.attrib["GraphicFile1"] = os.path.split(child.attrib["GraphicFile1"])[1]
                    transform_obj_files(child.attrib["GraphicFile1"], args.input_graphics_folder, args.output_graphics_folder,
                                        rotation_centre, rotation, body_translation[child.attrib["ID"]] , args.verbose, args.force)
                if child.attrib["GraphicFile2"]:
                    if not args.retain_graphics_path: child.attrib["GraphicFile2"] = os.path.split(child.attrib["GraphicFile2"])[1]
                    transform_obj_files(child.attrib["GraphicFile2"], args.input_graphics_folder, args.output_graphics_folder,
                                        rotation_centre, rotation, body_translation[child.attrib["ID"]] , args.verbose, args.force)
                if child.attrib["GraphicFile3"]:
                    if not args.retain_graphics_path: child.attrib["GraphicFile3"] = os.path.split(child.attrib["GraphicFile3"])[1]
                    transform_obj_files(child.attrib["GraphicFile3"], args.input_graphics_folder, args.output_graphics_folder,
                                        rotation_centre, rotation, body_translation[child.attrib["ID"]] , args.verbose, args.force)
                
            if args.zero_start_poses:
                if args.verbose:
                    print('Old: BODY ID="%s" Position="%s" Quaternion="%s"' % (child.attrib["ID"], child.attrib["Position"], child.attrib["Quaternion"]))
                child.attrib["Position"] = child.attrib["ConstructionPosition"]
                child.attrib["Quaternion"] = "1 0 0 0"
                if args.verbose:
                    print('New: BODY ID="%s" Position="%s" Quaternion="%s"' % (child.attrib["ID"], child.attrib["Position"], child.attrib["Quaternion"]))
        
        if child.tag == "MARKER":
            body = child.attrib["BodyID"]
            if args.bodies_list and not body in args.bodies_list:
                if args.verbose:
                    print('Skipping : Marker ID="%s"' % (child.attrib["ID"]))
                continue
            p1 = get_relative_vector(child.attrib["Position"], 3)[1]
            p2 = QuaternionVectorRotate(rotation, p1)
            child.attrib["Position"] = " ".join(format(x, ".18e") for x in p2)
            q1 = get_relative_vector(child.attrib["Quaternion"], 4)[1]
            q2 = QuaternionQuaternionMultiply(rotation, q1)
            child.attrib["Quaternion"] = " ".join(format(x, ".18e") for x in q2)
            if args.verbose:
                print('New: MARKER ID="%s" Position="%s" Quaternion="%s"' % (child.attrib["ID"], child.attrib["Position"], child.attrib["Quaternion"]))
        
    with open(args.output_xml_file, "wb") as out_file:
        out_file.write(xml.etree.ElementTree.tostring(input_root, encoding="utf-8", method="xml"))

def percentEncode(input_string, encode_list):

    # this routine encodes the characters in encode_list

    #    UTF-8 cheat sheet
    #    Binary    Hex          Comments
    #    0xxxxxxx  0x00..0x7F   Only byte of a 1-byte character encoding
    #    10xxxxxx  0x80..0xBF   Continuation bytes (1-3 continuation bytes)
    #    110xxxxx  0xC0..0xDF   First byte of a 2-byte character encoding
    #    1110xxxx  0xE0..0xEF   First byte of a 3-byte character encoding
    #    11110xxx  0xF0..0xF4   First byte of a 4-byte character encoding

    # what this means is that no UTF-8 character looks like a 1-byte colon or 1-byte percent
    output = ''
    digits = "0123456789ABCDEF"
    for i in range(0, len(input_string)):
        if encode_list.find(input_string[i]) == -1:
            output += input_string[i]
            continue
        output.append('%')
        quotient = ord(input_string[i]) / 16
        remainder = ord(input_string[i]) % 16
        output += digits[quotient]
        output +=digits[remainder]
    return output

def get_relative_vector(string, vector_length):
    tokens = string.split()
    if len(tokens) != vector_length + 1:
        print('Error converting "%s" to relative vector len(tokens)=%d' % (string, len(tokens)))
        sys.exit(1)
    name = tokens[0]
    vector = []
    for i in range(1, len(tokens)):
        if not is_a_number(tokens[i]):
            print('Error converting "%s" to relative vector "%s" is not a number' % (string, tokens[i]))
            sys.exit(1)
        vector.append(float(tokens[i]))
    return (name, vector)

def transform_obj_files(obj_file, input_folder, output_folder, rotation_centre, rotation, translation, verbose, force):
    input_file = os.path.join(input_folder, os.path.basename(obj_file))
    preflight_read_file(input_file)
    output_file = os.path.join(output_folder, os.path.basename(obj_file))
    preflight_write_file(output_file, force)
    (vertices, triangles, objects) = read_obj_file(input_file, verbose)
    for i in range(0, len(vertices)):
        p1 = vertices[i]
        p2 = Sub3x1(p1, rotation_centre)
        p3 = QuaternionVectorRotate(rotation, p2)
        p4 = Add3x1(p3, rotation_centre)
        p5 = Add3x1(p4, translation)
        vertices[i] = p5
    write_obj_file(output_file, vertices, triangles, objects, verbose)

def write_obj_file(filename, vertices, triangles, objects, verbose):
    if verbose:
        print('Writing "%s"' % (filename))
    with open(filename, 'w') as f_out:
        f_out.write('# Generated by convert_obj_to_sac.py\n')
        f_out.write('# Vertices: %d\n' % (len(vertices)))
        f_out.write('# Faces: %d\n' % (len(triangles)))
        for i in range(0, len(vertices)):
            v = vertices[i]
            f_out.write('v %f %f %f\n' % (v[0], v[1], v[2]))
        for i in range(0, len(triangles)):
            if i in objects:
                f_out.write('g %s\n' % objects[i])
                f_out.write('o %s\n' % objects[i])
            t = triangles[i]
            f_out.write('f %d %d %d\n' % (t[0] + 1, t[1] + 1, t[2] + 1))
        f_out.write('# End of file\n')
        if verbose:
            print('Written %d vertices and %d triangles' % (len(vertices), len(triangles)))

def read_obj_file(filename, verbose):
    vertices = []
    triangles = []
    objects = {}
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
        if tokens[0] == 'o':
            objects[len(triangles)] = tokens[1]
    if verbose:
        print('Read %d vertices and %d triangles' % (len(vertices), len(triangles)))
        print('%d objects found' % (len(objects)))
    return (vertices, triangles, objects)
    
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

def preflight_write_folder(folder):
    if os.path.exists(folder):
        if not os.path.isdir(folder):
            print("Error: \"%s\" exists and is not a folder" % (folder))
            sys.exit(1)
    else:
        try:
            os.makedirs(folder, exist_ok = True)
        except OSError as error:
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
    transform_bodies_construction()