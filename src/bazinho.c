#include "bazinho.h"

//====================================== FORK ======================================
pid_t *cria_n_forks(int n_forks, void(*func_filho)()) {
    pid_t *lista_pids = (pid_t *) malloc(n_forks * sizeof(pid_t));
    int i;
    pid_t rtn;
    for (i = 0; i < n_forks; i++) {
        rtn = fork();
        if (rtn == 0) {
            (func_filho)();
            exit(0);
        }
        lista_pids[i] = rtn;
        }
    return lista_pids;
}

pid_t *cria_n_forks_args(int n_forks, void(*func_filho)(), void*(*cria_args)(int), size_t tamanho_arg) {
    pid_t *lista_pids = (pid_t *) malloc(n_forks * sizeof(pid_t));
    char *lista_args = (char*)(cria_args)(n_forks);
    int i;
    pid_t rtn;
    for (i = 0; i < n_forks; i++) {
        rtn = fork();
        if (rtn == 0) {
            (func_filho)((void*)&(lista_args[i * tamanho_arg]));
            exit(0);
        }
        lista_pids[i] = rtn;
    }
    return lista_pids;
}

pid_t *cria_n_forks_arglist(int n_forks, void(*func_filho)(), void* args, size_t tamanho_arg) {
    pid_t *lista_pids = (pid_t *) malloc(n_forks * sizeof(pid_t));
    char *lista_args = (char*) args;
    int i;
    pid_t rtn;
    for (i = 0; i < n_forks; i++) {
        rtn = fork();
        if (rtn == 0) {
            (func_filho)((void*)&(lista_args[i * tamanho_arg]));
            exit(0);
        }
        lista_pids[i] = rtn;
    }
    return lista_pids;
}


void espera_n_filhos(int n_filhos) {
    while (n_filhos > 0) { wait(NULL); n_filhos--; }
}

int *espera_n_filhos_exitstatus(int n_filhos) {
    int wstatus;
    int *lista_status = (int*) malloc(n_filhos * sizeof(int));
    int i;
    for (i = 0; i < n_filhos; i++) {
        wait(&wstatus);
        if (WIFEXITED(wstatus)) {
            lista_status[i] = WEXITSTATUS(wstatus);
        }
    }
    return lista_status;
}

void mata_n_filhos(int n_filhos, pid_t *filhos) {
    int i;
    for (i = 0; i < n_filhos; i++) kill(filhos[i], SIGKILL);
}

//=============================== FILA DE MENSAGENS  ===============================
int cria_fila(key_t chave) {
    int queue_id;
    if( (queue_id = msgget(chave, IPC_CREAT | 0666)) == -1 ) {
        perror("msgget");
        fprintf(stderr,"Chamada a msgget() falhou: impossivel criar a fila de mensagens!\n");
        exit(1);
    }
    return queue_id;
}

int pega_id_fila(key_t chave) {
    int queue_id;
    if( (queue_id = msgget(chave, IPC_CREAT | 0666)) == -1 ) {
        perror("msgget");
        fprintf(stderr,"Chamada a msgget() falhou: impossivel acessar a fila de mensagens!\n");
        exit(1);
    }
    return queue_id;
}

void apaga_fila_id(int queue_id) {
    if( msgctl(queue_id,IPC_RMID,NULL) != 0 ) {
        perror("msgctl");
        fprintf(stderr,"Chamada a msgctl() falhou: impossivel remover a fila de mensagens!\n");
        exit(1);
    }
}

void apaga_fila(key_t chave) {
    int queue_id = pega_id_fila(chave);
    apaga_fila_id(queue_id);
}

void envia_mensagem_id(int queue_id, msgb *msg_buffer, size_t msg_size) {
    if( msgsnd(queue_id, msg_buffer, msg_size, 0) == -1 ) {
        perror("msgsnd");
        fprintf(stderr, "Chamada a msgsnd() falhou: impossivel enviar mensagem!\n");
        exit(1);
    }
}

void envia_mensagem(key_t chave, msgb *msg_buffer, size_t msg_size) {
    int queue_id = pega_id_fila(chave);
    envia_mensagem_id(queue_id, msg_buffer, msg_size);
}

void recebe_mensagem_id(int queue_id, msgb *msg_buffer, long msgtyp, size_t msg_size) {
    if( msgrcv(queue_id, msg_buffer, msg_size, msgtyp, 0) == -1 ) {
        perror("msgrcv");
        fprintf(stderr, "Chamada a msgrcv() falhou: impossivel receber mensagem!\n");
        exit(1);
    }
}

void recebe_mensagem(key_t chave, msgb *msg_buffer, long msgtyp, size_t msg_size) {
    int queue_id = pega_id_fila(chave);
    recebe_mensagem_id(queue_id, msg_buffer, msgtyp, msg_size);
}

//============================= MEMORIA COMPARTILHADA  =============================
void *cria_memoria_compartilhada(key_t chave, size_t tamanho) {
    int g_shm_id;
    if( (g_shm_id = shmget(chave, tamanho, IPC_CREAT | 0666)) == -1 ) {
        perror("semget");
        fprintf(stderr,"Chamada a shmget() falhou: impossivel criar o segmento de memoria compartilhada!\n");
        exit(1);
    }
    void *g_shm_addr;
    if( (g_shm_addr = shmat(g_shm_id, NULL, 0)) == (int *)-1 ) {
        perror("shmat");
        fprintf(stderr,"Chamada a shmat() falhou: impossivel associar o segmento de memoria compartilhada!\n");
        exit(1);
    }
    return g_shm_addr;
}

void *pega_memoria_compartilhada(key_t chave, size_t tamanho) {
    int g_shm_id;
    if( (g_shm_id = shmget(chave, tamanho, 0666)) == -1 ) {
        perror("semget");
        fprintf(stderr,"Chamada a shmget() falhou: impossivel acessar o segmento de memoria compartilhada!\n");
        exit(1);
    }
    void *g_shm_addr;
    if( (g_shm_addr = shmat(g_shm_id, NULL, 0)) == (int *)-1 ) {
        perror("shmat");
        fprintf(stderr,"Chamada a shmat() falhou: impossivel associar o segmento de memoria compartilhada!\n");
        exit(1);
    }
    return g_shm_addr;
}

void apaga_memoria_compartilhada(key_t key) {
    if( shmctl(shmget(key, 0, 0),IPC_RMID,NULL) != 0 ) {
        perror("shmctl");
        fprintf(stderr,"Chamada a shmget() falhou: impossivel remover o segmento de memoria compartilhada!\n");
        exit(1);
    }
}

//==================================== SEMAFORO ====================================
int cria_semaforo(key_t chave) {
    int temp_sem;
    if( ( temp_sem = semget( chave, 1, IPC_CREAT | 0666 ) ) == -1 ) {   
        perror("semget"); 
        fprintf(stderr,"Chamada a semget() falhou: impossivel criar o conjunto de semaforos!");
        exit(1);
    }
    return temp_sem;
}

int cria_semaforo_nocheck(key_t chave) {
    return semget( chave, 1, IPC_EXCL | IPC_CREAT | 0666 );
}

int pega_id_semaforo(key_t chave) {
    int temp_sem;
    if( ( temp_sem = semget( chave, 1, 0666 ) ) == -1 ) {    
        perror("semget");
        fprintf(stderr,"Chamada a semget() falhou: impossivel acessar o conjunto de semaforos!");
        exit(1);
    }
    return temp_sem;
}

void apaga_semaforo_id(int sem_id) {
    if( semctl(sem_id, 0, IPC_RMID, 0) != 0 ) {
        perror("semctl");
        fprintf(stderr,"Chamada a semctl() falhou: impossivel remover o conjunto de semaforos!\n");
        exit(1);
    }
}

void apaga_semaforo(key_t chave) {
    int sem_id = pega_id_semaforo(chave);
    apaga_semaforo_id(sem_id);
}

void opera_semaforo_id(int sem_id, int valor) {
    struct sembuf g_sem_op;
    g_sem_op.sem_num =  0;
    g_sem_op.sem_op  =  valor;
    g_sem_op.sem_flg =  0;
    if( semop( sem_id, &g_sem_op, 1 ) == -1 ) {
        perror("semop");
        fprintf(stderr,"chamada semop() falhou, impossivel operar o semaforo!");
        exit(1);
    }
}

void opera_semaforo(key_t chave, int valor) {
    int sem_id = pega_id_semaforo(chave);
    opera_semaforo_id(sem_id, valor);
}

void opera_semaforo_id_e(int sem_id, enum mutex_ops_e valor) {
    struct sembuf g_sem_op;
    g_sem_op.sem_num =  0;
    g_sem_op.sem_op  =  valor;
    g_sem_op.sem_flg =  0;
    if( semop( sem_id, &g_sem_op, 1 ) == -1 ) {
        perror("semop");
        fprintf(stderr,"chamada semop() falhou, impossivel operar o semaforo!");
        exit(1);
    }
}

void opera_semaforo_e(key_t chave, enum mutex_ops_e valor) {
    int sem_id = pega_id_semaforo(chave);
    opera_semaforo_id_e(sem_id, valor);
}

int opera_semaforo_nowait_id(int sem_id, int valor) {
    struct sembuf g_sem_op;
    g_sem_op.sem_num =  0;
    g_sem_op.sem_op  =  valor;
    g_sem_op.sem_flg =  IPC_NOWAIT;
    if( semop( sem_id, &g_sem_op, 1 ) == -1 ) {
        return -1;
    }
    return 0;
}

int opera_semaforo_nowait(key_t chave, int valor) {
    int sem_id = pega_id_semaforo(chave);
    return opera_semaforo_nowait_id(sem_id, valor);
}

//===================================== THREAD =====================================
pthread_t *cria_n_threads_args(int n_threads, void(*func_thread)(void *), void*(*cria_args)(int), size_t tamanho_arg) {
    int i, status_thread_atual;
    char *lista_args = (char*)(cria_args)(n_threads);
    pthread_t *lista_threads = (pthread_t *) malloc(n_threads * sizeof(pthread_t));
    if (lista_threads == NULL) {
        perror("malloc");
        fprintf(stderr, "Chamada malloc() falhou, impossivel alocar memoria para um thread.\n");
        exit(-1);
    }
    for (i = 0; i < n_threads; i++) {
        status_thread_atual = pthread_create(&lista_threads[i], NULL, (void*)func_thread, (void*)(&(lista_args[i * tamanho_arg])));
        if (status_thread_atual) {
            perror("pthread_create");
            fprintf(stderr, "Chamada pthread_create() falhou, impossivel criar um thread.\n");
            exit(-1);
        }
    }
    return lista_threads;
}

pthread_t *cria_n_threads_arglist(int n_threads, void(*func_thread)(void *), void* args, size_t tamanho_arg) {
    int i, status_thread_atual;
    pthread_t *lista_threads = (pthread_t *) malloc(n_threads * sizeof(pthread_t));
    if (lista_threads == NULL) {
        perror("malloc");
        fprintf(stderr, "Chamada malloc() falhou, impossivel alocar memoria para um thread.\n");
        exit(-1);
    }
    char *lista_args = (char*) args;
    for (i = 0; i < n_threads; i++) {
        status_thread_atual = pthread_create(&lista_threads[i], NULL, (void*)func_thread, (void*)(&lista_args[i * tamanho_arg]));
        if (status_thread_atual) {
            perror("pthread_create");
            fprintf(stderr, "Chamada pthread_create() falhou, impossivel criar um thread.\n");
            exit(-1);
        }
    }
    return lista_threads;
}

void espera_n_threads(int n_threads, pthread_t *threads) {
    while (n_threads > 0) pthread_join(threads[--n_threads], NULL);
}

void mata_n_threads(int n_threads, pthread_t *threads) {
    while (n_threads > 0) pthread_cancel(threads[--n_threads]);
}

pthread_mutex_t *cria_n_mutex(int n_mutex) {
    pthread_mutex_t *mutex = (pthread_mutex_t *) malloc(n_mutex * sizeof(pthread_mutex_t));
    while (n_mutex > 0) pthread_mutex_init(&mutex[--n_mutex], NULL);
    return mutex;
}

void trava_n_mutex(int n_mutex, pthread_mutex_t* mutex) {
    while (n_mutex > 0) pthread_mutex_lock(&mutex[--n_mutex]);
}

void libera_n_mutex(int n_mutex, pthread_mutex_t* mutex) {
    while (n_mutex > 0) pthread_mutex_unlock(&mutex[--n_mutex]);
}

void trava_mutex(pthread_mutex_t* mutex) {
    pthread_mutex_lock(mutex);
}

void libera_mutex(pthread_mutex_t* mutex) {
    pthread_mutex_unlock(mutex);
}

void apaga_n_mutex(int n_mutex, pthread_mutex_t *mutex) {
    while (n_mutex > 0) pthread_mutex_destroy(&mutex[--n_mutex]);
}
