#ifndef TP_STRING_H
#define TP_STRING_H

#include "tp.h"

tp_obj tp_string_t(TP, int n);
tp_obj tp_printf(TP, char const *fmt,...);
int _tp_str_index(tp_obj s, tp_obj k);
tp_obj tp_join(TP);
tp_obj tp_string_slice(TP,tp_obj s, int a, int b);
tp_obj tp_split(TP);
tp_obj tp_find(TP);
tp_obj tp_str_index(TP);
tp_obj tp_str2(TP);
tp_obj tp_chr(TP);
tp_obj tp_ord(TP);
tp_obj tp_strip(TP);
tp_obj tp_replace(TP);

#endif
