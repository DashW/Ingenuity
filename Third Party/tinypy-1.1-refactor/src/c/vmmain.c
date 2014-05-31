#include "vm.h"

int main(int argc, char *argv[]) {
	if(argc > 1)
	{
		tp_vm *tp = tp_init(argc,argv);
		tp_import(tp,argv[1],"__main__",0);
		tp_deinit(tp);
	}
    return(0);
}
