#include <stdio.h>
#include <jni.h>
#include <cjson/cJSON.h>
#include "libproceso_JavaProceso.h"
#include <stdlib.h>

typedef struct {
    int id_proceso;
    int tiempo_rafaga;
    int prioridad;  // Menor valor = mayor prioridad
    int tiempo_llegada;
    int finish;
    int tiempo_inicio;
    int tiempo_fin;
    int tiempo_espera;
    int tiempo_sistema;
} Proceso;

static int planificar_por_prioridad(Proceso *procesos, int cantidad) {
    int reloj = 0;
    int terminados = 0;

    while (terminados < cantidad) {
        int idx = -1;
        int mayor_prioridad = 9999; // Asumimos prioridades positivas

        // Buscar el proceso de mayor prioridad (menor valor) que haya llegado
        for (int i = 0; i < cantidad; i++) {
            if (procesos[i].tiempo_llegada <= reloj && procesos[i].finish == 0) {
                if (procesos[i].prioridad < mayor_prioridad) {
                    mayor_prioridad = procesos[i].prioridad;
                    idx = i;
                }
            }
        }

        if (idx != -1) {
            // Ejecutar el proceso completo (no expulsivo)
            procesos[idx].tiempo_inicio = reloj;
            procesos[idx].tiempo_espera = reloj - procesos[idx].tiempo_llegada;
            reloj += procesos[idx].tiempo_rafaga;
            procesos[idx].tiempo_fin = reloj;
            procesos[idx].tiempo_sistema = procesos[idx].tiempo_espera + procesos[idx].tiempo_rafaga;
            procesos[idx].finish = 1;
            terminados++;
        } else {
            // No hay procesos listos, avanzar el reloj
            reloj++;
        }
    }
    return reloj;
}

static void generar_diagrama_gantt(cJSON *array_diagrama, int tiempo_completo, Proceso *proceso, int cant_procesos) {
    for (int i = 0; i < cant_procesos; i++) {
        cJSON *fila = cJSON_CreateArray();
        for (int j = 0; j < tiempo_completo; j++) {
            int estado;
            if (j < proceso[i].tiempo_llegada) {
                estado = 0; // No ha llegado
            } else {
                if (j < proceso[i].tiempo_inicio && j >= proceso[i].tiempo_llegada) {
                    estado = 1; // En espera
                } else if (j >= proceso[i].tiempo_inicio && j < proceso[i].tiempo_fin) {
                    estado = 2; // En ejecución
                } else {
                    estado = 0; // Terminado o no llegado
                }
            }
            cJSON_AddItemToArray(fila, cJSON_CreateNumber(estado));
        }
        cJSON_AddItemToArray(array_diagrama, fila);
    }
}


/*
 * Class:     libproceso_JavaProceso
 * Method:    algoritmo_Prioridades
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_libproceso_JavaProceso_algoritmo_1Prioridades
  (JNIEnv *env, jobject obj, jstring json) {
const char *json_input = (*env)->GetStringUTFChars(env, json, 0);
    cJSON *entrada = cJSON_Parse(json_input);
    cJSON *objeto_final = cJSON_CreateObject();
    cJSON *sistema_espera = cJSON_CreateObject();

    // Validar entrada JSON
    cJSON *array_procesos = cJSON_GetObjectItem(entrada, "procesos");
    if (array_procesos == NULL) {
        cJSON_AddStringToObject(sistema_espera, "error", "No se encontraron procesos");
        cJSON_AddItemToObject(objeto_final, "Sin resultados", sistema_espera);
        char *resultado = cJSON_Print(objeto_final);
        jstring salida = (*env)->NewStringUTF(env, resultado);

        // Liberar recursos
        free(resultado);
        cJSON_Delete(entrada);
        cJSON_Delete(objeto_final);
        (*env)->ReleaseStringUTFChars(env, json, json_input);

        return salida;
    }

    // Procesar los datos de entrada
    int size = cJSON_GetArraySize(array_procesos);
    Proceso *procesos = (Proceso *)calloc(size, sizeof(Proceso));

    cJSON *proceso_iterador = NULL;
    int i = 0;
    cJSON_ArrayForEach(proceso_iterador, array_procesos) {
        cJSON *id = cJSON_GetObjectItem(proceso_iterador, "id");
        cJSON *llegada = cJSON_GetObjectItem(proceso_iterador, "llegada");
        cJSON *rafaga = cJSON_GetObjectItem(proceso_iterador, "rafaga");
        cJSON *prioridad = cJSON_GetObjectItem(proceso_iterador, "prioridad");

        procesos[i].id_proceso = id->valueint;
        procesos[i].tiempo_rafaga = rafaga->valueint;
        procesos[i].prioridad = prioridad->valueint;
        procesos[i].tiempo_llegada = llegada->valueint;
        procesos[i].finish = 0; // Inicializar como no terminado
        i++;
    }

    // Ejecutar el algoritmo de planificación
    int reloj = planificar_por_prioridad(procesos, size);

    // Generar el diagrama de Gantt
    cJSON *array_diagrama = cJSON_CreateArray();
    generar_diagrama_gantt(array_diagrama, reloj, procesos, size);

    // Preparar los resultados
    for (int i = 0; i < size; i++) {
        cJSON *proceso_n = cJSON_CreateObject();
        cJSON_AddNumberToObject(proceso_n, "tiempo_sistema", procesos[i].tiempo_sistema);
        cJSON_AddNumberToObject(proceso_n, "tiempo_espera", procesos[i].tiempo_espera);
        char nombre_proceso[15];
        sprintf(nombre_proceso, "Proceso %d", procesos[i].id_proceso);
        cJSON_AddItemToObject(sistema_espera, nombre_proceso, proceso_n);
    }

    // Construir el objeto final de respuesta
    cJSON_AddItemToObject(objeto_final, "Procesos", sistema_espera);
    cJSON_AddItemToObject(objeto_final, "Diagrama", array_diagrama);

    // Convertir a cadena JSON
    char *resultado = cJSON_Print(objeto_final);
    jstring salida = (*env)->NewStringUTF(env, resultado);

    // Liberar memoria
    cJSON_Delete(entrada);
    cJSON_Delete(objeto_final);
    free(procesos);
    free(resultado);
    (*env)->ReleaseStringUTFChars(env, json, json_input);
    return salida;
}
