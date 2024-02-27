#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import argparse
import re
import xml.etree.ElementTree

def merge_config_files():

    parser = argparse.ArgumentParser(description="Merge two GaitSym files")
    parser.add_argument("-i1", "--input_xml_file1", required=True, help="First input GaitSym XML config file")
    parser.add_argument("-i2", "--input_xml_file2", required=True, help="Second input GaitSym XML config file")
    parser.add_argument("-o", "--output_xml_file", required=True, help="Output GaitSym XML config file")
    parser.add_argument("-c", "--convert_markers_to_global", action="store_true", help="Merge markers using global coordinates")
    parser.add_argument("-f", "--force", action="store_true", help="Force overwrite of destination file")
    parser.add_argument("-d", "--debug", action="store_true", help="Write out debug information whilst processing")
    parser.add_argument("-v", "--verbose", action="store_true", help="Write out more information whilst processing")
    args = parser.parse_args()

    if args.debug:
        args.verbose = True
    if args.verbose:
        pretty_print_sys_argv(sys.argv)
        pretty_print_argparse_args(args)
    
    preflight_read_file(args.input_xml_file1, args.verbose)
    preflight_read_file(args.input_xml_file2, args.verbose)
    preflight_write_file(args.output_xml_file, args.force, args.verbose)

    # read the input XML file
    if args.verbose: print('Reading "%s"' % (args.input_xml_file1))
    input_tree1 = xml.etree.ElementTree.parse(args.input_xml_file1)
    input_root1 = input_tree1.getroot()
    if args.verbose: print('Reading "%s"' % (args.input_xml_file2))
    input_tree2 = xml.etree.ElementTree.parse(args.input_xml_file2)
    input_root2 = input_tree2.getroot()
    
    new_tree = xml.etree.ElementTree.Element(input_root1.tag)
    new_tree.text = '\n'    
    
    # loop first tree and append to output tree
    if args.verbose: print('Iterating input_root1')
    for child in input_root1:
        new_tree.append(child)
    
    # loop through second tree and append all new IDs
    if args.verbose: print('Iterating input_root2')
    body_list = {}
    for child in input_root2:
        if not 'ID' in child.attrib:
            continue
        ID = child.attrib['ID']
        if child.tag == "BODY":
            body_list[ID] = child
        if not id_exists(new_tree, ID):
            if args.verbose: print('Adding ID="%s"' % (ID))
            if args.convert_markers_to_global and child.tag == "MARKER":
                convert_local_to_global(body_list, child)
            new_tree.append(child)
        else:
            if args.debug: print('Skipping ID="%s"' % (ID))
    
    if args.verbose: print('Writing "%s"' % (args.output_xml_file))
    with open(args.output_xml_file, "wb") as out_file:
        out_file.write(xml.etree.ElementTree.tostring(new_tree, encoding="utf-8", method="xml"))

def convert_local_to_global(body_list, child):
    marker_p = [float(s) for s in child.attrib["Position"].split()[1:4]]
    marker_q = [float(s) for s in child.attrib["Quaternion"].split()[1:5]]
    marker_p_body_id = child.attrib["Position"].split()[0]
    marker_q_body_id = child.attrib["Quaternion"].split()[0]
    marker_body_id = child.attrib["BodyID"]
    if not (marker_p_body_id == marker_q_body_id and marker_p_body_id == marker_body_id):
        print('Error: convert_local_to_global only works if all coordinates are from the same body')
        sys.exit()
    body = body_list[marker_body_id]
    body_q = [float(s) for s in body.attrib["Quaternion"].split()[0:4]]
    body_p = [float(s) for s in body.attrib["Position"].split()[0:3]]
    world_q = QuaternionQuaternionMultiply(marker_q, body_q) # CHECK ME
    offset = QuaternionVectorRotate(marker_q, marker_p) # CHECK ME
    world_p = Add3x1(body_p, offset)
    child.attrib["Position"] = "World " + " ".join(format(x, ".18e") for x in world_p)
    child.attrib["Quaternion"] = "World " + " ".join(format(x, ".18e") for x in world_q)
    return child

def id_exists(new_tree, ID):
    for child in new_tree:
        if not 'ID' in child.attrib:
            continue
        if child.attrib['ID'] == ID:
            return True
    return False

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

if __name__ == '__main__':
    merge_config_files()