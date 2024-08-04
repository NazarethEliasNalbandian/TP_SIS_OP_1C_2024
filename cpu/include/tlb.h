#ifndef TLB_H_
#define TLB_H_

#include "cpu_gestor.h"

int consulta_TLB(int n_pag);
void actualizar_tlb(elemento_TLB* nuevo_elemento); 

#endif