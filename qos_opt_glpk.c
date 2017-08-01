/* Dear Tom,
It is not easy to explain this piece of code. I will try to do it in the best possible way. I think it is working now but I still have to prove it again with more tasks. Actually is running for 3 app, 3 Qos, 3 res, 2 power modes.

1) I have changed the penalty function for changing the QoS as I explained it to you in the mail.
2) I have defined the qos_opt structure with many fields. This structure has capacity for ten applications with 10 qos, 3 resources with 10 power modes. Of course I am not sure if it really works with such a big numbers.
3) I have defined a new function to create the problem under the glpk model. This function creates the problem from the data stored in the qos_opt structure.
4) I have introduced some debug printings to follow the program. Anyway commenting the define will disallow all of them.


*/

#include "qos_opt_glpk.h"

#include <stdio.h>
#include <stdlib.h>
#include <glpk.h>
#include <string.h>
#include <time.h>
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

#define LPX glp_prob
#define GLP_E_OK 0

typedef struct qos_opt_glpk {
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
  LPX *lp;
} qos_opt_glpk;

#ifdef DEBUG_QOS_OPT
#  define dbg_printf(fmt, args...) do { printf(fmt,##args); fflush(stdout); } while (0)
#else
#  define dbg_printf(fmt, args...) do { } while (0)
#endif

/** Return the ONE-BASED column index corresponding to the supplied ZERO-BASED parameters **/
static int col_idx(qos_opt_glpk *p_op, int app, int app_mode, int res, int res_mode) {
  return 1 + app_mode
           + res_mode * p_op->app_modes
           + app * p_op->app_modes * p_op->res_modes
           + res * p_op->app_modes * p_op->res_modes * p_op->num_apps;
}

/** Return the ZERO-BASED index within the penalty[] array corresponding to the supplied ZERO-BASED parameters */
static int pen_idx(qos_opt_glpk *p_op, int app, int am1, int am2) {
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
  qos_opt_glpk *p_op = (qos_opt_glpk *) this;
  int a, am1, am2;
  p_op->num_apps=num_apps;
  p_op->app_modes=app_modes;
  p_op->num_res=num_res;
  p_op->res_modes=res_modes;
  p_op->lp = NULL;
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
static void set_qos_level(qos_opt *this, int app, int app_mode, qos_level lev) {
  qos_opt_glpk *p_op = (qos_opt_glpk *) this;
  int pointer;
  pointer = app * p_op->app_modes + app_mode;
  p_op->levels[pointer] = lev;
  dbg_printf("Set qos for app %d in mode %d (idx %d) to lev %d\n", app, app_mode, pointer, p_op->levels[pointer]);
}

static qos_level get_qos_level(qos_opt *this, int app, int app_mode) {
  qos_opt_glpk *p_op = (qos_opt_glpk *) this;
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
static void set_qos_gain(qos_opt *this, int app, qos_weight w) {
  qos_opt_glpk *p_op = (qos_opt_glpk *) this;
  int pointer;
  pointer = app;
  p_op->weights[pointer]=w;
  dbg_printf("Set weight for app %d (idx %d) to weight %d\n", app, pointer, p_op->weights[pointer]);
}

static qos_weight get_qos_gain(qos_opt *this, int app) {
  qos_opt_glpk *p_op = (qos_opt_glpk *) this;
  int pointer;
  pointer = app;
  return p_op->weights[pointer];
}

/** Explicitly set the current app mode for proper accounting of the contributes
 ** to the objective function due to application mode variations.
 **/
static void set_app_mode(qos_opt *this, int app, int from_app_mode) {
  qos_opt_glpk *p_op = (qos_opt_glpk *) this;
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
static void set_qos_var_penalty(qos_opt *this, int app, int app_mode1, int app_mode2, int w) {
  qos_opt_glpk *p_op = (qos_opt_glpk *) this;
  int pointer;
  pointer = pen_idx(p_op, app, app_mode1, app_mode2);
  p_op->penalty[pointer] = w;
  pointer = pen_idx(p_op, app, app_mode2, app_mode1);
  p_op->penalty[pointer] = w;
  dbg_printf("Set penalty for app %d from mode %d to mode %d (idx %i) to %d\n",
              app, app_mode1, app_mode2, pointer, p_op->penalty[pointer]);
}

static int get_qos_var_penalty(qos_opt *this, int app, int app_mode1, int app_mode2) {
  qos_opt_glpk *p_op = (qos_opt_glpk *) this;
  int pointer;
  pointer = pen_idx(p_op, app, app_mode1, app_mode2);
  return p_op->penalty[pointer];
}

/** Set the value of power consumption for each resource power mode
 **
 ** @note
 ** In the problem formulation, these are the coefficients of the vector k_r.
 **/
static void set_pow_level(qos_opt *this, int res, int res_mode, pow_level l) {
  qos_opt_glpk *p_op = (qos_opt_glpk *) this;
  int pointer;
  pointer = res * p_op->res_modes + res_mode;
  p_op->powers[pointer]=l;
  dbg_printf("Set res %d in mode %d (idx %d) to consume %d\n", res, res_mode, pointer, p_op->powers[pointer]);
}

static pow_level get_pow_level(qos_opt *this, int res, int res_mode) {
  qos_opt_glpk *p_op = (qos_opt_glpk *) this;
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
static void set_load(qos_opt *this, int app, int app_mode, int res, int res_mode, qos_load l) {
  qos_opt_glpk *p_op = (qos_opt_glpk *) this;
  int idx = col_idx(p_op, app, app_mode, res, res_mode);
  p_op->bands[idx] = l;
  dbg_printf("Set app %d in mode %d on res %d in mode %d (idx %d) to load %f\n",app, app_mode, res, res_mode, idx, l);
}

static qos_load get_load(qos_opt *this, int app, int app_mode, int res, int res_mode) {
  qos_opt_glpk *p_op = (qos_opt_glpk *) this;
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
static void set_pow_penalty(qos_opt *this, int res, int penalty) {
/*   qos_opt_glpk *p_op = (qos_opt_glpk *) this; */
/* 	int pointer; */
/* 	pointer = 1 + res * p_op->res_modes + res_mode; */
/* 	p_op->power[pointer]=l; */
}

static int get_pow_penalty(qos_opt *this, int res) {
/*   qos_opt_glpk *p_op = (qos_opt_glpk *) this; */
  return 1;
}

/** Find a good solution, not necessarily the optimum **/
static int qos_opt_solve_lp(qos_opt_glpk *p_op,LPX *lp) {
  	int n,m,r,p,am,rm;//apl,eng,vble; //,pot,paso2,paso3,modo;
	int ret = 0, glp_rv, glp_mip = -578;

	int *ind4;//,*ind2,*ind3,*ind4;
        int col,a,res;
        int cols,rows;  //,vinc,length,paso,salto;
	//char str[10];
	//char x[]="x";
	//char v[]="v";
	int *cof;
	double *b;
	double *vale;//,*val1,*val2,*val3,*val4;
//	float var;
	int pointer;

	//cols stands for the number of binary variables that are generated
	n=p_op->num_apps;
	m=p_op->app_modes;
	r=p_op->num_res;
	p=p_op->res_modes;
	cols=n*m*p*r;

	// rows is the number of constraints present in the system.
	// The constraints are of mode, energy/resource, application mode/resource, bandwidth.
	// Quadratic terms are due to unicity constraints ?
	// - only 1 app_mode per application may be active, for all resources and res modes:
	// - only 1 res_mode per resource may be active, for all application and app_modes:
	rows = n*r + p*(p-1)*n*(n-1)/2*r + n*m*(r-1) + r;

	cof=(int*) calloc(cols+1,sizeof(double)); 
        b=(double*) calloc(cols+1,sizeof(double));

	for(col=1;col<=cols;col++) {
		b[col]=p_op->bands[col];
		dbg_printf("Bands \n %i\t%f\n",col,b[col]);
	}

	int length=n*m*p;
	ind4=(int*)calloc(length+1,sizeof(int)); 
	vale=(double*)calloc(length+1,sizeof(double)); 
	for (int i=n*r+p*(p-1)*n*(n-1)/2*r+m*n*(r-1)+1;i<=rows;i++){
	  for (int j=1;j<=length;j++){
	    ind4[j]=j+length*(i-(n*r+p*(p-1)*n*(n-1)/2*r+m*n*(r-1)+1));
	    vale[j]=b[j+length*(i-(n*r+p*(p-1)*n*(n-1)/2*r+m*n*(r-1)+1))]; //  (i-r*(n+p)-m*n-1)];
	  }
	  glp_set_mat_row(lp,i,length,ind4,vale);
	  glp_set_row_bnds(lp, i, GLP_UP, 0, 1);
	}

	//The first approach is with all the applications at high quality maximum power.
        //This means the penalty for transitions to other modes is from 1->2 1->3 etc.
	//I have to distinguish between appl and power modes.
	//appl_modes[10]
        //Verificar los indices en los arreglos para multirecurso! Esto debe tener errores
	dbg_printf("\n Coefficients optimization\n");
	for (res = 0; res < r; ++res) {
	  for (a = 0; a < n; ++a) {
	    for (rm = 0; rm < p; ++rm) {
	      for (am = 0; am < m; ++am) {
			if (am == p_op->appl_mode[a]) {
			  cof[col_idx(p_op, a, am, res, rm)] =
			    p_op->levels[a*m+am] / r - p_op->powers[rm+res*p] / n;
				dbg_printf("Coeficientes Columnas\n %i\t%i\n",p_op->levels[a*m+am],p_op->powers[rm+res*p]);
			} else {
			  pointer = pen_idx(p_op, a, p_op->appl_mode[a], am);
			  cof[col_idx(p_op, a, am, res, rm)] =
			    p_op->levels[a*m+am] / r - p_op->powers[rm+res*p] / n - p_op->penalty[pointer] / r;
				dbg_printf("Coeficientes Columnas\n %i\t%i\t%i\n",p_op->levels[a*m+am],p_op->powers[rm+res*p],p_op->penalty[pointer]);
			}
			dbg_printf("Coeficientes Columnas\n %i\t%i\n",col_idx(p_op, a, am, res, rm), cof[col_idx(p_op, a, am, res, rm)]);
	      }
	    }
	  }
	}
       for (col=1;col<=cols;col++) {
         glp_set_obj_coef(lp,col,(double)cof[col]);
       }
       //glp_set_int_parm(lp,LPX_K_MSGLEV,0);
#ifdef DEBUG_QOS_OPT
       //glp_print_lp(lp,"ejem_prob.txt");
#endif
       if ((glp_rv = glp_simplex(lp, NULL)) != GLP_E_OK)
         goto err;
       if ((glp_rv = glp_intopt(lp, NULL)) != GLP_E_OK)
	 goto err;
       if ((glp_mip = glp_mip_status(lp)) != GLP_OPT)
         goto err;
       //if ((glp_rv = glp_simplex(lp, NULL)) != GLP_E_OK)
       //  goto err;
       //if ((glp_rv = glp_integer(lp)) != GLP_E_OK)
       //  goto err;
       ret = 1;
#ifdef DEBUG_QOS_OPT
       //glp_write_mps(lp,"ejem_mps.txt");
       //glp_print_sol(lp, "ejem_sol.txt");
#endif

       int q;
       for (res = 0; res < r; ++res) {
         for (a = 0; a < n; ++a) {
           for (rm = 0; rm < p; ++rm) {
             for (am = 0; am < m; ++am) {
               q = (int) glp_mip_col_val(lp, col_idx(p_op, a, am, res, rm)); //verificar
               if (q == 1) {
                 p_op->appl_mode[a] = am; // stored app_mode is 0-based
                 p_op->res_pot[res] = rm; // stored res_mode is 0-based
               }
             }
           }
         }
       }
       dbg_printf("End of optimization: ");
       err:
       p_op->obj_value=glp_mip_obj_val(lp);
       switch (glp_rv) {
        case GLP_E_OK: dbg_printf("Simplex Success\n"); break;
        /* case GLP_E_EMPTY: dbg_printf("Empty\n"); break; */
        /* case GLP_E_BADB: dbg_printf("Bad basis\n"); break; */
        /* case GLP_E_INFEAS: dbg_printf("Infeasible initial solution\n"); break; */
        /* case GLP_E_FAULT: dbg_printf("Fault\n"); break; */
        /* case GLP_E_NOFEAS: dbg_printf("No feasible solution exists\n"); break; */
        /* case GLP_E_INSTAB: dbg_printf("Numerically unstable\n"); break; */
        default: dbg_printf("Uncatalogued glp_rv code\n"); break;
       }
       switch (glp_mip) {
       case GLP_UNDEF: dbg_printf("Undefined MIP solution\n"); break;
       case GLP_OPT: dbg_printf("Optimum MIP solution found\n"); break;
       case GLP_FEAS: dbg_printf("FEAS MIP\n"); break;
       case GLP_NOFEAS: dbg_printf("NOFEAS MIP\n"); break;
       case -578: dbg_printf("glp_mip error code not set\n"); break;
       default: dbg_printf("Uncatalogued glp_mip code\n"); break;
       }
       free(cof);
       free(b);
       return ret;
}

/** Initializes the problem for glpk**/
static LPX *qos_opt_problem(qos_opt_glpk *p_op) {
	int n,m,r,p,am,apl,eng,vble,rm,paso2,paso3,modo, res, a;
	LPX *lp;
	int *ind1,*ind2,*ind3,*ind4;
        int i,j;
        int cols,rows,vinc,length,paso; //salto;
	char str[16];
	int *cof;
	double *b;
	double *vale,*val1,*val2,*val3;//,*val4;
//	float var;

	//cols stands for the number of binary variables that are generated
	n=p_op->num_apps;
	m=p_op->app_modes;
	r=p_op->num_res;
	p=p_op->res_modes;
	cols=n*m*p*r;

	dbg_printf("Variables %i\n\n\n\n\n\n",n*m*r*p);
	//rows is the number of constraints present in the system.

	//The constraints are of mode, energy/resource, application mode/resource, bandwidth.
	rows=n*r+p*(p-1)*n*(n-1)/2*r+n*m*(r-1)+r;

	cof=(int*) calloc(cols+1,sizeof(double)); 
        b=(double*) calloc(cols+1,sizeof(double)); 

	for(i=1;i<=cols;i++) {
		b[i]=p_op->bands[i];
	}
	//The first approach is with all the applications at high quality maximum power.
        //This means the penalty for transitions to other modes is from 1->2 1->3 etc.
	//I have to distinguish between appl and power modes.
	//appl_modes[10]
        //Verificar los indices en los arreglos para multirecurso! Esto debe tener errores

	for (res = 0; res < r; ++res) {
	  for (a = 0; a < n; ++a) {
	    for (rm = 0; rm < p; ++rm) {
	      for (am = 0; am < m; ++am) {
		if (am == p_op->appl_mode[a]) {
		  cof[col_idx(p_op, a, am, res, rm)] = 
		    p_op->levels[a*m+am] - p_op->powers[rm+res*p];
		} else {
		  int pointer = pen_idx(p_op, a, p_op->appl_mode[a], am);
		  cof[col_idx(p_op, a, am, res, rm)] = 
		    p_op->levels[a*m+am] - p_op->powers[rm+res*p] - p_op->penalty[pointer];
		}
	      }
	    }
	  }
	}
        //Matrix vincoli
//	int w;

	lp = glp_create_prob(); //this function creates the problem in glpk
        //glp_set_class(lp,GLP_MIP); //the kind of problem to be solved, in this case integer.
        glp_set_prob_name(lp, "Maxi_Profit"); //name of the problem
        glp_set_obj_dir(lp, GLP_MAX); //we are trying to maximize the profit.

        //cols is the number of binary variables in the system. Each tuple
        //(application, mode,power, resource) represents a variable.
        glp_add_cols(lp, cols);
	//This for cycle construct the columns under glpk. It can be avoided on run time once
	//the problem is defined.
        for (res = 0; res < r; ++res) {
          for (a = 0; a < n; ++a) {
            for (rm = 0; rm < p; ++rm) {
              for (am = 0; am < m; ++am) {
                int idx = col_idx(p_op, a, am, res, rm);
		sprintf(str, "x%dr%da%drm%dam%d", idx, res, a, rm, am);
		glp_set_col_name(lp, idx, str);
                glp_set_col_bnds(lp, idx, GLP_DB,0.0,1.0);
                glp_set_col_kind(lp, idx, GLP_IV);
              }
            }
          }
	}

   //Vincoli definition. Like in the case of columns this has to be created only once

        glp_add_rows(lp, rows);
        for (i=1; i <= rows; i++) {
          sprintf(str, "v%d", i);
          glp_set_row_name(lp, i, str);
        }

      //Constraints definition.
      //vincoli modi-The application can only be in one mode and power.
      vinc=m*p;
      length=vinc+1;
      ind1=(int*)calloc(length,sizeof(int)); 
      val1=(double*)calloc(length,sizeof(double)); 
      int g;
      paso=0;
      for (g=1;g<=r;g++){
       	for (i=1;i<=n;i++){
      		for (j=1;j<=vinc;j++){
			ind1[j]=j+vinc*(i-1)+n*m*p*(g-1);
			val1[j]=1.0;
		}
	        glp_set_mat_row(lp,(i+paso),vinc,ind1,val1);
        	glp_set_row_bnds(lp, (i+paso), GLP_FX, 1, 1);
	}
	paso+=n;
      }

      //vincoli energia-each resource should be on only one power mode for every application.
      length=2*m;
      ind2=(int*) calloc(length+1,sizeof(int)); 
      val2=(double*) calloc(length+1,sizeof(double)); 
      int recurso=n*m*p;

      i=n*r+1;
      for (res=1; res<=r; res++) {
        for (apl=1; apl<n; apl++) {
          for (eng=1; eng<=p; eng++) {
            for (vble=apl+1; vble<=n; vble++) {
              for (rm=1; rm<=p; rm++) {
                if (rm!=eng) {
                  for (paso=0; paso<=1; paso++)
                    for (j=1; j<=m; j++) {
                      ind2[j+m*paso]=j+(eng-1)*m+m*p*(apl-1)+paso*(m*p*(vble-apl)
                          +(rm-eng)*m)+recurso*(res-1);
                      val2[j+m*paso]=1;
                    }
                  glp_set_mat_row(lp, i, length, ind2, val2);
                  glp_set_row_bnds(lp, i, GLP_UP, 1, 1);
                  i++;
                }
              }
            }
          }
        }
      }

      //vincoli modo resource-Each resource should run on
      //the same mode on every resource involved.
      length=p+(m-1)*p;
      ind3=(int*)calloc(length+1,sizeof(int)); 
      val3=(double*)calloc(length+1,sizeof(double)); 
      int aplica=m*p*n;
      i=n*r+p*(p-1)*n*(n-1)/2*r;//n*r+n*(n-1)*p*r+1;

      for (apl=1;apl<=n;apl++){
        for (res=1;res<=r-1;res++){
		for (modo=1;modo<=m;modo++){
		   paso2=1;
		   paso3=p+1;
		   for (rm=1;rm<=p;rm++){
		      for (j=1;j<=m;j++){
		      	 if(j==modo){
			 	ind3[paso2]=j+(apl-1)*m*p+m*(rm-1);
			 	val3[paso2]=1.0;
				paso2++;
			 }
			 else{
			 	ind3[paso3]=j+(apl-1)*m*p+(aplica*res+m*(rm-1));
				val3[paso3]=1.0;
				paso3++;
			 }
	      	      }
	   	   }
	        glp_set_mat_row(lp,i,length,ind3,val3);
        	glp_set_row_bnds(lp,i, GLP_UP, 1,1);
                i++; 
	    }			 
        }
      }

      //vincoli banda-The bandwidth requirement for each resource should be below the upper bound
      //Up to this moment all the parameters are fixed and depend on the system to optimize. 
      //The bandwidth constraint should be updated on each case because it may be different
      //as a result of an actualization of its values.
     length=n*m*p;
     ind4=(int*)calloc(length+1,sizeof(int)); 
     vale=(double*)calloc(length+1,sizeof(double)); 
     for (i=n*r+p*(p-1)*n*(n-1)/2*r+m*n*(r-1)+1;i<=rows;i++){
	     for (j=1;j<=length;j++){
      		ind4[j]=j+length*(i-(n*r+p*(p-1)*n*(n-1)/2*r+m*n*(r-1)+1));
		vale[j]=b[j+length*(i-(n*r+p*(p-1)*n*(n-1)/2*r+m*n*(r-1)+1))]; //  (i-r*(n+p)-m*n-1)];
	     }
	     glp_set_mat_row(lp,i,length,ind4,vale);
      	     glp_set_row_bnds(lp, i, GLP_UP, 0, 1);
      }
      free(ind1);
      free(ind2);
      free(ind3);
      free(ind4);
      free(val1);
      free(val2);
      free(val3);
      free(vale);
      free(cof);
      free(b);
      return lp;
}

static int solve(qos_opt *this) {
  qos_opt_glpk *p_op = (qos_opt_glpk *) this;
  if (p_op->lp == NULL)
    p_op->lp = qos_opt_problem(p_op);
  return qos_opt_solve_lp(p_op, p_op->lp);
}

/** Get the zero-based application operation modes to be used as found during the last solver run,
 ** or set through the last set_app_mode() call.
 **/
static int get_app_mode(qos_opt *this, int app) {
  qos_opt_glpk *p_op = (qos_opt_glpk *) this;
  return p_op->appl_mode[app];
}

/** Get the zero-based resource power modes to be used as found during the last solver run. **/
static int get_res_mode(qos_opt *this, int res) {
  qos_opt_glpk *p_op = (qos_opt_glpk *) this;
  return p_op->res_pot[res];
}

/** Set artificially the zero-based resource power modes
 ** (does not affect solver, is merely returned back when asked through get_res_mode).
 **/
static void set_res_mode(qos_opt *this, int res, int mode) {
  qos_opt_glpk *p_op = (qos_opt_glpk *) this;
  p_op->res_pot[res] = mode;
}

/** Get the objective function value corresponding to the solution found during the last solver run. **/
static double get_obj_value(qos_opt *this) {
  qos_opt_glpk *p_op = (qos_opt_glpk *) this;
  return p_op->obj_value;
}

/** Set iteration limit if supported by implementation */
static int set_it_limit(qos_opt *this, unsigned long it_limit) {
  qos_opt_glpk *p_op = (qos_opt_glpk *) this;
  glp_set_it_cnt(p_op->lp, it_limit);
  return 0;
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

qos_opt *qos_opt_glpk_create(int num_apps, int app_modes,
			     int num_res, int res_modes)
{
  qos_opt *p_op = malloc(sizeof(qos_opt_glpk));
  if (p_op == NULL)
    return NULL;
  p_op->vtable = &vtable;
  qos_opt_init(p_op, num_apps, app_modes, num_res, res_modes);

  return p_op;
}
