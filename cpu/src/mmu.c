#include "../include/mmu.h"
#include "../include/tlb.h"

int MMU(uint32_t dir_logica)
{
    t_paquete *paquete;
    int num_pagina = floor(dir_logica / TAM_PAGINA);
    desplazamiento = dir_logica - num_pagina * TAM_PAGINA;
    int dir_fisica;

    if (CANT_ENTRADAS_TLB == 0)
    {
        paquete = crear_super_paquete(CONSULTA_DE_PAGINA_CPU_MEMORIA);
        cargar_int_al_super_paquete(paquete, un_contexto->pID);
        cargar_int_al_super_paquete(paquete, num_pagina);
        enviar_paquete(paquete, fd_memoria);
        eliminar_paquete(paquete);
        sem_wait(&sem_sol_marco);
        log_info(cpu_log_obligatorio, "PID: <%d> - OBTENER MARCO - Página: <%d> - Marco: <%d>", un_contexto->pID, num_pagina, nro_marco);
    }
    else
    {

        nro_marco = consulta_TLB(num_pagina);
    }

    if (nro_marco >= 0) // TLB HIT
    {
        dir_fisica = nro_marco * TAM_PAGINA + desplazamiento;
        sem_post(&sem_nro_marco);
        log_trace(cpu_log_debug,"marco: %d", nro_marco);
        log_trace(cpu_log_debug,"DIR: %d", dir_fisica);
        return dir_fisica;
    }
    else // TLB MISS
    {
        paquete = crear_super_paquete(CONSULTA_DE_PAGINA_CPU_MEMORIA);
        cargar_int_al_super_paquete(paquete, un_contexto->pID);
        cargar_int_al_super_paquete(paquete, num_pagina);
        enviar_paquete(paquete, fd_memoria);
        eliminar_paquete(paquete);
        sem_wait(&sem_sol_marco);
        log_info(cpu_log_obligatorio, "PID: <%d> - OBTENER MARCO - Página: <%d> - Marco: <%d>", un_contexto->pID, num_pagina, nro_marco);

        elemento_TLB* elemento_tlb_a_agregar = malloc(sizeof(elemento_TLB));
        elemento_tlb_a_agregar->nro_marco = nro_marco;
        elemento_tlb_a_agregar->nro_pagina = num_pagina;
        elemento_tlb_a_agregar->pid = un_contexto->pID;

        actualizar_tlb(elemento_tlb_a_agregar);
        log_trace(cpu_log_debug,"marco: %d", nro_marco);
        dir_fisica = TAM_PAGINA * nro_marco + desplazamiento;
        log_trace(cpu_log_debug,"DIR: %d", dir_fisica);

        sem_post(&sem_nro_marco);
        return dir_fisica;
    }
}