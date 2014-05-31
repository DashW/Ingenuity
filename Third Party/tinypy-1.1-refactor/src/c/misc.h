#ifndef TP_MISC_H
#define TP_MISC_H

#include "tp.h"

tp_obj *tp_ptr(tp_obj o);
tp_obj _tp_dcall(TP,tp_obj fnc(TP));
tp_obj _tp_tcall(TP,tp_obj fnc);
tp_obj tp_fnc_new(TP,int t, void *v, tp_obj s, tp_obj g);
tp_obj tp_def(TP,void *v, tp_obj g);
tp_obj tp_fnc(TP,tp_obj v(TP));
tp_obj tp_method(TP,tp_obj self,tp_obj v(TP));
tp_obj tp_data(TP,int magic,void *v);
tp_obj tp_params(TP);
tp_obj tp_params_n(TP,int n, tp_obj argv[]);
tp_obj tp_params_v(TP,int n,...);

#endif
