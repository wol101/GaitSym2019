#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import argparse
import re
import xml.etree.ElementTree
import math

def transform_bodies():

    parser = argparse.ArgumentParser(description="Transform the bodies in a GaitSym file")
    parser.add_argument("-i", "--input_xml_file", required=True, help="input GaitSym XML config file")
    parser.add_argument("-o", "--output_xml_file", required=True, help="output GaitSym XML config file")
    parser.add_argument("-t", "--translation", nargs=3, type=float, default=[0.0, 0.0, 0.0], help="translation vector x y z [0, 0, 0])")
    parser.add_argument("-rc", "--rotation_centre", nargs=3, type=float, default=[0.0, 0.0, 0.0], help="rotation centre x y z [0, 0, 0])")
    parser.add_argument("-r1", "--rotation_angle_axis_1", nargs=4, type=float, default=[0.0, 1.0, 0.0, 0.0], help="rotation angle axis r x y z degrees [0, 1, 0, 0])")
    parser.add_argument("-r2", "--rotation_angle_axis_2", nargs=4, type=float, default=[0.0, 1.0, 0.0, 0.0], help="rotation angle axis r x y z degrees [0, 1, 0, 0])")
    parser.add_argument("-r3", "--rotation_angle_axis_3", nargs=4, type=float, default=[0.0, 1.0, 0.0, 0.0], help="rotation angle axis r x y z degrees [0, 1, 0, 0])")
    parser.add_argument("-b", "--bodies_list", nargs='*', type=list, default=[], help="only process these bodies if defined []")
    parser.add_argument("-f", "--force", action="store_true", help="force overwrite of destination file")
    parser.add_argument("-v", "--verbose", action="store_true", help="write out more information whilst processing")
    args = parser.parse_args()

    if args.verbose:
        pretty_print_sys_argv(sys.argv)
        pretty_print_argparse_args(args)

    preflight_read_file(args.input_xml_file)
    preflight_write_file(args.output_xml_file, args.force)

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
    
    # loop through bodies
    for child in input_root:
        if child.tag == "BODY":
            if args.bodies_list and not child.attrib["ID"] in args.bodies_list:
                if args.verbose:
                    print('Skipping : BODY ID="%s"' % (child.attrib["ID"]))
                continue
            if args.verbose:
                print('Old: BODY ID="%s" Position="%s" Quaternion="%s"' % (child.attrib["ID"], child.attrib["Position"], child.attrib["Quaternion"]))
            p1 = [float(i) for i in child.attrib["Position"].split()]
            p2 = Sub3x1(p1, rotation_centre)
            p3 = QuaternionVectorRotate(rotation, p2)
            p4 = Add3x1(p3, rotation_centre)
            p5 = Add3x1(p4, translation)
            child.attrib["Position"] = " ".join(format(x, ".18e") for x in p5)
            q1 = [float(i) for i in child.attrib["Quaternion"].split()]
            q2 = QuaternionQuaternionMultiply(rotation, q1)
            child.attrib["Quaternion"] = " ".join(format(x, ".18e") for x in q2)
            if args.verbose:
                print('New: BODY ID="%s" Position="%s" Quaternion="%s"' % (child.attrib["ID"], child.attrib["Position"], child.attrib["Quaternion"]))
    with open(args.output_xml_file, "wb") as out_file:
        out_file.write(xml.etree.ElementTree.tostring(input_root, encoding="utf-8", method="xml"))

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
    transform_bodies()