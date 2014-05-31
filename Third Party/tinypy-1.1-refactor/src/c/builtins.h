#ifndef TP_BUILTINS_H
#define TP_BUILTINS_H

#include "tp.h"

tp_obj tp_print(TP);
tp_obj tp_bind(TP);
tp_obj tp_min(TP);
tp_obj tp_max(TP);
tp_obj tp_copy(TP);
tp_obj tp_len_(TP);
tp_obj tp_assert(TP);
tp_obj tp_range(TP);
tp_obj tp_system(TP);
tp_obj tp_istype(TP);
tp_obj tp_float(TP);
tp_obj tp_save(TP);
tp_obj tp_load(TP);
tp_obj tp_fpack(TP);
tp_obj tp_abs(TP);
tp_obj tp_int(TP);
tp_num _roundf(tp_num v);
tp_obj tp_round(TP);
tp_obj tp_exists(TP);
tp_obj tp_mtime(TP);

#endif
