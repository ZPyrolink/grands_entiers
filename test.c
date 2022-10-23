#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <getopt.h>
#include "parametrized_output.h"
#include "grand_entier.h"

#define NUMBER 10
#define check(x) ((x)?printf("Check passed\n"):\
                  printf("*** CHECK FAILED *** at line %d in %s\n", __LINE__, __FILE__))

void usage(char *nom) {
  printf(
         "Usage :\n"
         "%s [ -h ] [ -n num_test ] [ -s graine ] [ -b bits-count ]\n"
         // "%s [ -h ] [ -n num_test ] [ -s graine ] [ -b bits-count ] [ -m memory-limit ] [ -a allocs-limit ]\n"
         "\n"
         "Tests the proper behavior of big integers, handles the following options (all of them with an argument):\n"
         "-n, --num-test           Test number:\n"
         "                         0. Allocations and deallocations\n"
         "                         1. Bits manipulations\n"
         "                         2. Arithmetics\n"
         "                         3. Usage of all the allocated bits\n"
         "                         4. Efficient reallocation\n"
         "-s, --seed               Seed for the random generation\n"
         "-b, --bits-count         Maximal number of bits in the big integers\n"
         // "-m, --memory-limit       Nombre d'octets alloués à ne pas dépasser\n"
         // "-a, --allocs-limit       Nombre d'allocations mémoire à ne pas dépasser\n"
         , nom);
  exit(0);
}

grand_entier_t *ge_cree_random(int nb_bits) {
  grand_entier_t *e;

  e = ge_cree();
  if (e != NULL) {
    for (int i=0; i<nb_bits*2; i++) {
      int bit = random()%nb_bits;
      grand_entier_t *res;
      if (random() % 3) {
        // debug("Setting bit %d\n", bit);
        res = ge_set_bit(e, bit);
      } else {
        // debug("Clearing bit %d\n", bit);
        res = ge_clr_bit(e, bit);
      }
      if (res == NULL) {
        ge_libere(e);
        return NULL;
      }
    }
  }
  return e;
}

void ge_print(char *p, grand_entier_t *e, char *s) {
  int nb_bits = ge_nb_bits(e);
  printf("%s(%d bits) ", p, nb_bits);
  for (int i=nb_bits-1; i>=0; i--) {
    printf("%d", ge_get_bit(e, i));
    if (i%10 == 0)
      printf(" ");
  }
  printf("%s", s);
}

int main(int argc, char *argv[]) {
  int num_test, seed, bits_count, memory_limit, allocs_limit;
  int opt;

  num_test = 0;
  seed = 42;
  bits_count = 32;
  memory_limit = 0;
  allocs_limit = 0;

  struct option longopts[] = {
        { "num-test", required_argument, NULL, 'n' },
        { "seed", required_argument, NULL, 's' },
        { "bits-count", required_argument, NULL, 'b' },
        { "memory-limit", required_argument, NULL, 'm' },
        { "allocs-limit", required_argument, NULL, 'a' },
        { "help", no_argument, NULL, 'h' },
        { NULL, 0, NULL, 0 }
  };

  while ((opt = getopt_long(argc, argv, "n:s:b:m:a:h", longopts, NULL)) != -1) {
    switch(opt) {
    case 'n':
      num_test = atoi(optarg);
      break;
    case 's':
      seed = atoi(optarg);
      break;
    case 'b':
      bits_count = atoi(optarg);
      break;
    case 'm':
      memory_limit = atoi(optarg);
      break;
    case 'a':
      allocs_limit = atoi(optarg);
      break;
    case 'h':
      usage(argv[0]);
      break;
    default:
      fprintf(stderr, "Unrecognized option %c\n", opt);
      exit(1);
    }
  }
  printf("Tests invoked with num-test %d, seed %d, bits-count %d, memory-limit %d and allocs-limit %d\n",
         num_test, seed, bits_count, memory_limit, allocs_limit);
  if (num_test == 0) {
    grand_entier_t *e[NUMBER];

    printf("Allocating %d big integers\n", NUMBER);
    for (int i=0; i<NUMBER; i++) {
      e[i] = ge_cree();
    }
    printf("Freeing half of them\n");
    for (int i=1; i<NUMBER; i+=2) {
      if (e[i] != NULL)
        ge_libere(e[i]);
    }
    printf("Checking that the number of allocations is at least %d\n", NUMBER/2);
    check(current_allocs() >= (NUMBER/2));
    printf("Allocating %d big integers\n", NUMBER/2);
    for (int i=1; i<NUMBER; i+=2) {
      e[i] = ge_cree();
    }
    printf("Checking that the number of allocations is at least %d\n", NUMBER);
    check(current_allocs() >= NUMBER);
    printf("Freeing all the big integers\n");
    for (int i=0; i<NUMBER; i++) {
      if (e[i] != NULL)
        ge_libere(e[i]);
    }
    printf("Checking that the number of allocations is 0\n");
    check(current_allocs() == 0);
  } else {
    grand_entier_t *a, *b;

    srandom(seed);
    a = ge_cree_random(bits_count);
    b = ge_cree_random(bits_count);

    if ((a == NULL) || (b == NULL)) {
      if (a != NULL)
        ge_libere(a);
      printf("Allocation failed, allocs count %d\n", current_allocs());
      printf("Checking that at least one realloc has been successfully done\n");
      check(reallocs_done() > 0);
      exit(1);
    }

    ge_print("Entier a : ", a, "\n");
    ge_print("Entier b : ", b, "\n");

    if (num_test > 1) {
      grand_entier_t *c;

      c = ge_cree();
      ge_add(c, a);
      ge_add(c, b);
      ge_print("Entier a+b : ", c, "\n");
      int dec = random() % (bits_count / 2 + 1);
      ge_shift(c, dec);
      printf("Entier (a+b)<<%d : ", dec);
      ge_print("", c, "\n");
      ge_libere(c);

      // We test memory before the multiplication because it depends too much on the algorithm in use
      if (num_test > 2) {
        int nb_bytes = 3*(bits_count/8);
        debug("Memory allocated (%lu bytes)\n", max_memory_used());
        printf("Checking that the bits are actually allocated (%d bytes required)\n", nb_bytes);
        check(max_memory_used() >= nb_bytes);

        // For each big integer we count:
        // - 4 bits per byte (efficient reallocation scheme)
        // - 24 additional bytes (structure to manage the big integer)
        // We multiply by 4 because c might end up with more than 1.5 times the size of a or b
        nb_bytes = 4*(bits_count/4 + 24);
        printf("Checking that the maximal size of memory allocated is no more than %d\n", nb_bytes);
        check(max_memory_used() <= nb_bytes);
      }

      if (num_test > 3) {
        int allocs_bound = 3;
        debug("Total number of allocations (%d)\n", total_allocs());
        printf("Checking that there is a least one allocation for each integer (%d required)\n", allocs_bound);
        check(total_allocs() >= allocs_bound);

        // With an efficient reallocation scheme, the number of allocation should be logaritmic
        allocs_bound = 1;
        int target = 8;
        while (target < bits_count) {
          allocs_bound++;
          target *= 2;
        }
        // We add 4 for the potential additional reallocation in c and the management structures
        allocs_bound = allocs_bound*3+4;
        printf("Checking that the number of allocations is no more than %d\n", allocs_bound);
        check(total_allocs() <= allocs_bound);
      }

      c = ge_cree();
      ge_add(c, a);
      ge_mul(c, b);
      ge_print("Entier a*b : ", c, "\n");
      ge_libere(c);
    }
    ge_libere(a);
    ge_libere(b);
    printf("Checking that the number of allocations is 0\n");
    check(current_allocs() == 0);
  }
}
