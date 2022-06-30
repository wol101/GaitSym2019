#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import argparse
import re
import scipy
import scipy.optimize
import json
import GaitSym2019

# this line means that all the math functions do not require the math. prefix
from math import *

def scipy_bfgs_minimise_embedded():

    parser = argparse.ArgumentParser(description="Use various scipy.optimize routines on a gaitsym problem (minimise score)")
    parser.add_argument("-i", "--input_xml_file", required=True, help="GaitSym config file with substitution indicators")
    parser.add_argument("-o", "--output_xml_file", required=True, help="GaitSym config file with best solution substituted")
    parser.add_argument("-r", "--ranges_file", required=True, help="Ranges file")
    parser.add_argument("-l", "--log_file", default="", help="If set log to the specified file")
    parser.add_argument("-m", "--max_workers", type=int, help="Override the default number of worker threads")
    parser.add_argument("-j", "--json_string", default="", help="Use this JSON string to set the arguments required or read from this file")
    parser.add_argument("-n", "--negate_score", action="store_true", help="Negate the score")
    parser.add_argument("-p", "--parallel", action="store_true", help="Use the parallel version of the optimiser")
    parser.add_argument("-f", "--force", action="store_true", help="Force overwrite of existing files")
    parser.add_argument("-v", "--verbose", action="store_true", help="Write out more information whilst processing")
    args = parser.parse_args()

    if len(args.log_file) > 0:
        preflight_write_file(args.log_file, args.force, args.verbose)
        # enable some logging (extra argument turns off buffering)
        so = se = open(args.log_file, 'wb', 0)
        # redirect stdout and stderr to the log file opened above
        os.dup2(so.fileno(), sys.stdout.fileno())
        os.dup2(se.fileno(), sys.stderr.fileno())

    if args.verbose:
        pretty_print_sys_argv(sys.argv)
        pretty_print_argparse_args(args)
    
    # preflight
    preflight_read_file(args.input_xml_file, args.verbose)
    preflight_read_file(args.ranges_file, args.verbose)
    preflight_write_file(args.output_xml_file, args.force, args.verbose)
        
    if os.path.isfile(args.json_string): # if json_string is a valid file then read its contents
        with open(args.json_string, 'r', encoding='utf8') as f_in:
            data = f_in.read()
        args.json_string = data
    
    if args.json_string and args.json_string.strip():
        try:
            json_object = json.loads(args.json_string)
        except ValueError as e:
            print('"%s" is neither a valid file nor a valid JSON string' % (args.json_string))
            sys.exit(1)
        if not isinstance(json_object, dict):
            print('Error interpreting JSON string "%s" as dict' % (args.json_string))
            sys.exit(1)

    if args.verbose: print('Reading "%s"' % (args.ranges_file))
    ranges_dict = read_ranges(args.ranges_file)
    
    # read the config file
    if args.verbose: print('Reading "%s"' % (args.input_xml_file))
    with open(args.input_xml_file, 'r', encoding='utf8') as f_in:
        data = f_in.read()
    (smart_substitution_text_components, smart_substitution_parser_text) = create_smart_substitution(data)
    if args.verbose: print(f'{len(smart_substitution_text_components) = }')
    if args.verbose: print(f'{smart_substitution_parser_text = }')
    
    extra_args = []
    extra_args.append(smart_substitution_text_components)
    extra_args.append(smart_substitution_parser_text)
    extra_args.append(args.verbose)
    extra_args.append(args.negate_score)
    extra_args.append("")

    x0 = do_l_bgfs_b(ranges_dict, args.json_string, extra_args, args.verbose, args.parallel, args.max_workers)
    
    if len(args.output_xml_file) > 0 and len(x0) > 0:
        if args.verbose: print('Writing "%s"' % (args.output_xml_file))
        extra_args = []
        extra_args.append(smart_substitution_text_components)
        extra_args.append(smart_substitution_parser_text)
        extra_args.append(args.verbose)
        extra_args.append(args.negate_score)
        extra_args.append(args.output_xml_file)
        evaluate_fitness(x0, extra_args)

def do_l_bgfs_b(ranges_dict, json_string, extra_args, verbose, parallel, max_workers):

    x0_list = []
    bounds_list = []
    for i in range(0, len(ranges_dict['estimate'])):
        x0_list.append(ranges_dict['estimate'][i])
        bounds_list.append((ranges_dict['low'][i], ranges_dict['high'][i]))
    print(bounds_list)
    # note: null in a JSON string gets mapped to python None
    default_json = '{"disp": null, "maxcor": 10, "ftol": 2.220446049250313e-09, "gtol": 1e-05, "eps": 1e-08, "maxfun": 15000, "maxiter": 15000, "iprint": -1, "maxls": 20, "finite_diff_rel_step": null}'
    if not json_string or not json_string.strip(): json_string = default_json
    parameters_dict = json.loads(json_string)
    if not isinstance(parameters_dict, dict):
        print('Error interpreting JSON string "%s" as dict' % (json_string))
        sys.exit(1)
    check_dict(parameters_dict, default_json)
    
    if verbose:
        parameters_dict['disp'] = True

    if parallel:
        if max_workers:
            res = minimize_parallel(fun=evaluate_fitness_parallel, x0=x0_list, args=extra_args, bounds=bounds_list, parallel={"max_workers": max_workers})
        else:
            res = minimize_parallel(fun=evaluate_fitness_parallel, x0=x0_list, args=extra_args, bounds=bounds_list)
    else:
        res = scipy.optimize.minimize(fun=evaluate_fitness, x0=x0_list, args=extra_args, bounds=bounds_list, method='L-BFGS-B')

    if verbose: print(res)
    return res.x
    
def evaluate_fitness_parallel(x, arg0, arg1, arg2, arg3, arg4):
    args = (arg0, arg1, arg2, arg3, arg4)
    return evaluate_fitness(x, args)

def evaluate_fitness(x, args):
    # create the file to assess
    smart_substitution_text_components = args[0]
    smart_substitution_parser_text = args[1]
    verbose = args[2]
    negate_score = args[3]
    output_filename = args[4]
    data_list = [smart_substitution_text_components[0]]
    for i in range(0, len(smart_substitution_parser_text)):
        if verbose: print(f'{smart_substitution_parser_text[i] = }')
        v = parse_insert(smart_substitution_parser_text[i], x, verbose)
        # v = eval(smart_substitution_parser_text[i], global_dict)
        data_list.append('%.17e' % (v))
        data_list.append(smart_substitution_text_components[i + 1])
    file_contents = ''.join(data_list)
    if output_filename:
        with open(output_filename, 'w') as f_out:
            f_out.write(file_contents)
    
    gaitsym = GaitSym2019.GaitSym2019()
    err = gaitsym.SetXML(file_contents)
    if err:
        print('Error setting XML file')
        sys.exit(1)
    err = gaitsym.Run()
    if err:
        print('Error running GaitSym2019')
        if verbose: print(file_contents)
        sys.exit(1)
    fitness = gaitsym.GetFitness()
    if negate_score:
        fitness = -fitness
    if verbose: print('fitness = ', fitness)

    return fitness

def check_dict(input_dict, example_json):
    example_dict = json.loads(example_json)
    for key in example_dict.keys():
        if not key in input_dict.keys():
            print('Key "%s" missing from JSON string. Typical example might be\n\'%s\'' % (key, example_json))
            sys.exit(1)
    for key in input_dict.keys():
        if not key in example_dict.keys():
            print('Warning key "%s" not used' % (key))
    return

def parse_insert(insert_string, genes, verbose):    
    # this routine is a little fragile because exprtk and python do not use quite the same syntax
    # we try to fix things up in a rather dumb way that won't work in all cases
    # most of the math functions come from the "from math import *" line so that the
    # python eval statement can do all the work
    # but I could probabl;y do something cleverer by spcifying globals and locals to the 
    # eval statement (but the globals() would be needed for all the math functions)

    # first step is to substitute the genes
    match = re.search(r'g *\(', insert_string)
    if match:
        closing_bracket = find_unmatched_close_bracket(insert_string[match.end():])
        if closing_bracket == -1:
            print('Error parsing genes in %s' % (insert_string))
            sys.exit(1)
        index = int(0.5 + parse_insert(insert_string[match.end(): match.end() + closing_bracket], genes, verbose))
        prefix = insert_string[:match.start()]
        if match.end() + closing_bracket + 1 < len(insert_string):
            suffix = insert_string[match.end() + closing_bracket + 1:]
        else:
            suffix = ''
        return parse_insert('%s%.18e%s' % (prefix, genes[index], suffix), genes, verbose)

    # second step is to handle functions not understood by python
    # if(test,result_when_true,result_when_false)
    match = re.search(r'if *\(', insert_string)
    if match:
        closing_bracket = find_unmatched_close_bracket(insert_string[match.end():])
        if closing_bracket == -1:
            print('Error parsing genes in %s' % (insert_string))
            sys.exit(1)
        argument_string = insert_string[match.end(): match.end() + closing_bracket]
        arguments = split_on_unbracketed_command(argument_string)
        if len(arguments) != 3:
            print('if() requires 3 arguments %s' % (insert_string))
            sys.exit(1)
        test_value = parse_insert(arguments[0], genes, verbose)
        if test_value == 0:
            result = arguments[2]
        else:
            result = arguments[1]
        prefix = insert_string[:match.start()]
        if match.end() + closing_bracket + 1 < len(insert_string):
            suffix = insert_string[match.end() + closing_bracket + 1:]
        else:
            suffix = ''
        return s('%s%s%s'% (prefix, result, suffix), genes, verbose)

    # exprtk uses ^ when python uses **
    insert_string = insert_string.replace('^', '**')
    
    # we could now handle the other operators in order of precidence unaries, **^, */%, +-, booleans 
    # but there are a few gotchas (e.g. identifying unary operators) and python can now handle this
    if verbose: print('eval("%s")' % (insert_string))
    ret_val = eval(insert_string)
    return ret_val

def find_unmatched_close_bracket(input_string):
    num_brackets = 0
    for i in range(0, len(input_string)):
        if input_string[i] == '(':
            num_brackets = num_brackets + 1
        if input_string[i] == ')':
            num_brackets = num_brackets - 1
        if num_brackets < 0:
            return i
    return -1

def split_on_unbracketed_command(input_string):
    num_brackets = 0
    split_string = []
    last_string_start = 0
    for i in range(0, len(input_string)):
        if input_string[i] == '(':
            num_brackets = num_brackets + 1
        if input_string[i] == ')':
            num_brackets = num_brackets - 1
        if num_brackets < 0:
            print('Unmatched bracket found in "%s"' % (input_string))
            sys.exit(1)
        if num_brackets == 0 and input_string[i] == ',':
            split_string.append(input_string[last_string_start: i])
            last_string_start = i + 1
    if last_string_start >= i:
        split_string.append('')
    else:
        split_string.append(input_string[last_string_start:])
    return split_string

def create_smart_substitution(data):
    smart_substitution_text_components = []
    smart_substitution_parser_text = []
    ptr1 = 0
    ptr2 = data.find("[[")
    if ptr2 == -1:
        print("Error: could not find any [[\n")
        sys.exit(1)
    while True:
        # print(ptr1, ptr2)
        smart_substitution_text_components.append(data[ptr1: ptr2])
        ptr2 += 2
        ptr1 = data[ptr2:].find("]]")
        if ptr1 == -1:
            print("Error: could not find matching ]]\n")
            sys.exit(1)
        ptr1 += ptr2
        smart_substitution_parser_text.append(data[ptr2: ptr1])
        ptr1 += 2
        ptr2 = data[ptr1:].find("[[")
        if ptr2 == -1:
            break
        ptr2 += ptr1
    smart_substitution_text_components.append(data[ptr1:])
    for i in range(0, len(smart_substitution_parser_text)): # convert g[] to g()
        smart_substitution_parser_text[i] = re.sub(r'g\[(\d+)\]', r'g(\1)', smart_substitution_parser_text[i])
    return (smart_substitution_text_components, smart_substitution_parser_text)

def read_ranges(file_name):
    ranges = {}
    with open(file_name, 'r', encoding='utf8') as f_in:
        lines = f_in.read().splitlines()
    if len(lines) == 0:
        print('Error: could not parse "%s"' % (file_name))
        sys.exit(1)
    column_names = lines[0].split()
    if len(column_names) == 0:
        print('Error: could not parse column_names in "%s"' % (file_name))
        sys.exit(1)
    for column_name in column_names:
        ranges[column_name] = []
    
    for i in range(1, len(lines)):
        tokens = lines[i].split()
        if len(tokens) == 0: # skip blank lines
            continue
        if len(tokens) != len(column_names):
            print('Error: could not parse data in "%s"' % (file_name))
            sys.exit(1)
        for j in range(0, len(tokens)):
            ranges[column_names[j]].append(float(tokens[j]))
    return ranges
    

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

"""
A parallel version of the L-BFGS-B optimizer of `scipy.optimize.minimize()`.

Using it can significantly reduce the optimization time. For an objective
function with p parameters the optimization speed increases by up to
factor 1+p, when no analytic gradient is specified and 1+p processor cores
with sufficient memory are available.

Functions
---------
- minimize_parallel: parallel version of `scipy.optimize.minimize()`
- optimParallel: same as `minimize_parallel()`
- optimparallel: same as `minimize_parallel()`
- fmin_l_bfgs_b_parallel: parallel version of `scipy.optimize.fmin_l_bfgs_b()`
"""

import warnings
import concurrent.futures
import functools
import itertools
import numpy as np
from scipy.optimize import minimize
import time

__all__ = ['minimize_parallel', 'fmin_l_bfgs_b_parallel', 'optimParallel', 'optimparallel']

class EvalParallel:
    def __init__(self, fun, jac=None, args=(), eps=1e-8,
                 executor=concurrent.futures.ProcessPoolExecutor(),
                 forward=True, loginfo=False, verbose=False, n=1):
        self.fun_in = fun
        self.jac_in = jac
        self.eps = eps
        self.forward = forward
        self.loginfo = loginfo
        self.verbose = verbose
        self.x_val = None
        self.fun_val = None
        self.jac_val = None
        if not (isinstance(args, list) or isinstance(args, tuple)):
            self.args = (args,)
        else:
            self.args = tuple(args)
        self.n = n
        self.executor = executor
        if self.loginfo:
            self.info = {k:[] for k in ['x', 'fun', 'jac']}
        self.np_precision = np.finfo(float).eps

    ## static helper methods are used for parallel execution with map()
    @staticmethod
    def _eval_approx_args(args, eps_at, fun, x, eps):
        ## 'fun' has additional 'args'
        if eps_at == 0:
            x_ = x
        elif eps_at <= len(x):
            x_ = x.copy()
            x_[eps_at-1] += eps
        else:
            x_ = x.copy()
            x_[eps_at-1-len(x)] -= eps
        return fun(x_, *args)

    @staticmethod
    def _eval_approx(eps_at, fun, x, eps):
        ## 'fun' has no additional 'args'
        if eps_at == 0:
            x_ = x
        elif eps_at <= len(x):
            x_ = x.copy()
            x_[eps_at-1] += eps
        else:
            x_ = x.copy()
            x_[eps_at-1-len(x)] -= eps
        return fun(x_)

    @staticmethod
    def _eval_fun_jac_args(args, which, fun, jac, x):
        ## 'fun' and 'jec; have additional 'args'
        if which == 0:
            return fun(x, *args)
        return np.array(jac(x, *args))

    @staticmethod
    def _eval_fun_jac(which, fun, jac, x):
        ## 'fun' and 'jac' have no additionals 'args'
        if which == 0:
            return fun(x)
        return np.array(jac(x))

    def eval_parallel(self, x):
        ## function to evaluate 'fun' and 'jac' in parallel
        ## - if 'jac' is None, the gradient is computed numerically
        ## - if 'forward' is True, the numerical gradient uses the
        ##       forward difference method,
        ##       otherwise, the central difference method is used
        x = np.array(x)
        if (self.x_val is not None and
            all(abs(self.x_val - x) <= self.np_precision*2)):
            if self.verbose:
                print('re-use')
        else:
            self.x_val = x.copy()
            if self.jac_in is None:
                if self.forward:
                    eps_at = range(len(x)+1)
                else:
                    eps_at = range(2*len(x)+1)

                ## pack 'self.args' into function because otherwise it
                ## cannot be serialized by
                ## 'concurrent.futures.ProcessPoolExecutor()'
                if len(self.args) > 0:
                    ftmp = functools.partial(self._eval_approx_args, self.args)
                else:
                    ftmp = self._eval_approx

                ret = self.executor.map(ftmp, eps_at,
                                    itertools.repeat(self.fun_in),
                                    itertools.repeat(x),
                                    itertools.repeat(self.eps))
                ret = np.array(list(ret))
                self.fun_val = ret[0]
                if self.forward:
                    self.jac_val = (ret[1:(len(x)+1)] - self.fun_val ) / self.eps
                else:
                    self.jac_val = (ret[1:(len(x)+1)]
                                - ret[(len(x)+1):2*len(x)+1]) / (2*self.eps)

            # 'jac' function is not None
            else:
                if len(self.args) > 0:
                    ftmp = functools.partial(self._eval_fun_jac_args, self.args)
                else:
                    ftmp = self._eval_fun_jac

                ret = self.executor.map(ftmp, [0,1],
                                        itertools.repeat(self.fun_in),
                                        itertools.repeat(self.jac_in),
                                        itertools.repeat(x))
                ret = list(ret)
                self.fun_val = ret[0]
                self.jac_val = ret[1]

            self.jac_val = self.jac_val.reshape((self.n,))

            if self.loginfo:
                self.info['fun'].append(self.fun_val)
                if self.n >= 2:
                    self.info['x'].append(self.x_val.tolist())
                    self.info['jac'].append(self.jac_val.tolist())
                else:
                    self.info['x'].append(self.x_val[0])
                    self.info['jac'].append(self.jac_val[0])
        return None

    def fun(self, x):
        self.eval_parallel(x=x)
        if self.verbose:
            print('fun(' + str(x) + ') = ' + str(self.fun_val))
        return self.fun_val

    def jac(self, x):
        self.eval_parallel(x=x)
        if self.verbose:
            print('jac(' + str(x)+ ') = ' + str(self.jac_val))
        return self.jac_val


def minimize_parallel(fun, x0,
                      args=(),
                      jac=None,
                      bounds=None,
                      tol=None,
                      options=None,
                      callback=None,
                      parallel=None):

    """
    A parallel version of the L-BFGS-B optimizer of
    `scipy.optimize.minimize()`. Using it can significantly reduce the
    optimization time. For an objective function with an execution time
    of more than 0.1 seconds and p parameters the optimization speed
    increases by about factor 1+p when no analytic gradient is specified
    and 1+p processor cores with sufficient memory are available.

    Parameters
    ----------
    `fun`, `x0`, `args`, `jac`, `bounds`, `tol`, `options`, and `callback`
    are the same as the corresponding arguments of `scipy.optimize.minimize()`.
    See the documentation of `scipy.optimize.minimize()` and
    `scipy.optimize.minimize(method='L-BFGS-B')` for more information.

    Additional arguments controlling the parallel execution are:

    parallel: dict
        max_workers: The maximum number of processes that can be
            used to execute the given calls. The value is passed
            to the `max_workers` argument of
            `concurrent.futures.ProcessPoolExecutor()`.

        forward: bool. If `True` (default), the forward difference method is
            used to approximate the gradient when `jac` is `None`.
            If `False`, the central difference method is used, which can be more
            accurate.

        verbose: bool. If `True`, additional output is printed to the console.

        loginfo: bool. If `True`, additional log information containing the
            evaluated parameters as well as return values of
            fun and jac is returned.

        time: bool. If `True`, a dict containing the elapsed time (seconds)
            and the elapsed time per step (evaluation of one 'fun' call and
            its jacobian) is returned.

    Note
    ----
    On Windows it may be necessary to run `minimize_parallel()` in the
    main scope. See the example in
    <https://github.com/florafauna/optimParallel-python/blob/master/example_windows_os.py>.

    Because of the parallel overhead, `minimize_parallel()` is only faster than
    `minimize()` for objective functions with an execution time of more than 0.1
    seconds.

    When `jac=None` and `bounds` are specified, it can be advisable to
    increase the lower bounds by `eps` and decrease the upper bounds by `eps`
    because the optimizer might try to evaluate fun(upper+eps) and
    fun(upper-eps). `eps` is specified in `options` and defaults to `1e-8`,
    see `scipy.optimize.minimize(method='L-BFGS-B')`.

    Example
    -------
    >>> from optimparallel import minimize_parallel
    >>> from scipy.optimize import minimize
    >>> import numpy as np
    >>> import time
    >>>
    >>> ## objective function
    ... def f(x, sleep_secs=.5):
    ...     print('*', end='')
    ...     time.sleep(sleep_secs)
    ...     return sum((x-14)**2)
    >>>
    >>> ## start value
    ... x0 = np.array([10,20])
    >>>
    >>> ## minimize with parallel evaluation of 'fun' and
    >>> ## its approximate gradient.
    >>> o1 = minimize_parallel(fun=f, x0=x0, args=.5)
    *********
    >>> print(o1)
           fun: 2.17075906477993e-12
     hess_inv: array([[0.84615401, 0.23076919],
           [0.23076919, 0.65384592]])
          jac: array([1.94333641e-06, 2.23379136e-06])
      message: b'CONVERGENCE: NORM_OF_PROJECTED_GRADIENT_<=_PGTOL'
         nfev: 3
          nit: 2
       status: 0
      success: True
            x: array([14.00000097, 14.00000111])
    >>>
    >>> ## test against scipy.optimize.minimize()
    >>> o2 = minimize(fun=f, x0=x0, args=.5, method='L-BFGS-B')
    *********
    >>> print(all(np.isclose(o1.x, o2.x, atol=1e-10)),
    ...       np.isclose(o1.fun, o2.fun, atol=1e-10),
    ...       all(np.isclose(o1.jac, o2.jac, atol=1e-10)))
    True True True

    References
    ----------
    When using the package please cite:
    F. Gerber and R. Furrer (2019) optimParallel: An R package providing
    a parallel version of the L-BFGS-B optimization method.
    The R Journal, 11(1):352-358, 2019,
    https://doi.org/10.32614/RJ-2019-030

    R package with similar functionality:
    https://CRAN.R-project.org/package=optimParallel

    Source code of Python module:
    https://github.com/florafauna/optimParallel-python

    References for the L-BFGS-B optimization code are listed in the help
    page of `scipy.optimize.minimize()`.

    Authors
    -------
    Florian Gerber, flora.fauna.gerber@gmail.com
    https://user.math.uzh.ch/gerber/index.html

    Lewis Blake (testing, 'loginfo', 'time' features).
    """

    ## get length of x0
    try:
        n = len(x0)
    except Exception:
        n = 1

    if jac is True:
        raise ValueError("'fun' returning the function AND its "
                         "gradient is not supported.\n"
                         "Please specify separate functions in "
                         "'fun' and 'jac'.")

    ## update default options with specified options
    options_used = {'disp': None, 'maxcor': 10,
                    'ftol': 2.220446049250313e-09, 'gtol': 1e-05,
                    'eps': 1e-08, 'maxfun': 15000,
                    'maxiter': 15000, 'iprint': -1, 'maxls': 20}
    if not options is None:
        if not isinstance(options, dict):
            raise TypeError("argument 'options' must be of type 'dict'")
        options_used.update(options)
    if not tol is None:
        if not options is None and 'gtol' in options:
            warnings.warn("'tol' is ignored and 'gtol' in 'options' is used instead.",
                          RuntimeWarning)
        else:
            options_used['gtol'] = tol

    parallel_used = {'max_workers': None, 'forward': True, 'verbose': False,
                     'loginfo': False, 'time': False}
    if not parallel is None:
        if not isinstance(parallel, dict):
            raise TypeError("argument 'parallel' must be of type 'dict'")
        parallel_used.update(parallel)

    if parallel_used.get('time'):
        time_start = time.time()

    with concurrent.futures.ProcessPoolExecutor(max_workers=
                         parallel_used.get('max_workers')) as executor:
        fun_jac = EvalParallel(fun=fun,
                               jac=jac,
                               args=args,
                               eps=options_used.get('eps'),
                               executor=executor,
                               forward=parallel_used.get('forward'),
                               loginfo=parallel_used.get('loginfo'),
                               verbose=parallel_used.get('verbose'),
                               n=n)
        out = minimize(fun=fun_jac.fun,
                       x0=x0,
                       jac=fun_jac.jac,
                       method='L-BFGS-B',
                       bounds=bounds,
                       callback=callback,
                       options=options_used)

    if parallel_used.get('loginfo'):
        out.loginfo = {k: (lambda x: np.array(x) if isinstance(x[0], list)
                           else np.array(x)[np.newaxis].T)(v) for k, v in fun_jac.info.items()}

    if parallel_used.get('time'):
        time_end = time.time()
        out.time = {'elapsed': time_end - time_start,
                    'step': (time_end - time_start) / out.nfev}

    return out


def fmin_l_bfgs_b_parallel(func, x0, fprime=None, args=(), approx_grad=0,
                           bounds=None, m=10, factr=1e7, pgtol=1e-5,
                           epsilon=1e-8, iprint=-1, maxfun=15000,
                           maxiter=15000, disp=None, callback=None, maxls=20,
                           parallel=None):

    """
    A parallel version of the L-BFGS-B optimizer `fmin_l_bfgs_b()`.
    Using it can significantly reduce the optimization time.
    For an objective function with an execution time of more than
    0.1 seconds and p parameters the optimization speed increases
    by about factor 1+p when no analytic gradient is specified
    and 1+p processor cores with sufficient memory are available.

    Parameters
    ----------
    `func`, `x0`, `fprime`, `args`, `approx_grad`, `bounds`, `m`, `factr`,
    `pgtol`, `epsilon`, `iprint`, `maxfun`, `maxiter`, `disp`, `callback`,
    `maxls` are the same as in  `fmin_l_bfgs_b()`.

    Additional arguments controlling the parallel execution are:

    parallel: dict
        max_workers: The maximum number of processes that can be
            used to execute the given calls. The value is passed
            to the `max_workers` argument of
            `concurrent.futures.ProcessPoolExecutor()`.

        forward: bool. If `True` (default), the forward difference method is
            used to approximate the gradient when `jac` is `None`.
            If `False` the central difference method is used.

        verbose: bool. If `True`, additional output is printed to the console.

        loginfo: bool. If `True`, additional log information containing the
            evaluated parameters as well as return values of
            fun and jac is returned.

        time: bool. If `True`, a dict containing the elapsed time (seconds)
            and the elapsed time per step (evaluation of one 'fun' call and
            its jacobian) is returned.

    Note
    ----
    On Windows it may be necessary to run `minimize_parallel()` in the
    main scope. See the example in
    <https://github.com/florafauna/optimParallel-python/blob/master/example_windows_os.py>.

    Because of the parallel overhead, `minimize_parallel()` is only faster than
    `minimize()` for objective functions with an execution time of more than 0.1
    seconds.

    When `approx_grad=True` and `bounds` are specified, it can be advisable to
    increase the lower bounds by `eps` and decrease the upper bounds by `eps`
    because the optimizer might try to evaluate fun(upper+eps) and if 'forward
    is True, fun(upper-eps).

    References
    ----------
    When using the package please cite:
    F. Gerber and R. Furrer (2019) optimParallel: An R package providing
    a parallel version of the L-BFGS-B optimization method.
    The R Journal, 11(1):352-358, 2019,
    https://doi.org/10.32614/RJ-2019-030

    R package with similar functionality:
    https://CRAN.R-project.org/package=optimParallel

    Source code of Python module:
    https://github.com/florafauna/optimParallel-python

    References for the L-BFGS-B optimization code are listed in the help
    page of `scipy.optimize.minimize()`.

    Authors
    -------
    Florian Gerber, flora.fauna.gerber@gmail.com
    https://user.math.uzh.ch/gerber/index.html

    Lewis Blake (testing, 'loginfo', 'time' features).
    """

    fun = func
    if approx_grad:
        jac = None
    else:
        if fprime is None:
            raise ValueError("'func' returning the function AND its "
                             "gradient is not supported.\n"
                             "Please specify separate functions in "
                             "'func' and 'fprime'.")
        jac = fprime

    # build options
    if disp is None:
        disp = iprint
    options = {'disp': disp,
               'iprint': iprint,
               'maxcor': m,
               'ftol': factr * np.finfo(float).eps,
               'gtol': pgtol,
               'eps': epsilon,
               'maxfun': maxfun,
               'maxiter': maxiter,
               'maxls': maxls}

    res = minimize_parallel(fun=fun, x0=x0, args=args, jac=jac, bounds=bounds,
                            options=options, callback=callback,
                            parallel=parallel)
    x = res['x']
    f = res['fun']
    d = {'grad': res['jac'],
         'task': res['message'],
         'funcalls': res['nfev'],
         'nit': res['nit'],
         'warnflag': res['status']}

    return x, f, d

# program starts here

if __name__ == '__main__':
    scipy_bfgs_minimise_embedded()
