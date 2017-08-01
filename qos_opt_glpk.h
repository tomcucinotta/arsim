#ifndef __QOS_OPT_GLPK_H__
#define __QOS_OPT_GLPK_H__

#include "qos_opt.h"

#ifdef  __cplusplus
extern "C" {
#endif

extern qos_opt *qos_opt_glpk_create(int num_apps, int app_modes,
				    int num_res, int res_modes);

#ifdef  __cplusplus
}
#endif

#endif /* __QOS_OPT_GLPK_H__ */
