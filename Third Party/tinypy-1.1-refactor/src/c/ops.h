#ifndef TP_OPS_H
#define TP_OPS_H

#include "tp.h"

tp_obj tp_str(TP,tp_obj self);
int tp_bool(TP,tp_obj v);
tp_obj tp_has(TP,tp_obj self, tp_obj k);
void tp_del(TP,tp_obj self, tp_obj k);
tp_obj tp_iter(TP,tp_obj self, tp_obj k);
tp_obj tp_get(TP,tp_obj self, tp_obj k);
int tp_iget(TP,tp_obj *r, tp_obj self, tp_obj k);
void tp_set(TP,tp_obj self, tp_obj k, tp_obj v);
tp_obj tp_add(TP,tp_obj a, tp_obj b);
tp_obj tp_mul(TP,tp_obj a, tp_obj b);
tp_obj tp_len(TP,tp_obj self);
int tp_cmp(TP,tp_obj a, tp_obj b);

#define TP_OP(name,expr) \
    tp_obj name(TP,tp_obj _a,tp_obj _b) { \
    if (_a.type == TP_NUMBER && _a.type == _b.type) { \
        tp_num a = _a.number.val; tp_num b = _b.number.val; \
        return tp_number(expr); \
    } \
    tp_raise(tp_None,"%s(%s,%s)",#name,TP_CSTR(_a),TP_CSTR(_b)); \
}

TP_OP(tp_and,((long)a)&((long)b));
TP_OP(tp_or,((long)a)|((long)b));
TP_OP(tp_mod,((long)a)%((long)b));
TP_OP(tp_lsh,((long)a)<<((long)b));
TP_OP(tp_rsh,((long)a)>>((long)b));
TP_OP(tp_sub,a-b);
TP_OP(tp_div,a/b);
TP_OP(tp_pow,pow(a,b));


/**/

#endif
