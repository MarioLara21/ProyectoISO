#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <dirent.h>

#define NUM_THREADS 4  
#define BUFFER_SIZE 1024

pthread_mutex_t lock;
pthread_mutex_t log_lock;  
char **files;
int file_count = 0;
FILE *logfile;  

typedef struct {
    int thread_id;
    char *src_dir;
    char *dst_dir;
} thread_data_t;

void copy_file(char *src, char *dst, int thread_id) {
    FILE *src_file, *dst_file;
    char buffer[BUFFER_SIZE];
    size_t bytes;
    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    src_file = fopen(src, "rb");
    if (src_file == NULL) {
        perror("Error al abrir archivo fuente");
        return;
    }

    dst_file = fopen(dst, "wb");
    if (dst_file == NULL) {
        perror("Error al crear archivo destino");
        fclose(src_file);
        return;
    }

    while ((bytes = fread(buffer, 1, BUFFER_SIZE, src_file)) > 0) {
        fwrite(buffer, 1, bytes, dst_file);
    }

    fclose(src_file);
    fclose(dst_file);
    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_taken = (end.tv_sec - start.tv_sec) * 1e9;
    time_taken = (time_taken + (end.tv_nsec - start.tv_nsec)) * 1e-9;  // Convertir a segundos
    pthread_mutex_lock(&log_lock);
    fprintf(logfile, "%s,%d,%.9f\n", src, thread_id, time_taken);
    pthread_mutex_unlock(&log_lock);
}
void create_directory(char *path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        if (mkdir(path, 0700) == -1) {
            perror("Error al crear directorio");
            exit(EXIT_FAILURE);
        }
    }
}

void read_directory(char *src_dir, char *dst_dir) {
    DIR *dir;
    struct dirent *ent;

    if ((dir = opendir(src_dir)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            // Omitir . y ..
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
                continue;
            }

            char src_path[512];
            char dst_path[512];
            snprintf(src_path, sizeof(src_path), "%s/%s", src_dir, ent->d_name);
            snprintf(dst_path, sizeof(dst_path), "%s/%s", dst_dir, ent->d_name);

            if (ent->d_type == DT_DIR) {
                // Es un subdirectorio
                create_directory(dst_path);  // Crear subdirectorio en el destino
                read_directory(src_path, dst_path);  // Recursión en el subdirectorio
            } else if (ent->d_type == DT_REG) {
                // Es un archivo
                pthread_mutex_lock(&lock);
                files[file_count++] = strdup(src_path);  // Guardar archivo para copiarlo
                pthread_mutex_unlock(&lock);
            }
        }
        closedir(dir);
    } else {
        perror("No se puede abrir el directorio");
        exit(EXIT_FAILURE);
    }
}

// Función para que el hilo copie archivos
void *copy_thread(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;

    while (1) {
        pthread_mutex_lock(&lock);
        if (file_count == 0) {
            pthread_mutex_unlock(&lock);
            break;
        }

        char *file = files[--file_count];
        pthread_mutex_unlock(&lock);

        // Obtener nombre del archivo para el destino
        char dst_path[512];
        snprintf(dst_path, sizeof(dst_path), "%s/%s", data->dst_dir, file + strlen(data->src_dir) + 1);  // Quitar la parte src_dir de la ruta

        printf("Thread %d copiando: %s\n", data->thread_id, file);
        copy_file(file, dst_path, data->thread_id);  // Pasar el ID del hilo al copiar
        free(file);
    }

    pthread_exit(NULL);
}

//Creacion del pool de hilos
void init_thread_pool(pthread_t *threads, thread_data_t *thread_data, int num_threads, char *src_dir, char *dst_dir) {
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].src_dir = src_dir;
        thread_data[i].dst_dir = dst_dir;
        int rc = pthread_create(&threads[i], NULL, copy_thread, (void *)&thread_data[i]);
        if (rc) {
            fprintf(stderr, "Error al crear el hilo %d, código de retorno: %d\n", i, rc);
            exit(-1);
        }
    }
}

//realizar espera de los hilos
void wait_for_threads(pthread_t *threads, int num_threads) {
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
}



int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <directorio_origen> <directorio_destino>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    char *src_dir = argv[1];
    char *dst_dir = argv[2];
    create_directory(dst_dir);
    logfile = fopen("logfile.csv", "w");
    if (logfile == NULL) {
        perror("No se puede crear el archivo de bitácora");
        exit(EXIT_FAILURE);
    }
    fprintf(logfile, "Archivo,Hilo,Tiempo(segundos)\n");
    files = malloc(1000 * sizeof(char *));
    if (files == NULL) {
        perror("Error al reservar memoria");
        exit(EXIT_FAILURE);
    }
    read_directory(src_dir, dst_dir);

    pthread_mutex_init(&lock, NULL);
    pthread_mutex_init(&log_lock, NULL);

     // Crear hilos
    pthread_t threads[NUM_THREADS];
    thread_data_t thread_data[NUM_THREADS];

    // Inicializar pool de hilos
    init_thread_pool(threads, thread_data, NUM_THREADS, src_dir, dst_dir);

    // Esperar a que los hilos terminen
    wait_for_threads(threads, NUM_THREADS);
    
    // Limpiar y liberar recursos
    fclose(logfile);
    pthread_mutex_destroy(&lock);
    pthread_mutex_destroy(&log_lock);
    free(files);

    return 0;
}
