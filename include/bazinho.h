#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <wait.h>
#include <sys/msg.h>
#include <errno.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <pthread.h>

//Enums
enum mutex_ops_e {Trava = -1, Libera = 1};

//Typedefs
typedef struct msgbuf msgb;

//====================================== FORK ======================================
pid_t *cria_n_forks(int n_forks, void(*func_filho)());
pid_t *cria_n_forks_args(int n_forks, void(*func_filho)(), void*(*cria_args)(int), size_t tamanho_arg);
pid_t *cria_n_forks_arglist(int n_forks, void(*func_filho)(), void* args, size_t tamanho_arg);
//void espera_n_filhos(int n_filhos);
//int *espera_n_filhos_exitstatus(int n_filhos);
void mata_n_filhos(int n_filhos, pid_t *filhos);

//=============================== FILA DE MENSAGENS  ===============================
int cria_fila(key_t chave);
int pega_id_fila(key_t chave);
void apaga_fila_id(int queue_id);
void apaga_fila(key_t chave);
void envia_mensagem_id(int queue_id, msgb *msg_buffer, size_t msg_size);
void envia_mensagem(key_t chave, msgb *msg_buffer, size_t msg_size);
void recebe_mensagem_id(int queue_id, msgb *msg_buffer, long msgtyp, size_t msg_size);
void recebe_mensagem(key_t chave, msgb *msg_buffer, long msgtyp, size_t msg_size);

//============================= MEMORIA COMPARTILHADA  =============================
void *cria_memoria_compartilhada(key_t chave, size_t tamanho);
void *pega_memoria_compartilhada(key_t chave, size_t tamanho);
void apaga_memoria_compartilhada(key_t key);

//==================================== SEMAFORO ====================================
int cria_semaforo(key_t chave);
int cria_semaforo_nocheck(key_t chave);
int pega_id_semaforo(key_t chave);
void apaga_semaforo_id(int sem_id);
void apaga_semaforo(key_t chave);
void opera_semaforo_id(int sem_id, int valor);
void opera_semaforo(key_t chave, int valor);
void opera_semaforo_id_e(int sem_id, enum mutex_ops_e valor);
void opera_semaforo_e(key_t chave, enum mutex_ops_e valor);
int opera_semaforo_nowait_id(int sem_id, int valor);
int opera_semaforo_nowait(key_t chave, int valor);

//===================================== THREAD =====================================
pthread_t *cria_n_threads_args(int n_threads, void(*func_thread)(void *), void*(*cria_args)(int), size_t tamanho_arg);
pthread_t *cria_n_threads_arglist(int n_threads, void(*func_thread)(void *), void* args, size_t tamanho_arg);
void espera_n_threads(int n_threads, pthread_t *threads);
void mata_n_threads(int n_threads, pthread_t *threads);
pthread_mutex_t *cria_n_mutex(int n_mutex);
void trava_n_mutex(int n_mutex, pthread_mutex_t* mutex);
void libera_n_mutex(int n_mutex, pthread_mutex_t* mutex);
void trava_mutex(pthread_mutex_t* mutex);
void libera_mutex(pthread_mutex_t* mutex);
void apaga_n_mutex(int n_mutex, pthread_mutex_t *mutex);
