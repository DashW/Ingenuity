#ifndef TP_DICT_H
#define TP_DICT_H

#include "tp.h"

int tp_lua_hash(void const *v,int l);
void _tp_dict_free(_tp_dict *self);
/* void _tp_dict_reset(_tp_dict *self) {
       memset(self->items,0,self->alloc*sizeof(tp_item));
       self->len = 0;
       self->used = 0;
       self->cur = 0;
   }*/

int tp_hash(TP,tp_obj v);
void _tp_dict_hash_set(TP,_tp_dict *self, int hash, tp_obj k, tp_obj v);
void _tp_dict_tp_realloc(TP,_tp_dict *self,int len);
int _tp_dict_hash_find(TP,_tp_dict *self, int hash, tp_obj k);
int _tp_dict_find(TP,_tp_dict *self,tp_obj k);
void _tp_dict_setx(TP,_tp_dict *self,tp_obj k, tp_obj v);
void _tp_dict_set(TP,_tp_dict *self,tp_obj k, tp_obj v);
tp_obj _tp_dict_get(TP,_tp_dict *self,tp_obj k, const char *error);
void _tp_dict_del(TP,_tp_dict *self,tp_obj k, const char *error);
_tp_dict *_tp_dict_new(void);
tp_obj _tp_dict_copy(TP,tp_obj rr);
int _tp_dict_next(TP,_tp_dict *self);
tp_obj tp_merge(TP);
tp_obj tp_dict(TP);
tp_obj tp_dict_n(TP,int n, tp_obj* argv);

#endif
