#include "../include/tlb.h"

int consulta_TLB(int num_pagina) {
    for (int i = 0; i < list_size(TLB); i++) {
        elemento_TLB* elemento = list_get(TLB, i);
        if (elemento->nro_pagina == num_pagina) {
            log_info(cpu_log_obligatorio, "PID: <%d> - TLB HIT - Pagina: <%d>", un_contexto->pID, num_pagina);
            if (strcmp(ALGORITMO_TLB, "LRU") == 0) {
                // Mover el índice al final de la lista de índices para LRU
                for (int j = 0; j < list_size(TLB_indices); j++) {
                    int* indice = list_get(TLB_indices, j);
                    if (*indice == i) {
                        int* removed_indice = list_remove(TLB_indices, j);
                        list_add(TLB_indices, removed_indice);
                        break;
                    }
                }
            }
            return elemento->nro_marco;
        }
    }
    log_info(cpu_log_obligatorio, "PID: <%d> - TLB MISS - Pagina: <%d>", un_contexto->pID, num_pagina);
    return -1;
}

void actualizar_tlb(elemento_TLB* nuevo_elemento) {
    if (list_size(TLB) < CANT_ENTRADAS_TLB) {
        // Todavía hay espacio en la TLB
        int* nuevo_indice = malloc(sizeof(int));
        *nuevo_indice = list_size(TLB);
        list_add(TLB, nuevo_elemento);
        list_add(TLB_indices, nuevo_indice);
    } else if(list_size(TLB) == CANT_ENTRADAS_TLB){
        // La TLB está llena, necesitamos reemplazar una entrada
        int *puntero_indice_victima = (int*) list_remove(TLB_indices, 0);
        int indice_victima = *puntero_indice_victima;

        if(puntero_indice_victima != NULL){

            free(puntero_indice_victima);
            puntero_indice_victima = NULL;
        }

        // int indice_victima = *(int*) list_remove(TLB_indices, 0);

        list_replace_and_destroy_element(TLB, indice_victima, nuevo_elemento, (void*) safe_free);

        int* nuevo_indice = malloc(sizeof(int));
        *nuevo_indice = indice_victima;
        list_add(TLB_indices, nuevo_indice);
    }
}

