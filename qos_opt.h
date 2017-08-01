#ifndef QOS_OPT_H_
#define QOS_OPT_H_

#include <stdlib.h>

#ifdef  __cplusplus
extern "C" {
#endif

/** Ranges from 0 to 100 **/
typedef unsigned int qos_level;

/** Ranges from 0 to 100 **/
typedef unsigned int qos_weight;

/** Power consumption, measurement unit to be defined **/
typedef unsigned int pow_level;

/** Resource load, in the range from 0.0 to 1.0 **/
typedef double qos_load;

/** The QoS global optimizer data type **/
typedef struct qos_opt {
  struct qos_opt_vtable *vtable;	/**< vtable */
} qos_opt;

typedef struct qos_opt_vtable {

  /** Initialise the data type supplying the maximum allowed number of things */
  void (*qos_opt_init)(qos_opt *p_op, int num_apps, int app_modes,
		    int num_res, int res_modes);

  /** Cleanup the data type freeing possibly associated resources (but not the data structure pointed to by p_op) */
  void (*qos_opt_cleanup)(qos_opt *p_op);

  /** Set the value of QoS level for each application mode
   **
   ** @note
   ** In the problem formulation, these are the coefficients of the v^(i) vector.
   **/
  void (*set_qos_level)(qos_opt *p_op, int app, int app_mode, qos_level l);

  qos_level (*get_qos_level)(qos_opt *p_op, int app, int app_mode);

  /** Set weight for application QoS mode.
   **
   ** This value constitutes the increment in the objective function per
   ** unit of QoS value due to an application mode.
   **
   ** @note
   ** In the problem formulation, these are the coefficients w_Q^(i).
   **/
  void (*set_qos_gain)(qos_opt *p_op, int app, qos_weight w);

  qos_weight (*get_qos_gain)(qos_opt *p_op, int app);

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
  void (*set_qos_var_penalty)(qos_opt *p_op, int app, int app_mode1, int app_mode2, int w);

  int (*get_qos_var_penalty)(qos_opt *p_op, int app, int app_mode1, int app_mode2);

  /** Explicitly set the current app mode for proper accounting of the contributes
   ** to the objective function due to application mode variations.
   **/
  void (*set_app_mode)(qos_opt *p_op, int app, int from_app_mode);

  /** Set the value of power consumption for each resource power mode
   **
   ** @note
   ** In the problem formulation, these are the coefficients of the vector k_r.
   **/
  void (*set_pow_level)(qos_opt *p_op, int res, int res_mode, pow_level l);

  pow_level (*get_pow_level)(qos_opt *p_op, int res, int res_mode);

  /** Set the weight for a resource power mode.
   **
   ** This value constitutes the decrement in the objective function per
   ** unit of power consumed due to a resource power mode.
   **
   ** @note
   ** In the problem formulation, these are the coefficients w_r.
   **/
  void (*set_pow_penalty)(qos_opt *p_op, int res, int penalty);

  int (*get_pow_penalty)(qos_opt *p_op, int res);

  /** Set the bandwidth requirement for each application mode and resource power mode.
   **
   ** @note
   ** In the problem formulation, these are the coefficients of the matrix B_r^(i).
   **/
  void (*set_load)(qos_opt *p_op, int app, int app_mode, int res, int res_mode, qos_load l);

  qos_load (*get_load)(qos_opt *p_op, int app, int app_mode, int res, int res_mode);

  /** Set the maximum allowed power consumption for the system.
   **
   ** @note
   ** In the problem formulation, this is the P coefficient.
   **/
  void (*set_max_power)(qos_opt *p_op, pow_level l);

  /** Find a good solution, not necessarily the optimum **/
  int (*qos_opt_solve)(qos_opt *p_op);

  /** Get the application operation modes to be used as found during the last solver run. **/
  int (*get_app_mode)(qos_opt *p_op, int app);

  /** Get the resource power modes to be used as found during the last solver run. **/
  int (*get_res_mode)(qos_opt *p_op, int res);

  /** Set artificially the zero-based resource power modes (does not affect solver,
   ** this is simply returned back when asked through get_res_mode() function).
   **/
  void (*set_res_mode)(qos_opt *p_op, int res, int mode);

  /** Get the objective function value corresponding to the solution found during the last solver run. **/
  double (*get_obj_value)(qos_opt *p_op);

  /** Set iteration limit if supported by implementation
   **
   ** @return
   ** -1 if iteration limit is not supported by implementation, or 0 if succesful.
   **/
  int (*set_it_limit)(qos_opt *p_op, unsigned long it_limit);

} qos_opt_vtable;	/* qos_opt_vtable */

/** Initialise the data type supplying the maximum allowed number of things */
static inline
void qos_opt_init(qos_opt *p_op, int num_apps, int app_modes,
                  int num_res, int res_modes) {
  p_op->vtable->qos_opt_init(p_op, num_apps, app_modes, num_res, res_modes);
}

static inline
void qos_opt_cleanup(qos_opt *p_op) {
  p_op->vtable->qos_opt_cleanup(p_op);
}

/** Destroy and free the data type pointed to by p_op, comprising any associated resources */
static inline
void qos_opt_destroy(qos_opt *p_op) {
  p_op->vtable->qos_opt_cleanup(p_op);
  free(p_op);
}

/** Set the value of QoS level for each application mode
 **
 ** @note
 ** In the problem formulation, these are the coefficients of the v^(i) vector.
 **/
static inline
void qos_opt_set_qos_level(qos_opt *p_op, int app, int app_mode, qos_level l) {
  p_op->vtable->set_qos_level(p_op, app, app_mode, l);
}

static inline
qos_level qos_opt_get_qos_level(qos_opt *p_op, int app, int app_mode) {
  return p_op->vtable->get_qos_level(p_op, app, app_mode);
}

/** Set weight for application QoS mode.
 **
 ** This value constitutes the increment in the objective function per
 ** unit of QoS value due to an application mode.
 **
 ** @note
 ** In the problem formulation, these are the coefficients w_Q^(i).
 **/
static inline
void qos_opt_set_qos_gain(qos_opt *p_op, int app, qos_weight w) {
  p_op->vtable->set_qos_gain(p_op, app, w);
}

static inline
qos_weight qos_opt_get_qos_gain(qos_opt *p_op, int app) {
  return p_op->vtable->get_qos_gain(p_op, app);
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
static inline
void qos_opt_set_qos_var_penalty(qos_opt *p_op, int app, int app_mode1, int app_mode2, int w) {
  p_op->vtable->set_qos_var_penalty(p_op, app, app_mode1, app_mode2, w);
}

static inline
int qos_opt_get_qos_var_penalty(qos_opt *p_op, int app, int app_mode1, int app_mode2) {
  return p_op->vtable->get_qos_var_penalty(p_op, app, app_mode1, app_mode2);
}

/** Explicitly set the current app mode for proper accounting of the contributes
 ** to the objective function due to application mode variations.
 **/
static inline
void qos_opt_set_app_mode(qos_opt *p_op, int app, int from_app_mode) {
  p_op->vtable->set_app_mode(p_op, app, from_app_mode);
}

/** Set the value of power consumption for each resource power mode
 **
 ** @note
 ** In the problem formulation, these are the coefficients of the vector k_r.
 **/
static inline
void qos_opt_set_pow_level(qos_opt *p_op, int res, int res_mode, pow_level l) {
  p_op->vtable->set_pow_level(p_op, res, res_mode, l);
}

static inline
pow_level qos_opt_get_pow_level(qos_opt *p_op, int res, int res_mode) {
  return p_op->vtable->get_pow_level(p_op, res, res_mode);
}

/** Set the weight for a resource power mode.
 **
 ** This value constitutes the decrement in the objective function per
 ** unit of power consumed due to a resource power mode.
 **
 ** @note
 ** In the problem formulation, these are the coefficients w_r.
 **/
static inline
void qos_opt_set_pow_penalty(qos_opt *p_op, int res, int penalty) {
  p_op->vtable->set_pow_penalty(p_op, res, penalty);
}

static inline
int qos_opt_get_pow_penalty(qos_opt *p_op, int res) {
  return p_op->vtable->get_pow_penalty(p_op, res);
}

/** Set the bandwidth requirement for each application mode and resource power mode.
 **
 ** @note
 ** In the problem formulation, these are the coefficients of the matrix B_r^(i).
 **/
static inline
void qos_opt_set_load(qos_opt *p_op, int app, int app_mode, int res, int res_mode, qos_load l) {
  p_op->vtable->set_load(p_op, app, app_mode, res, res_mode, l);
}

static inline
qos_load qos_opt_get_load(qos_opt *p_op, int app, int app_mode, int res, int res_mode) {
  return p_op->vtable->get_load(p_op, app, app_mode, res, res_mode);
}

/** Set the maximum allowed power consumption for the system.
 **
 ** @note
 ** In the problem formulation, this is the P coefficient.
 **/
static inline
void qos_opt_set_max_power(qos_opt *p_op, pow_level l) {
  p_op->vtable->set_max_power(p_op, l);
}

/** Find a good solution, not necessarily the optimum **/
static inline
int qos_opt_solve(qos_opt *p_op) {
  return p_op->vtable->qos_opt_solve(p_op);
}

/** Get the application operation modes to be used as found during the last solver run. **/
static inline
int qos_opt_get_app_mode(qos_opt *p_op, int app) {
  return p_op->vtable->get_app_mode(p_op, app);
}

/** Get the resource power modes to be used as found during the last solver run. **/
static inline
int qos_opt_get_res_mode(qos_opt *p_op, int res) {
  return p_op->vtable->get_res_mode(p_op, res);
}

/** Set artificially the zero-based resource power modes (does not affect solver,
 ** this is simply returned back when asked through get_res_mode() function).
 **/
static inline
void qos_opt_set_res_mode(qos_opt *p_op, int res, int res_mode) {
  p_op->vtable->set_res_mode(p_op, res, res_mode);
}

/** Get the objective function value corresponding to the solution found during the last solver run. **/
static inline
double qos_opt_get_obj_value(qos_opt *p_op) {
  return p_op->vtable->get_obj_value(p_op);
}

/** Set iteration limit if supported by implementation
 **
 ** @return
 ** -1 if iteration limit is not supported by implementation, or 0 if succesful.
 **/
static inline
int qos_opt_set_it_limit(qos_opt *p_op, unsigned long it_limit) {
  return p_op->vtable->set_it_limit(p_op, it_limit);
}

#ifdef  __cplusplus
}
#endif

#endif /*QOS_OPT_H_*/
