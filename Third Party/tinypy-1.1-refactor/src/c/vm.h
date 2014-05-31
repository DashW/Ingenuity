#ifndef TP_VM_H
#define TP_VM_H

#include "tp.h"

tp_vm *_tp_init(void);
void tp_deinit(TP);
void tp_frame(TP,tp_obj globals,tp_code *codes,tp_obj *ret_dest);
void _tp_raise(TP,tp_obj e);
void tp_print_stack(TP);
void tp_handle(TP);
void _tp_call(TP,tp_obj *dest, tp_obj fnc, tp_obj params);
void tp_return(TP, tp_obj v);

enum {
    TP_IEOF,TP_IADD,TP_ISUB,TP_IMUL,TP_IDIV,TP_IPOW,TP_IAND,TP_IOR,TP_ICMP,TP_IGET,TP_ISET,
    TP_INUMBER,TP_ISTRING,TP_IGGET,TP_IGSET,TP_IMOVE,TP_IDEF,TP_IPASS,TP_IJUMP,TP_ICALL,
    TP_IRETURN,TP_IIF,TP_IDEBUG,TP_IEQ,TP_ILE,TP_ILT,TP_IDICT,TP_ILIST,TP_INONE,TP_ILEN,
    TP_ILINE,TP_IPARAMS,TP_IIGET,TP_IFILE,TP_INAME,TP_INE,TP_IHAS,TP_IRAISE,TP_ISETJMP,
    TP_IMOD,TP_ILSH,TP_IRSH,TP_IITER,TP_IDEL,TP_IREGS,
    TP_ITOTAL
};

#define VA ((int)e.regs.a)
#define VB ((int)e.regs.b)
#define VC ((int)e.regs.c)
#define RA regs[e.regs.a]
#define RB regs[e.regs.b]
#define RC regs[e.regs.c]
#define UVBC (unsigned short)(((VB<<8)+VC))
#define SVBC (short)(((VB<<8)+VC))
#define GA tp_grey(tp,RA)
#define SR(v) f->cur = cur; return(v);

int tp_step(TP);
void tp_run(TP,int cur);
tp_obj tp_call(TP, const char *mod, const char *fnc, tp_obj params);
tp_obj tp_import(TP, char const *fname, char const *name, void *codes);
tp_obj tp_exec_(TP);
tp_obj tp_import_(TP);
void tp_builtins(TP);
void tp_args(TP,int argc, char *argv[]);
tp_obj tp_main(TP,char *fname, void *code);
tp_obj tp_compile(TP, tp_obj text, tp_obj fname);
tp_obj tp_exec(TP,tp_obj code, tp_obj globals);
tp_obj tp_eval(TP, char *text, tp_obj globals);
tp_vm *tp_init(int argc, char *argv[]);


/**/

#endif
