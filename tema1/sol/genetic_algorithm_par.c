#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "genetic_algorithm_par.h"
#include <pthread.h>

int read_input(long *NUM_THREADS, sack_object **objects, int *object_count, int *sack_capacity, int *generations_count, int argc, char *argv[])
{
    FILE *fp;

    if (argc < 4) {
        fprintf(stderr, "Usage:\n\t./tema1_par <fisier_intrare> <numar_generatii> <P>\n");
        return 0;
    }

    fp = fopen(argv[1], "r");
    if (fp == NULL) {
        return 0;
    }

    if (fscanf(fp, "%d %d", object_count, sack_capacity) < 2) {
        fclose(fp);
        return 0;
    }

    if (*object_count % 10) {
        fclose(fp);
        return 0;
    }

    sack_object *tmp_objects = (sack_object *) calloc(*object_count, sizeof(sack_object));

    for (int i = 0; i < *object_count; ++i) {
        if (fscanf(fp, "%d %d", &tmp_objects[i].profit, &tmp_objects[i].weight) < 2) {
            free(objects);
            fclose(fp);
            return 0;
        }
    }

    fclose(fp);

    *generations_count = (int) strtol(argv[2], NULL, 10);
    *NUM_THREADS = (int) strtol(argv[3], NULL, 10);

    if (*generations_count == 0) {
        free(tmp_objects);

        return 0;
    }

    *objects = tmp_objects;

    return 1;
}

void print_objects(const sack_object *objects, int object_count)
{
    for (int i = 0; i < object_count; ++i) {
        printf("%d %d\n", objects[i].weight, objects[i].profit);
    }
}

void print_generation(const individual *generation, int limit)
{
    for (int i = 0; i < limit; ++i) {
        for (int j = 0; j < generation[i].chromosome_length; ++j) {
            printf("%d ", generation[i].chromosomes[j]);
        }

        printf("\n%d - %d\n", i, generation[i].fitness);
    }
}

void print_best_fitness(const individual *generation)
{
    printf("%d\n", generation[0].fitness);
}

void compute_fitness_function(const sack_object *objects, individual *generation, int object_count, int sack_capacity)
{
    int weight;
    int profit;

    for (int i = 0; i < object_count; ++i) {
        weight = 0;
        profit = 0;

        for (int j = 0; j < generation[i].chromosome_length; ++j) {
            if (generation[i].chromosomes[j]) {
                weight += objects[j].weight;
                profit += objects[j].profit;
            }
        }

        generation[i].fitness = (weight <= sack_capacity) ? profit : 0;
    }
}

int cmpfunc(const void *a, const void *b)
{
    individual *first = (individual *) a;
    individual *second = (individual *) b;

    int res = second->fitness - first->fitness; // decreasing by fitness
    if (res == 0) {
        res = first->count - second->count; // increasing by number of objects in the sack
        if (res == 0) {
            return second->index - first->index;
        }
    }

    return res;
}

void mutate_bit_string_1(const individual *ind, int generation_index)
{
    int i, mutation_size;
    int step = 1 + generation_index % (ind->chromosome_length - 2);

    if (ind->index % 2 == 0) {
        // for even-indexed individuals, mutate the first 40% chromosomes by a given step
        mutation_size = ind->chromosome_length * 4 / 10;
        for (i = 0; i < mutation_size; i += step) {
            ind->chromosomes[i] = 1 - ind->chromosomes[i];
        }
    } else {
        // for even-indexed individuals, mutate the last 80% chromosomes by a given step
        mutation_size = ind->chromosome_length * 8 / 10;
        for (i = ind->chromosome_length - mutation_size; i < ind->chromosome_length; i += step) {
            ind->chromosomes[i] = 1 - ind->chromosomes[i];
        }
    }
}

void mutate_bit_string_2(const individual *ind, int generation_index)
{
    int step = 1 + generation_index % (ind->chromosome_length - 2);

    // mutate all chromosomes by a given step
    for (int i = 0; i < ind->chromosome_length; i += step) {
        ind->chromosomes[i] = 1 - ind->chromosomes[i];
    }
}

void crossover(individual *parent1, individual *child1, int generation_index)
{
    individual *parent2 = parent1 + 1;
    individual *child2 = child1 + 1;
    int count = 1 + generation_index % parent1->chromosome_length;

    memcpy(child1->chromosomes, parent1->chromosomes, count * sizeof(int));
    memcpy(child1->chromosomes + count, parent2->chromosomes + count, (parent1->chromosome_length - count) * sizeof(int));

    memcpy(child2->chromosomes, parent2->chromosomes, count * sizeof(int));
    memcpy(child2->chromosomes + count, parent1->chromosomes + count, (parent1->chromosome_length - count) * sizeof(int));
}

void copy_individual(const individual *from, const individual *to)
{
    memcpy(to->chromosomes, from->chromosomes, from->chromosome_length * sizeof(int));
}

void free_generation(individual *generation)
{
    int i;

    for (i = 0; i < generation->chromosome_length; ++i) {
        free(generation[i].chromosomes);
        generation[i].chromosomes = NULL;
        generation[i].fitness = 0;
    }
}

void *threadFn(void *arg)
{
    struct my_arg* data = (struct my_arg*) arg;
    long ID = data->id;
    int start, end;

    start = ID * (double)data->object_count / data->NUM_THREADS;
    end = min((ID + 1) * (double)data->object_count / data->NUM_THREADS, data->object_count);
    for (int i = start; i < end; ++i) {
        data->current_generation[i].fitness = 0;
        data->current_generation[i].chromosomes = (int*) calloc(data->object_count, sizeof(int));
        data->current_generation[i].chromosomes[i] = 1;
        data->current_generation[i].index = i;
        data->current_generation[i].chromosome_length = data->object_count;

        data->next_generation[i].fitness = 0;
        data->next_generation[i].chromosomes = (int*) calloc(data->object_count, sizeof(int));
        data->next_generation[i].index = i;
        data->next_generation[i].chromosome_length = data->object_count;
    }

    pthread_barrier_wait(data->barrier);
    int count, cursor;
    int generations_count = data->generations_count;
    int sack_capacity = data->sack_capacity;

    int object_count = data->object_count;

    for (int k = 0; k < generations_count; ++k) {
        cursor = 0;

        // compute fitness and sort by it
        compute_fitness_function(data->objects, data->current_generation + start, end - start, sack_capacity);
        pthread_barrier_wait(data->barrier);

        // precalculam countul de cromozomi pentru a optimiza functia de cmp
        for (int i = start; i < end; i++) {
            int countIndividual = 0;
            for (int q = 0; q < data->current_generation[i].chromosome_length; q++) {
                countIndividual += data->current_generation[i].chromosomes[q];
            }
            data->current_generation[i].count = countIndividual;
        }

        pthread_barrier_wait(data->barrier);

        if (ID == 0) {
            qsort(data->current_generation, object_count, sizeof(individual), cmpfunc);
        }
        pthread_barrier_wait(data->barrier);

        // keep first 30% children (elite children selection)
        count = object_count * 3 / 10;
        int startNew = ID * (double)count / data->NUM_THREADS;
        int endNew = min((ID + 1) * (double)count / data->NUM_THREADS, count);
        for (int i = startNew; i < endNew; ++i) {
            copy_individual(data->current_generation + i, data->next_generation + i);
        }
        cursor = count;
        pthread_barrier_wait(data->barrier);

        // mutate first 20% children with the first version of bit string mutation
        count = object_count * 2 / 10;
        startNew = ID * (double)count / data->NUM_THREADS;
        endNew = min((ID + 1) * (double)count / data->NUM_THREADS, count);
        for (int i = startNew; i < endNew; ++i) {
            copy_individual(data->current_generation + i, data->next_generation + cursor + i);
            mutate_bit_string_1(data->next_generation + cursor + i, k);
        }
        cursor += count;
        pthread_barrier_wait(data->barrier);

        // mutate next 20% children with the second version of bit string mutation
        count = object_count * 2 / 10;
        startNew = ID * (double)count / data->NUM_THREADS;
        endNew = min((ID + 1) * (double)count / data->NUM_THREADS, count);
        for (int i = startNew; i < endNew; ++i) {
            copy_individual(data->current_generation + i + count, data->next_generation + cursor + i);
            mutate_bit_string_2(data->next_generation + cursor + i, k);
        }
        cursor += count;
        pthread_barrier_wait(data->barrier);

        // crossover first 30% parents with one-point crossover
        // (if there is an odd number of parents, the last one is kept as such)
        count = object_count * 3 / 10;

        if (count % 2 == 1 && ID == 0) {
            copy_individual(data->current_generation + object_count - 1, data->next_generation + cursor + count - 1);
            count--;
        }
        pthread_barrier_wait(data->barrier);
        count = count / 2;
        startNew = ID * (double)count / data->NUM_THREADS;
        endNew = min((ID + 1) * (double)count / data->NUM_THREADS, count);
        for (int i = startNew; i < endNew; i++) {
            crossover(data->current_generation + i * 2, data->next_generation + cursor + i * 2, k);
        }
        pthread_barrier_wait(data->barrier);

        // switch to new generation
        individual *tmp = NULL;
        tmp = data->current_generation;
        data->current_generation = data->next_generation;
        data->next_generation = tmp;

        pthread_barrier_wait(data->barrier);

        for (int i = start; i < end; ++i) {
            data->current_generation[i].index = i;
        }
        pthread_barrier_wait(data->barrier);

        if (k % 5 == 0 && ID == 0) {
            print_best_fitness(data->current_generation);
        }
        pthread_barrier_wait(data->barrier);

    }
    pthread_barrier_wait(data->barrier);

    compute_fitness_function(data->objects, data->current_generation + start, end - start, sack_capacity);

    pthread_barrier_wait(data->barrier);

    if (ID == 0) {
        qsort(data->current_generation, object_count, sizeof(individual), cmpfunc);
    }

    pthread_exit(NULL);
}
void run_genetic_algorithm(long NUM_THREADS, const sack_object *objects, int object_count, int generations_count, int sack_capacity)
{
    individual *current_generation = (individual*) calloc(object_count, sizeof(individual));
    individual *next_generation = (individual*) calloc(object_count, sizeof(individual));

    pthread_barrier_t barrier;
    pthread_t threads[NUM_THREADS];
    int r;
    long id;
    void *status;
    struct my_arg *arguments = (struct my_arg*) malloc(NUM_THREADS * sizeof(struct my_arg));

    pthread_barrier_init(&barrier, NULL, NUM_THREADS);

    for (id = 0; id < NUM_THREADS; id++) {
        arguments[id].id = id;
        arguments[id].NUM_THREADS = NUM_THREADS;
        arguments[id].object_count = object_count;
        arguments[id].generations_count = generations_count;
        arguments[id].current_generation = current_generation;
        arguments[id].next_generation = next_generation;
        arguments[id].objects = objects;
        arguments[id].sack_capacity = sack_capacity;
        arguments[id].barrier = &barrier;

        r = pthread_create(&threads[id], NULL, threadFn, (void *) &arguments[id]);

        if (r) {
            printf("Eroare la crearea thread-ului %ld\n", id);
            exit(-1);
        }
    }

    for (id = 0; id < NUM_THREADS; id++) {
        r = pthread_join(threads[id], &status);
        if (r) {
            printf("Eroare la asteptarea thread-ului %ld\n", id);
            exit(-1);
        }
    }
    pthread_barrier_destroy(&barrier);

    print_best_fitness(current_generation);

    // free resources for old generation
    free_generation(current_generation);
    free_generation(next_generation);

    // free resources
    free(current_generation);
    free(next_generation);

}