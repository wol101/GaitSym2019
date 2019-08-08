/**********************************************************************
  nn_util.h
 **********************************************************************

  nn_util - Simple multi-layer Neural Network routines.
  Copyright ©2001-2003, The Regents of the University of California.
  All rights reserved.
  Primary author: "Stewart Adcock" <stewart@linux-domain.com>

  The latest version of this program should be available at:
  http://www.linux-domain.com/

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.  Alternatively, if your project
  is incompatible with the GPL, I will probably agree to requests
  for permission to use the terms of any other license.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY WHATSOEVER.

  A full copy of the GNU General Public License should be in the file
  "COPYING" provided with this distribution; if not, see:
  http://www.gnu.org/

 **********************************************************************

  Synopsis:	Multi-layer NN trained using backpropagation with
		momentum.

 **********************************************************************/

#ifndef NN_UTIL_H_INCLUDED
#define NN_UTIL_H_INCLUDED

/*
 * Includes.
 */

#include "gaul/gaul_util.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#ifndef NN_DEBUG
# ifdef DEBUG
#  define NN_DEBUG DEBUG
# else
#  define NN_DEBUG 0
# endif
#endif

#include "gaul/memory_util.h"
#include "gaul/random_util.h"

/*
 * The NN layer data structure.  This is used to represent a single
 * layer of neurons.
 */
typedef struct
  {
  int        neurons;         /* Number of neurons in this layer. */
  float      *output;         /* Neuron's output. */
  float      *error;          /* Node error factor. */
  float      **weight;        /* Synapse weights. */
  float      **weight_save;   /* Saved weights for stopped training. */
  float      **weight_change; /* Last weight deltas for momentum. */
  } layer_t;

/*
 * The NN data structure.  This is used to represent a Neural Network
 * which consists of an arbitrary number of layers.
 */
typedef struct
  {
  float      momentum;        /* Momentum factor. */
  float      rate;            /* Learning rate. */
  float      gain;            /* Gain of sigmoidal function. */
  float      bias;            /* Network bias. */
  float      decay;           /* Weight decay factor. */
  float      error;           /* Total network error. */
  layer_t    *layer;          /* Layers of neurons. */
  int        num_layers;      /* Number of layers of neurons (incl. input_output). */
  } network_t;

/*
 * Compilation constants.
 */
#define NN_MAX_FNAME_LEN	128
#define NN_DATA_ALLOC_SIZE	1024

/*
 * Default parameter constants.
 */
#define NN_SIGNAL_OFF		0.1
#define NN_SIGNAL_ON		0.9
#define NN_DEFAULT_BIAS		1.0

#define NN_DEFAULT_SEED		42

#define NN_DEFAULT_MOMENTUM	0.75
#define NN_DEFAULT_RATE		0.1
#define NN_DEFAULT_GAIN		1.0
#define NN_DEFAULT_DECAY	0.005

#define NN_DEFAULT_MAX_EPOCHS	10000
#define NN_DEFAULT_TEST_STEP	20
#define NN_DEFAULT_STOP_RATIO	1.25

/*
 * Switch check macro.
 */
#define NN_IS_ON(x)	((x)>=NN_SIGNAL_ON)
#define NN_IS_OFF(x)	((x)<=NN_SIGNAL_OFF)

typedef void	(*NN_training_func)(network_t *network, const int num_epochs);

/*
 * Prototypes.
 *
 * I wouldn't anticipate that all of these are directly useful, but
 * they are available for the slight chance that you wish to create
 * your own training scheme.
 */

FUNCPROTO void	NN_diagnostics(void);
FUNCPROTO void	NN_display_summary(network_t *network);
FUNCPROTO network_t	*NN_new(int num_layers, int *neurons);
FUNCPROTO network_t	*NN_clone(network_t *src);
FUNCPROTO void	NN_copy(network_t *src, network_t *dest);
FUNCPROTO void	NN_set_bias(network_t *network, const float bias);
FUNCPROTO void	NN_set_layer_bias(network_t *network, const int layer, const float bias);
FUNCPROTO void	NN_set_gain(network_t *network, const float gain);
FUNCPROTO void	NN_set_rate(network_t *network, const float rate);
FUNCPROTO void	NN_set_momentum(network_t *network, const float momentum);
FUNCPROTO void	NN_set_decay(network_t *network, const float decay);
FUNCPROTO void	NN_write(network_t *network, const char *fname);
FUNCPROTO network_t	*NN_read_compat(const char *fname);
FUNCPROTO network_t	*NN_read(const char *fname);
FUNCPROTO void	NN_destroy(network_t *network);
FUNCPROTO void	NN_set_all_weights(network_t *network, const float weight);
FUNCPROTO void	NN_randomize_weights(network_t *network, const float lower, const float upper);
FUNCPROTO void	NN_randomize_weights_11(network_t *network);
FUNCPROTO void	NN_randomize_weights_01(network_t *network);
FUNCPROTO void	NN_input(network_t *network, float *input);
FUNCPROTO void	NN_output(network_t *network, float *output);
FUNCPROTO void	NN_save_weights(network_t *network);
FUNCPROTO void	NN_restore_weights(network_t *network);
FUNCPROTO void	NN_propagate(network_t *network);
FUNCPROTO void	NN_output_error(network_t *network, float *target);
FUNCPROTO void	NN_backpropagate(network_t *network);
FUNCPROTO void	NN_decay_weights(network_t *network);
FUNCPROTO void	NN_adjust_weights(network_t *network);
FUNCPROTO void	NN_adjust_weights_decay(network_t *network);
FUNCPROTO void	NN_adjust_weights_momentum(network_t *network);
FUNCPROTO void	NN_simulate(network_t *network, float *input, float *target);
FUNCPROTO void	NN_simulate_with_output(network_t *network, float *input, float *target, float *output);
FUNCPROTO void	NN_run(network_t *network, float *input, float *output);
FUNCPROTO void	NN_train_random(network_t *network, const int num_epochs);
FUNCPROTO void	NN_train_systematic(network_t *network, const int num_epochs);
FUNCPROTO void	NN_train_batch_random(network_t *network, const int num_epochs);
FUNCPROTO void	NN_train_batch_systematic(network_t *network, const int num_epochs);
FUNCPROTO void	NN_test(network_t *network, float *trainerror, float *testerror);
FUNCPROTO void	NN_evaluate(network_t *network);
FUNCPROTO void	NN_predict(network_t *network);
FUNCPROTO void	NN_define_train_data(int ndata, float **data, float **prop);
FUNCPROTO void	NN_define_test_data(int ndata, float **data, float **prop);
FUNCPROTO void	NN_define_eval_data(int ndata, float **data, float **prop);
FUNCPROTO void	NN_define_predict_data(int ndata, float **data);
FUNCPROTO int	NN_read_fingerprint_binary_header(FILE *fp);
FUNCPROTO int	NN_read_data(char *fname, float ***data, char ***labels, int *num_data, int *max_data);
FUNCPROTO void	NN_read_prop(char *fname, float ***data, char ***labels, int *num_prop, int *num_data, int *dimensions);

#endif /* NN_UTIL_H_INCLUDED */

