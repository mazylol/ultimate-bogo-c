#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_THREADS 4

void shuffle(int *array, const int n) {
    for (int i = n - 1; i > 0; i--) {
        const int j = rand() % (i + 1);
        const int temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

int check(const int *array, const int n) {
    int first = 1;
    int prev = array[0];

    for (int i = 0; i < n; i++) {
        if (first) {
            first = 0;
            continue;
        }

        if (array[i] < prev) {
            return 0;
        }

        prev = array[i];
    }

    return 1;
}

int shuffles = 0;

int stop = 0;
pthread_mutex_t stopMutex;

int getStop(void) {
    int ret = 0;
    pthread_mutex_lock(&stopMutex);
    ret = stop;
    pthread_mutex_unlock(&stopMutex);
    return ret;
}

void setStop(const int val) {
    pthread_mutex_lock(&stopMutex);
    stop = val;
    pthread_mutex_unlock(&stopMutex);
}

struct thread_args {
    int *array;
    int n;
};

void *sort(void *arguments) {
    struct thread_args *args = arguments;

    while (check(args->array, args->n) == 0) {
        if (getStop()) {
            free(args->array);
            free(args);
            return NULL;
        }

        shuffle(args->array, args->n);
        pthread_mutex_lock(&stopMutex);
        shuffles += 1;
        pthread_mutex_unlock(&stopMutex);
    }

    setStop(1);

    printf("Sorted array: ");
    for (int i = 0; i < args->n; i++) {
        printf("%d ", args->array[i]);
    }
    printf("\n");

    free(args->array);
    free(args);

    return NULL;
}

int main() {
    srand(time(NULL));

    int array[] = {1, 2, 3, 4, 5, 6, 7, 8};
    const int n = sizeof(array) / sizeof(array[0]);

    shuffle(array, n);

    if (pthread_mutex_init(&stopMutex, NULL) != 0) {
        printf("\n mutex init has failed\n");
        return 1;
    }

    pthread_t threads[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        struct thread_args *args = malloc(sizeof *args);
        if (args == NULL) {
            perror("Failed to allocate memory");
            exit(EXIT_FAILURE);
        }

        args->array = malloc(sizeof(int) * n);
        if (args->array == NULL) {
            perror("Failed to allocate memory for array");
            free(args);
            exit(EXIT_FAILURE);
        }

        memcpy(args->array, array, sizeof(int) * n);
        memcpy(&args->n, &n, sizeof(int));

        if (pthread_create(&threads[i], NULL, &sort, args)) {
            perror("Failed to create thread");
            free(args->array);
            free(args);
            pthread_mutex_destroy(&stopMutex);
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Shuffles: %d\n", shuffles);

    pthread_mutex_destroy(&stopMutex);

    return 0;
}
