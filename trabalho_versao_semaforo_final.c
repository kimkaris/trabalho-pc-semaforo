#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

typedef struct tipo_obj_thread {
    char mensagem[18];
    int coluna;
    int linha;
    int pronta;
    sem_t semaforo;
} tipo_obj_thread;

tipo_obj_thread* vetor_obj_threads;

int matriz[1000][1000]; // tamanho maximo de matriz = 1000x1000
int num_linhas = 3; // tamanho padrao linhas matrizes = 3
int num_colunas = 4; // tamanho padrao colunas matrizes = 4
int num_threads = 3; // numero padrao de threads = 3

void pegar_input_usuario() {
    while (1) {
        printf("Numero de linhas matriz >>> ");
        scanf("%d", &num_linhas);

        printf("Numero de colunas matriz >>> ");
        scanf("%d", &num_colunas);

        if (num_linhas <= 0 || num_linhas > 1000 || num_colunas <= 0 || num_colunas > 1000) {
            printf("\nEntrada invalida. Tente novamente com valores entre 1 e 1000.\n\n");
        } else {
            break;
        }
    }
}

void gerar_matriz() {
    pegar_input_usuario();

    for (int i = 0; i < num_linhas; i++) {
        for (int j = 0; j < num_colunas; j++) {
            matriz[i][j] = rand() % 10;
        }
    }
}

void imprimir_matriz() {
    printf("\nMatriz:\n");
    for (int i = 0; i < num_linhas; i++) {
        for (int j = 0; j < num_colunas; j++) {
            printf("[%d] ", matriz[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

tipo_obj_thread* iniciar_vetor_obj_threads() {
    tipo_obj_thread* vetor_obj_threads_local = malloc(num_threads * sizeof(tipo_obj_thread));

    for (int i = 0; i < num_threads; i++) {
        tipo_obj_thread thread;

        sprintf(thread.mensagem, "Sou a thread %d", i + 1);
        thread.linha = i;
        thread.coluna = 0;
        thread.pronta = 0;
        sem_init(&thread.semaforo, 0, 0);

        vetor_obj_threads_local[i] = thread;
    }

    return vetor_obj_threads_local;
}

int media_vizinhos(int i, int j) {
    int soma = 0;
    int qtd_vizinhos = 0;

    for (int x = 0; x < num_linhas; x++) {
        for (int y = 0; y < num_colunas; y++) {
            if (i == x && j == y) {
                continue;
            }

            int vizinho = 0;

            if (((i - x == -1) || (i - x == 0) || (i - x == 1)) && ((j - y == -1) || (j - y == 0) || (j - y == 1))) {
                vizinho = 1;
                qtd_vizinhos += 1;
            }

            if (vizinho == 1) {
                soma += matriz[x][y];
            }
        }
    }

    if (soma == 0 && qtd_vizinhos == 0) {
        return matriz[i][j];
    }

    return soma / qtd_vizinhos;
}

void* calcular_medias(void* ptr) {
    tipo_obj_thread* thread_atual = (tipo_obj_thread*) ptr;

    if (thread_atual->linha != 0) {
        sem_wait(&vetor_obj_threads[thread_atual->linha - 1].semaforo);
        printf("%s e comecei a executar aqui\n", vetor_obj_threads[thread_atual->linha].mensagem);
    }

    for (int j = 0; j < num_colunas; j++) {
        printf("%s e estou calculando media para elemento da linha %d coluna %d...\n", thread_atual->mensagem, (thread_atual->linha + 1), j + 1);

        matriz[thread_atual->linha][j] = media_vizinhos(thread_atual->linha, j);
        thread_atual->coluna += 1;

        if (thread_atual->linha < (num_linhas - 1)) {
            if (thread_atual->pronta == 0) {
                // linha de baixo já terminou OU está duas colunas na frente
                if (((thread_atual->coluna) - (vetor_obj_threads[thread_atual->linha + 1].coluna) >= 2) || (thread_atual->coluna >= num_colunas)) {
                    sem_post(&thread_atual->semaforo);
                    thread_atual->pronta = 1;
                }
            }
        }

        // linha de cima não terminou E nem está 2 colunas na frente
        if ((thread_atual->linha != 0) && (vetor_obj_threads[thread_atual->linha - 1].coluna < num_colunas) && ((vetor_obj_threads[thread_atual->linha - 1].coluna - thread_atual->coluna) < 2)) {
            vetor_obj_threads[thread_atual->linha - 1].pronta = 0;
            sem_wait(&vetor_obj_threads[thread_atual->linha - 1].semaforo);
        }
    }

    // libera pra próxima thread
    if (thread_atual->linha < (num_linhas - 1)) {
        sem_post(&thread_atual->semaforo);
        thread_atual->pronta = 1;
    }
}

void executar_threads(pthread_t* vetor_pthread_t, tipo_obj_thread* vetor_obj_threads_local) {
    for (int x = 0; x < num_threads; x++) {
        int iret = pthread_create(&vetor_pthread_t[x], NULL, calcular_medias, (void*) &vetor_obj_threads_local[x]);
    }

    for (int y = 0; y < num_threads; y++) {
        pthread_join(vetor_pthread_t[y], NULL);
        sem_destroy(&vetor_obj_threads_local[y].semaforo);
    }
}

void main() {
    srand(time(NULL));

    gerar_matriz();
    imprimir_matriz();

    num_threads = num_linhas; // 1 thread para cada linha da matriz
    pthread_t vetor_pthread_t[num_threads];
    vetor_obj_threads = iniciar_vetor_obj_threads();

    executar_threads(vetor_pthread_t, vetor_obj_threads);
    imprimir_matriz();

    exit(0);
}
