#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import argparse
import re
import tempfile
import scipy
import scipy.optimize
import json
import GaitSym2019

# this line means that all the math functions do not require the math. prefix
from math import *

def scipy_optimize_bind():

    parser = argparse.ArgumentParser(description="Use various scipy.optimize routines on a gaitsym problem (minimise score)")
    parser.add_argument("-i", "--input_xml_file", required=True, help="GaitSym config file with substitution indicators")
    parser.add_argument("-o", "--output_xml_file", required=True, help="GaitSym config file with best solution substituted")
    parser.add_argument("-r", "--ranges_file", required=True, help="Ranges file")
    parser.add_argument("-m", "--method", required=True, help="scipy.optimize method to use [brute]")
    parser.add_argument("-l", "--log_file", default="", help="If set log to the specified file")
    parser.add_argument("-j", "--json_string", default="", help="Use this JSON string to set the arguments required or read from this file")
    parser.add_argument("-n", "--negate_score", action="store_true", help="Negate the score")
    parser.add_argument("-f", "--force", action="store_true", help="Force overwrite of existing files")
    parser.add_argument("-v", "--verbose", action="store_true", help="Write out more information whilst processing")
    args = parser.parse_args()

    if len(args.log_file) > 0:
        preflight_write_file(args.log_file, args.force)
        # enable some logging (extra argument turns off buffering)
        so = se = open(args.log_file, 'wb', 0)
        # redirect stdout and stderr to the log file opened above
        os.dup2(so.fileno(), sys.stdout.fileno())
        os.dup2(se.fileno(), sys.stderr.fileno())

    if args.verbose:
        pretty_print_sys_argv(sys.argv)
        pretty_print_argparse_args(args)

    # preflight
    preflight_read_file(args.input_xml_file)
    preflight_read_file(args.ranges_file)
    preflight_write_file(args.output_xml_file, args.force)
        
    if os.path.isfile(args.json_string):
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

    
    methods = ['basinhopping', 'brute', 'differential_evolution', 'shgo', 'dual_annealing']
    if not args.method in methods:
        print('\'%s\' method not recognised\nAllowed values are:' % (args.method))
        print(methods)
        sys.exit(1)
    
    if args.verbose: print('Reading "%s"' % (args.ranges_file))
    ranges_dict = read_ranges(args.ranges_file)
    
    # read the config file
    if args.verbose: print('Reading "%s"' % (args.input_xml_file))
    with open(args.input_xml_file, 'r', encoding='utf8') as f_in:
        data = f_in.read()
    (smart_substitution_text_components, smart_substitution_parser_text) = create_smart_substitution(data)
    if args.verbose: print(len(smart_substitution_text_components))
    if args.verbose: print(smart_substitution_parser_text)
    
    extra_args = []
    extra_args.append(smart_substitution_text_components)
    extra_args.append(smart_substitution_parser_text)
    extra_args.append(args.verbose)
    extra_args.append(args.negate_score)
    extra_args.append("")
    
    x0 = []
    if args.method == 'basinhopping':
        x0 = do_basinhopping(ranges_dict, args.json_string, extra_args, args.verbose)
    if args.method == 'brute':
        x0 = do_brute(ranges_dict, args.json_string, extra_args, args.verbose)
    if args.method == 'differential_evolution':
        x0 = do_differential_evolution(ranges_dict, args.json_string, extra_args, args.verbose)
    if args.method == 'shgo':
        x0 = do_shgo(ranges_dict, args.json_string, extra_args, args.verbose)
    if args.method == 'dual_annealing':
        x0 = do_dual_annealing(ranges_dict, args.json_string, extra_args, args.verbose)
    
    if len(args.output_xml_file) > 0 and len(x0) > 0:
        if args.verbose: print('Writing "%s"' % (args.output_xml_file))
        extra_args = []
        extra_args.append(smart_substitution_text_components)
        extra_args.append(smart_substitution_parser_text)
        extra_args.append(args.verbose)
        extra_args.append(args.negate_score)
        extra_args.append(args.output_xml_file)
        evaluate_fitness(x0, extra_args)

def do_brute(ranges_dict, json_string, extra_args, verbose):

    ranges_list = []
    for i in range(0, len(ranges_dict['low'])):
        ranges_list.append((ranges_dict['low'][i], ranges_dict['high'][i]))
    
    default_json = '{"Ns": 20, "workers": 1}'
    if not json_string or not json_string.strip(): json_string = default_json
    parameters_dict = json.loads(json_string)
    if not isinstance(parameters_dict, dict):
        print('Error interpreting JSON string "%s" as dict' % (json_string))
        sys.exit(1)
    check_dict(parameters_dict, default_json)

    parameters_dict['full_output'] = verbose
    parameters_dict['finish'] = None
    parameters_dict['disp'] = verbose

# scipy.optimize.brute = brute(func, ranges, args=(), Ns=20, full_output=0, finish=<function fmin>, disp=False, workers=1)
#    Minimize a function over a given range by brute force.
#
#    Uses the "brute force" method, i.e. computes the function's value
#    at each point of a multidimensional grid of points, to find the global
#    minimum of the function.
#
#    The function is evaluated everywhere in the range with the datatype of the
#    first call to the function, as enforced by the ``vectorize`` NumPy
#    function.  The value and type of the function evaluation returned when
#    ``full_output=True`` are affected in addition by the ``finish`` argument
#    (see Notes).
#
#    The brute force approach is inefficient because the number of grid points
#    increases exponentially - the number of grid points to evaluate is
#    ``Ns ** len(x)``. Consequently, even with coarse grid spacing, even
#    moderately sized problems can take a long time to run, and/or run into
#    memory limitations.
#
#    Parameters
#    ----------
#    func : callable
#        The objective function to be minimized. Must be in the
#        form ``f(x, *args)``, where ``x`` is the argument in
#        the form of a 1-D array and ``args`` is a tuple of any
#        additional fixed parameters needed to completely specify
#        the function.
#    ranges : tuple
#        Each component of the `ranges` tuple must be either a
#        "slice object" or a range tuple of the form ``(low, high)``.
#        The program uses these to create the grid of points on which
#        the objective function will be computed. See `Note 2` for
#        more detail.
#    args : tuple, optional
#        Any additional fixed parameters needed to completely specify
#        the function.
#    Ns : int, optional
#        Number of grid points along the axes, if not otherwise
#        specified. See `Note2`.
#    full_output : bool, optional
#        If True, return the evaluation grid and the objective function's
#        values on it.
#    finish : callable, optional
#        An optimization function that is called with the result of brute force
#        minimization as initial guess.  `finish` should take `func` and
#        the initial guess as positional arguments, and take `args` as
#        keyword arguments.  It may additionally take `full_output`
#        and/or `disp` as keyword arguments.  Use None if no "polishing"
#        function is to be used. See Notes for more details.
#    disp : bool, optional
#        Set to True to print convergence messages from the `finish` callable.
#    workers : int or map-like callable, optional
#        If `workers` is an int the grid is subdivided into `workers`
#        sections and evaluated in parallel (uses
#        `multiprocessing.Pool <multiprocessing>`).
#        Supply `-1` to use all cores available to the Process.
#        Alternatively supply a map-like callable, such as
#        `multiprocessing.Pool.map` for evaluating the grid in parallel.
#        This evaluation is carried out as ``workers(func, iterable)``.
#        Requires that `func` be pickleable.
#
#
#    Returns
#    -------
#    x0 : ndarray
#        A 1-D array containing the coordinates of a point at which the
#        objective function had its minimum value. (See `Note 1` for
#        which point is returned.)
#    fval : float
#        Function value at the point `x0`. (Returned when `full_output` is
#        True.)
#    grid : tuple
#        Representation of the evaluation grid.  It has the same
#        length as `x0`. (Returned when `full_output` is True.)
#    Jout : ndarray
#        Function values at each point of the evaluation
#        grid, `i.e.`, ``Jout = func(*grid)``. (Returned
#        when `full_output` is True.

    try:
        (x0, fval, grid, Jout) = scipy.optimize.brute(func = evaluate_fitness,
                                                      ranges = ranges_list,
                                                      args = [extra_args],
                                                      Ns = parameters_dict['Ns'],
                                                      full_output = parameters_dict['full_output'],
                                                      finish = parameters_dict['finish'],
                                                      disp = parameters_dict['disp'],
                                                      workers = parameters_dict['workers'])
    except ValueError as e:
        print('ValueError in scipy.optimize.brute')
        print(e)
        sys.exit(1)
    
    if verbose: print(x0, fval, grid, Jout)
    return x0

def do_basinhopping(ranges_dict, json_string, extra_args, verbose):

    x0_list = []
    for i in range(0, len(ranges_dict['estimate'])):
        x0_list.append(ranges_dict['estimate'][i])
    
    default_json = '{"niter": 100, "T": 1.0, "stepsize": 0.5, "interval": 50}'
    if not json_string or not json_string.strip(): json_string = default_json
    parameters_dict = json.loads(json_string)
    if not isinstance(parameters_dict, dict):
        print('Error interpreting JSON string "%s" as dict' % (json_string))
        sys.exit(1)
    check_dict(parameters_dict, default_json)

    parameters_dict['minimizer_kwargs'] = {'args': [extra_args]}
    parameters_dict['take_step'] = None
    parameters_dict['accept_test'] = None
    parameters_dict['callback'] = None
    parameters_dict['disp'] = verbose
    parameters_dict['niter_success'] = None
    parameters_dict['seed'] = None
    
# scipy.optimize.basinhopping = basinhopping(func, x0, niter=100, T=1.0, stepsize=0.5, minimizer_kwargs=None, take_step=None, accept_test=None, callback=None, interval=50, disp=False, niter_success=None, seed=None)
#    Find the global minimum of a function using the basin-hopping algorithm
#
#    Basin-hopping is a two-phase method that combines a global stepping
#    algorithm with local minimization at each step.  Designed to mimic
#    the natural process of energy minimization of clusters of atoms, it works
#    well for similar problems with "funnel-like, but rugged" energy landscapes
#    [5]_.
#
#    As the step-taking, step acceptance, and minimization methods are all
#    customizable, this function can also be used to implement other two-phase
#    methods.
#
#    Parameters
#    ----------
#    func : callable ``f(x, *args)``
#        Function to be optimized.  ``args`` can be passed as an optional item
#        in the dict ``minimizer_kwargs``
#    x0 : array_like
#        Initial guess.
#    niter : integer, optional
#        The number of basin-hopping iterations
#    T : float, optional
#        The "temperature" parameter for the accept or reject criterion.  Higher
#        "temperatures" mean that larger jumps in function value will be
#        accepted.  For best results ``T`` should be comparable to the
#        separation (in function value) between local minima.
#    stepsize : float, optional
#        Maximum step size for use in the random displacement.
#    minimizer_kwargs : dict, optional
#        Extra keyword arguments to be passed to the local minimizer
#        ``scipy.optimize.minimize()`` Some important options could be:
#
#            method : str
#                The minimization method (e.g. ``"L-BFGS-B"``)
#            args : tuple
#                Extra arguments passed to the objective function (``func``) and
#                its derivatives (Jacobian, Hessian).
#
#    take_step : callable ``take_step(x)``, optional
#        Replace the default step-taking routine with this routine.  The default
#        step-taking routine is a random displacement of the coordinates, but
#        other step-taking algorithms may be better for some systems.
#        ``take_step`` can optionally have the attribute ``take_step.stepsize``.
#        If this attribute exists, then ``basinhopping`` will adjust
#        ``take_step.stepsize`` in order to try to optimize the global minimum
#        search.
#    accept_test : callable, ``accept_test(f_new=f_new, x_new=x_new, f_old=fold, x_old=x_old)``, optional
#        Define a test which will be used to judge whether or not to accept the
#        step.  This will be used in addition to the Metropolis test based on
#        "temperature" ``T``.  The acceptable return values are True,
#        False, or ``"force accept"``. If any of the tests return False
#        then the step is rejected. If the latter, then this will override any
#        other tests in order to accept the step. This can be used, for example,
#        to forcefully escape from a local minimum that ``basinhopping`` is
#        trapped in.
#    callback : callable, ``callback(x, f, accept)``, optional
#        A callback function which will be called for all minima found.  ``x``
#        and ``f`` are the coordinates and function value of the trial minimum,
#        and ``accept`` is whether or not that minimum was accepted.  This can
#        be used, for example, to save the lowest N minima found.  Also,
#        ``callback`` can be used to specify a user defined stop criterion by
#        optionally returning True to stop the ``basinhopping`` routine.
#    interval : integer, optional
#        interval for how often to update the ``stepsize``
#    disp : bool, optional
#        Set to True to print status messages
#    niter_success : integer, optional
#        Stop the run if the global minimum candidate remains the same for this
#        number of iterations.
#    seed : int or `np.random.RandomState`, optional
#        If `seed` is not specified the `np.RandomState` singleton is used.
#        If `seed` is an int, a new `np.random.RandomState` instance is used,
#        seeded with seed.
#        If `seed` is already a `np.random.RandomState instance`, then that
#        `np.random.RandomState` instance is used.
#        Specify `seed` for repeatable minimizations. The random numbers
#        generated with this seed only affect the default Metropolis
#        `accept_test` and the default `take_step`. If you supply your own
#        `take_step` and `accept_test`, and these functions use random
#        number generation, then those functions are responsible for the state
#        of their random number generator.
#
#    Returns
#    -------
#    res : OptimizeResult
#        The optimization result represented as a ``OptimizeResult`` object.
#        Important attributes are: ``x`` the solution array, ``fun`` the value
#        of the function at the solution, and ``message`` which describes the
#        cause of the termination. The ``OptimizeResult`` object returned by the
#        selected minimizer at the lowest minimum is also contained within this
#        object and can be accessed through the ``lowest_optimization_result``
#        attribute.  See `OptimizeResult` for a description of other attributes.

    try:
        res = scipy.optimize.basinhopping(func = evaluate_fitness,
                                          x0 = x0_list,
                                          niter = parameters_dict['niter'],
                                          T = parameters_dict['T'],
                                          stepsize = parameters_dict['stepsize'],
                                          minimizer_kwargs = parameters_dict['minimizer_kwargs'],
                                          take_step = parameters_dict['take_step'],
                                          accept_test = parameters_dict['accept_test'],
                                          callback = parameters_dict['callback'],
                                          interval = parameters_dict['interval'],
                                          disp = parameters_dict['disp'],
                                          niter_success = parameters_dict['niter_success'],
                                          seed = parameters_dict['seed'])
    except ValueError as e:
        print('ValueError in scipy.optimize.basinhopping')
        print(e)
        sys.exit(1)
    
    if verbose: print(res)
    return res.x

def do_differential_evolution(ranges_dict, json_string, extra_args, verbose):

    x0_list = []
    for i in range(0, len(ranges_dict['estimate'])):
        x0_list.append(ranges_dict['estimate'][i])
    
    ranges_list = []
    for i in range(0, len(ranges_dict['low'])):
        ranges_list.append((ranges_dict['low'][i], ranges_dict['high'][i]))

    default_json = '{"strategy": "best1bin", "maxiter": 1000, "popsize": 15, "tol": 0.01, "mutation": [0.5, 1], "recombination": 0.7, "polish": 0, "init": "latinhypercube", "atol": 0, "updating": "immediate", "workers": 1}'
    if not json_string or not json_string.strip(): json_string = default_json
    parameters_dict = json.loads(json_string)
    if not isinstance(parameters_dict, dict):
        print('Error interpreting JSON string "%s" as dict' % (json_string))
        sys.exit(1)
    check_dict(parameters_dict, default_json)

    parameters_dict['seed'] = None
    parameters_dict['callback'] = None
    parameters_dict['disp'] = verbose
    
# scipy.optimize.differential_evolution = differential_evolution(func, bounds, args=(), strategy='best1bin', maxiter=1000, popsize=15, tol=0.01, mutation=(0.5, 1), recombination=0.7, seed=None, callback=None, disp=False, polish=True, init='latinhypercube', atol=0, updating='immediate', workers=1)
#    Finds the global minimum of a multivariate function.
#
#    Differential Evolution is stochastic in nature (does not use gradient
#    methods) to find the minimium, and can search large areas of candidate
#    space, but often requires larger numbers of function evaluations than
#    conventional gradient based techniques.
#
#    The algorithm is due to Storn and Price [1]_.
#
#    Parameters
#    ----------
#    func : callable
#        The objective function to be minimized.  Must be in the form
#        ``f(x, *args)``, where ``x`` is the argument in the form of a 1-D array
#        and ``args`` is a  tuple of any additional fixed parameters needed to
#        completely specify the function.
#    bounds : sequence or `Bounds`, optional
#        Bounds for variables.  There are two ways to specify the bounds:
#        1. Instance of `Bounds` class.
#        2. ``(min, max)`` pairs for each element in ``x``, defining the finite
#        lower and upper bounds for the optimizing argument of `func`. It is
#        required to have ``len(bounds) == len(x)``. ``len(bounds)`` is used
#        to determine the number of parameters in ``x``.
#    args : tuple, optional
#        Any additional fixed parameters needed to
#        completely specify the objective function.
#    strategy : str, optional
#        The differential evolution strategy to use. Should be one of:
#
#            - 'best1bin'
#            - 'best1exp'
#            - 'rand1exp'
#            - 'randtobest1exp'
#            - 'currenttobest1exp'
#            - 'best2exp'
#            - 'rand2exp'
#            - 'randtobest1bin'
#            - 'currenttobest1bin'
#            - 'best2bin'
#            - 'rand2bin'
#            - 'rand1bin'
#
#        The default is 'best1bin'.
#    maxiter : int, optional
#        The maximum number of generations over which the entire population is
#        evolved. The maximum number of function evaluations (with no polishing)
#        is: ``(maxiter + 1) * popsize * len(x)``
#    popsize : int, optional
#        A multiplier for setting the total population size.  The population has
#        ``popsize * len(x)`` individuals (unless the initial population is
#        supplied via the `init` keyword).
#    tol : float, optional
#        Relative tolerance for convergence, the solving stops when
#        ``np.std(pop) <= atol + tol * np.abs(np.mean(population_energies))``,
#        where and `atol` and `tol` are the absolute and relative tolerance
#        respectively.
#    mutation : float or tuple(float, float), optional
#        The mutation constant. In the literature this is also known as
#        differential weight, being denoted by F.
#        If specified as a float it should be in the range [0, 2].
#        If specified as a tuple ``(min, max)`` dithering is employed. Dithering
#        randomly changes the mutation constant on a generation by generation
#        basis. The mutation constant for that generation is taken from
#        ``U[min, max)``. Dithering can help speed convergence significantly.
#        Increasing the mutation constant increases the search radius, but will
#        slow down convergence.
#    recombination : float, optional
#        The recombination constant, should be in the range [0, 1]. In the
#        literature this is also known as the crossover probability, being
#        denoted by CR. Increasing this value allows a larger number of mutants
#        to progress into the next generation, but at the risk of population
#        stability.
#    seed : int or `np.random.RandomState`, optional
#        If `seed` is not specified the `np.RandomState` singleton is used.
#        If `seed` is an int, a new `np.random.RandomState` instance is used,
#        seeded with seed.
#        If `seed` is already a `np.random.RandomState instance`, then that
#        `np.random.RandomState` instance is used.
#        Specify `seed` for repeatable minimizations.
#    disp : bool, optional
#        Display status messages
#    callback : callable, `callback(xk, convergence=val)`, optional
#        A function to follow the progress of the minimization. ``xk`` is
#        the current value of ``x0``. ``val`` represents the fractional
#        value of the population convergence.  When ``val`` is greater than one
#        the function halts. If callback returns `True`, then the minimization
#        is halted (any polishing is still carried out).
#    polish : bool, optional
#        If True (default), then `scipy.optimize.minimize` with the `L-BFGS-B`
#        method is used to polish the best population member at the end, which
#        can improve the minimization slightly.
#    init : str or array-like, optional
#        Specify which type of population initialization is performed. Should be
#        one of:
#
#            - 'latinhypercube'
#            - 'random'
#            - array specifying the initial population. The array should have
#              shape ``(M, len(x))``, where len(x) is the number of parameters.
#              `init` is clipped to `bounds` before use.
#
#        The default is 'latinhypercube'. Latin Hypercube sampling tries to
#        maximize coverage of the available parameter space. 'random'
#        initializes the population randomly - this has the drawback that
#        clustering can occur, preventing the whole of parameter space being
#        covered. Use of an array to specify a population subset could be used,
#        for example, to create a tight bunch of initial guesses in an location
#        where the solution is known to exist, thereby reducing time for
#        convergence.
#    atol : float, optional
#        Absolute tolerance for convergence, the solving stops when
#        ``np.std(pop) <= atol + tol * np.abs(np.mean(population_energies))``,
#        where and `atol` and `tol` are the absolute and relative tolerance
#        respectively.
#    updating : {'immediate', 'deferred'}, optional
#        If ``'immediate'``, the best solution vector is continuously updated
#        within a single generation [4]_. This can lead to faster convergence as
#        trial vectors can take advantage of continuous improvements in the best
#        solution.
#        With ``'deferred'``, the best solution vector is updated once per
#        generation. Only ``'deferred'`` is compatible with parallelization, and
#        the `workers` keyword can over-ride this option.
#
#        .. versionadded:: 1.2.0
#
#    workers : int or map-like callable, optional
#        If `workers` is an int the population is subdivided into `workers`
#        sections and evaluated in parallel
#        (uses `multiprocessing.Pool <multiprocessing>`).
#        Supply -1 to use all available CPU cores.
#        Alternatively supply a map-like callable, such as
#        `multiprocessing.Pool.map` for evaluating the population in parallel.
#        This evaluation is carried out as ``workers(func, iterable)``.
#        This option will override the `updating` keyword to
#        ``updating='deferred'`` if ``workers != 1``.
#        Requires that `func` be pickleable.
#
#        .. versionadded:: 1.2.0
#
#    Returns
#    -------
#    res : OptimizeResult
#        The optimization result represented as a `OptimizeResult` object.
#        Important attributes are: ``x`` the solution array, ``success`` a
#        Boolean flag indicating if the optimizer exited successfully and
#        ``message`` which describes the cause of the termination. See
#        `OptimizeResult` for a description of other attributes.  If `polish`
#        was employed, and a lower minimum was obtained by the polishing, then
#        OptimizeResult also contains the ``jac`` attribute.

    try:
        res = scipy.optimize.differential_evolution(func = evaluate_fitness,
                                                    bounds = ranges_list,
                                                    args = [extra_args],
                                                    strategy = parameters_dict['strategy'],
                                                    maxiter = parameters_dict['maxiter'],
                                                    popsize = parameters_dict['popsize'],
                                                    tol = parameters_dict['tol'],
                                                    mutation = parameters_dict['mutation'],
                                                    recombination = parameters_dict['recombination'],
                                                    seed = parameters_dict['seed'],
                                                    callback = parameters_dict['callback'],
                                                    disp = parameters_dict['disp'],
                                                    polish = parameters_dict['polish'],
                                                    init = parameters_dict['init'],
                                                    atol = parameters_dict['atol'],
                                                    updating = parameters_dict['updating'],
                                                    workers = parameters_dict['workers'])
    except ValueError as e:
        print('ValueError in scipy.optimize.basinhopping')
        print(e)
        sys.exit(1)
    
    if verbose: print(res)
    return res.x

def do_shgo(ranges_dict, json_string, extra_args, verbose):

    ranges_list = []
    for i in range(0, len(ranges_dict['low'])):
        ranges_list.append((ranges_dict['low'][i], ranges_dict['high'][i]))

    default_json = '{"n": 100, "iters": 100}'
    if not json_string or not json_string.strip(): json_string = default_json
    parameters_dict = json.loads(json_string)
    if not isinstance(parameters_dict, dict):
        print('Error interpreting JSON string "%s" as dict' % (json_string))
        sys.exit(1)
    check_dict(parameters_dict, default_json)

    parameters_dict['constraints'] = None
    parameters_dict['callback'] = None
    parameters_dict['minimizer_kwargs'] = {'args': [extra_args]}
    parameters_dict['options'] = None
    parameters_dict['sampling_method'] = None

# scipy.optimize.shgo = shgo(func, bounds, args=(), constraints=None, n=100, iters=1, callback=None, minimizer_kwargs=None, options=None, sampling_method='simplicial')
#    Finds the global minimum of a function using SHG optimization.
#
#    SHGO stands for "simplicial homology global optimization".
#
#    Parameters
#    ----------
#    func : callable
#        The objective function to be minimized.  Must be in the form
#        ``f(x, *args)``, where ``x`` is the argument in the form of a 1-D array
#        and ``args`` is a tuple of any additional fixed parameters needed to
#        completely specify the function.
#    bounds : sequence
#        Bounds for variables.  ``(min, max)`` pairs for each element in ``x``,
#        defining the lower and upper bounds for the optimizing argument of
#        `func`. It is required to have ``len(bounds) == len(x)``.
#        ``len(bounds)`` is used to determine the number of parameters in ``x``.
#        Use ``None`` for one of min or max when there is no bound in that
#        direction. By default bounds are ``(None, None)``.
#    args : tuple, optional
#        Any additional fixed parameters needed to completely specify the
#        objective function.
#    constraints : dict or sequence of dict, optional
#        Constraints definition.
#        Function(s) ``R**n`` in the form::
#
#            g(x) <= 0 applied as g : R^n -> R^m
#            h(x) == 0 applied as h : R^n -> R^p
#
#        Each constraint is defined in a dictionary with fields:
#
#            type : str
#                Constraint type: 'eq' for equality, 'ineq' for inequality.
#            fun : callable
#                The function defining the constraint.
#            jac : callable, optional
#                The Jacobian of `fun` (only for SLSQP).
#            args : sequence, optional
#                Extra arguments to be passed to the function and Jacobian.
#
#        Equality constraint means that the constraint function result is to
#        be zero whereas inequality means that it is to be non-negative.
#        Note that COBYLA only supports inequality constraints.
#
#        .. note::
#
#           Only the COBYLA and SLSQP local minimize methods currently
#           support constraint arguments. If the ``constraints`` sequence
#           used in the local optimization problem is not defined in
#           ``minimizer_kwargs`` and a constrained method is used then the
#           global ``constraints`` will be used.
#           (Defining a ``constraints`` sequence in ``minimizer_kwargs``
#           means that ``constraints`` will not be added so if equality
#           constraints and so forth need to be added then the inequality
#           functions in ``constraints`` need to be added to
#           ``minimizer_kwargs`` too).
#
#    n : int, optional
#        Number of sampling points used in the construction of the simplicial
#        complex. Note that this argument is only used for ``sobol`` and other
#        arbitrary `sampling_methods`.
#    iters : int, optional
#        Number of iterations used in the construction of the simplicial complex.
#    callback : callable, optional
#        Called after each iteration, as ``callback(xk)``, where ``xk`` is the
#        current parameter vector.
#    minimizer_kwargs : dict, optional
#        Extra keyword arguments to be passed to the minimizer
#        ``scipy.optimize.minimize`` Some important options could be:
#
#            * method : str
#                The minimization method (e.g. ``SLSQP``).
#            * args : tuple
#                Extra arguments passed to the objective function (``func``) and
#                its derivatives (Jacobian, Hessian).
#            * options : dict, optional
#                Note that by default the tolerance is specified as
#                ``{ftol: 1e-12}``
#
#    options : dict, optional
#        A dictionary of solver options. Many of the options specified for the
#        global routine are also passed to the scipy.optimize.minimize routine.
#        The options that are also passed to the local routine are marked with
#        "(L)".
#
#        Stopping criteria, the algorithm will terminate if any of the specified
#        criteria are met. However, the default algorithm does not require any to
#        be specified:
#
#        * maxfev : int (L)
#            Maximum number of function evaluations in the feasible domain.
#            (Note only methods that support this option will terminate
#            the routine at precisely exact specified value. Otherwise the
#            criterion will only terminate during a global iteration)
#        * f_min
#            Specify the minimum objective function value, if it is known.
#        * f_tol : float
#            Precision goal for the value of f in the stopping
#            criterion. Note that the global routine will also
#            terminate if a sampling point in the global routine is
#            within this tolerance.
#        * maxiter : int
#            Maximum number of iterations to perform.
#        * maxev : int
#            Maximum number of sampling evaluations to perform (includes
#            searching in infeasible points).
#        * maxtime : float
#            Maximum processing runtime allowed
#        * minhgrd : int
#            Minimum homology group rank differential. The homology group of the
#            objective function is calculated (approximately) during every
#            iteration. The rank of this group has a one-to-one correspondence
#            with the number of locally convex subdomains in the objective
#            function (after adequate sampling points each of these subdomains
#            contain a unique global minimum). If the difference in the hgr is 0
#            between iterations for ``maxhgrd`` specified iterations the
#            algorithm will terminate.
#
#        Objective function knowledge:
#
#        * symmetry : bool
#            Specify True if the objective function contains symmetric variables.
#            The search space (and therefore performance) is decreased by O(n!).
#
#        * jac : bool or callable, optional
#            Jacobian (gradient) of objective function. Only for CG, BFGS,
#            Newton-CG, L-BFGS-B, TNC, SLSQP, dogleg, trust-ncg. If ``jac`` is a
#            boolean and is True, ``fun`` is assumed to return the gradient along
#            with the objective function. If False, the gradient will be
#            estimated numerically. ``jac`` can also be a callable returning the
#            gradient of the objective. In this case, it must accept the same
#            arguments as ``fun``. (Passed to `scipy.optimize.minmize` automatically)
#
#        * hess, hessp : callable, optional
#            Hessian (matrix of second-order derivatives) of objective function
#            or Hessian of objective function times an arbitrary vector p.
#            Only for Newton-CG, dogleg, trust-ncg. Only one of ``hessp`` or
#            ``hess`` needs to be given. If ``hess`` is provided, then
#            ``hessp`` will be ignored. If neither ``hess`` nor ``hessp`` is
#            provided, then the Hessian product will be approximated using
#            finite differences on ``jac``. ``hessp`` must compute the Hessian
#            times an arbitrary vector. (Passed to `scipy.optimize.minmize`
#            automatically)
#
#        Algorithm settings:
#
#        * minimize_every_iter : bool
#            If True then promising global sampling points will be passed to a
#            local minimisation routine every iteration. If False then only the
#            final minimiser pool will be run. Defaults to False.
#        * local_iter : int
#            Only evaluate a few of the best minimiser pool candidates every
#            iteration. If False all potential points are passed to the local
#            minimisation routine.
#        * infty_constraints: bool
#            If True then any sampling points generated which are outside will
#            the feasible domain will be saved and given an objective function
#            value of ``inf``. If False then these points will be discarded.
#            Using this functionality could lead to higher performance with
#            respect to function evaluations before the global minimum is found,
#            specifying False will use less memory at the cost of a slight
#            decrease in performance. Defaults to True.
#
#        Feedback:
#
#        * disp : bool (L)
#            Set to True to print convergence messages.
#
#    sampling_method : str or function, optional
#        Current built in sampling method options are ``sobol`` and
#        ``simplicial``. The default ``simplicial`` uses less memory and provides
#        the theoretical guarantee of convergence to the global minimum in finite
#        time. The ``sobol`` method is faster in terms of sampling point
#        generation at the cost of higher memory resources and the loss of
#        guaranteed convergence. It is more appropriate for most "easier"
#        problems where the convergence is relatively fast.
#        User defined sampling functions must accept two arguments of ``n``
#        sampling points of dimension ``dim`` per call and output an array of
#        sampling points with shape `n x dim`.
#
#    Returns
#    -------
#    res : OptimizeResult
#        The optimization result represented as a `OptimizeResult` object.
#        Important attributes are:
#        ``x`` the solution array corresponding to the global minimum,
#        ``fun`` the function output at the global solution,
#        ``xl`` an ordered list of local minima solutions,
#        ``funl`` the function output at the corresponding local solutions,
#        ``success`` a Boolean flag indicating if the optimizer exited
#        successfully,
#        ``message`` which describes the cause of the termination,
#        ``nfev`` the total number of objective function evaluations including
#        the sampling calls,
#        ``nlfev`` the total number of objective function evaluations
#        culminating from all local search optimisations,
#        ``nit`` number of iterations performed by the global routine.

    try:
        res = scipy.optimize.shgo(func = evaluate_fitness,
                                  bounds = ranges_list,
                                  args = None,
                                  constraints = parameters_dict['constraints'],
                                  n = parameters_dict['n'],
                                  iters = parameters_dict['iters'],
                                  callback = parameters_dict['callback'],
                                  minimizer_kwargs = parameters_dict['minimizer_kwargs'],
                                  options = parameters_dict['options'],
                                  sampling_method = parameters_dict['sampling_method'])
    except ValueError as e:
        print('ValueError in scipy.optimize.basinhopping')
        print(e)
        sys.exit(1)
    
    if verbose: print(res)
    return res.x

def do_dual_annealing(ranges_dict, json_string, extra_args, verbose):

    ranges_list = []
    for i in range(0, len(ranges_dict['low'])):
        ranges_list.append((ranges_dict['low'][i], ranges_dict['high'][i]))

    x0_list = []
    for i in range(0, len(ranges_dict['estimate'])):
        x0_list.append(ranges_dict['estimate'][i])
    

    default_json = '{"maxiter": 1000, "initial_temp": 5230.0, "restart_temp_ratio": 2e-05, "visit": 2.62, "accept": -5.0, "maxfun": 10000000.0, "use_x0": 0}'
    if not json_string or not json_string.strip(): json_string = default_json
    parameters_dict = json.loads(json_string)
    if not isinstance(parameters_dict, dict):
        print('Error interpreting JSON string "%s" as dict' % (json_string))
        sys.exit(1)
    check_dict(parameters_dict, default_json)

    parameters_dict['local_search_options'] = {}
    parameters_dict['seed'] = None
    parameters_dict['no_local_search'] = False
    parameters_dict['callback'] = None
    if parameters_dict['use_x0']:
        parameters_dict['x0'] = x0_list
    else:
        parameters_dict['x0'] = None

# scipy.optimize.dual_annealing = dual_annealing(func, bounds, args=(), maxiter=1000, local_search_options={}, initial_temp=5230.0, restart_temp_ratio=2e-05, visit=2.62, accept=-5.0, maxfun=10000000.0, seed=None, no_local_search=False, callback=None, x0=None)
#    Find the global minimum of a function using Dual Annealing.
#
#    Parameters
#    ----------
#    func : callable
#        The objective function to be minimized.  Must be in the form
#        ``f(x, *args)``, where ``x`` is the argument in the form of a 1-D array
#        and ``args`` is a  tuple of any additional fixed parameters needed to
#        completely specify the function.
#    bounds : sequence, shape (n, 2)
#        Bounds for variables.  ``(min, max)`` pairs for each element in ``x``,
#        defining bounds for the objective function parameter.
#    args : tuple, optional
#        Any additional fixed parameters needed to completely specify the
#        objective function.
#    maxiter : int, optional
#        The maximum number of global search iterations. Default value is 1000.
#    local_search_options : dict, optional
#        Extra keyword arguments to be passed to the local minimizer
#        (`minimize`). Some important options could be:
#        ``method`` for the minimizer method to use and ``args`` for
#        objective function additional arguments.
#    initial_temp : float, optional
#        The initial temperature, use higher values to facilitates a wider
#        search of the energy landscape, allowing dual_annealing to escape
#        local minima that it is trapped in. Default value is 5230. Range is
#        (0.01, 5.e4].
#    restart_temp_ratio : float, optional
#        During the annealing process, temperature is decreasing, when it
#        reaches ``initial_temp * restart_temp_ratio``, the reannealing process
#        is triggered. Default value of the ratio is 2e-5. Range is (0, 1).
#    visit : float, optional
#        Parameter for visiting distribution. Default value is 2.62. Higher
#        values give the visiting distribution a heavier tail, this makes
#        the algorithm jump to a more distant region. The value range is (0, 3].
#    accept : float, optional
#        Parameter for acceptance distribution. It is used to control the
#        probability of acceptance. The lower the acceptance parameter, the
#        smaller the probability of acceptance. Default value is -5.0 with
#        a range (-1e4, -5].
#    maxfun : int, optional
#        Soft limit for the number of objective function calls. If the
#        algorithm is in the middle of a local search, this number will be
#        exceeded, the algorithm will stop just after the local search is
#        done. Default value is 1e7.
#    seed : {int or `~numpy.random.mtrand.RandomState` instance}, optional
#        If `seed` is not specified the `~numpy.random.mtrand.RandomState`
#        singleton is used.
#        If `seed` is an int, a new ``RandomState`` instance is used,
#        seeded with `seed`.
#        If `seed` is already a ``RandomState`` instance, then that
#        instance is used.
#        Specify `seed` for repeatable minimizations. The random numbers
#        generated with this seed only affect the visiting distribution
#        function and new coordinates generation.
#    no_local_search : bool, optional
#        If `no_local_search` is set to True, a traditional Generalized
#        Simulated Annealing will be performed with no local search
#        strategy applied.
#    callback : callable, optional
#        A callback function with signature ``callback(x, f, context)``,
#        which will be called for all minima found.
#        ``x`` and ``f`` are the coordinates and function value of the
#        latest minimum found, and ``context`` has value in [0, 1, 2], with the
#        following meaning:
#
#            - 0: minimum detected in the annealing process.
#            - 1: detection occured in the local search process.
#            - 2: detection done in the dual annealing process.
#
#        If the callback implementation returns True, the algorithm will stop.
#    x0 : ndarray, shape(n,), optional
#        Coordinates of a single n-dimensional starting point.
#
#    Returns
#    -------
#    res : OptimizeResult
#        The optimization result represented as a `OptimizeResult` object.
#        Important attributes are: ``x`` the solution array, ``fun`` the value
#        of the function at the solution, and ``message`` which describes the
#        cause of the termination.
#        See `OptimizeResult` for a description of other attributes.

    try:
        res = scipy.optimize.dual_annealing(func = evaluate_fitness,
                                            bounds = ranges_list,
                                            args = [extra_args],
                                            maxiter = parameters_dict['maxiter'],
                                            local_search_options = parameters_dict['local_search_options'],
                                            initial_temp = parameters_dict['initial_temp'],
                                            restart_temp_ratio = parameters_dict['restart_temp_ratio'],
                                            visit = parameters_dict['visit'],
                                            accept = parameters_dict['accept'],
                                            maxfun = parameters_dict['maxfun'],
                                            seed = parameters_dict['seed'],
                                            no_local_search = parameters_dict['no_local_search'],
                                            callback = parameters_dict['callback'],
                                            x0 = parameters_dict['x0'])
    except ValueError as e:
        print('ValueError in scipy.optimize.basinhopping')
        print(e)
        sys.exit(1)
    
    if verbose: print(res)
    return res.x
    
def evaluate_fitness(x, args):
    # create the file to assess
    smart_substitution_text_components = args[0]
    smart_substitution_parser_text = args[1]
    verbose = args[2]
    negate_score = args[3]
    output_filename = args[4]
    data_list = [smart_substitution_text_components[0]]
    # create the globals dictionary
    g = {}
    g['g'] = x
    if verbose: print(g)
    for i in range(0, len(smart_substitution_parser_text)):
        v = eval(smart_substitution_parser_text[i], g)
        data_list.append('%.17e' % (v))
        data_list.append(smart_substitution_text_components[i + 1])
    file_contents = ''.join(data_list)
    
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
    for i in range(0, len(smart_substitution_parser_text)):
        smart_substitution_parser_text[i] = re.sub(r'g\(([0-9]+)\)', r'g[\1]', smart_substitution_parser_text[i])
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
    

def find_file(folder, filename):
    for dirpath, dirnames, files in os.walk(folder):
        for name in files:
            if name == filename:
                return os.path.join(dirpath, name)
    return ''
    
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

# program starts here
if __name__ == "__main__":
    scipy_optimize_bind()
