#!/usr/bin/python -u

import sys
import os
import subprocess


def generate_graph_paper():
    usageString = ("Generates a PPM file of a 2x2 graph paper pattern\n"
                   "\n"
                   "Usage:\n%s [options] output_file\n"
                   "\n"
                   "Options:\n"
                   "--fine_colour r g b         sets the fine line colour (179,179,255)\n"
                   "--medium_colour r g b       sets the medium line colour (56,56,56)\n"
                   "--coarse_colour r g b       sets the coarse line colour (179,0,0)\n"
                   "--background_colour r g b   sets the background colour (255,255,255)\n"
                   "--dimensions x y            sets the width and height of the PPM (512, 512)\n"
                   "\n"
                   % sys.argv[0])
    
    if len(sys.argv) < 2: PrintExit(usageString)
    
    bare_args = []
    fine_colour = [179,179,255]
    medium_colour = [56,56,56]
    coarse_colour = [179,0,0]
    background_colour = [255,255,255]
    dimensions = [512,512]
    fine_width = 1
    medium_width = 2
    coarse_width = 4
    n_fine = 21
    n_medium = 5
    n_coarse = 3
    
    i = 1
    while i < len(sys.argv):

        if sys.argv[i] == '--fine_colour':
            i = i + 1
            if (i >= len(sys.argv)): PrintExit(usageString)
            fine_colour[0] = int(sys.argv[i])
            i = i + 1
            if (i >= len(sys.argv)): PrintExit(usageString)
            fine_colour[1] = int(sys.argv[i])
            i = i + 1
            if (i >= len(sys.argv)): PrintExit(usageString)
            fine_colour[2] = int(sys.argv[i])
            i = i + 1
            continue

        if sys.argv[i] == '--medium_colour':
            i = i + 1
            if (i >= len(sys.argv)): PrintExit(usageString)
            medium_colour[0] = int(sys.argv[i])
            i = i + 1
            if (i >= len(sys.argv)): PrintExit(usageString)
            medium_colour[1] = int(sys.argv[i])
            i = i + 1
            if (i >= len(sys.argv)): PrintExit(usageString)
            medium_colour[2] = int(sys.argv[i])
            i = i + 1
            continue

        if sys.argv[i] == '--background_colour':
            i = i + 1
            if (i >= len(sys.argv)): PrintExit(usageString)
            background_colour[0] = int(sys.argv[i])
            i = i + 1
            if (i >= len(sys.argv)): PrintExit(usageString)
            background_colour[1] = int(sys.argv[i])
            i = i + 1
            if (i >= len(sys.argv)): PrintExit(usageString)
            background_colour[2] = int(sys.argv[i])
            i = i + 1
            continue

        if sys.argv[i] == '--dimensions':
            i = i + 1
            if (i >= len(sys.argv)): PrintExit(usageString)
            dimensions[0] = int(sys.argv[i])
            i = i + 1
            if (i >= len(sys.argv)): PrintExit(usageString)
            dimensions[1] = int(sys.argv[i])
            i = i + 1
            continue

        bare_args.append(sys.argv[i])
        i = i + 1

    if (len(bare_args) < 1): PrintExit(usageString)
    output_file = bare_args[0]

    x_fine = []
    y_fine = []
    for i in range(0, n_fine):
        x_position = int((float(i) / (n_fine - 1)) * (dimensions[0] - 1) + 0.5)
        x_fine.append(x_position)
        y_position = int((float(i) / (n_fine - 1)) * (dimensions[1] - 1) + 0.5)
        y_fine.append(y_position)

    x_medium = []
    y_medium = []
    for i in range(0, n_medium):
        x_position = int((float(i) / (n_medium - 1)) * (dimensions[0] - 1) + 0.5)
        x_medium.append(x_position)
        y_position = int((float(i) / (n_medium - 1)) * (dimensions[1] - 1) + 0.5)
        y_medium.append(y_position)

    x_coarse = []
    y_coarse = []
    for i in range(0, n_coarse):
        x_position = int((float(i) / (n_coarse - 1)) * (dimensions[0] - 1) + 0.5)
        x_coarse.append(x_position)
        y_position = int((float(i) / (n_coarse - 1)) * (dimensions[1] - 1) + 0.5)
        y_coarse.append(y_position)

    buf = [0] * (dimensions[0] * dimensions[1] * 3)
    for iy in range(0, dimensions[1]):
        for ix in range(0, dimensions[0]):
            buf[iy * dimensions[0] * 3 + ix * 3] = background_colour[0]
            buf[iy * dimensions[0] * 3 + ix * 3 + 1] = background_colour[1]
            buf[iy * dimensions[0] * 3 + ix * 3 + 2] = background_colour[2]
    
    # do the fine lines first (these will get overwritten where necessary)
    for iy in range(0, dimensions[1]):
        for ix in x_fine:
            SetPixel(buf, dimensions[0], dimensions[1], ix, iy, fine_width, fine_colour)
    for ix in range(0, dimensions[0]):
        for iy in y_fine:
            SetPixel(buf, dimensions[0], dimensions[1], ix, iy, fine_width, fine_colour)
            
    # do the medium lines next (these will get overwritten where necessary)
    for iy in range(0, dimensions[1]):
        for ix in x_medium:
            SetPixel(buf, dimensions[0], dimensions[1], ix, iy, medium_width, medium_colour)
    for ix in range(0, dimensions[0]):
        for iy in y_medium:
            SetPixel(buf, dimensions[0], dimensions[1], ix, iy, medium_width, medium_colour)
            
    # finally the coarse lines
    for iy in range(0, dimensions[1]):
        for ix in x_coarse:
            SetPixel(buf, dimensions[0], dimensions[1], ix, iy, coarse_width, coarse_colour)
    for ix in range(0, dimensions[0]):
        for iy in y_coarse:
            SetPixel(buf, dimensions[0], dimensions[1], ix, iy, coarse_width, coarse_colour)
            
    if output_file.lower().endswith('.png'):
        WritePNG(buf, dimensions[0], dimensions[1], 255, output_file)
    else:
        if output_file.lower().endswith('.ppm'):
            WriteASCIIPPM(buf, dimensions[0], dimensions[1], 255, output_file, "Generated by generate_graph_paper.py")
        else:
            PrintExit('Filename must end with .png or .ppm')

def SetPixel(buf, width, height, x, y, line_width, colour):
    # print 'SetPixel ', x, y
    if line_width < 2:
        ix = int(x + 0.5)
        iy = int(y + 0.5)
        if ix >= 0 and ix < width and iy >= 0 and iy < height:
            buf[iy * width * 3 + ix * 3] = colour[0]
            buf[iy * width * 3 + ix * 3 + 1] = colour[1]
            buf[iy * width * 3 + ix * 3 + 2] = colour[2]
    else:
        for ix in range(int(0.5 + x - line_width / 2), int(0.5 + x + line_width / 2)):
            for iy in range(int(0.5 + y - line_width / 2), int(0.5 + y + line_width / 2)):
                if ix >= 0 and ix < width and iy >= 0 and iy < height:
                    buf[iy * width * 3 + ix * 3] = colour[0]
                    buf[iy * width * 3 + ix * 3 + 1] = colour[1]
                    buf[iy * width * 3 + ix * 3 + 2] = colour[2]

def WriteASCIIPGM(buf, width, height, max_val, filename, comment):
    fout = open(filename, 'w')
    if comment:
        comment_lines = comment.split('\n')
        fout.write('P2\n')
        for comment_line in comment_lines:
            if not comment_line.startswith('#'):
                comment_line = '#' + comment_line
            fout.write('%s\n' % comment_line)
        fout.write('%d\n%d\n%d\n' % (width, height, max_val));
    else:
        fout.write('P2\n%d\n%d\n%d\n' % (width, height, max_val));
    i = 0
    for iy in range(0, height):
        for ix in range(0, width - 1):
            fout.write('%d ' % buf[i])
            i = i + 1
        fout.write('%d\n' % buf[i])
        i = i + 1
    fout.close();
    
def WriteBinaryPGM(buf, width, height, max_val, filename, comment):
    fout = open(filename, 'w')
    if comment:
        comment_lines = comment.split('\n')
        fout.write('P5\n')
        for comment_line in comment_lines:
            if not comment_line.startswith('#'):
                comment_line = '#' + comment_line
            fout.write('%s\n' % comment_line)
        fout.write('%d\n%d\n%d\n' % (width, height, max_val));
    else:
        fout.write('P5\n%d\n%d\n%d\n' % (width, height, max_val));
    buf_size = width * height
    binary_image = "";
    for i in range(0, buf_size):
        binary_image += chr(buf[i])
    fout.write(binary_image)
    fout.close();

def WriteASCIIPPM(buf, width, height, max_val, filename, comment):
    fout = open(filename, 'w')
    if comment:
        comment_lines = comment.split('\n')
        fout.write('P3\n')
        for comment_line in comment_lines:
            if not comment_line.startswith('#'):
                comment_line = '#' + comment_line
            fout.write('%s\n' % comment_line)
        fout.write('%d\n%d\n%d\n' % (width, height, max_val));
    else:
        fout.write('P3\n%d\n%d\n%d\n' % (width, height, max_val));
    i = 0
    for iy in range(0, height):
        for ix in range(0, width - 1):
            fout.write('%d %d %d ' % (buf[i], buf[i + 1], buf[i + 2]))
            i = i + 3
        fout.write('%d %d %d\n' % (buf[i], buf[i + 1], buf[i + 2]))
        i = i + 3
    fout.close();
    
def WriteBinaryPPM(buf, width, height, max_val, filename, comment):
    fout = open(filename, 'w')
    if comment:
        comment_lines = comment.split('\n')
        fout.write('P6\n')
        for comment_line in comment_lines:
            if not comment_line.startswith('#'):
                comment_line = '#' + comment_line
            fout.write('%s\n' % comment_line)
        fout.write('%d\n%d\n%d\n' % (width, height, max_val));
    else:
        fout.write('P6\n%d\n%d\n%d\n' % (width, height, max_val));
    buf_size = width * height * 3
    binary_image = "";
    for i in range(0, buf_size):
        binary_image += chr(buf[i])
    fout.write(binary_image)
    fout.close();

def WritePNG(buf, width, height, max_val, filename):
    if which('ffmpeg') == None:
        PrintExit('ffmpeg needed on PATH to write PNG files')
    path, ext = os.path.splitext(filename)
    ppm_file = path + '.ppm'
    png_file = path + '.png'
    fout = open(ppm_file, 'w')
    fout.write('P6\n%d\n%d\n%d\n' % (width, height, max_val));
    buf_size = width * height * 3
    binary_image = "";
    for i in range(0, buf_size):
        binary_image += chr(buf[i])
    fout.write(binary_image)
    fout.close();
    command = ['ffmpeg','-i',ppm_file, png_file]
    print command
    subprocess.call(command)
    os.remove(ppm_file)

def PrintExit(value):
    sys.stderr.write(str(value) + "\n");
    sys.exit(1)

def which(program):
    def is_exe(fpath):
        return os.path.isfile(fpath) and os.access(fpath, os.X_OK)

    fpath, fname = os.path.split(program)
    if fpath:
        if is_exe(program):
            return program
    else:
        for path in os.environ["PATH"].split(os.pathsep):
            path = path.strip('"')
            exe_file = os.path.join(path, program)
            if is_exe(exe_file):
                return exe_file

    return None
# program starts here

if __name__ == '__main__':
    generate_graph_paper()
