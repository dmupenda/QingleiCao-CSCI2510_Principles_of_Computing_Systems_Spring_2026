#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t crypt_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    char salt[3];
    char target[14];
    int keysize;
    int thread_id;
    int num_threads;
    int found;
    char password[9];
    pthread_mutex_t result_mutex;
} thread_data_t;

// Iterative password generator for exact length 'len'
void try_exact_length(int len, thread_data_t *tdata) {
    if (tdata->found) return;

    int indices[len];
    for (int i = 0; i < len; i++) indices[i] = 0; // start at 'a'

    while (1) {
        // Build candidate string
        char candidate[len + 1];
        for (int i = 0; i < len; i++)
            candidate[i] = 'a' + indices[i];
        candidate[len] = '\0';

        // Hash and compare
        pthread_mutex_lock(&crypt_mutex);
        char *hash = crypt(candidate, tdata->salt);
        pthread_mutex_unlock(&crypt_mutex);

        if (strcmp(hash, tdata->target) == 0) {
            pthread_mutex_lock(&tdata->result_mutex);
            if (!tdata->found) {
                tdata->found = 1;
                strcpy(tdata->password, candidate);
            }
            pthread_mutex_unlock(&tdata->result_mutex);
            return;
        }

        // Increment the counter (like base-26 number)
        int pos = len - 1;
        while (pos >= 0) {
            indices[pos]++;
            if (indices[pos] < 26) break;
            indices[pos] = 0;
            pos--;
        }
        if (pos < 0) break; // all combinations done
    }
}

void *worker(void *arg) {
    thread_data_t *tdata = (thread_data_t *)arg;

    // Shorter passwords (1 .. keysize-1) – only thread 0 handles them
    if (tdata->thread_id == 0) {
        for (int len = 1; len < tdata->keysize; len++) {
            if (tdata->found) break;
            try_exact_length(len, tdata);
        }
    }

    // Passwords of exactly 'keysize' length – split by first character
    if (!tdata->found) {
        int chars_per_thread = 26 / tdata->num_threads;
        int start = tdata->thread_id * chars_per_thread;
        int end = (tdata->thread_id == tdata->num_threads - 1) ? 26 : start + chars_per_thread;

        for (int first = start; first < end; first++) {
            if (tdata->found) break;
            // Build a separate generator for the rest (keysize-1 positions)
            int sub_len = tdata->keysize - 1;
            if (sub_len == 0) {
                // keysize == 1 – just the single character
                char candidate[2];
                candidate[0] = 'a' + first;
                candidate[1] = '\0';
                pthread_mutex_lock(&crypt_mutex);
                char *hash = crypt(candidate, tdata->salt);
                pthread_mutex_unlock(&crypt_mutex);
                if (strcmp(hash, tdata->target) == 0) {
                    pthread_mutex_lock(&tdata->result_mutex);
                    if (!tdata->found) {
                        tdata->found = 1;
                        strcpy(tdata->password, candidate);
                    }
                    pthread_mutex_unlock(&tdata->result_mutex);
                }
            } else {
                // Iterate over all suffixes of length sub_len
                int indices[sub_len];
                for (int i = 0; i < sub_len; i++) indices[i] = 0;
                while (1) {
                    char candidate[tdata->keysize + 1];
                    candidate[0] = 'a' + first;
                    for (int i = 0; i < sub_len; i++)
                        candidate[i + 1] = 'a' + indices[i];
                    candidate[tdata->keysize] = '\0';

                    pthread_mutex_lock(&crypt_mutex);
                    char *hash = crypt(candidate, tdata->salt);
                    pthread_mutex_unlock(&crypt_mutex);

                    if (strcmp(hash, tdata->target) == 0) {
                        pthread_mutex_lock(&tdata->result_mutex);
                        if (!tdata->found) {
                            tdata->found = 1;
                            strcpy(tdata->password, candidate);
                        }
                        pthread_mutex_unlock(&tdata->result_mutex);
                        return NULL;
                    }

                    // increment suffix counter
                    int pos = sub_len - 1;
                    while (pos >= 0) {
                        indices[pos]++;
                        if (indices[pos] < 26) break;
                        indices[pos] = 0;
                        pos--;
                    }
                    if (pos < 0) break;
                }
            }
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <threads> <keysize> <target>\n", argv[0]);
        return 1;
    }

    int num_threads = atoi(argv[1]);
    int keysize = atoi(argv[2]);
    char *target = argv[3];
    if (keysize > 8) keysize = 8;

    char salt[3];
    strncpy(salt, target, 2);
    salt[2] = '\0';

    pthread_t threads[num_threads];
    thread_data_t tdata[num_threads];
    pthread_mutex_t result_mutex = PTHREAD_MUTEX_INITIALIZER;

    for (int i = 0; i < num_threads; i++) {
        strcpy(tdata[i].salt, salt);
        strcpy(tdata[i].target, target);
        tdata[i].keysize = keysize;
        tdata[i].thread_id = i;
        tdata[i].num_threads = num_threads;
        tdata[i].found = 0;
        tdata[i].password[0] = '\0';
        tdata[i].result_mutex = result_mutex;
    }

    for (int i = 0; i < num_threads; i++)
        pthread_create(&threads[i], NULL, worker, &tdata[i]);

    for (int i = 0; i < num_threads; i++)
        pthread_join(threads[i], NULL);

    for (int i = 0; i < num_threads; i++) {
        if (tdata[i].found) {
            printf("%s\n", tdata[i].password);
            return 0;
        }
    }
    return 0;
}
