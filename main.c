#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>

void *calculate_primes_thread(void *arguments);
void calculate_primes(int slave_id, int num_slaves, unsigned long max_prime);

// this struct will contain the command line arguments
// which will be passed to the function that is to be called
struct arg_struct {
    int slave_id;
    int num_slaves;
    unsigned long max_prime;
};

int main(int argc, char *argv[]) {
    struct timeval start, end;
    if (argc != 4) {
        perror("Invalid number of arguments\n");
        return 1;
    }
    if (atol(argv[1]) < 0) {
        perror("1st argument (max_prime) must be positive\n");
        return 2;
    }
    unsigned long max_prime = atol(argv[1]);
    if (atoi(argv[2]) < 0) {
        perror("2nd argument (num_slaves) must be positive\n");
        return 3;
    }
    int num_slaves = atoi(argv[2]);
    if (strcmp(argv[3], "thread") == 0) {
        pthread_t threads[num_slaves];
        struct arg_struct args[num_slaves];
        // start time
        gettimeofday(&start, NULL);
        for (int i = 0; i < num_slaves; i++) {
            args[i].max_prime = max_prime;
            args[i].num_slaves = num_slaves;
            args[i].slave_id = i;
            if (pthread_create(&threads[i], NULL, &calculate_primes_thread, (void *)&args[i]) != 0) {
                perror("thread_create: can not create thread \n");
                printf("%d", i);
                return -1;
            }
        }
        for (int i = 0; i < num_slaves; ++i) {
            pthread_join(threads[i], 0);
        }
        gettimeofday(&end, NULL);
    } else if (strcmp(argv[3], "process") == 0) {
        struct arg_struct args[num_slaves];
        gettimeofday(&start, NULL);
        for (int i = 0; i < num_slaves; i++) {
            if (fork() == 0) {
				calculate_primes(i, num_slaves, max_prime);
				exit(0);
            }
        }
        for (int i = 0; i < num_slaves; i++) {
			wait(NULL);
        }
        gettimeofday(&end, NULL);
    } else {
        printf("3rd argument must be either \"process\" or \"thread\"\n");
        return 4;
    }

    unsigned int delta = (end.tv_usec - start.tv_usec) / 1000;

    printf("This machine calculated all prime numbers under %lu using %d slaves in %u milliseconds.\n",
           max_prime, num_slaves, delta);
    return 0;
}

void *calculate_primes_thread(void *arguments) {
    struct arg_struct *args = (struct arg_struct *)arguments;
    struct timeval start, end;
    gettimeofday(&start, NULL);

    unsigned long num, i, primes = 0, flag;
    num = 3 + 2 * args->slave_id;

    while (num < args->max_prime) {
        flag = 0;
        for (i = 2; i <= num / 2; i++) {
            if (num % i == 0) {
                flag = 1;
                break;
            }
        }
        if (flag == 0 && (num > 1)) {
            ++primes;
        }
        num += 2 * args->num_slaves;
    }
    gettimeofday(&end, NULL);
    unsigned int delta = (end.tv_usec - start.tv_usec) / 1000;
    printf("Thread %d computed %lu prime numbers in %u milliseconds.\n", args->slave_id, primes, delta);
    return NULL;
}

void calculate_primes(int slave_id, int num_slaves, unsigned long max_prime) {
    struct timeval start, end;
    gettimeofday(&start, NULL);

    unsigned long num, i, primes = 0, flag;
    num = 3 + 2 * slave_id;

    while (num < max_prime) {
        flag = 0;
        for (i = 2; i <= num / 2; i++) {
            if (num % i == 0) {
                flag = 1;
                break;
            }
        }
        if (flag == 0 && (num > 1)) {
            ++primes;
        }
        num += 2 * num_slaves;
    }
    gettimeofday(&end, NULL);
    unsigned int delta = (end.tv_usec - start.tv_usec) / 1000;
    printf("Process %d computed %lu prime numbers in %u milliseconds.\n", slave_id, primes, delta);
}