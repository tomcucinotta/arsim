extern "C" {
#include "qos_opt.h"
}

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <assert.h>

#define DEBUG

struct qos_opt opt;

int main(int argc, char *argv[])
{
  int n,m,r,p;
  unsigned long rows, cols;
  struct timeval tv_beg, tv_end;
  int i, j, h, k, rec, pot;

  if (argc != 2) {
    printf("Usage: %s na,nam,nr,nrm\n", argv[0]);
    exit(-1);
  }

  assert(sscanf(argv[1], "%d,%d,%d,%d", &n, &m, &r, &p) == 4);

    //cols stands for the number of binary variables that are generated
    cols=n*m*p*r;

    //rows is the number of constraints present in the system.
    //The constraints are of mode, energy/resource, application mode/resource, bandwidth.
    rows=n*r+p*(p-1)*n*(n-1)/2*r+n*m*(r-1)+r;

    qos_opt_init(&opt, n, m, r, p);

#ifdef DEBUG
    printf("\n QoS levels profit\n");
#endif
    for (i = 0; i < n; i++) {
      for (j = 0; j < m; j++) {
	//int l = (int) (rand() * 100 / RAND_MAX);
	int l = (m - j) * 100 / m;
	set_qos_level(&opt, i, j, l);
      }
    }

#ifdef DEBUG
    printf("\n QoS gains\n");
#endif
    for (i = 0; i < n; i++)
      set_qos_gain(&opt, i, 1);

#ifdef DEBUG
    printf("\n QoS change Penalties\n");
#endif
    for (i = 0; i < n; i++) {
      for (j = 0; j < m; j++) {
	for (k = 0; k < m; k++) {
	  set_qos_var_penalty(&opt, i, j, k, 0);
	}
      }
    }

#ifdef DEBUG
    printf("\n Starting application modes\n");
#endif
    for (i = 0; i < n; i++)
      set_app_mode(&opt, i, 0);

#ifdef DEBUG
    printf("\n Power modes penalties\n");
#endif
    for (rec = 0 ; rec < r; rec++) {
      for (pot = 0; pot < p; pot++) {
	//int pow = (int)(rand() * 100 / RAND_MAX);
	int pow = (p - pot) * 100 / p;
	set_pow_level(&opt, rec, pot, pow);
      }
    }

#ifdef DEBUG
    printf("\n Power penalties\n");
#endif
    for (rec = 0; rec < r; rec++)
      set_pow_penalty(&opt, rec, 1);

#ifdef DEBUG
    printf("\n Starting power modes\n");
#endif
    for (rec = 0; rec < n; rec++)
      set_res_mode(&opt, rec, 0);

#ifdef DEBUG
    printf("\n Loads\n");
#endif
    for (rec = 0; rec < r; rec++) {
      for (j = 0; j < n; j++) {
	for (pot = 0; pot < p; pot++) {
	  for (h = 1; h <m; h++) {
	    double bw = rand();
	    set_load(&opt, j, h, rec, pot, bw);
	  }
	}
      }
    }

    gettimeofday(&tv_beg, NULL);

    if (! qos_opt_solve(&opt)) {
      fprintf(stderr, "WARNING: could not solve the opt problem !\n");
    }

    gettimeofday(&tv_end, NULL);

    printf("\n Solution to the problem \n\n Application's modes\n");
    for (i = 0; i < n; i++) {
      int appl_mode = get_app_mode(&opt, i);
      printf("%i\t%i\n", i, appl_mode);
    }
    printf("Resource's modes\n");
    for (i = 0; i < r ; i++) {
      int pow_mode = get_res_mode(&opt, i);
      printf("%i\t%i\n", i, pow_mode);
    }

    double obj_val = get_obj_value(&opt);
    long unsigned elapsed = (tv_end.tv_sec - tv_beg.tv_sec) * 1000000ul + (tv_end.tv_usec - tv_beg.tv_usec);
    printf("na, nam, nr, nrm: %d %d %d %d\n", n, m, r, p);
    printf("rows: %lu\n", rows);
    printf("cols: %lu\n", cols);
    printf("obj_val: %g\n", obj_val);
    printf("elapsed: %lu us\n", elapsed);

  return 0;
}
