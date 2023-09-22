#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import re
import xml.etree.ElementTree
import argparse
import difflib
import copy
import math

def model_symmetry_checker():

    parser = argparse.ArgumentParser(description="Tests a model for symmetry. Always uses the left side as the primary reference.")

    parser.add_argument("-l", "--left_side_regex_find", default='(.*)left(.*)', help="regex pattern identifying left side items [(.*)left(.*)]")
    parser.add_argument("-r", "--right_side_regex_replace", default='\\1right\\2', help="regex pattern to generate the right side names via re.sub [\\1right\\2]")
    parser.add_argument("-i", "--input_xml_file", required=True, help="file name for input XML config file")
    parser.add_argument("-o", "--output_xml_file", help="file name for output corrected XML config file")
    parser.add_argument("-om", "--output_mapping_file", help="file name for output name mapping file")
    parser.add_argument("-ft", "--fix_tags", nargs='+', help="list of tags to fix in the output corrected XML config file")
    parser.add_argument("-fa", "--fix_attributes", nargs='+', help="list of attributes to fix in the output corrected XML config file")
    parser.add_argument("-m", "--mirror_axis", default=1, type=int, help="mirror axis x=0 y=1 z=2 [1]")
    parser.add_argument("-p", "--replace_list", nargs='+', help="search and replace list using whole words in pairs (e.g. trunk torso thigh femur). If this is a filename then read from file.")
    parser.add_argument("-rt", "--relative_tolerance", default=0.001, type=float, help="relative tolerance value (0.001 is 0.1%%) [0.001]")
    parser.add_argument("-rtt", "--relative_tolerance_threshold", default=1e-10, type=float, help="any magnitude below this value and switch to absolute comparison [1e-10]")
    parser.add_argument("-at", "--absolute_tolerance", default=1e-8, type=float, help="absolute tolerance value used for small values [1e-8]")
    parser.add_argument("-s", "--string_match_ratio", default=0.9, type=float, help="difflib SequenceMatcher ratio threshold [0.9]")
    parser.add_argument("-c", "--case_insensitive", action="store_true", help="use case insensitive ID lookup")
    parser.add_argument("-n", "--numbers_only", action="store_true", help="only compare numbers and not strings")
    parser.add_argument("-f", "--force", action="store_true", help="force overwrite of destination file")
    parser.add_argument("-v", "--verbose", action="store_true", help="write out more information whilst processing")
    args = parser.parse_args()

    if args.verbose:
        pretty_print_sys_argv(sys.argv)
        pretty_print_argparse_args(args)

    tolerance = (args.relative_tolerance, args.relative_tolerance_threshold, args.absolute_tolerance)

    preflight_read_file(args.input_xml_file, args.verbose)
    if args.output_xml_file: preflight_write_file(args.output_xml_file, args.force, args.verbose)
    if args.output_mapping_file: preflight_write_file(args.output_mapping_file, args.force, args.verbose)

    if args.verbose: print('Reading "%s"' % (args.input_xml_file))
    tree = xml.etree.ElementTree.parse(args.input_xml_file)
    root = tree.getroot()

    if args.case_insensitive:
        lower_case_ids(root)

    if args.replace_list:
        if len(args.replace_list) == 1:
            preflight_read_file(args.replace_list[0], args.verbose)
            if args.verbose: print('Reading "%s"' % (args.replace_list[0]))
            with open(args.replace_list[0]) as f_in:
                tokens = f_in.read().split()
                if len(tokens) < 2:
                    print('Error: replace list file must contain at least 2 entries')
                    sys.exit()
            args.replace_list = tokens
        if len(args.replace_list) % 2:
            print('Error: replace list must have an even number of entries')
            sys.exit()
        if args.case_insensitive: args.replace_list = [s.lower() for s in args.replace_list]
        count = replace_ids(root, args.replace_list, args.verbose)
        if args.verbose: print('Information: %d manual replacements made' % (count))

    input_text = str(xml.etree.ElementTree.tostring(root, encoding="utf-8", method="xml"), "utf-8")

    item_list = {}
    id_list = []
    child_list = []
    for child in root:
        if 'ID' in child.attrib:
            item_list[child.attrib["ID"]] = copy.deepcopy(child)
            id_list.append(child.attrib["ID"])
            child_list.append(copy.deepcopy(child))

    # first check for non sided items
    left_sided_items = {}
    right_sided_hopefuls = {}
    left_sided_keys = {}
    right_sided_keys = {}
    non_sided_ids = []

    # get a list of left items and hopeful right items
    for item_id in item_list:
        if re.search(args.left_side_regex_find, item_id):
            left_id = item_id
            left_sided_items[left_id] = item_list[left_id]
            right_id = re.sub(args.left_side_regex_find, args.right_side_regex_replace, left_id)
            right_sided_hopefuls[right_id] = left_id

    (changed_ids, changed_ids_scores) = find_close_match_list(right_sided_hopefuls, item_list)
    for best_match in changed_ids:
        if changed_ids_scores[best_match] >= args.string_match_ratio:
            left_id = right_sided_hopefuls[changed_ids[best_match]]
            right_id = best_match
            left_sided_keys[left_id] = right_id
            right_sided_keys[right_id] = left_id
            if args.verbose and changed_ids_scores[best_match] != 1.0:
                print('Warning: Right side ID="%s" fuzzy matched to "%s"' % (changed_ids[best_match], best_match))
    for key, value in changed_ids_scores.items():
        if value < args.string_match_ratio:
            del changed_ids[key]

    if args.output_mapping_file:
        if args.verbose: print('Writing "%s"' % (args.output_mapping_file))
        with open(args.output_mapping_file, 'w') as out_f:
            out_f.write('%s\t%s\t%s\n' % ('best_match', 'search_string', 'ratio'))
            for key, value in changed_ids.items():
                out_f.write('%s\t%s\t%f\n' % (key, value, changed_ids_scores[key]))

    for item_id in item_list:
        if not (item_id in left_sided_keys or item_id in right_sided_keys):
            non_sided_ids.append(item_id)
            if args.verbose: print('Information: Unsided TAG "%s" ID="%s" count=%d' % (item_list[item_id].tag, item_id, count_words(item_id, input_text)))

    for id_string in id_list:
        if id_string in left_sided_keys:
            left_id = id_string
            left_item = item_list[left_id]
            right_item = item_list[left_sided_keys[left_id]]
            test_item(left_item, right_item, tolerance, args.verbose, args.numbers_only, args.mirror_axis)

    if not args.output_xml_file:
        return

    new_tree = xml.etree.ElementTree.Element(root.tag)
    new_tree.text = '\n'

    if args.fix_tags and args.fix_attributes:
        for id_string in id_list:
            if item_list[id_string].tag in args.fix_tags:
                if id_string in left_sided_keys:
                    left_id = id_string
                    left_item = item_list[left_id]
                    right_item = item_list[left_sided_keys[left_id]]
                    fix_item(left_item, right_item, args.verbose, args.mirror_axis, args.fix_attributes)
                    new_tree.append(left_item)
                    new_tree.append(right_item)
                    continue

                if id_string in right_sided_keys:
                    continue

            new_tree.append(item_list[id_string])

    apply_changed_ids(new_tree, changed_ids)

    if args.verbose: print('Writing "%s"' % (args.output_xml_file))
    with open(args.output_xml_file, 'wb') as out:
        xml_text = xml.etree.ElementTree.tostring(new_tree, encoding="utf-8", method="xml")
        out.write(xml_text)

def count_words(word_to_find, src_string):
    match = re.findall('\\b' + word_to_find + '\\b', src_string)
    return len(match)

def flip_dictionary(input_dict):
    output_dict = {}
    for key, value in input_dict.items():
        if value not in output_dict:
            output_dict[value] = [key]
        else:
            output_dict[value].append(key)
    return output_dict

def replace_ids(tree, input_replace_list, verbose):
    count = 0
    search_list = []
    replace_list = []
    for i in range(0, len(input_replace_list), 2):
        search_list.append(input_replace_list[i])
        replace_list.append(input_replace_list[i + 1])
    for child in tree:
        if 'ID' in child.attrib: original_id = child.attrib['ID']
        for attrib in child.attrib:
            if attrib.find('ID') != -1:
                tokens = child.attrib[attrib].split()
                for i in range(0, len(tokens)):
                    if tokens[i] in search_list:
                        if verbose: print('%s %s %s[%d]: Replacing "%s" with "%s"' % (child.tag, original_id, attrib, i, tokens[i], replace_list[search_list.index(tokens[i])]))
                        tokens[i] = replace_list[search_list.index(tokens[i])]
                        count = count + 1
                child.attrib[attrib] = ' '.join(tokens)

        # special treatment for markers
        if child.tag == 'MARKER':
            position = child.attrib['Position'].split()
            if position[0] in search_list:
                position[0] = replace_list[search_list.index(position[0])]
                count = count + 1
            child.attrib['Position'] = ' '.join(position)
            quaternion = child.attrib['Quaternion'].split()
            if quaternion[0] in search_list:
                quaternion[0] = replace_list[search_list.index(quaternion[0])]
                count = count + 1
            child.attrib['Quaternion'] = ' '.join(quaternion)
    return count

def lower_case_ids(tree):
    for child in tree:
        # change the IDs to lower case
        for attrib in child.attrib:
            if attrib.find('ID') != -1:
                tokens = child.attrib[attrib].split()
                for i in range(0, len(tokens)):
                    tokens[i] = tokens[i].lower()
                child.attrib[attrib] = ' '.join(tokens)

        # special treatment for markers
        if child.tag == 'MARKER':
            if child.attrib['BodyID'] == 'world':
                child.attrib['BodyID'] = 'World'
            position = child.attrib['Position'].split()
            position[0] = position[0].lower()
            child.attrib['Position'] = ' '.join(position)
            quaternion = child.attrib['Quaternion'].split()
            quaternion[0] = quaternion[0].lower()
            child.attrib['Quaternion'] = ' '.join(quaternion)

def apply_changed_ids(tree, changed_ids):
    for child in tree:
        for attrib in child.attrib:
            if attrib.find('ID') != -1:
                tokens = child.attrib[attrib].split()
                for i in range(0, len(tokens)):
                    if tokens[i] in changed_ids:
                        tokens[i] = changed_ids[tokens[i]]
                child.attrib[attrib] = ' '.join(tokens)

        # special treatment for markers
        if child.tag == 'MARKER':
            position = child.attrib['Position'].split()
            if position[0] in changed_ids:
                position[0] = changed_ids[position[0]]
            child.attrib['Position'] = ' '.join(position)
            quaternion = child.attrib['Quaternion'].split()
            if quaternion[0] in changed_ids:
                quaternion[0] = changed_ids[quaternion[0]]
            child.attrib['Quaternion'] = ' '.join(quaternion)

def find_close_match_list(input_str_list, str_list):
    mapped_strings = {}
    mapped_strings_scores = {}
    strings_to_match = set(input_str_list)
    strings_matched = set()
    # first do some exact matching
    for input_str in strings_to_match:
        if input_str in str_list:
            best_score = 1.0
            best_match = input_str
            best_matched = input_str
            mapped_strings[best_match] = best_matched
            mapped_strings_scores[best_match] = best_score
    for mapped_string in mapped_strings:
        strings_to_match.discard(mapped_string)
        strings_matched.add(mapped_string)
    # now handle the unmatched strings
    while len(strings_to_match):
        best_score = -sys.float_info.max
        best_match = ''
        best_matched = ''
        for input_str in strings_to_match:
            for test_str in str_list:
                if not test_str in strings_matched:
                    score = difflib.SequenceMatcher(None, input_str, test_str).ratio()
                    if score > best_score:
                        best_score = score
                        best_match = test_str
                        best_matched = input_str
        mapped_strings[best_match] = best_matched
        mapped_strings_scores[best_match] = best_score
        strings_to_match.discard(input_str)
        strings_matched.add(best_match)
    return (mapped_strings, mapped_strings_scores)

def find_close_match(input_str, str_list):
    best_match = ''
    best_score = -sys.float_info.max
    unique_solution = True
    for test_str in str_list:
        score = difflib.SequenceMatcher(None, input_str, test_str).ratio()
        if score > best_score:
            best_score = score
            best_match = test_str
            unique_solution = True
            continue
        if score == best_score:
            unique_solution = False
    return (best_match, best_score, unique_solution)

def fix_item(left, right, verbose, mirror_axis, fix_attributes):
    if verbose: print('Fixing ID="%s" to ID="%s"' % (left.attrib['ID'], right.attrib['ID']))
    mirror_offset = {'Position':[mirror_axis], 'Quaternion':[mirror_axis + 1], 'ConstructionPosition':[mirror_axis]}
    if mirror_axis == 0: mirror_offset['MOI'] = [3, 4]
    if mirror_axis == 1: mirror_offset['MOI'] = [3, 5]
    if mirror_axis == 2: mirror_offset['MOI'] = [4, 5]
    for attr in left.attrib:
        if not attr in fix_attributes:
            continue
        left_tokens = left.attrib[attr].split()
        right_tokens = right.attrib[attr].split()
        if attr in mirror_offset:
            if is_a_number(left_tokens[0]): token_skip = 0
            else: token_skip = 1
            for i in mirror_offset[attr]:
                left_tokens[i + token_skip] = negate_string(left_tokens[i + token_skip])
        for i in range(0, len(left_tokens)):
            if is_a_number(left_tokens[i]):
                right_tokens[i] = left_tokens[i]
        right.attrib[attr] = ' '.join(right_tokens)

def test_item(left, right, tolerance, verbose, numbers_only, mirror_axis):
    if verbose: print('Comparing ID="%s" to ID="%s"' % (left.attrib['ID'], right.attrib['ID']))
    exclude = {'Colour1','Colour2','Colour3','Size1','Size2','Size3','ID','GraphicFile1','GraphicFile2','GraphicFile3','WorldPosition','WorldQuaternion'}
    mirror_offset = {'Position':[mirror_axis], 'Quaternion':[mirror_axis + 1], 'ConstructionPosition':[mirror_axis]}
    if mirror_axis == 0: mirror_offset['MOI'] = [3, 4]
    if mirror_axis == 1: mirror_offset['MOI'] = [3, 5]
    if mirror_axis == 2: mirror_offset['MOI'] = [4, 5]
    errors_found = False
    for attr in left.attrib:
        if attr in exclude:
            continue
        if attr in mirror_offset:
            messages = test_equal(left, right, tolerance, attr, numbers_only, mirror_offset[attr])
        else:
            messages = test_equal(left, right, tolerance, attr, numbers_only)
        if messages:
            print('\n'.join(messages))
            errors_found = True
    return errors_found

def test_equal(left, right, tolerance, attrib, numbers_only, negate_index, left_side_regex_find, right_side_regex_replace): # text test does not do a left/right search and replace like it should
    messages = []
    left_id = left.attrib['ID']
    right_id = right.attrib['ID']
    if left.tag != right.tag:
        messages.append('left tag "%s" not equal to right tag "%s"' % (left.tag, right.tag))
        return messages
    if not attrib in left.attrib:
        messages.append('%s attrib "%s" not found in ID="%s"\n' % (left.tag, attrib, left_id))
    if not attrib in right.attrib:
        messages.append('%s attrib "%s" not found in ID="%s"' % (right.tag, attrib, right_id))
    if messages: return messages
    tokens1 = left.attrib[attrib].split()
    tokens2 = right.attrib[attrib].split()
    if len(tokens1) != len(tokens2):
        messages.append('%s "%s": "%s" has %d tokens; "%s" has %d tokens' % (left.tag, attrib, left_id, len(tokens1), right_id, len(tokens2)))
        return messages
    for i in range(0, len(tokens1)):
        token1 = tokens1[i]
        token2 = tokens2[i]
        if is_a_number(token1) != is_a_number(token2):
            messages.append('Number/String mismatch%s %s[%d]: "%s" "%s" != "%s" "%s"' % (left.tag, attrib, i, left_id, token1, right_id, token2))
        if not is_a_number(token1): # both strings
            if numbers_only: continue
            test_right = re.sub(left_side_regex_find, right_side_regex_replace, token1)
            if test_right != token2:
                messages.append('String mismatch%s %s[%d]: "%s" "%s" != "%s" "%s"' % (left.tag, attrib, i, left_id, token1, right_id, token2))
            continue
        x1 = float(token1)
        x2 = float(token2)
        if i in negate_index:
            if not isclose_rel_abs(x1, -x2, tolerance):
                messages.append('%s %s[%d]: "%s" %g != "%s" %g * -1' % (left.tag, attrib, i, left_id, x1, right_id, x2))
        else:
            if not isclose_rel_abs(x1, x2, tolerance):
                messages.append('%s %s[%d]: "%s" %g != "%s" %g' % (left.tag, attrib, i, left_id, x1, right_id, x2))
    return messages

def isclose_rel_abs(a, b, tolerance):
    (relative_tolerance, relative_tolerance_threshold, absolute_tolerance) = tolerance
    if math.fabs(a) < relative_tolerance_threshold or math.fabs(b) < relative_tolerance_threshold:
        if math.fabs(a - b) < absolute_tolerance: return True
        else : return False
    if a * b < 0: # only happens if the signs are different or one value is zero
        if math.fabs(a - b) < absolute_tolerance: return True
        else : return False
    # signs must be the same and magnitudes >= than relative_tolerance_threshold
    if math.fabs(1 - a / b) < relative_tolerance: return True
    else: return False

def isclose_dp(a, b, dp):
    match_dp = dp_compare(a, b)
    if match_dp >= dp:
        return True
    return False

def dp_compare(a, b):
    if math.fabs(a) < 1e-90 and math.fabs(b) < 1e-90:
        return 90
    a_str = format(a, '.99f')
    b_str = format(b, '.99f')
    dp = 0
    for i in range(0, len(a_str)):
        if dp == 0 and a_str[i] in '-.0' and b_str[i] in '-.0' and a_str[i] == b_str[i]:
            continue
        if a_str[i] != b_str[i]:
            return dp
        dp = dp + 1
    return dp

def negate_string(inp_str):
    out_str = inp_str.strip()
    if out_str.startswith('-'):
        return out_str[1:]
    if out_str.startswith('+'):
        return '-' + out_str[1:]
    return '-' + out_str

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
    model_symmetry_checker()
