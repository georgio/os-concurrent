#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>

// this function is to be used for thread execution
void *calculate_primes_thread(void *arguments);
// this function is to be used for child process execution
void calculate_primes(int slave_id, int num_slaves, unsigned long max_prime);

// this struct will contain the command line arguments
// which will be passed to the function that is to be called
struct arg_struct {
    int slave_id;
    int num_slaves;
    unsigned long max_prime;
};

int main(int argc, char *argv[]) {
    // declare timeval values for total time
    struct timeval start, end;

    // basic input validation

    // checking for number of arguments provided
    if (argc != 4) {
        perror("Invalid number of arguments\n");
        return 1;
    }
    if (atol(argv[1]) < 0) {
        perror("1st argument (max_prime) must be positive\n");
        return 2;
    }
    // parse first argument into unsigned long
    // after checking that it is positive
    unsigned long max_prime = atol(argv[1]);
    if (atoi(argv[2]) < 0) {
        perror("2nd argument (num_slaves) must be positive\n");
        return 3;
    }
    // parse second argument into int
    // after checking that it is positive
    int num_slaves = atoi(argv[2]);

    // thread block
    if (strcmp(argv[3], "thread") == 0) {
        // create an array of threads
        pthread_t threads[num_slaves];
        // create an array of arg_struct
        // each element would correspond
        // to a thread
        struct arg_struct args[num_slaves];
        // start timer for total computation time
        gettimeofday(&start, NULL);
        // create a thread for each slave
        // initialize an arg_struct for each slave
        // execute calculate_primes_thread
        // on each thread concurrently
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
        // join every thread after it halts
        for (int i = 0; i < num_slaves; i++) {
            pthread_join(threads[i], 0);
        }
    }

    // child process block
    else if (strcmp(argv[3], "process") == 0) {
        // start timer for child process execution time
        gettimeofday(&start, NULL);
        // fork a child process for each slave
        // then execute calculate_primes
        // i is the slave_id in this case
        for (int i = 0; i < num_slaves; i++) {
            if (fork() == 0) {
                calculate_primes(i, num_slaves, max_prime);
                exit(0);
            }
        }
        // wait for each child processes to halt
        for (int i = 0; i < num_slaves; i++) {
            wait(NULL);
        }
    } else {
        printf("3rd argument must be either \"process\" or \"thread\"\n");
        return 4;
    }

    // stop counter for total execution time because
    // computation should be over by the
    // time this line of code is reached
    gettimeofday(&end, NULL);
    // compute difference between timers
    // tv.usec provides time in microseconds,
    // dividing by 1000 yields the time in milliseconds
    unsigned int delta = (end.tv_usec - start.tv_usec) / 1000;

    printf("This machine calculated all prime numbers under %lu using %d slaves in %u milliseconds.\n",
           max_prime, num_slaves, delta);

    return 0;
}

void *calculate_primes_thread(void *arguments) {
    // initialize an arg_struct pointer to absorb
    // the arguments provided to this function
    struct arg_struct *args = (struct arg_struct *)arguments;
    // declare timeval values for
    // thread execution time
    struct timeval start, end;
    // start timer for thread computation time
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
    // stop timer for thread computation time
    gettimeofday(&end, NULL);
    // compute difference between timers
    // tv.usec provides time in microseconds,
    // dividing by 1000 yields the time in milliseconds
    unsigned int delta = (end.tv_usec - start.tv_usec) / 1000;
    printf("Thread %d computed %lu prime numbers in %u milliseconds.\n", args->slave_id, primes, delta);
    return NULL;
}

void calculate_primes(int slave_id, int num_slaves, unsigned long max_prime) {
    struct timeval start, end;
    // start timer for child process computation time
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
    // stop counter for child execution time because
    // computation should be over by the
    // time this line of code is reached
    gettimeofday(&end, NULL);
    // compute difference between timers
    // tv.usec provides time in microseconds,
    // dividing by 1000 yields the time in milliseconds
    unsigned int delta = (end.tv_usec - start.tv_usec) / 1000;
    printf("Process %d computed %lu prime numbers in %u milliseconds.\n", slave_id, primes, delta);
}