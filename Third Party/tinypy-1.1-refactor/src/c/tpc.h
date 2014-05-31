#ifndef TP_COMPILER_H
#define TP_COMPILER_H
#endif

#include "bc.h"
#include "vm.h"

void tp_compiler(TP) {
    tp_import(tp,0,"tokenize",tp_tokenize);
    tp_import(tp,0,"parse",tp_parse);
    tp_import(tp,0,"encode",tp_encode);
    tp_import(tp,0,"py2bc",tp_py2bc);
    tp_call(tp,"py2bc","_init",tp_None);
}

/**/
