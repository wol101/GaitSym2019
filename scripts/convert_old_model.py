#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import argparse
import math
import re
import xml.etree.ElementTree

def convert_old_model():

    parser = argparse.ArgumentParser(description="Try to convert an old GaitSym XML config file (needs to be saved in World format)")
    parser.add_argument("-i", "--input_xml_file", required=True, help="the input old format GaitSym XML config file")
    parser.add_argument("-o", "--output_xml_file", required=True, help="the output GaitSym2019 XML config file")
    parser.add_argument("-oo", "--original_obj_folder", default="", help="read existing meshes here")
    parser.add_argument("-to", "--translated_obj_folder", default = "", help="output translated meshes here")
    parser.add_argument("-m1", "--mesh1regex", nargs=2, help="Set mesh1 based on a regex search and replace of GraphicFile")
    parser.add_argument("-m2", "--mesh2regex", nargs=2, help="Set mesh2 based on a regex search and replace of GraphicFile")
    parser.add_argument("-m3", "--mesh3regex", nargs=2, help="Set mesh3 based on a regex search and replace of GraphicFile")
    parser.add_argument("-t", "--tag_attribute_value", nargs=3, action="append", help="add a new attribute to a tag. Format TAG ATTRIBUTE VALUE")
    parser.add_argument("-z", "--zero_rotations", action="store_true", help="rezero all the body rotations")
    parser.add_argument("-f", "--force", action="store_true", help="force overwrite of destination file")
    parser.add_argument("-v", "--verbose", action="store_true", help="write out more information whilst processing")
    args = parser.parse_args()

    if args.verbose:
        pretty_print_sys_argv(sys.argv)
        pretty_print_argparse_args(args)

    # preflight
    if not os.path.exists(args.input_xml_file):
        print("Error: \"%s\" missing" % (args.input_xml_file))
        sys.exit(1)
    if os.path.exists(args.output_xml_file) and not args.force:
        print("Error: \"%s\" exists. Use --force to overwrite" % (args.output_xml_file))
        sys.exit(1)
    if args.original_obj_folder and not os.path.isdir(args.original_obj_folder):
        print("Error: \"%s\" is not a folder" % (args.original_obj_folder))
        sys.exit(1)
    if args.translated_obj_folder:
        if os.path.exists(args.translated_obj_folder):
            if not os.path.isdir(args.translated_obj_folder):
                print("Error: \"%s\" exists and is not a folder" % (args.translated_obj_folder))
                sys.exit(1)
            else:
                onlyfiles = [f for f in os.listdir(args.translated_obj_folder) if os.path.isfile(os.path.join(args.translated_obj_folder, f))]
                if len(onlyfiles) > 0 and not args.force:
                    print("Error: \"%s\" exists and contains files. Use --force to overwrite" % (args.translated_obj_folder))
                    sys.exit(1)
        else:
            if args.verbose: print('Creating folder "%s"' % (args.translated_obj_folder))
            os.mkdir(args.translated_obj_folder)

    # handle the attributes to add
    add_tags = {}
    if args.tag_attribute_value:
        for tag_attribute_value in args.tag_attribute_value:
            if not tag_attribute_value[0] in add_tags:
                add_tags[tag_attribute_value[0]] = [[tag_attribute_value[1], tag_attribute_value[2]]]
            else:
                add_tags[tag_attribute_value[0]] = add_tags[tag_attribute_value[0]].append([tag_attribute_value[1], tag_attribute_value[2]])

    # read the input XML file
    if args.verbose: print('Reading "%s"' % (args.input_xml_file))
    input_tree = xml.etree.ElementTree.parse(args.input_xml_file)
    input_root = input_tree.getroot()

    # loop and parse
    global cfm
    global erp
    body_list = {}
    joint_list = {}
    geom_list = {}
    muscle_list = {}
    marker_list = {}
    driver_list = {}
    data_target_list = {}
    for child in input_root:

        if child.tag == "GLOBAL":
            global_element = child
            cfm = global_element.attrib['CFM']
            erp = global_element.attrib['ERP']

        if child.tag == "BODY":
            body_list[child.attrib["ID"]] = child

        if child.tag == "JOINT":
            joint_list[child.attrib["ID"]] = child

        if child.tag == "GEOM":
            geom_list[child.attrib["ID"]] = child

        if child.tag == "MUSCLE":
            muscle_list[child.attrib["ID"]] = child

        if child.tag == "MARKER":
            marker_list[child.attrib["ID"]] = child

        if child.tag == "DRIVER":
            driver_list[child.attrib["ID"]] = child

        if child.tag == "DATATARGET":
            data_target_list[child.attrib["ID"]] = child

        if child.tag == "ENVIRONMENT":
            if "Environment" in geom_list:
                print('Error: GEOM called "Environment" already exists')
                sys.exit(1)
            new_geom = xml.etree.ElementTree.Element("GEOM")
            new_geom.attrib["ID"] = "Environment"
            new_geom.attrib["Type"] = "Plane"
            new_geom.attrib["ABCD"] = child.attrib["Plane"]
            geom_list[new_geom.attrib["ID"]] = new_geom

    # create an empty GAITSYM tree
    new_tree = xml.etree.ElementTree.Element("GAITSYM2019")
    new_tree.text = "\n"
    new_tree.tail = "\n"

    # just copy the global element
    if args.verbose: print('Copying the GLOBAL')
    global_element.text = ""
    global_element.tail = "\n"
    new_tree.append(convert_global(global_element, args))

    # collect the markers
    for key in joint_list:
        convert_joint(joint_list[key], marker_list, markers_only = True)
    for key in geom_list:
        convert_geom(geom_list[key], marker_list, markers_only = True)
    for key in muscle_list:
        convert_muscle(muscle_list[key], marker_list, markers_only = True)

    # now create the tree in the right order
    if args.verbose: print('Creating the BODYs')
    for key in body_list:
        new_tree.append(convert_body(body_list[key], marker_list, args))
    if args.verbose: print('Creating the MARKERs')
    for key in marker_list:
        new_tree.append(marker_list[key])
    if args.verbose: print('Creating the JOINTs')
    for key in joint_list:
        new_tree.append(convert_joint(joint_list[key], marker_list, markers_only = False))
    if args.verbose: print('Creating the GEOMs')
    for key in geom_list:
        new_tree.append(convert_geom(geom_list[key], marker_list, markers_only = False))
    if args.verbose: print('Creating the MUSCLEs and STRAPs')
    for key in muscle_list:
        (new_strap, new_muscle) = convert_muscle(muscle_list[key], marker_list, markers_only = False)
        new_tree.append(new_strap)
        new_tree.append(new_muscle)
    if args.verbose: print('Creating the DATATARGETs')
    for key in data_target_list:
        new_tree.append(data_target_list[key])
    if args.verbose: print('Creating the DRIVERs')
    for key in driver_list:
        new_tree.append(convert_driver(driver_list[key]))

    # now do some generic size and colour fixes
    if args.verbose: print('Doing some fixups')
    for child in new_tree:
        if child.tag == "GLOBAL":
            child.attrib["Size1"] = "1.0"
            child.attrib["Colour1"] = "Purple1"
            if child.attrib["FitnessType"] == "DistanceTravelled":
                child.attrib["FitnessType"] = "KinematicMatch"

        if child.tag == "BODY":
            child.attrib["Size1"] = "0.1"
            child.attrib["Colour1"] = "Green1"
            child.attrib["Colour2"] = "Green2"
            child.attrib["Colour3"] = "Green3"

        if child.tag == "JOINT":
            child.attrib["Size1"] = "0.12"
            child.attrib["Colour1"] = "Red1"

        if child.tag == "GEOM":
            child.attrib["Size1"] = "0.1"
            child.attrib["Colour1"] = "Blue1"

        if child.tag == "MUSCLE":
            child.attrib["Size1"] = "0.02"
            child.attrib["Size2"] = "0.00001"
            child.attrib["Colour1"] = "Cyan1"

        if child.tag == "STRAP":
            child.attrib["Size1"] = "0.01"
            child.attrib["Size2"] = "0.1"
            child.attrib["Colour1"] = "Cyan2"
            child.attrib["Colour2"] = "Cyan3"

        if child.tag == "MARKER":
            child.attrib["Size1"] = "0.05"
            child.attrib["Colour1"] = "Magenta1"

        if child.tag == "DRIVER":
            child.attrib["Size1"] = "0.05"
            child.attrib["Colour1"] = "Yellow1"

        if child.tag == "DATATARGET":
            child.attrib["Size1"] = "0.05"
            child.attrib["Colour1"] = "Gold1"

        # always do the custom tag code at the end
        if child.tag in add_tags:
            tags_to_add = add_tags[child.tag]
            for tag_to_add in tags_to_add:
                child.attrib[tag_to_add[0]] = tag_to_add[1]

    if args.verbose: print('Writing "%s"' % (args.output_xml_file))
    out_file = open(args.output_xml_file, "wb")
    out_file.write(xml.etree.ElementTree.tostring(new_tree, encoding="utf-8", method="xml"))
    out_file.close()

def convert_global(global_element, args):
    new_global = xml.etree.ElementTree.Element("GLOBAL")
    new_global.tail = "\n"
    required_attributes = ["AllowInternalCollisions", "BMR", "CFM", "ContactMaxCorrectingVel",
                           "ContactSurfaceLayer", "DistanceTravelledBodyID", "ERP", "FitnessType", "GravityVector",
                           "IntegrationStepSize", "MechanicalEnergyLimit", "MetabolicEnergyLimit", "TimeLimit"]
    test_required_attributes(global_element, required_attributes, required_only_flag = False, quiet_flag = False)
    if "AllowConnectedCollisions" not in global_element.attrib: new_global.attrib["AllowConnectedCollisions"] = "false"
    if "StepType" not in global_element.attrib: new_global.attrib["StepType"] = "World"
    for name in global_element.attrib:
        new_global.attrib[name] = global_element.attrib[name]
    erp = float(global_element.attrib["ERP"])
    cfm = float(global_element.attrib["CFM"])
    integration_stepsize = float(global_element.attrib["IntegrationStepSize"])
    (spring_constant, damping_constant) = convert_to_spring_and_damping_constants(erp, cfm, integration_stepsize)
    new_global.attrib["SpringConstant"] = format(spring_constant, ".18e")
    new_global.attrib["DampingConstant"] = format(damping_constant, ".18e")
    new_global.attrib["StepType"] = new_global.attrib["StepType"].replace("Step", "")
    new_global.attrib["ID"] = "global"
    new_global.attrib["MeshSearchPath"] = ".:" + os.path.relpath(args.translated_obj_folder, os.path.split(args.output_xml_file)[0])
    return new_global

def convert_body(body, marker_list, args):
    # body first
    new_body = xml.etree.ElementTree.Element("BODY")
    new_body.tail = "\n"
    new_body.attrib["ID"] = body.attrib["ID"]
    new_body.attrib["Mass"] = body.attrib["Mass"]
    new_body.attrib["Quaternion"] = "World 1 0 0 0"
    quaternion = [float(s) for s in strip_world(body.attrib["Quaternion"]).split()]
    angle = QuaternionGetAngle(quaternion)
    if math.fabs(angle) > 1.0e-7 and not args.zero_rotations:
        print('Error: BODY ID="%s" Quaternion="%s" and --zero_rotations not used' % (body.attrib["ID"], body.attrib["Quaternion"]))
        sys.exit(1)
    new_body.attrib["MOI"] = body.attrib["MOI"]
    if args.zero_rotations:
        if args.verbose:
            q_angle = QuaternionGetAngle(quaternion)
            q_axis = QuaternionGetAxis(quaternion)
            print('Rotating BODY ID="%s" %gr radian about %g,%g,%g' % (body.attrib["ID"], q_angle, q_axis[0], q_axis[1], q_axis[2]))
        matrix = MatrixFromQuaternion(quaternion)
        moi = [float(s) for s in body.attrib["MOI"].split()]
        moi_tensor = Matrix()
        moi_tensor.e11 = moi[0]
        moi_tensor.e22 = moi[1]
        moi_tensor.e33 = moi[2]
        moi_tensor.e12 = moi[3]
        moi_tensor.e21 = moi[3]
        moi_tensor.e13 = moi[4]
        moi_tensor.e31 = moi[4]
        moi_tensor.e23 = moi[5]
        moi_tensor.e32 = moi[5]
        moi_tensor_r = MatrixMul(MatrixMul(matrix, moi_tensor), MatrixTrans(matrix))
        moi_r = [moi_tensor_r.e11, moi_tensor_r.e22, moi_tensor_r.e33, moi_tensor_r.e12, moi_tensor_r.e13, moi_tensor_r.e23]
        new_body.attrib["MOI"] = " ".join(format(x, ".18g") for x in moi_r)

    new_body.attrib["ConstructionPosition"] = "World " + strip_world(body.attrib["Position"])
    new_body.attrib["ConstructionDensity"] = "1000"
    new_body.attrib["Position"] = "World " + strip_world(body.attrib["Position"])
    new_body.attrib["LinearVelocity"] = body.attrib["LinearVelocity"]
    new_body.attrib["AngularVelocity"] = body.attrib["AngularVelocity"]
    if "PositionLowBound" in body.attrib: new_body.attrib["PositionLowBound"] = body.attrib["PositionLowBound"]
    if "PositionHighBound" in body.attrib: new_body.attrib["PositionHighBound"] = body.attrib["PositionHighBound"]
    if "LinearVelocityLowBound" in body.attrib: new_body.attrib["LinearVelocityLowBound"] = body.attrib["LinearVelocityLowBound"]
    if "LinearVelocityHighBound" in body.attrib: new_body.attrib["LinearVelocityHighBound"] = body.attrib["LinearVelocityHighBound"]

    if args.mesh1regex:
        new_body.attrib["GraphicFile1"] = re.sub(args.mesh1regex[0], args.mesh1regex[1], body.attrib["GraphicFile"])
    else:
        new_body.attrib["GraphicFile1"] = body.attrib["GraphicFile"]
    if args.mesh2regex:
        new_body.attrib["GraphicFile2"] = re.sub(args.mesh2regex[0], args.mesh2regex[1], body.attrib["GraphicFile"])
    if args.mesh3regex:
        new_body.attrib["GraphicFile3"] = re.sub(args.mesh3regex[0], args.mesh3regex[1], body.attrib["GraphicFile"])

    if args.translated_obj_folder and body.attrib["GraphicFile"]:
        filename = os.path.join(args.original_obj_folder, body.attrib["GraphicFile"])
        if not os.path.isfile(filename):
            print('Error: mesh "%s" missing' % (filename))
            sys.exit(1)
        (vertices, triangles, objects) = read_obj_file(filename, args.verbose)
        position = [float(s) for s in strip_world(body.attrib["Position"]).split()]
        offset = [float(s) for s in strip_world(body.attrib["Offset"]).split()]
        scale = [1.0, 1.0, 1.0]
        if "Scale" in body.attrib:
            scale = [float(s) for s in body.attrib["Scale"].split()]
            if len(scale) != 3: scale = [scale[0], scale[0], scale[0]]
        new_vertices = []
        for vertex in vertices:
            v_scale = Mul3x1(scale, vertex)
            v_offset = Add3x1(v_scale, offset)
            if args.zero_rotations:
                v_rot = QuaternionVectorRotate(quaternion, v_offset)
            else:
                v_rot = v_offset
            v_position = Add3x1(v_rot, position)
            new_vertices.append(v_position)
        new_filename = os.path.join(args.translated_obj_folder, new_body.attrib["GraphicFile1"])
        if args.verbose:
            print('"%s" scale = %f, %f, %f' % (new_filename, scale[0], scale[1], scale[2]))
            print('"%s" offset = %f, %f, %f' % (new_filename, offset[0], offset[1], offset[2]))
        write_obj_file(new_filename, new_vertices, triangles, objects, args.verbose)

    # and then create any new markers needed
    new_marker = xml.etree.ElementTree.Element("MARKER")
    new_marker.tail = "\n"
    new_marker.attrib["ID"] = new_body.attrib["ID"] + "_COM_Marker"
    new_marker.attrib["BodyID"] = new_body.attrib["ID"]
    new_marker.attrib["Position"] = "World " + new_body.attrib["ConstructionPosition"]
    new_marker.attrib["Quaternion"] = "World " + "1.0 0.0 0.0 0.0"
    marker_list[new_marker.attrib["ID"]] = new_marker
    return new_body

def convert_joint(joint, marker_list, markers_only):
    if joint.attrib["Type"] == "Hinge":
        # create the joint markers
        body1_marker = xml.etree.ElementTree.Element("MARKER")
        body1_marker.tail = "\n"
        body1_marker.attrib["ID"] = joint.attrib["ID"] + "_Body1_Marker"
        body1_marker.attrib["BodyID"] = joint.attrib["Body1ID"]
        body1_marker.attrib["Position"] = "World " + strip_world(joint.attrib["HingeAnchor"])
        v1 = [float(i) for i in strip_world(joint.attrib["HingeAxis"]).split()]
        q = GetRotationTo([1.0, 0.0, 0.0], v1)
        body1_marker.attrib["Quaternion"] = "World " + " ".join(format(x, ".18e") for x in q)
        marker_list[body1_marker.attrib["ID"]] = body1_marker
        body2_marker = xml.etree.ElementTree.Element("MARKER")
        body2_marker.tail = "\n"
        body2_marker.attrib["ID"] = joint.attrib["ID"] + "_Body2_Marker"
        body2_marker.attrib["BodyID"] = joint.attrib["Body2ID"]
        body2_marker.attrib["Position"] = "World " + strip_world(joint.attrib["HingeAnchor"])
        v1 = [float(i) for i in strip_world(joint.attrib["HingeAxis"]).split()]
        q = GetRotationTo([1.0, 0.0, 0.0], v1)
        body2_marker.attrib["Quaternion"] = "World " + " ".join(format(x, ".18e") for x in q)
        marker_list[body2_marker.attrib["ID"]] = body2_marker
        if markers_only:
            return
        # then the joint
        new_joint = xml.etree.ElementTree.Element("JOINT")
        new_joint.tail = "\n"
        new_joint.attrib["ID"] = joint.attrib["ID"]
        new_joint.attrib["Type"] = joint.attrib["Type"]
        new_joint.attrib["Body1MarkerID"] = body1_marker.attrib["ID"]
        new_joint.attrib["Body2MarkerID"] = body2_marker.attrib["ID"]
        new_joint.attrib["LowStop"] = joint.attrib["ParamLoStop"]
        new_joint.attrib["HighStop"] = joint.attrib["ParamHiStop"]
    elif joint.attrib["Type"] == "FloatingHinge":
        # create the joint markers
        body1_marker = xml.etree.ElementTree.Element("MARKER")
        body1_marker.tail = "\n"
        body1_marker.attrib["ID"] = joint.attrib["ID"] + "_Body1_Marker"
        body1_marker.attrib["BodyID"] = joint.attrib["Body1ID"]
        body1_marker.attrib["Position"] = joint.attrib["Body1ID"] + " 0 0 0"
        v1 = [float(i) for i in strip_world(joint.attrib["FloatingHingeAxis"]).split()]
        q = GetRotationTo([1.0, 0.0, 0.0], v1)
        body1_marker.attrib["Quaternion"] = "World " + " ".join(format(x, ".18e") for x in q)
        marker_list[body1_marker.attrib["ID"]] = body1_marker
        body2_marker = xml.etree.ElementTree.Element("MARKER")
        body2_marker.tail = "\n"
        body2_marker.attrib["ID"] = joint.attrib["ID"] + "_Body2_Marker"
        body2_marker.attrib["BodyID"] = joint.attrib["Body2ID"]
        body2_marker.attrib["Position"] = joint.attrib["Body2ID"] + " 0 0 0"
        v1 = [float(i) for i in strip_world(joint.attrib["FloatingHingeAxis"]).split()]
        q = GetRotationTo([1.0, 0.0, 0.0], v1)
        body2_marker.attrib["Quaternion"] = "World " + " ".join(format(x, ".18e") for x in q)
        marker_list[body2_marker.attrib["ID"]] = body2_marker
        if markers_only:
            return
        # then the joint
        new_joint = xml.etree.ElementTree.Element("JOINT")
        new_joint.tail = "\n"
        new_joint.attrib["ID"] = joint.attrib["ID"]
        new_joint.attrib["Type"] = joint.attrib["Type"]
        new_joint.attrib["Body1MarkerID"] = body1_marker.attrib["ID"]
        new_joint.attrib["Body2MarkerID"] = body2_marker.attrib["ID"]
        new_joint.attrib["LowStop"] = joint.attrib["ParamLoStop"]
        new_joint.attrib["HighStop"] = joint.attrib["ParamHiStop"]
    elif joint.attrib["Type"] == "Universal":
        # create the joint markers
        body1_marker = xml.etree.ElementTree.Element("MARKER")
        body1_marker.tail = "\n"
        body1_marker.attrib["ID"] = joint.attrib["ID"] + "_Body1_Marker"
        body1_marker.attrib["BodyID"] = joint.attrib["Body1ID"]
        body1_marker.attrib["Position"] = "World " + strip_world(joint.attrib["UniversalAnchor"])
        v1 = [float(i) for i in strip_world(joint.attrib["UniversalAxis1"]).split()]
        v2 = [float(i) for i in strip_world(joint.attrib["UniversalAxis2"]).split()]
        v3 = Normalise3x1(CrossProduct3x1(v1, v2))
        R = Matrix()
        R.e11 = v1[0]; R.e21 = v1[1]; R.e31 = v1[2]
        R.e12 = v2[0]; R.e22 = v2[1]; R.e32 = v2[2]
        R.e13 = v3[0]; R.e23 = v3[1]; R.e33 = v3[2]
        q = QuaternionFromMatrix(R)
        body1_marker.attrib["Quaternion"] = "World " + " ".join(format(x, ".18e") for x in q)
        marker_list[body1_marker.attrib["ID"]] = body1_marker
        body2_marker = xml.etree.ElementTree.Element("MARKER")
        body2_marker.tail = "\n"
        body2_marker.attrib["ID"] = joint.attrib["ID"] + "_Body2_Marker"
        body2_marker.attrib["BodyID"] = joint.attrib["Body2ID"]
        body2_marker.attrib["Position"] = "World " + strip_world(joint.attrib["UniversalAnchor"])
        v1 = [float(i) for i in strip_world(joint.attrib["UniversalAxis1"]).split()]
        v2 = [float(i) for i in strip_world(joint.attrib["UniversalAxis2"]).split()]
        v3 = Normalise3x1(CrossProduct3x1(v1, v2))
        R.e11 = v1[0]; R.e21 = v1[1]; R.e31 = v1[2]
        R.e12 = v2[0]; R.e22 = v2[1]; R.e32 = v2[2]
        R.e13 = v3[0]; R.e23 = v3[1]; R.e33 = v3[2]
        q = QuaternionFromMatrix(R)
        body2_marker.attrib["Quaternion"] = "World " + " ".join(format(x, ".18e") for x in q)
        marker_list[body2_marker.attrib["ID"]] = body2_marker
        if markers_only:
            return
        # then the joint
        new_joint = xml.etree.ElementTree.Element("JOINT")
        new_joint.tail = "\n"
        new_joint.attrib["ID"] = joint.attrib["ID"]
        new_joint.attrib["Type"] = joint.attrib["Type"]
        new_joint.attrib["Body1MarkerID"] = body1_marker.attrib["ID"]
        new_joint.attrib["Body2MarkerID"] = body2_marker.attrib["ID"]
        new_joint.attrib["LowStop1"] = joint.attrib["ParamLoStop1"]
        new_joint.attrib["HighStop1"] = joint.attrib["ParamHiStop1"]
        new_joint.attrib["LowStop2"] = joint.attrib["ParamLoStop2"]
        new_joint.attrib["HighStop2"] = joint.attrib["ParamHiStop2"]
    elif joint.attrib["Type"] == "Fixed":
        # create the joint markers
        body1_marker = xml.etree.ElementTree.Element("MARKER")
        body1_marker.tail = "\n"
        body1_marker.attrib["ID"] = joint.attrib["ID"] + "_Body1_Marker"
        body1_marker.attrib["BodyID"] = joint.attrib["Body1ID"]
        if not "StressCalculationType" in joint.attrib:
            joint.attrib["StressCalculationType"] = "None"
        if joint.attrib["StressCalculationType"] == "None":
            body1_marker.attrib["Position"] = "0 0 0"
            body1_marker.attrib["Quaternion"] = "1 0 0 0"
        else:
            body1_marker.attrib["Position"] = joint.attrib["StressOrigin"]
            body1_marker.attrib["Quaternion"] = joint.attrib["StressOrientation"]
        marker_list[body1_marker.attrib["ID"]] = body1_marker
        body2_marker = xml.etree.ElementTree.Element("MARKER")
        body2_marker.tail = "\n"
        body2_marker.attrib["ID"] = joint.attrib["ID"] + "_Body2_Marker"
        body2_marker.attrib["BodyID"] = joint.attrib["Body2ID"]
        if joint.attrib["StressCalculationType"] == "None":
            body2_marker.attrib["Position"] = "0 0 0"
            body2_marker.attrib["Quaternion"] = "1 0 0 0"
        else:
            body2_marker.attrib["Position"] = joint.attrib["StressOrigin"]
            body2_marker.attrib["Quaternion"] = joint.attrib["StressOrientation"]
        marker_list[body2_marker.attrib["ID"]] = body2_marker
        if markers_only:
            return
        # then the joint
        new_joint = xml.etree.ElementTree.Element("JOINT")
        new_joint.tail = "\n"
        new_joint.attrib["ID"] = joint.attrib["ID"]
        new_joint.attrib["Type"] = joint.attrib["Type"]
        new_joint.attrib["Body1MarkerID"] = body1_marker.attrib["ID"]
        new_joint.attrib["Body2MarkerID"] = body2_marker.attrib["ID"]
        if joint.attrib["StressCalculationType"] == "None":
            new_joint.attrib["StressCalculationType"] = "None"
        else:
            new_joint.attrib["StressCalculationType"] = joint.attrib["StressCalculationType"]
            new_joint.attrib["StressBitmap"] = joint.attrib["StressBitmap"]
            new_joint.attrib["StressBitmapDimensions"] = joint.attrib["StressBitmapDimensions"]
            new_joint.attrib["StressBitmapPixelSize"] = joint.attrib["StressBitmapPixelSize"]
            new_joint.attrib["StressLimit"] = joint.attrib["StressLimit"]
            new_joint.attrib["StressBitmapDisplayRange"] = joint.attrib["StressBitmapDisplayRange"]
            new_joint.attrib["LowPassType"] = "Butterworth2ndOrderLowPass"
            if "StressLimitCutoffFrequency" in joint.attrib:
                new_joint.attrib["CutoffFrequency"] = joint.attrib["StressLimitCutoffFrequency"]
            else:
                default_cutoff_frequency = 5
                print('"StressLimitCutoffFrequency" missing. Setting to %d' % (default_cutoff_frequency))
                new_joint.attrib["CutoffFrequency"] = str(default_cutoff_frequency)
    else:
        print("Error: joint type \"%s\" unsupported" % (joint.attrib["Type"]))
        sys.exit(1)
    return new_joint

def convert_geom(geom, marker_list, markers_only):
    global cfm
    global erp
    if geom.attrib["Type"] == "Sphere":
        # create the geom markers
        centre_marker = xml.etree.ElementTree.Element("MARKER")
        centre_marker.tail = "\n"
        centre_marker.attrib["ID"] = geom.attrib["ID"] + "_Centre_Marker"
        centre_marker.attrib["BodyID"] = geom.attrib["BodyID"]
        centre_marker.attrib["Position"] = "World " + strip_world(geom.attrib["Position"])
        centre_marker.attrib["Quaternion"] = "1 0 0 0"
        marker_list[centre_marker.attrib["ID"]] = centre_marker
        if markers_only:
            return
        # then the geom
        new_geom = xml.etree.ElementTree.Element("GEOM")
        new_geom.tail = "\n"
        new_geom.attrib["ID"] = geom.attrib["ID"]
        new_geom.attrib["Type"] = geom.attrib["Type"]
        new_geom.attrib["MarkerID"] = centre_marker.attrib["ID"]
        if "SpringConstant" in geom.attrib: new_geom.attrib["SpringConstant"] = geom.attrib["SpringConstant"]
        if "ContactSoftERP" in geom.attrib: new_geom.attrib["ERP"] = geom.attrib["ContactSoftERP"]
        if "DampingConstant" in geom.attrib: new_geom.attrib["DampingConstant"] = geom.attrib["DampingConstant"]
        if "ContactSoftCFM" in geom.attrib: new_geom.attrib["CFM"] = geom.attrib["ContactSoftCFM"]
        new_geom.attrib["Mu"] = geom.attrib["Mu"]
        new_geom.attrib["Radius"] = geom.attrib["Radius"]
        new_geom.attrib["Bounce"] = (geom.attrib["Bounce"] if "Bounce" in geom.attrib else "0")
        new_geom.attrib["Abort"] = (geom.attrib["Abort"] if "Abort" in geom.attrib else "false")
        new_geom.attrib["Adhesion"] = (geom.attrib["Adhesion"] if "Adhesion" in geom.attrib else "false")
    elif geom.attrib["Type"] == "CappedCylinder":
        # create the geom markers
        centre_marker = xml.etree.ElementTree.Element("MARKER")
        centre_marker.tail = "\n"
        centre_marker.attrib["ID"] = geom.attrib["ID"] + "_Centre_Marker"
        centre_marker.attrib["BodyID"] = geom.attrib["BodyID"]
        centre_marker.attrib["Position"] = "World " + strip_world(geom.attrib["Position"])
        centre_marker.attrib["Quaternion"] = "World " + strip_world(geom.attrib["Quaternion"])
        marker_list[centre_marker.attrib["ID"]] = centre_marker
        if markers_only:
            return
        # then the geom
        new_geom = xml.etree.ElementTree.Element("GEOM")
        new_geom.tail = "\n"
        new_geom.attrib["ID"] = geom.attrib["ID"]
        new_geom.attrib["Type"] = geom.attrib["Type"]
        new_geom.attrib["MarkerID"] = centre_marker.attrib["ID"]
        if "SpringConstant" in geom.attrib: new_geom.attrib["SpringConstant"] = geom.attrib["SpringConstant"]
        if "ContactSoftERP" in geom.attrib: new_geom.attrib["ERP"] = geom.attrib["ContactSoftERP"]
        if "DampingConstant" in geom.attrib: new_geom.attrib["DampingConstant"] = geom.attrib["DampingConstant"]
        if "ContactSoftCFM" in geom.attrib: new_geom.attrib["CFM"] = geom.attrib["ContactSoftCFM"]
        new_geom.attrib["Mu"] = geom.attrib["Mu"]
        new_geom.attrib["Radius"] = geom.attrib["Radius"]
        new_geom.attrib["Length"] = geom.attrib["Length"]
        new_geom.attrib["Bounce"] = (geom.attrib["Bounce"] if "Bounce" in geom.attrib else "0")
        new_geom.attrib["Abort"] = (geom.attrib["Abort"] if "Abort" in geom.attrib else "false")
        new_geom.attrib["Adhesion"] = (geom.attrib["Adhesion"] if "Adhesion" in geom.attrib else "false")
    elif geom.attrib["Type"] == "Plane":
        # create the geom markers
        [a, b, c, d] = [float(i) for i in geom.attrib["ABCD"].split()]
        [point, v1, v2] = convert_cartesian_plane_to_parametric(a, b, c, d)
        # now calculate the normal (z)
        z = CrossProduct3x1(v1, v2)
        z = Normalise3x1(z)
        quaternion = GetRotationTo([0.0,0.0,1.0], z)
        centre_marker = xml.etree.ElementTree.Element("MARKER")
        centre_marker.tail = "\n"
        centre_marker.attrib["ID"] = geom.attrib["ID"] + "_Plane_Marker"
        centre_marker.attrib["BodyID"] = "World"
        centre_marker.attrib["Position"] = "World " + ' '.join(format(x, '.18e') for x in point)
        centre_marker.attrib["Quaternion"] = "World " + ' '.join(format(x, '.18e') for x in quaternion)
        marker_list[centre_marker.attrib["ID"]] = centre_marker
        if markers_only:
            return
        # then the geom
        new_geom = xml.etree.ElementTree.Element("GEOM")
        new_geom.tail = "\n"
        new_geom.attrib["ID"] = geom.attrib["ID"]
        new_geom.attrib["Type"] = geom.attrib["Type"]
        new_geom.attrib["MarkerID"] = centre_marker.attrib["ID"]
        new_geom.attrib["Size1"] = "100"
        new_geom.attrib["Size2"] = "100"
        new_geom.attrib["ERP"] = erp
        new_geom.attrib["CFM"] = cfm
        new_geom.attrib["Mu"] = "1"
        new_geom.attrib["Bounce"] = (geom.attrib["Bounce"] if "Bounce" in geom.attrib else "0")
        new_geom.attrib["Abort"] = (geom.attrib["Abort"] if "Abort" in geom.attrib else "false")
        new_geom.attrib["Adhesion"] = (geom.attrib["Adhesion"] if "Adhesion" in geom.attrib else "false")
    else:
        print("Error: geom type \"%s\" unsupported" % (geom.attrib["Type"]))
        sys.exit(1)
    return new_geom

def convert_muscle(muscle, marker_list, markers_only):
    # strap first
    if muscle.attrib["Strap"] == "TwoPoint":
        # create the muscle markers
        origin_marker = xml.etree.ElementTree.Element("MARKER")
        origin_marker.tail = "\n"
        origin_marker.attrib["ID"] = muscle.attrib["ID"] + "_Origin_Marker"
        origin_marker.attrib["BodyID"] = muscle.attrib["OriginBodyID"]
        origin_marker.attrib["Position"] = "World " + strip_world(muscle.attrib["Origin"])
        origin_marker.attrib["Quaternion"] = "1 0 0 0"
        marker_list[origin_marker.attrib["ID"]] = origin_marker
        insertion_marker = xml.etree.ElementTree.Element("MARKER")
        insertion_marker.tail = "\n"
        insertion_marker.attrib["ID"] = muscle.attrib["ID"] + "_Insertion_Marker"
        insertion_marker.attrib["BodyID"] = muscle.attrib["InsertionBodyID"]
        insertion_marker.attrib["Position"] = "World " + strip_world(muscle.attrib["Insertion"])
        insertion_marker.attrib["Quaternion"] = "1 0 0 0"
        marker_list[insertion_marker.attrib["ID"]] = insertion_marker
        if markers_only:
            return
        # then the strap
        new_strap = xml.etree.ElementTree.Element("STRAP")
        new_strap.tail = "\n"
        new_strap.attrib["ID"] = muscle.attrib["ID"] + "_Strap"
        new_strap.attrib["Type"] = muscle.attrib["Strap"]
        new_strap.attrib["OriginMarkerID"] = origin_marker.attrib["ID"]
        new_strap.attrib["InsertionMarkerID"] = insertion_marker.attrib["ID"]
        new_strap.attrib["Length"] = "-1"
    elif muscle.attrib["Strap"] == "ThreePoint":
        # create the muscle markers
        origin_marker = xml.etree.ElementTree.Element("MARKER")
        origin_marker.tail = "\n"
        origin_marker.attrib["ID"] = muscle.attrib["ID"] + "_Origin_Marker"
        origin_marker.attrib["BodyID"] = muscle.attrib["OriginBodyID"]
        origin_marker.attrib["Position"] = "World " + strip_world(muscle.attrib["Origin"])
        origin_marker.attrib["Quaternion"] = "1 0 0 0"
        marker_list[origin_marker.attrib["ID"]] = origin_marker
        insertion_marker = xml.etree.ElementTree.Element("MARKER")
        insertion_marker.tail = "\n"
        insertion_marker.attrib["ID"] = muscle.attrib["ID"] + "_Insertion_Marker"
        insertion_marker.attrib["BodyID"] = muscle.attrib["InsertionBodyID"]
        insertion_marker.attrib["Position"] = "World " + strip_world(muscle.attrib["Insertion"])
        insertion_marker.attrib["Quaternion"] = "1 0 0 0"
        marker_list[insertion_marker.attrib["ID"]] = insertion_marker
        midpoint_marker = xml.etree.ElementTree.Element("MARKER")
        midpoint_marker.tail = "\n"
        midpoint_marker.attrib["ID"] = muscle.attrib["ID"] + "_Via_Point_Marker_0"
        midpoint_marker.attrib["BodyID"] = muscle.attrib["MidPointBodyID"]
        midpoint_marker.attrib["Position"] = "World " + strip_world(muscle.attrib["MidPoint"])
        midpoint_marker.attrib["Quaternion"] = "1 0 0 0"
        marker_list[midpoint_marker.attrib["ID"]] = midpoint_marker
        if markers_only:
            return
        # then the strap
        new_strap = xml.etree.ElementTree.Element("STRAP")
        new_strap.tail = "\n"
        new_strap.attrib["ID"] = muscle.attrib["ID"] + "_Strap"
        new_strap.attrib["Type"] = "NPoint" # not muscle.attrib["Strap"] this time
        new_strap.attrib["OriginMarkerID"] = origin_marker.attrib["ID"]
        new_strap.attrib["InsertionMarkerID"] = insertion_marker.attrib["ID"]
        new_strap.attrib["ViaPointMarkerIDList"] = midpoint_marker.attrib["ID"]
        new_strap.attrib["Length"] = "-1"
    elif muscle.attrib["Strap"] == "CylinderWrap":
        # create the muscle markers
        origin_marker = xml.etree.ElementTree.Element("MARKER")
        origin_marker.tail = "\n"
        origin_marker.attrib["ID"] = muscle.attrib["ID"] + "_Origin_Marker"
        origin_marker.attrib["BodyID"] = muscle.attrib["OriginBodyID"]
        origin_marker.attrib["Position"] = "World " + strip_world(muscle.attrib["Origin"])
        origin_marker.attrib["Quaternion"] = "1 0 0 0"
        marker_list[origin_marker.attrib["ID"]] = origin_marker
        insertion_marker = xml.etree.ElementTree.Element("MARKER")
        insertion_marker.tail = "\n"
        insertion_marker.attrib["ID"] = muscle.attrib["ID"] + "_Insertion_Marker"
        insertion_marker.attrib["BodyID"] = muscle.attrib["InsertionBodyID"]
        insertion_marker.attrib["Position"] = "World " + strip_world(muscle.attrib["Insertion"])
        insertion_marker.attrib["Quaternion"] = "1 0 0 0"
        marker_list[insertion_marker.attrib["ID"]] = insertion_marker
        cylinder_marker = xml.etree.ElementTree.Element("MARKER")
        cylinder_marker.tail = "\n"
        cylinder_marker.attrib["ID"] = muscle.attrib["ID"] + "_Cylinder_Marker_0"
        cylinder_marker.attrib["BodyID"] = muscle.attrib["CylinderBodyID"]
        cylinder_marker.attrib["Position"] = "World " + strip_world(muscle.attrib["CylinderPosition"])
        cylinder_marker.attrib["Quaternion"] = "World " + fix_cylinder_rotation(strip_world(muscle.attrib["CylinderQuaternion"]))
        marker_list[cylinder_marker.attrib["ID"]] = cylinder_marker
        if markers_only:
            return
        # then the strap
        new_strap = xml.etree.ElementTree.Element("STRAP")
        new_strap.tail = "\n"
        new_strap.attrib["ID"] = muscle.attrib["ID"] + "_Strap"
        new_strap.attrib["Type"] = muscle.attrib["Strap"]
        new_strap.attrib["OriginMarkerID"] = origin_marker.attrib["ID"]
        new_strap.attrib["InsertionMarkerID"] = insertion_marker.attrib["ID"]
        new_strap.attrib["CylinderMarkerID"] = cylinder_marker.attrib["ID"]
        new_strap.attrib["CylinderRadius"] = muscle.attrib["CylinderRadius"]
        new_strap.attrib["Length"] = "-1"
    elif muscle.attrib["Strap"] == "TwoCylinderWrap":
        # create the muscle markers
        origin_marker = xml.etree.ElementTree.Element("MARKER")
        origin_marker.tail = "\n"
        origin_marker.attrib["ID"] = muscle.attrib["ID"] + "_Origin_Marker"
        origin_marker.attrib["BodyID"] = muscle.attrib["OriginBodyID"]
        origin_marker.attrib["Position"] = "World " + strip_world(muscle.attrib["Origin"])
        origin_marker.attrib["Quaternion"] = "1 0 0 0"
        marker_list[origin_marker.attrib["ID"]] = origin_marker
        insertion_marker = xml.etree.ElementTree.Element("MARKER")
        insertion_marker.tail = "\n"
        insertion_marker.attrib["ID"] = muscle.attrib["ID"] + "_Insertion_Marker"
        insertion_marker.attrib["BodyID"] = muscle.attrib["InsertionBodyID"]
        insertion_marker.attrib["Position"] = "World " + strip_world(muscle.attrib["Insertion"])
        insertion_marker.attrib["Quaternion"] = "1 0 0 0"
        marker_list[insertion_marker.attrib["ID"]] = insertion_marker
        cylinder1_marker = xml.etree.ElementTree.Element("MARKER")
        cylinder1_marker.tail = "\n"
        cylinder1_marker.attrib["ID"] = muscle.attrib["ID"] + "_Cylinder1_Marker_0"
        cylinder1_marker.attrib["BodyID"] = muscle.attrib["Cylinder1BodyID"]
        cylinder1_marker.attrib["Position"] = "World " + strip_world(muscle.attrib["Cylinder1Position"])
        cylinder1_marker.attrib["Quaternion"] = "World " + fix_cylinder_rotation(strip_world(muscle.attrib["CylinderQuaternion"]))
        marker_list[cylinder1_marker.attrib["ID"]] = cylinder1_marker
        cylinder2_marker = xml.etree.ElementTree.Element("MARKER")
        cylinder2_marker.tail = "\n"
        cylinder2_marker.attrib["ID"] = muscle.attrib["ID"] + "_cylinder2_marker_0"
        cylinder2_marker.attrib["BodyID"] = muscle.attrib["Cylinder2BodyID"]
        cylinder2_marker.attrib["Position"] = "World " + strip_world(muscle.attrib["Cylinder2Position"])
        cylinder2_marker.attrib["Quaternion"] = "World 1 0 0 0"
        marker_list[cylinder2_marker.attrib["ID"]] = cylinder2_marker
        if markers_only:
            return
        # then the strap
        new_strap = xml.etree.ElementTree.Element("STRAP")
        new_strap.tail = "\n"
        new_strap.attrib["ID"] = muscle.attrib["ID"] + "_Strap"
        new_strap.attrib["Type"] = muscle.attrib["Strap"]
        new_strap.attrib["OriginMarkerID"] = origin_marker.attrib["ID"]
        new_strap.attrib["InsertionMarkerID"] = insertion_marker.attrib["ID"]
        new_strap.attrib["Cylinder1MarkerID"] = cylinder1_marker.attrib["ID"]
        new_strap.attrib["Cylinder1Radius"] = muscle.attrib["Cylinder1Radius"]
        new_strap.attrib["Cylinder2MarkerID"] = cylinder2_marker.attrib["ID"]
        new_strap.attrib["Cylinder2Radius"] = muscle.attrib["Cylinder2Radius"]
        new_strap.attrib["Length"] = "-1"
    elif muscle.attrib["Strap"] == "NPoint":
        # create the muscle markers
        origin_marker = xml.etree.ElementTree.Element("MARKER")
        origin_marker.tail = "\n"
        origin_marker.attrib["ID"] = muscle.attrib["ID"] + "_Origin_Marker"
        origin_marker.attrib["BodyID"] = muscle.attrib["OriginBodyID"]
        origin_marker.attrib["Position"] = "World " + strip_world(muscle.attrib["Origin"])
        origin_marker.attrib["Quaternion"] = "1 0 0 0"
        marker_list[origin_marker.attrib["ID"]] = origin_marker
        insertion_marker = xml.etree.ElementTree.Element("MARKER")
        insertion_marker.tail = "\n"
        insertion_marker.attrib["ID"] = muscle.attrib["ID"] + "_Insertion_Marker"
        insertion_marker.attrib["BodyID"] = muscle.attrib["InsertionBodyID"]
        insertion_marker.attrib["Position"] = "World " + strip_world(muscle.attrib["Insertion"])
        insertion_marker.attrib["Quaternion"] = "1 0 0 0"
        marker_list[insertion_marker.attrib["ID"]] = insertion_marker
        n_viapoints = 0
        viapoint_names = []
        while True:
            viapoint_name = "ViaPoint" + str(n_viapoints)
            if viapoint_name in muscle.attrib:
                viapoint_marker = xml.etree.ElementTree.Element("MARKER")
                viapoint_marker.tail = "\n"
                viapoint_marker.attrib["ID"] = muscle.attrib["ID"] + "_ViaPoint_Marker_" + str(n_viapoints)
                if "ViaPointBody" + str(n_viapoints) in muscle.attrib:
                    viapoint_marker.attrib["BodyID"] = muscle.attrib["ViaPointBody" + str(n_viapoints)]
                else:
                    if "ViaPoint" + str(n_viapoints) + "BodyID" in muscle.attrib:
                        viapoint_marker.attrib["BodyID"] = muscle.attrib["ViaPoint" + str(n_viapoints) + "BodyID"]
                    else:
                        print('Error neither "%s" nor "%s" found' % ("ViaPointBody" + str(n_viapoints), "ViaPoint" + str(n_viapoints) + "BodyID"))
                        sys.exit(1)
                viapoint_marker.attrib["Position"] = "World " + strip_world(muscle.attrib[viapoint_name])
                viapoint_marker.attrib["Quaternion"] = "1 0 0 0"
                marker_list[viapoint_marker.attrib["ID"]] = viapoint_marker
                viapoint_names.append(viapoint_marker.attrib["ID"])
                n_viapoints = n_viapoints + 1
            else:
                break
        if markers_only:
            return
        # then the strap
        new_strap = xml.etree.ElementTree.Element("STRAP")
        new_strap.tail = "\n"
        new_strap.attrib["ID"] = muscle.attrib["ID"] + "_Strap"
        new_strap.attrib["Type"] = muscle.attrib["Strap"]
        new_strap.attrib["OriginMarkerID"] = origin_marker.attrib["ID"]
        new_strap.attrib["InsertionMarkerID"] = insertion_marker.attrib["ID"]
        new_strap.attrib["ViaPointMarkerIDList"] = " ".join(viapoint_names)
        new_strap.attrib["Length"] = "-1"
    else:
        print("Error: strap type \"%s\" unsupported" % (muscle.attrib["Strap"]))
        sys.exit(1)
    if not markers_only:
        # now the muscle
        if muscle.attrib["Type"] == "MinettiAlexanderComplete":
            required_attributes = ["ActivationK","ActivationKinetics","ActivationRate","FastTwitchProportion","FibreLength","ForcePerUnitArea",
                                    "ID","InitialFibreLength","PCA","ParallelStrainAtFmax","ParallelStrainModel","ParallelStrainRateAtFmax",
                                    "SerialStrainAtFmax","SerialStrainModel","SerialStrainRateAtFmax","StartActivation","TActivationA","TActivationB",
                                    "TDeactivationA","TDeactivationB","TendonLength","Type","VMaxFactor","Width"]
            test_required_attributes(muscle, required_attributes, required_only_flag = False, quiet_flag = True)
            new_muscle = xml.etree.ElementTree.Element("MUSCLE")
            new_muscle.tail = "\n"
            new_muscle.text = "\n"
            for attribute in required_attributes:
                new_muscle.attrib[attribute] = muscle.attrib[attribute]
            new_muscle.attrib["StartActivation"] = (muscle.attrib["StartActivation"] if "StartActivation" in muscle.attrib else "0")
            new_muscle.attrib["MinimumActivation"] = (muscle.attrib["MinimumActivation"] if "MinimumActivation" in muscle.attrib else "0.0001")
            new_muscle.attrib["StrapID"] = new_strap.attrib["ID"]
        elif muscle.attrib["Type"] == "MinettiAlexander":
            required_attributes = ["ActivationK","FibreLength","ForcePerUnitArea","ID","PCA","Type","VMaxFactor"]
            test_required_attributes(muscle, required_attributes, required_only_flag = False, quiet_flag = True)
            new_muscle = xml.etree.ElementTree.Element("MUSCLE")
            new_muscle.tail = "\n"
            new_muscle.text = "\n"
            for attribute in required_attributes:
                new_muscle.attrib[attribute] = muscle.attrib[attribute]
            new_muscle.attrib["StrapID"] = new_strap.attrib["ID"]
        else:
            print("Error: muscle type \"%s\" unsupported" % (muscle.attrib["Type"]))
            sys.exit(1)

    return (new_strap, new_muscle)

def convert_driver(driver_element):
    new_driver = xml.etree.ElementTree.Element("DRIVER")
    new_driver.tail = "\n"
    required_attributes = ["Type", "ID", "TargetID"]
    test_required_attributes(driver_element, required_attributes, required_only_flag = False, quiet_flag = True)
    for name in driver_element.attrib:
        new_driver.attrib[name] = driver_element.attrib[name]
    if new_driver.attrib["Type"] == "Cyclic" or new_driver.attrib["Type"] == "Cyclic":
        if "DurationValuePairs" in new_driver.attrib:
            tokens = new_driver.attrib["DurationValuePairs"].split()
            durations = []
            values = []
            for i in range(0, len(tokens), 2):
                durations.append(tokens[i])
                values.append(tokens[i + 1])
        new_driver.attrib["Durations"] = " ".join(durations)
        new_driver.attrib["Values"] = " ".join(values)
    if new_driver.attrib["Type"] == "StackedBoxCar":
          new_driver.attrib["Type"] = "StackedBoxcar"
          new_driver.attrib["CycleTime"] = new_driver.attrib["CycleTimes"].split()[0]
    new_driver.attrib["TargetIDList"] = new_driver.attrib["TargetID"]
    return new_driver

def test_required_attributes(element, required_attributes, required_only_flag, quiet_flag):
    attributes_count = [0] * len(required_attributes)
    for name in element.attrib:
        if name in required_attributes:
            index = required_attributes.index(name)
            if attributes_count[index]:
                print("Error: %s \"%s\" occurs more than once" % (element.tag, name))
                sys.exit(1)
            attributes_count[index] = 1
        else:
            if required_only_flag:
                print("Error: %s \"%s\" not in required attributes list" % (element.tag, name))
                sys.exit(1)
            else:
                if not quiet_flag: print("Warning: %s \"%s\" not in required attributes list" % (element.tag, name))
    if 0 in attributes_count:
        indices = [i for i, x in enumerate(attributes_count) if x == 0]
        for index in indices:
            print("Error: %s \"%s\" attribute not found" % (element.tag, required_attributes[index]))
        sys.exit(1)
    return

def convert_cartesian_plane_to_parametric(a, b, c, d):
    if a != 0:
        point = [ d / a, 0.0, 0.0]
        v1 =    [-b / a, 1.0, 0.0]
        v2 =    [-c / a, 0.0, 1.0]
    elif b != 0:
        point = [0.0,  d / b, 0.0]
        v1 =    [1.0, -a / b, 0.0]
        v2 =    [0.0, -c / b, 1.0]
    elif c != 0:
        point = [0.0, 0.0,  d / c]
        v1 =    [1.0, 0.0, -a / c]
        v2 =    [0.0, 1.0, -b / c]
    else:
        print('Error: Cannot convert plane %dx + %dy + %dz = %d' % (a, b, c, d))
        sys.exit(1)
    return [point, v1, v2]

def fix_cylinder_rotation(str):
    q1 = [float(i) for i in str.split()]
    # old style uses rotation of Z axis, new style uses rotation of X axis
    # should be able to obtain that by post multiplying by an (XtoZ) quaternion
    xtoz = QuaternionFromAxisAngle([0.0, 1.0, 0.0], -90.0 * math.pi / 180.0)
    new_rotation = QuaternionQuaternionMultiply(q1, xtoz)
    return ' '.join(format(x, '.18e') for x in new_rotation)

def strip_world(str):
    tokens = str.split()
    if not is_a_number(tokens[0]):
        if tokens[0] != "World":
            print("Error: \"%s\" should start with \"World\"" % (str))
            sys.exit(1)
        return " ".join(tokens[1:])
    return " ".join(tokens)

def is_a_number(str):
    """checks to see whether a string is a valid number"""
    if re.match(r'^([+-]?)(?=\d|\.\d)\d*(\.\d*)?([Ee]([+-]?\d+))?$', str.strip()) == None:
        return False
    return True

def convert_to_erp_cfm(spring_constant, damping_constant, integration_stepsize):
    h = integration_stepsize
    kp = spring_constant
    kd = damping_constant
    erp = (h * kp) / ((h * kp) + kd)
    cfm = 1.0 / ((h * kp) + kd)
    return (erp, cfm)

def convert_to_spring_and_damping_constants(erp, cfm, integration_stepsize):
    h = integration_stepsize
    kp = erp / (cfm * h)
    kd = (1.0 - erp) / cfm
    spring_constant = kp
    damping_constant = kd
    return (spring_constant, damping_constant)

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

def pretty_print_sys_argv(sys_argv):
    quoted_sys_argv = quoted_if_necessary(sys_argv)
    print((" ".join(quoted_sys_argv)))

def pretty_print_argparse_args(argparse_args):
    for arg in vars(argparse_args):
        print(("%s: %s" % (arg, getattr(argparse_args, arg))))

def quoted_if_necessary(input_list):
    output_list = []
    for item in input_list:
        if re.search(r"[^a-zA-Z0-9_./-]", item):
            item = "\"" + item + "\""
        output_list.append(item)
    return output_list

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

# multiply two vectors element by element
def Mul3x1(a, b):
    c = [0, 0, 0]
    c[0] = a[0] * b[0]
    c[1] = a[1] * b[1]
    c[2] = a[2] * b[2]
    return c

# Divide two vectors element by element
def Div3x1(a, b):
    c = [0, 0, 0]
    c[0] = a[0] / b[0]
    c[1] = a[1] / b[1]
    c[2] = a[2] / b[2]
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

class Matrix:
    def __init__(self,
                 r1c1=0.0, r1c2=0.0, r1c3=0.0,
                 r2c1=0.0, r2c2=0.0, r2c3=0.0,
                 r3c1=0.0, r3c2=0.0, r3c3=0.0):
        self.e11 = r1c1
        self.e12 = r1c2
        self.e13 = r1c3
        self.e21 = r2c1
        self.e22 = r2c2
        self.e23 = r2c3
        self.e31 = r3c1
        self.e32 = r3c2
        self.e33 = r3c3

class Quaternion:
    def __init__(self, nn=1.0, xx=0.0, yy=0.0, zz=0.0):
        self.n = nn
        self.x = xx
        self.y = yy
        self.z = zz

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

def MatrixFromQuaternion(R):
    # quaternion is R[w, x, y, z] (externally)
    # quaternion is q.n q.x qy q.z (internally)
    # matrix is:
    # m.e11 m.e12 m.e13
    # m.e21 m.e22 m.e23
    # m.e31 m.e32 m.e33
    m = Matrix()
    q = Quaternion(R[0], R[1], R[2], R[3])
    qq1 = 2*q.x*q.x
    qq2 = 2*q.y*q.y
    qq3 = 2*q.z*q.z
    m.e11 = 1 - qq2 - qq3
    m.e12 = 2*(q.x*q.y - q.n*q.z)
    m.e13 = 2*(q.x*q.z + q.n*q.y)
    m.e21 = 2*(q.x*q.y + q.n*q.z)
    m.e22 = 1 - qq1 - qq3
    m.e23 = 2*(q.y*q.z - q.n*q.x)
    m.e31 = 2*(q.x*q.z - q.n*q.y)
    m.e32 = 2*(q.y*q.z + q.n*q.x)
    m.e33 = 1 - qq1 - qq2
    return m

def MatrixMul(m1, m2):
    m = Matrix(m1.e11*m2.e11 + m1.e12*m2.e21 + m1.e13*m2.e31,
               m1.e11*m2.e12 + m1.e12*m2.e22 + m1.e13*m2.e32,
               m1.e11*m2.e13 + m1.e12*m2.e23 + m1.e13*m2.e33,
               m1.e21*m2.e11 + m1.e22*m2.e21 + m1.e23*m2.e31,
               m1.e21*m2.e12 + m1.e22*m2.e22 + m1.e23*m2.e32,
               m1.e21*m2.e13 + m1.e22*m2.e23 + m1.e23*m2.e33,
               m1.e31*m2.e11 + m1.e32*m2.e21 + m1.e33*m2.e31,
               m1.e31*m2.e12 + m1.e32*m2.e22 + m1.e33*m2.e32,
               m1.e31*m2.e13 + m1.e32*m2.e23 + m1.e33*m2.e33)
    return m

def MatrixTrans(m):
    mt = Matrix(m.e11,m.e21,m.e31,m.e12,m.e22,m.e32,m.e13,m.e23,m.e33)
    return mt

# program starts here
if __name__ == "__main__":
    convert_old_model()
