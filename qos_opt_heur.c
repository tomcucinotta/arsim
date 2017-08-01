#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "qos_opt.h"
#include <assert.h>

/* Maximum values for static arrays */

/** Maximum number of applications **/
#define MAX_N   16
/** Maximum number of application modes **/
#define MAX_M   16
/** Maximum number of resources **/
#define MAX_R   16
/** Maximum number of resource power modes **/
#define MAX_P   16

/** Maximum number of column variables in the GLPK formulation **/
#define MAX_COL (MAX_N * MAX_M * MAX_R * MAX_P)

typedef struct {
  struct qos_opt_vtable *vtable;	/**< vtable */

  int num_apps;
  int num_res;
  int app_modes;
  int res_modes;
  qos_level levels[MAX_N * MAX_M];      // zero-based indexed
  qos_weight weights[MAX_N];            // zero-based indexed
  pow_level powers[MAX_R * MAX_P];      // zero-based indexed
  qos_load bands[MAX_COL + 1];          // one-based indexed
  unsigned int penalty[MAX_N * MAX_M * MAX_M]; // zero-based indexed, contains a non-optimized symmetric matrix, with zeros on the main diagonal
  int appl_mode[MAX_N];                 // zero-based indexed, stored app_modes are 0-based as well
  unsigned int res_pot[MAX_R];          // zero-based indexed, stored res_modes are 0-based as well
  double obj_value;                     // value of objective function reached in the last optimization
} qos_opt_heur;

#ifdef DEBUG_QOS_OPT
#  define dbg_printf(fmt, args...) do { printf(fmt,##args); fflush(stdout); } while (0)
#else
#  define dbg_printf(fmt, args...) do { } while (0)
#endif

/** Return the ONE-BASED column index corresponding to the supplied ZERO-BASED parameters **/
static int col_idx(qos_opt_heur *p_op, int app, int app_mode, int res, int res_mode) {
  return 1 + app_mode
           + res_mode * p_op->app_modes
           + app * p_op->app_modes * p_op->res_modes
           + res * p_op->app_modes * p_op->res_modes * p_op->num_apps;
}

/** Return the ZERO-BASED index within the penalty[] array corresponding to the supplied ZERO-BASED parameters */
static int pen_idx(qos_opt_heur *p_op, int app, int am1, int am2) {
  return app * p_op->app_modes * p_op->app_modes
      + am1 * p_op->app_modes
      + am2;
}

/** Initialise the data type supplying the maximum allowed number of things
 **
 ** @TODO Add check of maxima numbers according to sizeof() of arrays in the qos_opt type.
 **/
static void init(qos_opt *this, int num_apps, int app_modes,
                  int num_res, int res_modes)
{
  qos_opt_heur * p_op = (qos_opt_heur *) this;
  int a, am1, am2;
  p_op->num_apps=num_apps;
  p_op->app_modes=app_modes;
  p_op->num_res=num_res;
  p_op->res_modes=res_modes;
  /* Ideally, static arrays might be more flexible, but now I need a quick check */
  assert(num_apps <= MAX_N);
  assert(app_modes <= MAX_M);
  assert(num_res <= MAX_R);
  assert(res_modes <= MAX_P);
  // Initializing all app mode transition penalty values to 0
  for (a = 0; a < num_apps; a++)
    for (am1 = 0; am1 < app_modes; am1++)
      for (am2 = 0; am2 < app_modes; am2++) {
        int idx = pen_idx(p_op, a, am1, am2);
        p_op->penalty[idx] = 0;
      }
}

static void cleanup(qos_opt *this) {
  /* @todo */
}

/** Set the value of QoS level for each application mode
 **
 ** @note
 ** In the problem formulation, these are the coefficients of the v^(i) vector.
 **/
void set_qos_level(qos_opt *this, int app, int app_mode, qos_level lev) {
  qos_opt_heur * p_op = (qos_opt_heur *) this;
  int pointer;
  pointer = app * p_op->app_modes + app_mode;
  p_op->levels[pointer] = lev;
  dbg_printf("Set qos for app %d in mode %d (idx %d) to lev %d\n", app, app_mode, pointer, p_op->levels[pointer]);
}

qos_level get_qos_level(qos_opt *this, int app, int app_mode) {
  qos_opt_heur * p_op = (qos_opt_heur *) this;
  int pointer;
  pointer = app * p_op->app_modes + app_mode;
  return p_op->levels[pointer];
}

/** Set weight for application QoS mode.
 **
 ** This value constitutes the increment in the objective function per
 ** unit of QoS value due to an application mode.
 **
 ** @note
 ** In the problem formulation, these are the coefficients w_Q^(i).
 **/
void set_qos_gain(qos_opt *this, int app, qos_weight w) {
  qos_opt_heur * p_op = (qos_opt_heur *) this;
  int pointer;
  pointer = app;
  p_op->weights[pointer]=w;
  dbg_printf("Set weight for app %d (idx %d) to weight %d\n", app, pointer, p_op->weights[pointer]);
}

qos_weight get_qos_gain(qos_opt *this, int app) {
  qos_opt_heur * p_op = (qos_opt_heur *) this;
  int pointer;
  pointer = app;
  return p_op->weights[pointer];
}

/** Explicitly set the current app mode for proper accounting of the contributes
 ** to the objective function due to application mode variations.
 **/
void set_app_mode(qos_opt *this, int app, int from_app_mode) {
  qos_opt_heur * p_op = (qos_opt_heur *) this;
  p_op->appl_mode[app] = from_app_mode;
  dbg_printf("Set app %d to initial mode %d\n", app, p_op->appl_mode[app]);
}

/** Set penalty for application QoS mode variation.
 **
 ** This value constitutes the decrement in the objective function per
 ** unit of absolute difference of QoS values due to an application mode
 ** change.
 **
 ** @note
 ** The current app modes is set either explicitly through the set_app_mode()
 ** function, or implicitly after the solver returned a solution.
 **
 ** @note
 ** In the problem formulation, these correspond to the product w'_Q^(i) * k_V^(i).
 **/
void set_qos_var_penalty(qos_opt *this, int app, int app_mode1, int app_mode2, int w) {
  qos_opt_heur * p_op = (qos_opt_heur *) this;
  int pointer;
  pointer = pen_idx(p_op, app, app_mode1, app_mode2);
  p_op->penalty[pointer] = w;
  pointer = pen_idx(p_op, app, app_mode2, app_mode1);
  p_op->penalty[pointer] = w;
  dbg_printf("Set penalty for app %d from mode %d to mode %d (idx %i) to %d\n",
              app, app_mode1, app_mode2, pointer, p_op->penalty[pointer]);
}

int get_qos_var_penalty(qos_opt *this, int app, int app_mode1, int app_mode2) {
  qos_opt_heur * p_op = (qos_opt_heur *) this;
  int pointer;
  pointer = pen_idx(p_op, app, app_mode1, app_mode2);
  return p_op->penalty[pointer];
}

/** Set the value of power consumption for each resource power mode
 **
 ** @note
 ** In the problem formulation, these are the coefficients of the vector k_r.
 **/
void set_pow_level(qos_opt *this, int res, int res_mode, pow_level l) {
  qos_opt_heur * p_op = (qos_opt_heur *) this;
  int pointer;
  pointer = res * p_op->res_modes + res_mode;
  p_op->powers[pointer]=l;
  dbg_printf("Set res %d in mode %d (idx %d) to consume %d\n", res, res_mode, pointer, p_op->powers[pointer]);
}

pow_level get_pow_level(qos_opt *this, int res, int res_mode) {
  qos_opt_heur * p_op = (qos_opt_heur *) this;
  int pointer;
  pointer = res * p_op->res_modes + res_mode;
  dbg_printf("Retrieving res %d in mode %d (idx %d) consumes %d\n", res, res_mode, pointer, p_op->powers[pointer]);
  return p_op->powers[pointer];
}

/** Set the bandwidth requirement for each application mode and resource power mode.
 **
 ** @note
 ** In the problem formulation, these are the coefficients of the matrix B_r^(i).
 **/
void set_load(qos_opt *this, int app, int app_mode, int res, int res_mode, qos_load l) {
  qos_opt_heur * p_op = (qos_opt_heur *) this;
  int idx = col_idx(p_op, app, app_mode, res, res_mode);
  p_op->bands[idx] = l;
  dbg_printf("Set app %d in mode %d on res %d in mode %d (idx %d) to load %f\n",app, app_mode, res, res_mode, idx, l);
}

qos_load get_load(qos_opt *this, int app, int app_mode, int res, int res_mode) {
  qos_opt_heur * p_op = (qos_opt_heur *) this;
  int idx = col_idx(p_op, app, app_mode, res, res_mode);
  return p_op->bands[idx];
}

/** Set the weight for a resource power mode.
 **
 ** This value constitutes the decrement in the objective function per
 ** unit of power consumed due to a resource power mode.
 **
 ** @note
 ** In the problem formulation, these are the coefficients w_r.
 **/
void set_pow_penalty(qos_opt *this, int res, int penalty) {
/* 	int pointer; */
/* 	pointer = 1 + res * p_op->res_modes + res_mode; */
/* 	p_op->power[pointer]=l; */
}

int get_pow_penalty(qos_opt *this, int res) {
/*   qos_opt_heur * p_op = (qos_opt_heur *) this; */
  return 1;
}


static double qos_compute(qos_opt *this)
{
  qos_opt_heur * p_op = (qos_opt_heur *) this;
  int i;
  double res = 0;

  for (i = 0; i < p_op->num_apps; i++) {
    res += p_op->weights[i] * get_qos_level(this, i, p_op->appl_mode[i]);
  }
  for (i = 0; i < p_op->num_res; i++) {
    res -= get_pow_level(this, i, p_op->res_pot[i]);
  }

  return res;
}

static int schedulable_res(qos_opt *this, int res) {
  qos_opt_heur * p_op = (qos_opt_heur *) this;
  int app;
  double u = 0;

  for (app = 0; app < p_op->num_apps; ++app) {
    u += get_load(this, app, p_op->appl_mode[app], res, p_op->res_pot[res]);  
  }

#ifdef DEBUG_QOS_OPT
  dbg_printf("U: %f (res: %d)\n", u, res);
  for (app = 0; app < p_op->num_apps; ++app)
    dbg_printf("%d: %d\n", app, p_op->appl_mode[app]);
#endif

  return (u <= 1);
}

static int schedulable(qos_opt *this) {
  qos_opt_heur * p_op = (qos_opt_heur *) this;
  int res;
  for (res = 0; res < p_op->num_res; ++res) {
    if (! schedulable_res(this, res))
      return 0;
  }
  return 1;
}

int solve(qos_opt *this) {
  qos_opt_heur * p_op = (qos_opt_heur *) this;
  int i, done;
  double val, val2;

  for (i = 0; i < p_op->num_apps; i++) {
    p_op->appl_mode[i] = p_op->app_modes - 1;
  }
  for (i = 0; i < p_op->num_res; i++) {
    p_op->res_pot[i] = /*p_op->res_modes - 1*/ 0;
  }

  val = qos_compute(this);
  done = 0;
  while (!done) {
    int good_idx, good_idx2;

    good_idx = good_idx2 = -1;

    for (i = 0; i < p_op->num_apps; i++) {
      p_op->appl_mode[i]--;
      val2 = qos_compute(this);
      dbg_printf("A%d: ", i);
      if ((val2 > val) && schedulable(this)) {
        val = val2;
        good_idx = i;
      }
      p_op->appl_mode[i]++;
    }

    dbg_printf("Solution %d\n", good_idx); 
    if (good_idx >= 0) {
      p_op->appl_mode[good_idx]--;
    } else {
      for (i = 0; i < p_op->num_res; i++) {
	p_op->res_pot[i]++;
	val2 = qos_compute(this);
	dbg_printf("R%d: ", i);
	if ((val2 > val) && schedulable(this)) {
	  val = val2;
	  good_idx2 = i;
	}
	p_op->res_pot[i]--;
      }
      if (good_idx2 >= 0) {
	p_op->res_pot[good_idx2]++;
      } else {
	done = 1;
      }
    }
  }
  schedulable(this);
  p_op->obj_value=qos_compute(this);

  return done;
}

/** Get the zero-based application operation modes to be used as found during the last solver run,
 ** or set through the last set_app_mode() call.
 **/
int get_app_mode(qos_opt *this, int app) {
  qos_opt_heur * p_op = (qos_opt_heur *) this;
  return p_op->appl_mode[app];
}

/** Get the zero-based resource power modes to be used as found during the last solver run. **/
int get_res_mode(qos_opt *this, int res) {
  qos_opt_heur * p_op = (qos_opt_heur *) this;
  return p_op->res_pot[res];
}

/** Set artificially the zero-based resource power modes
 ** (does not affect solver, is merely returned back when asked through get_res_mode).
 **/
void set_res_mode(qos_opt *this, int res, int mode) {
  qos_opt_heur * p_op = (qos_opt_heur *) this;
  p_op->res_pot[res] = mode;
}

/** Get the objective function value corresponding to the solution found during the last solver run. **/
double get_obj_value(qos_opt *this) {
  qos_opt_heur * p_op = (qos_opt_heur *) this;
  return p_op->obj_value;
}

int set_it_limit(qos_opt *this, unsigned long it_limit) {
/*   qos_opt_heur * p_op = (qos_opt_heur *) this; */
  return -1;
}

static void set_max_power(qos_opt *this, pow_level l) {
  /* @todo */
}

static qos_opt_vtable vtable = {
  init,
  cleanup,
  set_qos_level,
  get_qos_level,
  set_qos_gain,
  get_qos_gain,
  set_qos_var_penalty,
  get_qos_var_penalty,
  set_app_mode,
  set_pow_level,
  get_pow_level,
  set_pow_penalty,
  get_pow_penalty,
  set_load,
  get_load,
  set_max_power,
  solve,
  get_app_mode,
  get_res_mode,
  set_res_mode,
  get_obj_value,
  set_it_limit
};

qos_opt *qos_opt_heur_create(int num_apps, int app_modes,
			     int num_res, int res_modes)
{
  qos_opt *p_op = malloc(sizeof(qos_opt_heur));
  if (p_op == NULL)
    return NULL;
  p_op->vtable = &vtable;
  qos_opt_init(p_op, num_apps, app_modes, num_res, res_modes);

  return p_op;
}
