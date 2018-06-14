#include <pthread.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>

#include "semaphore.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
static const int N_w_elves = 120;
static const int N_c_elves = 120;

static int w_elfs;
static int c_elfs;

static semaphore_t santaSem;
//static semaphore_t santaSem;

static semaphore_t w_elfSem;
static semaphore_t c_elfSem;

static semaphore_t mutex;

static int sleep_count = 0;

pthread_t *CreateThread(void *(*f)(void *), void *a) {
    pthread_t *t = malloc(sizeof(pthread_t));
    assert(t != NULL);
    int ret = pthread_create(t, NULL, f, a);
    assert(ret == 0);
    return t;
}

void *SantaClaus(void *arg) {
    printf("This is Santa\n");
    while (true) {
        //printf("SANTA: going into Sem work_elfs: %d collect_elfs: %d \n", w_elfs, c_elfs);
        P(santaSem);
        printf("SANTA: WHO  WOKE ME UP !!! work_elfs: %d collect_elfs: %d \n", w_elfs, c_elfs);

        P(mutex);
        if (w_elfs >= 1 && c_elfs > 2) {
            printf("SANTA: Starting COLLECTION meeting\n");
            //dismiss extra working elves
            while (w_elfs > 1) {
                w_elfs--;
                V(w_elfSem);
            }
            //Record nr of collector elves that have entered the meeting (c_elfs might changes due to release of mutex)
            int nrOfCollectors = c_elfs;
            V(mutex);
            printf("SANTA: work_elfs: %d collect_elfs: %d \n", w_elfs, c_elfs);


            //Actually meet
            printf("SANTA: Meeting right now...\n");
            sleep(4);
            printf("SANTA: work_elfs: %d collect_elfs: %d \n", w_elfs, c_elfs);


            //Ending the meeting again
            P(mutex);
            printf("SANTA: Ending COLLECTION meeting.\n");
            sleep_count++;

            for (int i = 0; i < nrOfCollectors; i++) {
                V(c_elfSem);
                c_elfs--;
            }

            V(w_elfSem);
            w_elfs--;
        } else if (w_elfs >= 2) {
            printf("SANTA: Starting WORK meeting\n");
            //dismiss extra working elves
            while (w_elfs > 2) {
                w_elfs--;
                V(w_elfSem);
            }
            V(mutex);
            printf("SANTA: work_elfs: %d collect_elfs: %d \n", w_elfs, c_elfs);

            //Actually meet
            printf("SANTA: Meeting right now...\n");
            sleep(4);
            printf("SANTA: work_elfs: %d collect_elfs: %d \n", w_elfs, c_elfs);

            //Ending the meeting again
            P(mutex);
            printf("SANTA: Ending WORK meeting.\n");
            sleep_count++;

            V(w_elfSem); // release worker 1
            w_elfs--;
            V(w_elfSem); // release worker 2
            w_elfs--;
        }
        printf("SANTA: work_elfs: %d collect_elfs: %d \n", w_elfs, c_elfs);
        if ((c_elfs < 3 && w_elfs < 2) || sleep_count == 4) {
            printf("SANTA: sleeping\n");
            sleep(10);
            sleep_count = 0;
            printf("SANTA: woke up\n");
        } else {
            printf("sleep count: %d \n", sleep_count);
        }

        V(mutex);

    }

}

void *WorkElf(void *arg) {
    int id = (int) arg;
    srand(id + 2);
    printf("Starting WORKER %d\n", id);
    while (true) {
        bool need_meet = rand() % 100 < 10;

        if (need_meet) {
//            printf("work elf %d needs meet\n", id);
            P(mutex);
            w_elfs++;
            V(mutex);
            printf("WORKER %d ready to meet work_elfs: %d collect_elfs: %d)\n", id, w_elfs, c_elfs);

            // Wait to meet
            V(santaSem);
            P(w_elfSem);
            printf("WORKER %d: back to work\n", id);
        }
        sleep(2 + rand() % 5);
    }
}

void *CollectElf(void *arg) {
    int id = (int) arg;
    srand(id);
    printf("Starting COLLECTER: %d\n", id);

    while (true) {
        bool need_meet = rand() % 100 < 10;

        if (need_meet) {
            P(mutex);
            c_elfs++;
            V(mutex);
            printf("COLLECTER %d: ready to meet (work_elfs: %d collect_elfs: %d)\n", id, w_elfs, c_elfs);

            //Wait to meet
            V(santaSem);
            P(c_elfSem);
            printf("COLLECTER %d: back to work\n", id);

        }
        sleep(2 + rand() % 5);
    }
}

int main(int ac, char **av) {
    srand(233);

    w_elfs = 0;
    c_elfs = 0;

    santaSem = CreateSemaphore(0);

    w_elfSem = CreateSemaphore(0);
    c_elfSem = CreateSemaphore(0);


    mutex = CreateSemaphore(1);

    pthread_t *santa_claus = CreateThread(SantaClaus, 0);

    pthread_t *w_elfs[N_w_elves];
    for (int r = 0; r < N_w_elves; r++)
        w_elfs[r] = CreateThread(WorkElf, (void *) r + 1);

    pthread_t *c_elfs[N_c_elves];
    for (int e = 0; e < N_c_elves; e++)
        c_elfs[e] = CreateThread(CollectElf, (void *) e + 1);

    int ret = pthread_join(*santa_claus, NULL);
    assert(ret == 0);


}

#pragma clang diagnostic pop