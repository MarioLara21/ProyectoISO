# ProyectoISO
Este repositorio corresponde al I Proyecto del curso de Principios de Sistemas Operativos

## Pasos para su ejecución
- Se debe compilar el archivo IPSO.c com los siguientes comandos:
  - gcc -o IPSO IPSO.c -lpthread -lrt
  - ./IPSO <directorio_origen> <directorio_destino>

## Sobre la bitácora
El archivo "logfile.csv" corresponde a los datos sobre los archivos que se copiaron, el hilo encargado y el tiempo de ejecución, esto funciona como la bitácora solicitada en el proyecto.
Además, los archivos "Prueba2H.csv", "Prueba4H.csv" y de la misma manera respectivamente, corresponden a los datos utilizados al momento de realizar pruebas con 2,4,6, ... cantidad de hilos respectivamente.
