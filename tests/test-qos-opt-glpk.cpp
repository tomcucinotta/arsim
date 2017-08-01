extern "C" {
#include "qos_opt.h"
}

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>

/** This function calls all the rest. The qos_opt struct is defined in this function but its fields are not completed by the main but by the particular functions
You can change n (number of applications),m (qos modes),r (resources),p (power modes), but you will also need to complete the different arrays where the information corresponding to the fields of the structure are saved. They have the same name as in the structure so it is simple. For example, the starting application mode is appl_mode and in the structure you can access it by p_op->appl_mode.

Any doubt we can get in contact by means of skype. My nickname there is rmsmambi 
 **/

int main(void)
{
        int n,m,r,p,h,apl,eng,vble,pot,paso2,paso3,modo;
        clock_t init_time, end_time;
        float comp_time;
        FILE *fr;
        LPX *lp;
        int *ind1,*ind2,*ind3,*ind4;
        int i,j;
        int cols,rows,vinc,length,paso,salto;
        char str[10];
        char x[]="x";
        char v[]="v";
        int *cof;
        double *b;
        double *vale,*val1,*val2,*val3,*val4;
        float var;
        int rec, aux_1, aux_2;
        int pointer;
        struct qos_opt p_op;
        unsigned int levels[9]={10, 20, 1000, 10, 500, 5,1000,60,5};
        unsigned int  powers[6]={0,300,0,300,100,0};//, 10,0};
        unsigned int penalty[9]={5,10,5,5,10,5,5,10,5};
        int appl_mode[3]={1,1,1};
        double weights[3]={1,1,1};
        double bands[54]={0.3,0.2,0.1,0.15,0.1,0.05,0.4,0.3,0.2,0.2,0.15,0.1,0.3,0.2,0.1,0.15,0.1,0.05,0.3,0.2,0.1,0.15,0.1,0.05,0.4,0.3,0.2,0.2,0.15,0.1,0.3,0.2,0.1,0.15,0.1,0.05,0.3,0.2,0.1,0.15,0.1,0.05,0.4,0.3,0.2,0.2,0.15,0.1,0.3,0.2,0.1,0.15,0.1,0.05};
        double gain;
        //Data is read from a file that has been randomly generated
        n=3;
        m=3;
        r=3;
        p=2;
        //cols stands for the number of binary variables that are generated
        cols=n*m*p*r;
        
        //rows is the number of constraints present in the system.
        
        //The constraints are of mode, energy/resource, application mode/resource, bandwidth.
        rows=n*r+p*(p-1)*n*(n-1)/2*r+n*m*(r-1)+r;
        
        cof=(int*) calloc(cols+1,sizeof(double)); 
        b=(double*) calloc(cols+1,sizeof(double)); 
        for (rec=1;rec<=r;rec++){
          for (j=0;j<n;j++){
            for (pot=1;pot<=p;pot++){
              for (h=1;h<=m;h++){
                if (h==appl_mode[j]){
                  cof[h+(pot-1)*m+j*m*p+(rec-1)*n*m*p]=levels[j*m+h-1]-powers[pot-1];
                  pointer=-1;
                  
                }
                else{
                  if (h>appl_mode[j]){
                    aux_1=appl_mode[j];
                    aux_2=h;
        
                  }
                  else{
                    aux_1=h;
                    aux_2=appl_mode[j];
                  
                  }
                   pointer=j*m*(m-1)/2+(aux_1-1)*m-aux_1*(aux_1-1)/2+aux_2-aux_1-1;
                   cof[h+(pot-1)*m+j*m*p+(rec-1)*n*m*p]=levels[j*m+h-1]-powers[pot-1]-penalty[pointer];
                }
              }
            }
          }
        }

        qos_opt_init(&p_op, n, m, r, p);

        for (i=0;i<n;i++)
          set_qos_gain(&p_op, i, weights[i]);
#ifdef DEBUG
printf("\n Starting application modes\n");
#endif
        for (i=0;i<n;i++)
          set_app_mode(&p_op, i, appl_mode[i]);

#ifdef DEBUG
printf("\n QoS levels profit\n");
#endif
          for (i=0;i<n;i++){
            for (j=1;j<=m;j++){
              set_qos_level(&p_op, i,j,levels[i*m+j-1]);
            }
          }
        
#ifdef DEBUG
 printf("\n QoS change Penalties\n");
#endif
        set_qos_var_penalty(&p_op, 0,1,2,5);
        set_qos_var_penalty(&p_op, 0,1,3,10);
        set_qos_var_penalty(&p_op, 0,2,3,5);
        set_qos_var_penalty(&p_op, 1,1,2,5);
        set_qos_var_penalty(&p_op, 1,1,3,10);
        set_qos_var_penalty(&p_op, 1,2,3,5);
        set_qos_var_penalty(&p_op, 2,1,2,5);
        set_qos_var_penalty(&p_op, 2,1,3,10);
        set_qos_var_penalty(&p_op, 2,2,3,5);

#ifdef DEBUG
printf("\n Power modes penalties\n");
#endif
        for (rec=1;rec<=r;rec++){
          for (pot=1;pot<=p;pot++){
            set_pow_level(&p_op, rec, pot, powers[pot-1+(rec-1)*p]);
          }
        }
        for (rec=1;rec<=r;rec++){
          for (j=0;j<n;j++){
            for (pot=1;pot<=p;pot++){
              for (h=1;h<=m;h++){
                  set_load(&p_op, j, h, rec, pot, bands[(rec-1)*(n*m*p)+j*m*p+(pot-1)*m+h-1]);
              }
            }
          }
        }
        lp=qos_opt_problem(&p_op);
        qos_opt_solve(&p_op);
        printf("\n Solution to the problem \n\n Application's modes\n");
        for (i=0;i<n;i++){
              appl_mode[i]=get_app_mode(&p_op, i);
              printf("%i\t%i\n",i,appl_mode[i]);
        }
          printf("Resource's modes\n");
        for (i=1;i<=r;i++){
            powers[i]=get_res_mode(&p_op, i);
            printf("%i\t%i\n",i,powers[i]);
        }

            gain=get_obj_value(&p_op);    
      free(cof);
      free(b);

        return 1;
}
