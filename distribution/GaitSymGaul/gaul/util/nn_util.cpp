/**********************************************************************
  nn_util.c
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

		Warning: This code contains almost no error checking!

		This code uses neuronal input/response in the range 0.0>=x>=1.0.
		Note that best results will be acheived if data is
		similarly normalized.

		Most of these functions are NOT thread-safe!

  To do:	Need to define data from external sources.
		Alternative switching functions.
		Automated functions for "leave-one-out" validation.
		Full support for weight decay method starting at a given epoch.
		Node pruning. (prune zeros and +/- ones(by using BIAS as replacement))

 **********************************************************************/

#include "gaul/nn_util.h"

/*
 * Yucky global variables.
 */
static float      **train_data=NULL;       /* Input data for training. */
static float      **test_data=NULL;        /* Input data for testing. */
static float      **eval_data=NULL;        /* Input data for evaluation. */
static int        num_train_data=0;        /* Number of training target items. */
static int        num_test_data=0;         /* Number of testing target items. */
static int        num_eval_data=0;         /* Number of evaluation target items. */
static int        max_train_data=0;        /* Maximum number of training target items. */
static int        max_test_data=0;         /* Maximum number of testing target items. */
static int        max_eval_data=0;         /* Maximum number of evaluation target items. */

static float      **train_property=NULL;   /* Training target property. */
static float      **test_property=NULL;    /* Testing target property. */
static float      **eval_property=NULL;    /* Evaluation target property. */
static int        num_train_prop=0;        /* Number of training target properties. */
static int        num_test_prop=0;         /* Number of testing target properties. */
static int        num_eval_prop=0;         /* Number of evaluation target properties. */

static float      **predict_data=NULL;     /* Input data for prediction. */
static int        num_predict_data=0;      /* Number of sets of input data to predict. */
static int        max_predict_data=0;      /* Maximum number of sets of input data to predict. */

static char       **train_labels=NULL;     /* Labels for training data. */
static char       **test_labels=NULL;      /* Labels for test data. */
static char       **eval_labels=NULL;      /* Labels for evaluation data. */
static char       **predict_labels=NULL;   /* Labels for prediction data. */


/**********************************************************************
  NN_diagnostics()
  synopsis:     Display diagnostic information. 
  parameters:   none
  return:       none
  last updated: 13 Mar 2002
 **********************************************************************/

void NN_diagnostics(void)
  {

  printf("=== nn_util diagnostic information ===========================\n");
  printf("Version:                   %s\n", GA_VERSION_STRING);
  printf("Build date:                %s\n", GA_BUILD_DATE_STRING);
  printf("Compilation machine characteristics:\n%s\n", GA_UNAME_STRING);
  printf("--------------------------------------------------------------\n");
  printf("NN_DEBUG:                  %d\n", NN_DEBUG);
  printf("NN_MAX_FNAME_LEN:          %d\n", NN_MAX_FNAME_LEN);
  printf("NN_DATA_ALLOC_SIZE:        %d\n", NN_DATA_ALLOC_SIZE);
  printf("NN_SIGNAL_OFF:             %f\n", NN_SIGNAL_OFF);
  printf("NN_SIGNAL_ON:              %f\n", NN_SIGNAL_ON);
  printf("NN_DEFAULT_BIAS:           %f\n", NN_DEFAULT_BIAS);
  printf("NN_DEFAULT_SEED:           %d\n", NN_DEFAULT_SEED);
  printf("NN_DEFAULT_MOMENTUM:       %f\n", NN_DEFAULT_MOMENTUM);
  printf("NN_DEFAULT_RATE:           %f\n", NN_DEFAULT_RATE);
  printf("NN_DEFAULT_GAIN:           %f\n", NN_DEFAULT_GAIN);
  printf("NN_DEFAULT_DECAY:          %f\n", NN_DEFAULT_DECAY);
  printf("NN_DEFAULT_MAX_EPOCHS:     %d\n", NN_DEFAULT_MAX_EPOCHS);
  printf("NN_DEFAULT_TEST_STEP:      %d\n", NN_DEFAULT_TEST_STEP);
  printf("NN_DEFAULT_STOP_RATIO:     %f\n", NN_DEFAULT_STOP_RATIO);
  printf("--------------------------------------------------------------\n");
  printf("structure                  sizeof\n");
  printf("layer_t:                   %lu\n", (unsigned long) sizeof(layer_t));
  printf("network_t:                 %lu\n", (unsigned long) sizeof(network_t));
  printf("--------------------------------------------------------------\n");

  return;
  }


/**********************************************************************
  NN_display_summary()
  synopsis:     Display a summary of a Neural Network datastructure.
  parameters:   network_t *network
  return:       none
  last updated: 04 Dec 2001
 **********************************************************************/

void NN_display_summary(network_t *network)
  {
  int		l;		/* Layer index. */

  printf("num_layers = %d num_neurons =", network->num_layers);

  for (l=0; l<network->num_layers; l++)
    printf(" %d", network->layer[l].neurons);

  printf("\nmomentum = %f rate = %f gain = %f bias = %f decay = %f\n",
             network->momentum,
	     network->rate,
	     network->gain,
	     network->bias,
	     network->decay);

  return;
  }


/**********************************************************************
  NN_new()
  synopsis:     Allocate and initialise a Neural Network datastructure.
  parameters:   int num_layers	Number of layers (incl. input+output)
		int *neurons	Array containing number of nodes per layer.
  return:       network_t *network
  last updated: 01 Mar 2002
 **********************************************************************/

network_t *NN_new(int num_layers, int *neurons)
  {
  network_t	*network;	/* The new network. */
  int		l;		/* Layer index. */
  int		i;		/* Neuron index. */

  network = (network_t*) s_malloc(sizeof(network_t));
  network->layer = (layer_t*) s_malloc(num_layers*sizeof(layer_t));
  network->num_layers = num_layers;

  network->layer[0].neurons     = neurons[0];
  network->layer[0].output      = (float*) s_calloc(neurons[0]+1, sizeof(float));
  network->layer[0].error       = (float*) s_calloc(neurons[0]+1, sizeof(float));
  network->layer[0].weight      = NULL;
  network->layer[0].weight_save  = NULL;
  network->layer[0].weight_change = NULL;
  network->layer[0].output[0]   = NN_DEFAULT_BIAS;
   
  for (l=1; l<num_layers; l++)
    {
    network->layer[l].neurons     = neurons[l];
    network->layer[l].output      = (float*)  s_calloc(neurons[l]+1, sizeof(float));
    network->layer[l].error       = (float*)  s_calloc(neurons[l]+1, sizeof(float));
    network->layer[l].weight      = (float**) s_calloc(neurons[l]+1, sizeof(float*));
    network->layer[l].weight_save  = (float**) s_calloc(neurons[l]+1, sizeof(float*));
    network->layer[l].weight_change = (float**) s_calloc(neurons[l]+1, sizeof(float*));
    network->layer[l].output[0]   = NN_DEFAULT_BIAS;
      
    for (i=1; i<=neurons[l]; i++)
      {
      network->layer[l].weight[i]      = (float*) s_calloc(neurons[l-1]+1, sizeof(float));
      network->layer[l].weight_save[i]  = (float*) s_calloc(neurons[l-1]+1, sizeof(float));
      network->layer[l].weight_change[i] = (float*) s_calloc(neurons[l-1]+1, sizeof(float));
      }
    }

/* Tuneable parameters: */
  network->momentum = NN_DEFAULT_MOMENTUM;
  network->rate = NN_DEFAULT_RATE;
  network->gain = NN_DEFAULT_GAIN;
  network->bias = NN_DEFAULT_BIAS;
  network->decay = NN_DEFAULT_DECAY;

  return network;
  }


/**********************************************************************
  NN_clone()
  synopsis:     Allocate and initialise a Neural Network datastructure
  		using the contents of an existing datastructure.
  parameters:   network_t *network
  return:       network_t *network
  last updated: 01 Mar 2002
 **********************************************************************/

network_t *NN_clone(network_t *src)
  {
  network_t	*network;	/* The new network. */
  int		l;		/* Layer index. */
  int		i;		/* Neuron index. */

  network = (network_t*) s_malloc(sizeof(network_t));
  network->layer = (layer_t*) s_malloc(src->num_layers*sizeof(layer_t));
  network->num_layers = src->num_layers;

  network->layer[0].neurons     = src->layer[0].neurons;
  network->layer[0].output      = (float*) s_malloc((src->layer[0].neurons+1)*sizeof(float));
  memcpy(network->layer[0].output, src->layer[0].output, src->layer[0].neurons+1);
  network->layer[0].error       = (float*) s_malloc((src->layer[0].neurons+1)*sizeof(float));
  memcpy(network->layer[0].error, src->layer[0].error, src->layer[0].neurons+1);
  network->layer[0].weight      = NULL;
  network->layer[0].weight_save  = NULL;
  network->layer[0].weight_change = NULL;
   
  for (l=1; l<src->num_layers; l++)
    {
    network->layer[l].neurons     = src->layer[l].neurons;
    network->layer[l].output      = (float*)  s_malloc((src->layer[l].neurons+1)*sizeof(float));
    memcpy(network->layer[l].output, src->layer[l].output, src->layer[l].neurons+1);
    network->layer[l].error       = (float*)  s_malloc((src->layer[l].neurons+1)*sizeof(float));
    memcpy(network->layer[l].error, src->layer[l].error, src->layer[l].neurons+1);
    network->layer[l].weight      = (float**) s_malloc((src->layer[l].neurons+1)*sizeof(float*));
    network->layer[l].weight_save  = (float**) s_malloc((src->layer[l].neurons+1)*sizeof(float*));
    network->layer[l].weight_change = (float**) s_malloc((src->layer[l].neurons+1)*sizeof(float*));
      
    for (i=1; i<=src->layer[l].neurons; i++)
      {
      network->layer[l].weight[i]      = (float*) s_malloc((src->layer[l-1].neurons+1)*sizeof(float));
      memcpy(network->layer[l].weight[i], src->layer[l].weight[i], src->layer[l-1].neurons+1);
      network->layer[l].weight_save[i]  = (float*) s_malloc((src->layer[l-1].neurons+1)*sizeof(float));
      memcpy(network->layer[l].weight_save[i], src->layer[l].weight_save[i], src->layer[l-1].neurons+1);
      network->layer[l].weight_change[i] = (float*) s_malloc((src->layer[l-1].neurons+1)*sizeof(float));
      memcpy(network->layer[l].weight_change[i], src->layer[l].weight_change[i], src->layer[l-1].neurons+1);
      }
    }

/* Tuneable parameters: */
  network->momentum = src->momentum;
  network->rate = src->rate;
  network->gain = src->gain;
  network->bias = src->bias;
  network->decay = src->decay;

  return network;
  }


/**********************************************************************
  NN_copy()
  synopsis:     Copy the data in one Neural Network datastructure over
  		the data in another.
  parameters:   network_t *src
                network_t *dest
  return:       none
  last updated: 01 Mar 2002
 **********************************************************************/

void NN_copy(network_t *src, network_t *dest)
  {
  int		l;		/* Layer index. */
  int		i;		/* Neuron index. */

  if (dest->num_layers != src->num_layers) die("Incompatiable topology for copy (layers)");
  for (l=0; l<src->num_layers; l++)
    if (dest->layer[l].neurons != src->layer[l].neurons) die("Incompatiable topology for copy (neurons)");

  memcpy(dest->layer[0].output, src->layer[0].output, src->layer[0].neurons+1);
  memcpy(dest->layer[0].error, src->layer[0].error, src->layer[0].neurons+1);
  dest->layer[0].weight      = NULL;
  dest->layer[0].weight_save  = NULL;
  dest->layer[0].weight_change = NULL;
   
  for (l=1; l<src->num_layers; l++)
    {
    memcpy(dest->layer[l].output, src->layer[l].output, src->layer[l].neurons+1);
    memcpy(dest->layer[l].error, src->layer[l].error, src->layer[l].neurons+1);
      
    for (i=1; i<=src->layer[l].neurons; i++)
      {
      memcpy(dest->layer[l].weight[i], src->layer[l].weight[i], src->layer[l-1].neurons+1);
      memcpy(dest->layer[l].weight_save[i], src->layer[l].weight_save[i], src->layer[l-1].neurons+1);
      memcpy(dest->layer[l].weight_change[i], src->layer[l].weight_change[i], src->layer[l-1].neurons+1);
      }
    }

/* Tuneable parameters: */
  dest->momentum = src->momentum;
  dest->rate = src->rate;
  dest->gain = src->gain;
  dest->bias = src->bias;
  dest->decay = src->decay;

  return;
  }


/**********************************************************************
  NN_set_layer_bias()
  synopsis:     Change the bias of a single layer of a network to a
 		given value.
  parameters:   network_t	*network
  		const int	layer
		const float	bias
  return:       none
  last updated: 01 Mar 2002
 **********************************************************************/

void NN_set_layer_bias(network_t *network, const int layer, const float bias)
  {

  if (layer<0 || layer>=network->num_layers)
    dief("Invalid layer %d (0-%d)", layer, network->num_layers);
  
  network->layer[layer].output[0] = bias;

  return;
  }


/**********************************************************************
  NN_set_bias()
  synopsis:     Change the bias of all layers in a network to a given
 		value.
  parameters:   network_t	*network
		const float	bias
  return:       none
  last updated: 03 Dec 2001
 **********************************************************************/

void NN_set_bias(network_t *network, const float bias)
  {
  int l; 	/* Loop variable over layers. */

  if (network->bias != bias)
    {
    network->bias = bias;

    for (l=0; l<network->num_layers; l++)
      network->layer[l].output[0] = bias;
    }

  return;
  }


/**********************************************************************
  NN_set_gain()
  synopsis:     Change the gain of a network to a given value.
  parameters:   network_t *network
		float    gain
  return:       none
  last updated: 3 Dec 2001
 **********************************************************************/

void NN_set_gain(network_t *network, const float gain)
  {

  network->gain = gain;

  return;
  }


/**********************************************************************
  NN_set_rate()
  synopsis:     Change the learning rate of a network to a given value.
  parameters:   network_t *network
		float    rate
  return:       none
  last updated: 3 Dec 2001
 **********************************************************************/

void NN_set_rate(network_t *network, const float rate)
  {

  network->rate = rate;

  return;
  }


/**********************************************************************
  NN_set_momentum()
  synopsis:     Change the momentum of a network to a given value.
  parameters:   network_t *network
		float    momentum
  return:       none
  last updated: 3 Dec 2001
 **********************************************************************/

void NN_set_momentum(network_t *network, const float momentum)
  {

  network->momentum = momentum;

  return;
  }


/**********************************************************************
  NN_set_decay()
  synopsis:     Change the weight decay of a network to a given value.
  parameters:   network_t	*network
		const float	decay
  return:       none
  last updated: 01 Mar 2002
 **********************************************************************/

void NN_set_decay(network_t *network, const float decay)
  {

  network->decay = decay;

  return;
  }


/**********************************************************************
  NN_write()
  synopsis:     Write a network_t structure and its contents to disk
		in a binary format.
  parameters:   network_t *network
  		const char *fname
  return:       none
  last updated: 29 Apr 2002
 **********************************************************************/

void NN_write(network_t *network, const char *fname)
  {
  FILE		*fp;				/* File handle. */
  const char		*fmt_str="FORMAT NN: 002\n";	/* File identifier tag. */
  int		l;				/* Layer index. */
  int		i;				/* Neuron index. */

  if ( !(fp = fopen(fname, "w")) ) dief("Unable to open file \"%s\" for output.\n", fname);

  fwrite(fmt_str, sizeof(char), strlen(fmt_str), fp);

  fwrite(&(network->momentum), sizeof(float), 1, fp);
  fwrite(&(network->gain), sizeof(float), 1, fp);
  fwrite(&(network->rate), sizeof(float), 1, fp);
  fwrite(&(network->bias), sizeof(float), 1, fp);

  fwrite(&(network->num_layers), sizeof(int), 1, fp);

  for (l=0;l<network->num_layers; l++)
    {
    fwrite(&(network->layer[l].neurons), sizeof(int), 1, fp);
    }

  for (l=1; l<network->num_layers; l++)
    {
    for (i=1; i<=network->layer[l].neurons; i++)
      {
      fwrite(network->layer[l].weight[i], sizeof(float), network->layer[l-1].neurons+1, fp);
      }
    }

  fclose(fp);

  return;
  }


/**********************************************************************
  NN_read_compat()
  synopsis:     Read (and allocate) a network_t structure and its
		contents from a binary format file on disk.
		Version for backwards compatiability.
  parameters:   const char *fname
  return:	network_t *network
  last updated: 30 Nov 2001
 **********************************************************************/

network_t *NN_read_compat(const char *fname)
  {
  FILE		*fp;				/* File handle. */
  const char		*fmt_str="FORMAT NN: 001\n";	/* File identifier tag. */
  char		fmt_str_in[16];	/* File identifier tag. */
  network_t	*network;			/* The new network. */
  int		l;				/* Layer index. */
  int		i;				/* Neuron index. */

  if ( !(fp = fopen(fname, "r")) ) dief("Unable to open file \"%s\" for input.\n", fname);

  fread(fmt_str_in, sizeof(char), strlen(fmt_str), fp);
  
  if (strncmp(fmt_str, fmt_str_in, strlen(fmt_str)))
    die("Invalid neural network file header")

  network = (network_t*) s_malloc(sizeof(network_t));

  fread(&(network->momentum), sizeof(float), 1, fp);
  fread(&(network->gain), sizeof(float), 1, fp);
  fread(&(network->rate), sizeof(float), 1, fp);
  fread(&(network->bias), sizeof(float), 1, fp);

  fread(&(network->num_layers), sizeof(int), 1, fp);
  network->layer = (layer_t*) s_malloc(network->num_layers*sizeof(layer_t));

  fread(&(network->layer[0].neurons), sizeof(int), 1, fp);
  network->layer[0].output      = (float*) s_calloc(network->layer[0].neurons+1, sizeof(float));
  network->layer[0].error       = (float*) s_calloc(network->layer[0].neurons+1, sizeof(float));
  network->layer[0].weight      = NULL;
  network->layer[0].weight_save  = NULL;
  network->layer[0].weight_change = NULL;
  network->layer[0].output[0]   = network->bias;
   
  for (l=1; l<network->num_layers; l++)
    {
    fread(&(network->layer[l].neurons), sizeof(float), 1, fp);
    network->layer[l].output      = (float*)  s_calloc(network->layer[l].neurons+1, sizeof(float));
    network->layer[l].error       = (float*)  s_calloc(network->layer[l].neurons+1, sizeof(float));
    network->layer[l].weight      = (float**) s_calloc(network->layer[l].neurons+1, sizeof(float*));
    network->layer[l].weight_save  = (float**) s_calloc(network->layer[l].neurons+1, sizeof(float*));
    network->layer[l].weight_change = (float**) s_calloc(network->layer[l].neurons+1, sizeof(float*));
    network->layer[l].output[0]   = network->bias;
      
    for (i=1; i<=network->layer[l].neurons; i++)
      {
      network->layer[l].weight[i]      = (float*) s_calloc(network->layer[l-1].neurons+1, sizeof(float));
      fread(network->layer[l].weight[i], sizeof(float), network->layer[l-1].neurons, fp);
      network->layer[l].weight_save[i]  = (float*) s_calloc(network->layer[l-1].neurons+1, sizeof(float));
      network->layer[l].weight_change[i] = (float*) s_calloc(network->layer[l-1].neurons+1, sizeof(float));
      }
    }

  fclose(fp);

  return network;
  }


/**********************************************************************
  NN_read()
  synopsis:     Read (and allocate) a network_t structure and its
		contents from a binary format file on disk.
  parameters:   const char *fname
  return:	network_t *network
  last updated: 29 Apr 2002
 **********************************************************************/

network_t *NN_read(const char *fname)
  {
  FILE		*fp;				/* File handle. */
  const char		*fmt_str="FORMAT NN: 002\n";	/* File identifier tag. */
  char		fmt_str_in[16];	/* File identifier tag. */
  network_t	*network;			/* The new network. */
  int		l;				/* Layer index. */
  int		i;				/* Neuron index. */

  if ( !(fp = fopen(fname, "r")) ) dief("Unable to open file \"%s\" for input.\n", fname);

  fread(fmt_str_in, sizeof(char), strlen(fmt_str), fp);
  
  if (strncmp(fmt_str, fmt_str_in, strlen(fmt_str)))
    {
    return NN_read_compat(fname);
    }

  network = (network_t*) s_malloc(sizeof(network_t));

  fread(&(network->momentum), sizeof(float), 1, fp);
  fread(&(network->gain), sizeof(float), 1, fp);
  fread(&(network->rate), sizeof(float), 1, fp);
  fread(&(network->bias), sizeof(float), 1, fp);
  fread(&(network->decay), sizeof(float), 1, fp);

  fread(&(network->num_layers), sizeof(int), 1, fp);
  network->layer = (layer_t*) s_malloc(network->num_layers*sizeof(layer_t));

  fread(&(network->layer[0].neurons), sizeof(int), 1, fp);
  network->layer[0].output      = (float*) s_calloc(network->layer[0].neurons+1, sizeof(float));
  network->layer[0].error       = (float*) s_calloc(network->layer[0].neurons+1, sizeof(float));
  network->layer[0].weight      = NULL;
  network->layer[0].weight_save  = NULL;
  network->layer[0].weight_change = NULL;
   
  for (l=1; l<network->num_layers; l++)
    {
    fread(&(network->layer[l].neurons), sizeof(float), 1, fp);
    network->layer[l].output      = (float*)  s_calloc(network->layer[l].neurons+1, sizeof(float));
    network->layer[l].error       = (float*)  s_calloc(network->layer[l].neurons+1, sizeof(float));
    network->layer[l].weight      = (float**) s_calloc(network->layer[l].neurons+1, sizeof(float*));
    network->layer[l].weight_save  = (float**) s_calloc(network->layer[l].neurons+1, sizeof(float*));
    network->layer[l].weight_change = (float**) s_calloc(network->layer[l].neurons+1, sizeof(float*));
    network->layer[l].output[0]   = network->bias;
      
    for (i=1; i<=network->layer[l].neurons; i++)
      {
      network->layer[l].weight[i]      = (float*) s_calloc(network->layer[l-1].neurons+1, sizeof(float));
      fread(network->layer[l].weight[i], sizeof(float), network->layer[l-1].neurons+1, fp);
      network->layer[l].weight_save[i]  = (float*) s_calloc(network->layer[l-1].neurons+1, sizeof(float));
      network->layer[l].weight_change[i] = (float*) s_calloc(network->layer[l-1].neurons+1, sizeof(float));
      }
    }

  fclose(fp);

  return network;
  }


/**********************************************************************
  NN_destroy()
  synopsis:     Deallocate a network_t structure and its contents.
  parameters:   network_t *network
  return:       none
  last updated: 29 Nov 2001
 **********************************************************************/

void NN_destroy(network_t *network)
  {
  int l,i;

  for (l=0; l<network->num_layers; l++)
    {
    if (l != 0)
      {
      for (i=1; i<=network->layer[l].neurons; i++)
        {
        s_free(network->layer[l].weight[i]);
        s_free(network->layer[l].weight_save[i]);
        s_free(network->layer[l].weight_change[i]);
        }

      s_free(network->layer[l].output);
      s_free(network->layer[l].error);
      s_free(network->layer[l].weight);
      s_free(network->layer[l].weight_save);
      s_free(network->layer[l].weight_change);
      }
    }

  s_free(network->layer);
  s_free(network);

  return;
  }


/**********************************************************************
  NN_set_all_weights()
  synopsis:     Sets of of the weights of all neurons in a network to
  		a given value.
  parameters:   network_t *network
  		const float	weight
  return:       none
  last updated: 29 Nov 2001
 **********************************************************************/

void NN_set_all_weights(network_t *network, const float weight)
  {
  int l,i,j;
   
  for (l=1; l<network->num_layers; l++)
    {
    for (i=1; i<=network->layer[l].neurons; i++)
      {
      for (j=0; j<=network->layer[l-1].neurons; j++)
        {
        network->layer[l].weight[i][j] = weight;
        }
      }
    }

  return;
  }


/**********************************************************************
  NN_randomize_weights()
  synopsis:     Randomize the weights of all neurons in a network.
		Random values selected from a linear distribution
		between the passed values.
  parameters:   network_t *network
  return:       none
  last updated: 22 Jul 2002
 **********************************************************************/

void NN_randomize_weights(network_t *network, const float lower, const float upper)
  {
  int l,i,j;
   
  for (l=1; l<network->num_layers; l++)
    {
    for (i=1; i<=network->layer[l].neurons; i++)
      {
      for (j=0; j<=network->layer[l-1].neurons; j++)
        {
        network->layer[l].weight[i][j] = random_float_range(lower, upper);
        }
      }
    }

  return;
  }


/**********************************************************************
  NN_randomize_weights_11()
  synopsis:     Randomize the weights of all neurons in a network.
		Random values selected from a linear distribution
		between -1.0 and 1.0
  parameters:   network_t *network
  return:       none
  last updated: 29 Nov 2001
 **********************************************************************/

void NN_randomize_weights_11(network_t *network)
  {
  int l,i,j;
   
  for (l=1; l<network->num_layers; l++)
    {
    for (i=1; i<=network->layer[l].neurons; i++)
      {
      for (j=0; j<=network->layer[l-1].neurons; j++)
        {
        network->layer[l].weight[i][j] = random_float_range(-1.0, 1.0);
        }
      }
    }

  return;
  }


/**********************************************************************
  NN_randomize_weights_01()
  synopsis:     Randomize the weights of all neurons in a network.
		Random values selected from a linear distribution
		between 0.0 and 1.0
  parameters:   network_t *network
  return:       none
  last updated: 3 Dec 2001
 **********************************************************************/

void NN_randomize_weights_01(network_t *network)
  {
  int l,i,j;
   
  for (l=1; l<network->num_layers; l++)
    {
    for (i=1; i<=network->layer[l].neurons; i++)
      {
      for (j=0; j<=network->layer[l-1].neurons; j++)
        {
        network->layer[l].weight[i][j] = random_float(1.0);
        }
      }
    }

  return;
  }


/**********************************************************************
  NN_input()
  synopsis:     Write input values into network.
  parameters:   network_t *network
		float *input
  return:       none
  last updated: 
 **********************************************************************/

void NN_input(network_t *network, float *input)
  {
  int i;
   
  for (i=1; i<=network->layer[0].neurons; i++)
    {
    network->layer[0].output[i] = input[i-1];
    }

  return;
  }


/**********************************************************************
  NN_output()
  synopsis:     Read output values from network.
  parameters:   network_t *network
		float *input
  return:       none
  last updated: 
 **********************************************************************/

void NN_output(network_t *network, float *output)
  {
  int i;
   
  for (i=1; i<=network->layer[network->num_layers-1].neurons; i++)
    {
    output[i-1] = network->layer[network->num_layers-1].output[i];
    }

  return;
  }


/**********************************************************************
  NN_save_weights()
  synopsis:     Internally save the weights in a network.
  parameters:   network_t *network
  return:       none
  last updated: 
 **********************************************************************/

void NN_save_weights(network_t *network)
  {
  int l,i,j;

  for (l=1; l<network->num_layers; l++)
    {
    for (i=1; i<=network->layer[l].neurons; i++)
      {
      for (j=0; j<=network->layer[l-1].neurons; j++)
        {
        network->layer[l].weight_save[i][j] = network->layer[l].weight[i][j];
        }
      }
    }

  return;
  }


/**********************************************************************
  NN_restore_weights()
  synopsis:     Restore internally saved weights in a network.
  parameters:   network_t *network
  return:       none
  last updated: 
 **********************************************************************/

void NN_restore_weights(network_t *network)
  {
  int l,i,j;

  for (l=1; l<network->num_layers; l++)
    {
    for (i=1; i<=network->layer[l].neurons; i++)
      {
      for (j=0; j<=network->layer[l-1].neurons; j++)
        {
        network->layer[l].weight[i][j] = network->layer[l].weight_save[i][j];
        }
      }
    }

  return;
  }


/**********************************************************************
  NN_propagate()
  synopsis:     Propagate errors through network.
  parameters:   network_t *network
  return:       none
  last updated: 
 **********************************************************************/

void NN_propagate(network_t *network)
  {
  int l;
  int  i,j;
  float sum;
   
  for (l=0; l<network->num_layers-1; l++)
    {
    for (i=1; i<=network->layer[l+1].neurons; i++)
      {
      sum = 0;
      for (j=0; j<=network->layer[l].neurons; j++)
        {
        sum += network->layer[l+1].weight[i][j] * network->layer[l].output[j];
        }
      network->layer[l+1].output[i] = 1 / (1 + exp(-network->gain * sum));
      }
    }

  return;
  }


/**********************************************************************
  NN_output_error()
  synopsis:     Assess the error of a network against a given output
		vector.  (For sequential mode training)
  parameters:   network_t *network
		float *target
  return:       none
  last updated: 
 **********************************************************************/

void NN_output_error(network_t *network, float *target)
  {
  int  i;
  float out, err;
   
  network->error = 0;
  for (i=1; i<=network->layer[network->num_layers-1].neurons; i++)
    {
    out = network->layer[network->num_layers-1].output[i];
    err = target[i-1]-out;
    network->layer[network->num_layers-1].error[i] = network->gain * out * (1-out) * err;
    network->error += 0.5 * SQU(err);
    }

#if NN_DEBUG>2
  printf("network->error = %f\n", network->error);
#endif

  return;
  }


/**********************************************************************
  NN_output_error_sum()
  synopsis:     Sumate the error of a network against a given output
		vector.  (For batch mode training)
  parameters:   network_t *network
		float *target
  return:       none
  last updated: 25 Feb 2002
 **********************************************************************/

void NN_output_error_sum(network_t *network, float *target)
  {
  int  i;
  float out, err;
   
  network->error = 0;
  for (i=1; i<=network->layer[network->num_layers-1].neurons; i++)
    {
    out = network->layer[network->num_layers-1].output[i];
    err = target[i-1]-out;
    network->layer[network->num_layers-1].error[i] += network->gain * out * (1-out) * err;
    network->error += 0.5 * SQU(err);
    }

  return;
  }


/**********************************************************************
  NN_backpropagate()
  synopsis:     Perform one step of error back-propagation.
  parameters:   network_t *network
  return:       none
  last updated:
 **********************************************************************/

void NN_backpropagate(network_t *network)
  {
  int l;
  int  i,j;
  float out, err;
   
  for (l=network->num_layers-1; l>1; l--)
    {
    for (i=1; i<=network->layer[l-1].neurons; i++)
      {
      out = network->layer[l-1].output[i];
      err = 0;
      for (j=1; j<=network->layer[l].neurons; j++)
        {
        err += network->layer[l].weight[j][i] * network->layer[l].error[j];
        }
      network->layer[l-1].error[i] = network->gain * out * (1-out) * err;
      }
    }

  return;
  }


/**********************************************************************
  NN_decay_weights()
  synopsis:     Apply weight decay.
  parameters:   network_t *network
  return:       none
  last updated:	01 Mar 2002
 **********************************************************************/

void NN_decay_weights(network_t *network)
  {
  int  l,i,j;
   
  for (l=1; l<network->num_layers; l++)
    {
    for (i=1; i<=network->layer[l].neurons; i++)
      {
      for (j=0; j<=network->layer[l-1].neurons; j++)
        {
        network->layer[l].weight[i][j] -= network->layer[l].weight[i][j]*network->decay;
        }
      }
    }

  return;
  }


/**********************************************************************
  NN_adjust_weights()
  synopsis:     Modify network weights according to classic
  		back-propagated error.
  parameters:   network_t *network
  return:       none
  last updated: 01 Mar 2002
 **********************************************************************/

void NN_adjust_weights(network_t *network)
  {
  int  l,i,j;
  float out, err;
   
  for (l=1; l<network->num_layers; l++)
    {
    for (i=1; i<=network->layer[l].neurons; i++)
      {
      for (j=0; j<=network->layer[l-1].neurons; j++)
        {
        out = network->layer[l-1].output[j];
        err = network->layer[l].error[i];
        network->layer[l].weight[i][j] += network->rate * err * out;
        }
      }
    }

  return;
  }


/**********************************************************************
  NN_adjust_weights_decay()
  synopsis:     Modify network weights according to back-propagated
		error with weight decay.
  parameters:   network_t *network
  return:       none
  last updated:	01 Mar 2002
 **********************************************************************/

void NN_adjust_weights_decay(network_t *network)
  {
  int  l,i,j;
  float out, err;
   
  for (l=1; l<network->num_layers; l++)
    {
    for (i=1; i<=network->layer[l].neurons; i++)
      {
      for (j=0; j<=network->layer[l-1].neurons; j++)
        {
        out = network->layer[l-1].output[j];
        err = network->layer[l].error[i];
        network->layer[l].weight[i][j] += network->rate * err * out
                                       - network->decay * network->layer[l].weight[i][j];
        }
      }
    }

  return;
  }


/**********************************************************************
  NN_adjust_weights_momentum()
  synopsis:     Modify network weights according to back-propagated
		error with momentum.
  parameters:   network_t *network
  return:       none
  last updated:
 **********************************************************************/

void NN_adjust_weights_momentum(network_t *network)
  {
  int  l,i,j;
  float out, err;

#if NN_DEBUG>2
  printf("Adjusting weights with mmtm.  network->error = %f\n", network->error);
#endif
   
  for (l=1; l<network->num_layers; l++)
    {
    for (i=1; i<=network->layer[l].neurons; i++)
      {
      for (j=0; j<=network->layer[l-1].neurons; j++)
        {
        out = network->layer[l-1].output[j];
        err = network->layer[l].error[i];
        network->layer[l].weight[i][j] += network->rate * err * out
                                        + network->momentum * network->layer[l].weight_change[i][j];
        network->layer[l].weight_change[i][j] = network->rate * err * out;
        }
      }
    }

  return;
  }


/**********************************************************************
  NN_simulate_batch()
  synopsis:     Training simulation for batch-mode training.
  parameters:   network_t *network
		float *input
		float *target
  return:       none
  last updated: 25 Feb 2002
 **********************************************************************/

void NN_simulate_batch(network_t *network, float *input, float *target)
  {

  NN_input(network, input);
  NN_propagate(network);
   
  NN_output_error_sum(network, target);

  return;
  }


/**********************************************************************
  NN_simulate()
  synopsis:     Training simulation.
  parameters:   network_t *network
		float *input
		float *target
  return:       none
  last updated:
 **********************************************************************/

void NN_simulate(network_t *network, float *input, float *target)
  {
#if NN_DEBUG>2
  int i;	/* Debug. */
#endif

  NN_input(network, input);
  NN_propagate(network);
   
  NN_output_error(network, target);

#if NN_DEBUG>2
  for (i=1; i<=network->layer[network->num_layers-1].neurons; i++)
    printf("%f ", network->layer[network->num_layers-1].output[i]);
  printf("\n");
#endif

  return;
  }


/**********************************************************************
  NN_simulate_with_output()
  synopsis:     Training simulation which also returns the output
		vector.
  parameters:   network_t *network
		float *input
		float *target
		float *output
  return:       none
  last updated:
 **********************************************************************/

void NN_simulate_with_output(network_t *network, float *input, float *target, float *output)
  {

  NN_input(network, input);
  NN_propagate(network);
  NN_output(network, output);
   
  NN_output_error(network, target);

  return;
  }


/**********************************************************************
  NN_run()
  synopsis:     Prediction simulation.
  parameters:   network_t *network
		float *input
		float *output
  return:       none
  last updated:	28 Jan 2002
 **********************************************************************/

void NN_run(network_t *network, float *input, float *output)
  {

  NN_input(network, input);
  NN_propagate(network);
  NN_output(network, output);
   
  return;
  }


/**********************************************************************
  NN_train_random()
  synopsis:     Train network using back-propagation.
  parameters:   network_t *network
		int num_epochs
  return:       none
  last updated: 28 Jan 2002
 **********************************************************************/

void NN_train_random(network_t *network, const int num_epochs)
  {
  int  item, n;

  for (n=0; n<num_epochs*num_train_data; n++)
    {
    item = random_int(num_train_data);
    NN_simulate(network, train_data[item], train_property[item]);

    NN_backpropagate(network);
    NN_adjust_weights_momentum(network);
    }
 
  return;
  }


/**********************************************************************
  NN_train_systematic()
  synopsis:     Train network using back-propagation.
  parameters:   network_t *network
		int num_epochs
  return:       none
  last updated: 06 Feb 2002
 **********************************************************************/

void NN_train_systematic(network_t *network, const int num_epochs)
  {
  int  i, n;

  for (i=0; i<num_epochs; i++)
    {
    for (n=0; n<num_train_data; n++)
      {
      NN_simulate(network, train_data[n], train_property[n]);

      NN_backpropagate(network);
      NN_adjust_weights_momentum(network);
      }
    }

  return;
  }


/**********************************************************************
  NN_train_batch_random()
  synopsis:     Train network using back-propagation.
  parameters:   network_t *network
		int num_epochs
  return:       none
  last updated: 25 Feb 2002
 **********************************************************************/

void NN_train_batch_random(network_t *network, const int num_epochs)
  {
  int  i, n;
  int  item;

  for (i=0; i<num_epochs; i++)
    {
    for (n=0; n<num_train_data; n++)
      {
      item = random_int(num_train_data);
      NN_simulate_batch(network, train_data[item], train_property[item]);
      }

    NN_backpropagate(network);
    NN_adjust_weights_momentum(network);
    }
 
  return;
  }


/**********************************************************************
  NN_train_batch_systematic()
  synopsis:     Train network using back-propagation.
  parameters:   network_t *network
		int num_epochs
  return:       none
  last updated: 25 Feb 2002
 **********************************************************************/

void NN_train_batch_systematic(network_t *network, const int num_epochs)
  {
  int  i, n;

  for (i=0; i<num_epochs; i++)
    {
    for (n=0; n<num_train_data; n++)
      {
      NN_simulate_batch(network, train_data[n], train_property[n]);
      }

    NN_backpropagate(network);
    NN_adjust_weights_momentum(network);
    }
 
  return;
  }


/**********************************************************************
  NN_test()
  synopsis:     Test network.
  parameters:   network_t *network
		float *trainerror
		float *testerror
  return:       none
  last updated:
 **********************************************************************/

void NN_test(network_t *network, float *trainerror, float *testerror)
  {
  int  item;

  *trainerror = 0;
  for (item=0; item<num_train_data; item++)
    {
    NN_simulate(network, train_data[item], train_property[item]);
    *trainerror += network->error;
    }
  *trainerror /= num_train_data;

  *testerror = 0;
  for (item=0; item<num_test_data; item++)
    {
    NN_simulate(network, test_data[item], test_property[item]);
    *testerror += network->error;
    }
  *testerror /= num_test_data;

  return;
  }


/**********************************************************************
  NN_evaluate()
  synopsis:     Evaluate network and write result.
  parameters:   network_t *network
  return:       none
  last updated:
 **********************************************************************/

void NN_evaluate(network_t *network)
  {
  int		i;		/* Loop variable over output neurons. */
  int		item;		/* Loop variable over evaluation data. */
  float	*output;	/* Output results. */
  float	evalerror=0;	/* Network's output error. */

  output = (float *) s_malloc(network->layer[network->num_layers-1].neurons*sizeof(float));

  printf("\n\nItem  Field  Actual  Prediction\n\n");
  for (item=0; item<num_eval_data; item++)
    {
    NN_simulate_with_output(network, eval_data[item], eval_property[item], output);
    evalerror += network->error;
    printf("%4d  0    %0.4f  %0.4f\n",
           item, eval_property[item][0], output[0]);
    for (i=1; i<network->layer[network->num_layers-1].neurons; i++)
      {
      printf("     %3d  %0.4f  %0.4f\n",
             i, eval_property[item][i], output[i]);
      }
    }
  evalerror /= num_eval_data;

  printf("Error is %f on evaluation set.\n", evalerror);

  s_free(output);

  return;
  }


/**********************************************************************
  NN_predict()
  synopsis:     Use network to make predictions and write results.
  parameters:   network_t *network
  return:       none
  last updated:	24 Jan 2002
 **********************************************************************/

void NN_predict(network_t *network)
  {
  int		i;		/* Loop variable over output neurons. */
  int		item;		/* Loop variable over evaluation data. */
  float	*output;	/* Output results. */

  output = (float *) s_malloc(network->layer[network->num_layers-1].neurons*sizeof(float));

  printf("\n\nItem  Field  Prediction\n\n");
  for (item=0; item<num_eval_data; item++)
    {
    NN_run(network, eval_data[item], output);
    printf("%4d  0    %0.4f\n", item, output[0]);
    for (i=1; i<network->layer[network->num_layers-1].neurons; i++)
      {
      printf("     %3d  %0.4f\n", i, output[i]);
      }
    }

  s_free(output);

  return;
  }


/**********************************************************************
  NN_define_train_data()
  synopsis:     Define training data.
  parameters:
  return:
  last updated: 04 Feb 2002
 **********************************************************************/

void NN_define_train_data(int ndata, float **data, float **prop)
  {
  train_data=data;           /* Input data for training. */
  num_train_data=ndata;      /* Number of training target items. */
  max_train_data=ndata;      /* Maximum number of training target items. */
  train_property=prop;       /* Training target property. */
  num_train_prop=ndata;      /* Number of training target properties. */
  train_labels=NULL;         /* Labels for training data. */

  return;
  }


/**********************************************************************
  NN_define_test_data()
  synopsis:     Define testing data.
  parameters:
  return:
  last updated: 04 Feb 2002
 **********************************************************************/

void NN_define_test_data(int ndata, float **data, float **prop)
  {
  test_data=data;           /* Input data for testing. */
  num_test_data=ndata;      /* Number of testing target items. */
  max_test_data=ndata;      /* Maximum number of testing target items. */
  test_property=prop;       /* Testing target property. */
  num_test_prop=ndata;      /* Number of testing target properties. */
  test_labels=NULL;         /* Labels for testing data. */

  return;
  }


/**********************************************************************
  NN_define_eval_data()
  synopsis:     Define evaluation data.
  parameters:
  return:
  last updated: 04 Feb 2002
 **********************************************************************/

void NN_define_eval_data(int ndata, float **data, float **prop)
  {
  eval_data=data;           /* Input data for evaluation. */
  num_eval_data=ndata;      /* Number of evaluation target items. */
  max_eval_data=ndata;      /* Maximum number of evaluation target items. */
  eval_property=prop;       /* Evaluation target property. */
  num_eval_prop=ndata;      /* Number of evaluation target properties. */
  eval_labels=NULL;         /* Labels for evaluation data. */

  return;
  }


/**********************************************************************
  NN_define_predict_data()
  synopsis:     Define prediction data.
  parameters:
  return:
  last updated: 04 Feb 2002
 **********************************************************************/

void NN_define_predict_data(int ndata, float **data)
  {
  predict_data=data;           /* Input data for prediction. */
  num_predict_data=ndata;      /* Number of prediction target items. */
  max_predict_data=ndata;      /* Maximum number of prediction target items. */
  predict_labels=NULL;         /* Labels for prediction data. */

  return;
  }


/**********************************************************************
  NN_read_fingerprint_binary_header()
  synopsis:     Read binary fingerprint info from given filehandle.
                Designed for future expansion rather than current
                utility.
  parameters:   filehandle
  return:       int size
  last updated: 28 Nov 2001
 **********************************************************************/

int NN_read_fingerprint_binary_header(FILE *fp)
  {
  const char *fmt_str="FORMAT FP: 001\n";
  char fmt_str_in[16];
  int size;

  fread(fmt_str_in, sizeof(char), strlen(fmt_str), fp);

  if (strncmp(fmt_str, fmt_str_in, strlen(fmt_str)))
    die("Invalid fingerprint header");

  fread(&size, sizeof(int), 1, fp);

  return size;
  }


/**********************************************************************
  NN_read_data()
  synopsis:     Read binary fingerprint with label from given file.
  parameters:   filename
                data
  return:       int size
  last updated: 3 Dec 2001
 **********************************************************************/

int NN_read_data(char *fname, float ***data, char ***labels, int *num_data, int *max_data)
  {
  FILE  *fp;		/* Filehandle. */
  int   label_len;	/* Label length. */
  int	size;		/* Dimensions of fingerprint. */

  if ( !(fp = fopen(fname, "r")) ) dief("Unable to open file \"%s\" for input.\n", fname);

  size = NN_read_fingerprint_binary_header(fp);	/* Check validity of file. */

  while (fread(&label_len, sizeof(int), 1, fp) != 0)
    {
    if (*num_data == *max_data)
      {
      *max_data += NN_DATA_ALLOC_SIZE;
      *data = (float **) s_realloc(*data, sizeof(float *) * *max_data);
      *labels = (char **) s_realloc(*labels, sizeof(char *) * *max_data);
      }

    (*labels)[*num_data] = (char *) s_malloc(sizeof(char)*label_len+1);
    fread((*labels)[*num_data], sizeof(char), label_len, fp);
    (*labels)[*num_data][label_len] = '\0';

    (*data)[*num_data] = (float *) s_malloc(sizeof(float)*size);
    fread((*data)[*num_data], sizeof(float), size, fp);

    (*num_data)++;
    }

  fclose(fp);

  return size;
  }


/**********************************************************************
  nn_nreadline(FILE *fp, int len, char *dest)
  synopsis:	Reads upto newline/eof from specified stream, to a
		maximum of len characters, also
		ensures that the string is always null-terminated.
  parameters:   char    	*dest	The destination string.
		FILE		*fp	The input stream.
  return:	int	actual number of characters read. -1 on failure.
  last updated: 08 Jan 2003
 **********************************************************************/

static int nn_nreadline(FILE *fp, const int len, char *dest)
  {
  int		count=0, max_count;	/* Number of chars read */
  int		c;			/* Current character */

  if (!fp) die("Null file handle passed.\n");
  if (len < 1) die("Stupid length.\n");
  if (!dest) die("Null string pointer passed.\n");

  max_count = len-1;

/*  while((!feof(fp)) && (c=fgetc(fp)) && (c!='\n') && count<len)*/

  while(count<max_count && (c=fgetc(fp))!=EOF && ((char)c!='\n'))
    dest[count++]=(char)c;

  dest[count]='\0';

  return count-1;
  }


/**********************************************************************
  NN_read_prop()
  synopsis:     Read properties from given file.
  parameters:   char *fname	File to read.
		float ***data	Data.
		char ***labels	Data labels.
		int *num_prop	Number of data items read in total.
		int *num_data	Number of data items expected in total.
		int *dimensions	Dimensions of data, or -1 to determine here.
  return:       none.
  last updated: 18 Jul 2002
 **********************************************************************/

void NN_read_prop(char *fname, float ***data, char ***labels, int *num_prop, int *num_data, int *dimensions)
  {
  FILE  *fp;		        	/* Filehandle. */
  char  line_buffer[MAX_LINE_LEN];	/* Line buffer. */
  char  *line;                          /* Line pointer. */
  int   data_count;		        /* Number of fields input from current record. */

  if ( !(fp = fopen(fname, "r")) )
    dief("Unable to open file \"%s\" for input.\n", fname);

  *data = (float **) s_realloc(*data, sizeof(float*)*(*num_data));

/* Count data items on first line, if necessary. */
  if (*dimensions == -1)
    {
    char	line_copy[MAX_LINE_LEN];	/* Line buffer copy. */

    if (nn_nreadline(fp, MAX_LINE_LEN, line_buffer)<=0)
      dief("Error reading file \"%s\".\n", fname);

    strcpy(line_copy, line_buffer);
    line = line_copy;

    if (strncmp((*labels)[*num_prop], line, strlen((*labels)[*num_prop]))!=0)
      dief("Label mismatch \"%s\" to \"%s\"", (*labels)[*num_prop], line);

    line = strtok(&(line[strlen((*labels)[*num_prop])]), " ");
    *dimensions=1;

    while ( (line = strtok(NULL, " "))!=NULL )
      {
      (*dimensions)++;
      }

    line = line_buffer;

    if (strncmp((*labels)[*num_prop], line, strlen((*labels)[*num_prop]))!=0)
      dief("Label mismatch \"%s\" to \"%s\"", (*labels)[*num_prop], line);

    (*data)[*num_prop] = (float *) s_malloc((*dimensions)*sizeof(float));

    line = strtok(&(line[strlen((*labels)[*num_prop])]), " ");
    (*data)[*num_prop][0] = (float) atof(line);
    data_count=1;

    while ( (line = strtok(NULL, " "))!=NULL )
      {
      if (data_count==*dimensions) die("Internal error which should never occur.");

      (*data)[*num_prop][data_count] = (float) atof(line);

      data_count++;
      }

    (*num_prop)++;
    }

/* Read remainder of file. */
  while (nn_nreadline(fp, MAX_LINE_LEN, line_buffer)>0)
    {
    if (*num_prop > *num_data) die("Too many property records input.");

    line = line_buffer;

    if (strncmp((*labels)[*num_prop], line, strlen((*labels)[*num_prop]))!=0)
      dief("Label mismatch \"%s\" to \"%s\"", (*labels)[*num_prop], line);

    (*data)[*num_prop] = (float *) s_malloc((*dimensions)*sizeof(float));

    line = strtok(&(line[strlen((*labels)[*num_prop])]), " ");
    (*data)[*num_prop][0] = (float) atof(line);
    data_count=1;

    while ( (line = strtok(NULL, " "))!=NULL )
      {
      if (data_count==*dimensions) die("Too many data items.");

      (*data)[*num_prop][data_count] = (float) atof(line);

      data_count++;
      }

    (*num_prop)++;

    /* Simple check. */
    if (data_count!=*dimensions)
      {
      dief("Too few data items (%d instead of %d) for item %d.", data_count, *dimensions, *num_prop);
      }
    }

  fclose(fp);

  return;
  }


