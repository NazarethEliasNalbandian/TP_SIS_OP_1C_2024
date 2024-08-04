# tp-scaffold

Esta es una plantilla de proyecto diseñada para generar un TP de Sistemas
Operativos de la UTN FRBA.

## Dependencias

Para poder compilar y ejecutar el proyecto, es necesario tener instalada la
biblioteca [so-commons-library] de la cátedra:

```bash
git clone https://github.com/sisoputnfrba/so-commons-library
cd so-commons-library
make debug
make install
```

## Compilación

Cada módulo del proyecto se compila de forma independiente a través de un
archivo `makefile`. Para compilar un módulo, es necesario ejecutar el comando
`make` desde la carpeta correspondiente.

El ejecutable resultante se guardará en la carpeta `bin` del módulo.

## Importar desde Visual Studio Code

Para importar el workspace, debemos abrir el archivo `tp.code-workspace` desde
la interfaz o ejecutando el siguiente comando desde la carpeta raíz del
repositorio:

```bash
code tp.code-workspace
```

## Checkpoint

Para cada checkpoint de control obligatorio, se debe crear un tag en el
repositorio con el siguiente formato:

```
checkpoint-{número}
```

Donde `{número}` es el número del checkpoint.

Para crear un tag y subirlo al repositorio, podemos utilizar los siguientes
comandos:

```bash
git tag -a checkpoint-{número} -m "Checkpoint {número}"
git push origin checkpoint-{número}
```
Asegúrense de que el código compila y cumple con los requisitos del checkpoint
antes de subir el tag.

## Entrega

Para desplegar el proyecto en una máquina Ubuntu Server, podemos utilizar el
script [so-deploy] de la cátedra:

```bash
git clone https://github.com/sisoputnfrba/so-deploy.git
cd so-deploy
./deploy.sh -r=release -p=utils -p=kernel -p=cpu -p=memoria -p=entradasalida "tp-{año}-{cuatri}-{grupo}"
```

El mismo se encargará de instalar las Commons, clonar el repositorio del grupo
y compilar el proyecto en la máquina remota.

Ante cualquier duda, podés consultar la documentación en el repositorio de
[so-deploy], o utilizar el comando `./deploy.sh -h`.

[so-commons-library]: https://github.com/sisoputnfrba/so-commons-library
[so-deploy]: https://github.com/sisoputnfrba/so-deploy

## EVALUACION

Obtenemos la IP de la maquina usando ifconfig

Cada maquina va a ser un modulo

git clone https://github.com/sisoputnfrba/so-deploy.git

cd so-deploy

./deploy.sh -r=release -p=utils -p=kernel -p=cpu -p=memoria -p=entradasalida "tp-2024-1c-Grupo-X"

Ponemos la contraseña para sudo: utnso

Después:

cd tp-2024-1c-Grupo-X/memoria/bin

cd tp-2024-1c-Grupo-X/cpu/bin

cd tp-2024-1c-Grupo-X/kernel/bin

cd tp-2024-1c-Grupo-X/entradasalida/bin

## PRUEBA PLANI:

./memoria ../CONFIGS/PRUEBA_PLANI/memoria.config 

./cpu ../CONFIGS/PRUEBA_PLANI/cpu.config IP_MEMORIA

FIFO

./kernel ../CONFIGS/PRUEBA_PLANI/kernel_FIFO.config IP_MEMORIA IP_CPU

RR

./kernel ../CONFIGS/PRUEBA_PLANI/kernel_RR.config IP_MEMORIA IP_CPU

VRR

./kernel ../CONFIGS/PRUEBA_PLANI/kernel_RR_.config IP_MEMORIA IP_CPU

./entradasalida SLP1 ../CONFIGS/PRUEBA_PLANI/SLP1.config IP_MEMORIA IP_KERNEL

EJECUTAR_SCRIPT ../../scripts_kernel/PRUEBA_PLANI

FINALIZAR_PROCESO 4

## PRUEBA DEADLOCK (RESUELTA)

./memoria ../CONFIGS/PRUEBA_DEADLOCK/memoria.config 

./cpu ../CONFIGS/PRUEBA_DEADLOCK/cpu.config IP_MEMORIA

./kernel ../CONFIGS/PRUEBA_DEADLOCK/kernel.config IP_MEMORIA IP_CPU

./entradasalida ESPERA ../CONFIGS/PRUEBA_DEADLOCK/ESPERA.config IP_MEMORIA IP_KERNEL

EJECUTAR_SCRIPT ../../scripts_kernel/PRUEBA_DEADLOCK

## MEMORIA TLB (RESUELTA)

./memoria ../CONFIGS/MEMORIA_TLB/memoria.config

./cpu ../CONFIGS/MEMORIA_TLB/cpu_fifo.config IP_MEMORIA 

./cpu ../CONFIGS/MEMORIA_TLB/cpu_lru.config IP_MEMORIA 

./kernel ../CONFIGS/MEMORIA_TLB/kernel.config IP_MEMORIA IP_CPU

./entradasalida IO_GEN_SLEEP ../CONFIGS/MEMORIA_TLB/IO_GEN_SLEEP.config IP_MEMORIA IP_KERNEL

INICIAR_PROCESO ../../scripts_memoria/MEMORIA_1

INICIAR_PROCESO ../../scripts_memoria/MEMORIA_2

INICIAR_PROCESO ../../scripts_memoria/MEMORIA_3

## PRUEBA IO (RESUELTA)

./memoria ../CONFIGS/PRUEBA_IO/memoria.config

./cpu ../CONFIGS/PRUEBA_IO/cpu.config IP_MEMORIA

./kernel ../CONFIGS/PRUEBA_IO/kernel.config IP_MEMORIA IP_CPU

./entradasalida GENERICA ../CONFIGS/PRUEBA_IO/GENERICA.config IP_MEMORIA IP_KERNEL

./entradasalida TECLADO ../CONFIGS/PRUEBA_IO/TECLADO.config IP_MEMORIA IP_KERNEL

./entradasalida MONITOR ../CONFIGS/PRUEBA_IO/MONITOR.config IP_MEMORIA IP_KERNEL

EJECUTAR_SCRIPT ../../scripts_kernel/PRUEBA_IO

SI TAMAÑO = 20 ESCRIBIR: WAR NEVER CHANGES...

SI TAMAÑO = 26 ESCRIBIR: Sistemas Operativos 2c2023

EJECUTAR_SCRIPT ../../SCRIPTS/3_SCRIPT_IO_BASICO

## PRUEBA FS (RESUELTA)

./memoria ../CONFIGS/PRUEBA_FS/memoria.config

./cpu ../CONFIGS/PRUEBA_FS/cpu.config IP_MEMORIA 

./kernel ../CONFIGS/PRUEBA_FS/kernel.config IP_MEMORIA IP_CPU

HACER MKDIR ANTES DE CORRER EL MODULO FS ()

mkdir ../FILESYSTEM 

./entradasalida FS ../CONFIGS/PRUEBA_FS/FS.config IP_MEMORIA IP_KERNEL

./entradasalida TECLADO ../CONFIGS/PRUEBA_FS/TECLADO.config IP_MEMORIA IP_KERNEL

./entradasalida MONITOR ../CONFIGS/PRUEBA_FS/MONITOR.config IP_MEMORIA IP_KERNEL

INICIAR_PROCESO ../../scripts_memoria/FS_PRUEBA_FS_CREATE

INICIAR_PROCESO ../../scripts_memoria/FS_1

INICIAR_PROCESO ../../scripts_memoria/FS_2

INGRESO EN FS1: Fallout 1 Fallout 2 Fallout 3 Fallout: New Vegas Fallout 4 Fallout 76

INICIAR_PROCESO ../../scripts_memoria/FS_3

INICIAR_PROCESO ../../scripts_memoria/FS_4

## PRUEBA SALVATION 

./memoria ../CONFIGS/SALVATION/memoria.config

./cpu ../CONFIGS/SALVATION/cpu.config IP_MEMORIA

./kernel ../CONFIGS/SALVATION/kernel.config IP_MEMORIA IP_CPU

./entradasalida ESPERA ../CONFIGS/SALVATION/ESPERA.config IP_MEMORIA IP_KERNEL

./entradasalida GENERICA ../CONFIGS/SALVATION/GENERICA.config IP_MEMORIA IP_KERNEL

./entradasalida MONITOR ../CONFIGS/SALVATION/MONITOR.config IP_MEMORIA IP_KERNEL

./entradasalida SLP1 ../CONFIGS/SALVATION/SLP1.config IP_MEMORIA IP_KERNEL

./entradasalida TECLADO ../CONFIGS/SALVATION/TECLADO.config IP_MEMORIA IP_KERNEL

EJECUTAR_SCRIPT ../../scripts_kernel/PRUEBA_SALVATIONS_EDGE

MULTIPROGRAMACION 100

DETENER_PLANIFICACION

PROCESO_ESTADO

INICIAR_PLANIFICACION

## VALGRIND
valgrind -s --leak-check=full ./ARCHIVO

## PRUEBAS INICIALES:
1) EJECUTAR_SCRIPT ../../SCRIPTS/1_SCRIPT_INICIAL.txt
2) EJECUTAR_SCRIPT ../../SCRIPTS/2_SCRIPT_INS_BASICAS.txt

SI TIRA SEGMENTATION FAULT, CORRER CON valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes 
DARLE EL REPORTE A CHATGPT Y PREGUNTAR POR LA LINEA Y ARCHIVO DONDE HUBO SEGMENTATION FAULT

## PRUEBA PLANI: (NO USAR PARA EVALUACION DEL TP)

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes  --leak-resolution=low ./memoria ../CONFIGS/PRUEBA_PLANI/memoria.config 

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes  --leak-resolution=low ./cpu ../CONFIGS/PRUEBA_PLANI/cpu.config 127.0.0.1

FIFO
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --leak-resolution=low ./kernel ../CONFIGS/PRUEBA_PLANI/kernel_FIFO.config 127.0.0.1 127.0.0.1

RR
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --leak-resolution=low ./kernel ../CONFIGS/PRUEBA_PLANI/kernel_RR.config 127.0.0.1 127.0.0.1

VRR
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --leak-resolution=low ./kernel ../CONFIGS/PRUEBA_PLANI/kernel_RR_.config 127.0.0.1 127.0.0.1

./entradasalida SLP1 ../CONFIGS/PRUEBA_PLANI/SLP1.config 127.0.0.1 127.0.0.1

EJECUTAR_SCRIPT ../../scripts_kernel/PRUEBA_PLANI

## PRUEBA DEADLOCK (RESUELTA) (NO USAR PARA EVALUACION DEL TP)
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --leak-resolution=low ./memoria ../CONFIGS/PRUEBA_DEADLOCK/memoria.config 
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --leak-resolution=low ./cpu ../CONFIGS/PRUEBA_DEADLOCK/cpu.config 127.0.0.1
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --leak-resolution=low ./kernel ../CONFIGS/PRUEBA_DEADLOCK/kernel.config 127.0.0.1 127.0.0.1
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --leak-resolution=low ./entradasalida ESPERA ../CONFIGS/PRUEBA_DEADLOCK/ESPERA.config 127.0.0.1 127.0.0.1

EJECUTAR_SCRIPT ../../scripts_kernel/PRUEBA_DEADLOCK

## MEMORIA TLB (RESUELTA) (NO USAR PARA EVALUACION DEL TP)
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --leak-resolution=low  ./memoria ../CONFIGS/MEMORIA_TLB/memoria.config
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --leak-resolution=low  ./cpu ../CONFIGS/MEMORIA_TLB/cpu_fifo.config 127.0.0.1
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --leak-resolution=low  ./cpu ../CONFIGS/MEMORIA_TLB/cpu_lru.config 127.0.0.1 
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --leak-resolution=low  ./kernel ../CONFIGS/MEMORIA_TLB/kernel.config 127.0.0.1 127.0.0.1
./entradasalida IO_GEN_SLEEP ../CONFIGS/MEMORIA_TLB/IO_GEN_SLEEP.config 127.0.0.1 127.0.0.1

INICIAR_PROCESO ../../scripts_memoria/MEMORIA_1
INICIAR_PROCESO ../../scripts_memoria/MEMORIA_2
INICIAR_PROCESO ../../scripts_memoria/MEMORIA_3

## PRUEBA IO (RESUELTA) (NO USAR PARA EVALUACION DEL TP)
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./memoria ../CONFIGS/PRUEBA_IO/memoria.config
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./cpu ../CONFIGS/PRUEBA_IO/cpu.config 127.0.0.1
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./kernel ../CONFIGS/PRUEBA_IO/kernel.config 127.0.0.1 127.0.0.1
./entradasalida GENERICA ../CONFIGS/PRUEBA_IO/GENERICA.config 127.0.0.1 127.0.0.1
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./entradasalida TECLADO ../CONFIGS/PRUEBA_IO/TECLADO.config 127.0.0.1 127.0.0.1
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./entradasalida MONITOR ../CONFIGS/PRUEBA_IO/MONITOR.config 127.0.0.1 127.0.0.1

EJECUTAR_SCRIPT ../../scripts_kernel/PRUEBA_IO

WAR NEVER CHANGES...
Sistemas Operativos 2c2023

EJECUTAR_SCRIPT ../../SCRIPTS/3_SCRIPT_IO_BASICO

## PRUEBA FS (RESUELTA) (NO USAR PARA EVALUACION DEL TP)
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./memoria ../CONFIGS/PRUEBA_FS/memoria.config
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./cpu ../CONFIGS/PRUEBA_FS/cpu.config 127.0.0.1
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./kernel ../CONFIGS/PRUEBA_FS/kernel.config 127.0.0.1 127.0.0.1
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./entradasalida FS ../CONFIGS/PRUEBA_FS/FS.config 127.0.0.1 127.0.0.1
./entradasalida TECLADO ../CONFIGS/PRUEBA_FS/TECLADO.config 127.0.0.1 127.0.0.1
./entradasalida MONITOR ../CONFIGS/PRUEBA_FS/MONITOR.config 127.0.0.1 127.0.0.1

INICIAR_PROCESO ../../scripts_memoria/FS_PRUEBA_FS_CREATE
INICIAR_PROCESO ../../scripts_memoria/FS_1
INICIAR_PROCESO ../../scripts_memoria/FS_2
INGRESO EN FS1: Fallout 1 Fallout 2 Fallout 3 Fallout: New Vegas Fallout 4 Fallout 76
INICIAR_PROCESO ../../scripts_memoria/FS_3
INICIAR_PROCESO ../../scripts_memoria/FS_4

## PRUEBA SALVATION (NO USAR PARA EVALUACION DEL TP)
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./memoria ../CONFIGS/SALVATION/memoria.config
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./cpu ../CONFIGS/SALVATION/cpu.config 127.0.0.1
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./kernel ../CONFIGS/SALVATION/kernel.config 127.0.0.1 127.0.0.1
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./entradasalida ESPERA ../CONFIGS/SALVATION/ESPERA.config 127.0.0.1 127.0.0.1
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./entradasalida GENERICA ../CONFIGS/SALVATION/GENERICA.config 127.0.0.1 127.0.0.1
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./entradasalida MONITOR ../CONFIGS/SALVATION/MONITOR.config 127.0.0.1 127.0.0.1
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./entradasalida SLP1 ../CONFIGS/SALVATION/SLP1.config 127.0.0.1 127.0.0.1
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./entradasalida TECLADO ../CONFIGS/SALVATION/TECLADO.config 127.0.0.1 127.0.0.1

EJECUTAR_SCRIPT ../../scripts_kernel/PRUEBA_SALVATIONS_EDGE
MULTIPROGRAMACION 100
DETENER_PLANIFICACION
PROCESO_ESTADO
INICIAR_PLANIFICACION