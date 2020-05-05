#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import argparse
import subprocess
import re
import shlex

def movies_to_ppt_compatible_mp4():
    parser = argparse.ArgumentParser(description='Convert movie files to PowerPoint compatible mp4')
    parser.add_argument('items', type=str, nargs='+', help='list of files')
    parser.add_argument('-m', '--ffmpeg', type=str, default='ffmpeg', help='name of the ffmpeg executable [ffmpeg]')
    parser.add_argument('-b', '--ffprobe', type=str, default='ffprobe', help='name of the ffprobe executable [ffprobe]')
    parser.add_argument('-i', '--file', type=str, default='file', help='name of the file executable [file]')
    parser.add_argument('-r', '--regex', type=str, default='movie|video', help='movie file ID regexp [movie|video]')
    parser.add_argument('-a', '--args', type=str, default='', help='extra arguments to pass to ffmpeg')
    parser.add_argument('-s', '--suffix', type=str, default='_recompressed', help='suffix to add to file name for output')
    parser.add_argument('-l', '--log_file_name', type=str, required=False, default='', help='Name of log file []')
    parser.add_argument('-f', '--force', action='store_true', help='Force overwrite of files')
    parser.add_argument('-v', '--verbose', action='store_true', help='Print progress in more detail')
    args = parser.parse_args()

    if (len(args.log_file_name) > 0):
        # enable some logging
        so = se = open(args.log_file_name, 'w', 0)
        # re-open stdout without buffering
        sys.stdout = os.fdopen(sys.stdout.fileno(), 'w', 0)
        # redirect stdout and stderr to the log file opened above
        os.dup2(so.fileno(), sys.stdout.fileno())
        os.dup2(se.fileno(), sys.stderr.fileno())

    if args.verbose:
        pretty_print_sys_argv(sys.argv)
        pretty_print_argparse_args(args)


    # check files
    for current_file in args.items:
        if args.verbose:
            print('Testing %s\n' % (current_file))
        file_type = subprocess.check_output([args.file, current_file])
        if args.verbose:
            print('File type is %s\n' % (file_type))
        if re.search(args.regex, file_type):
            print('File %s type matches %s\n' % (current_file, args.regex))
            media_data = subprocess.check_output([args.ffprobe, '-i', current_file], stderr=subprocess.STDOUT)
            video_codec = 'copy'
            audio_codec = 'copy'
            if re.search('h264', media_data) == None:
                video_codec = 'libx264'
            if re.search('aac', media_data) == None:
                audio_codec = 'aac -strict -2'
            if video_codec == 'copy' and audio_codec == 'copy':
                print('%s already in correct format\n' % (current_file))
            else:
                (path, extension) = os.path.splitext(current_file)
                new_movie_path = path + args.suffix + '.mp4'
                if (args.args):
                    command = [args.ffmpeg, '-i', current_file, '-acodec', audio_codec, '-vcodec', video_codec, '-pix_fmt', 'yuv420p', '-crf', '17', shlex.split(args.args), new_movie_path]
                else:
                    command = [args.ffmpeg, '-i', current_file, '-acodec', audio_codec, '-vcodec', video_codec, '-pix_fmt', 'yuv420p', '-crf', '17', new_movie_path]
                if args.verbose: pretty_print_sys_argv(command)
                subprocess.call(command)


def pretty_print_sys_argv(sys_argv):
    quoted_sys_argv = quoted_if_necessary(sys_argv)
    print(' '.join(quoted_sys_argv))

def pretty_print_argparse_args(argparse_args):
    for arg in vars(argparse_args):
        print('%s: %s' % (arg, getattr(argparse_args, arg)))

def quoted_if_necessary(input_list):
    output_list = []
    for item in input_list:
        if re.search('[^a-zA-Z0-9_-.]', item):
            item = '"' + item + '"'
        output_list.append(item)
    return output_list

# program starts here

if __name__ == '__main__':
    movies_to_ppt_compatible_mp4()

