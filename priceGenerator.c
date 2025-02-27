#include <getopt.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
  uint8_t uniformFlag = 1;
  uint8_t gaussianFlag = 0;
  uint8_t exponentialFlag = 0;
  uint8_t autoregressiveFlag = 0;

  double uniformHigh = 1;
  double gaussianMean = 1;
  double exponentialMean = 1;
  double autoregressivePhi = 1;

  uint8_t roundsFlag = 0;
  uint8_t pricesFlag = 0;

  uint64_t totalRounds = 1000;
  uint64_t pricesPerRound = 10;

  int opt;

  opterr = 0;

  while ((opt = getopt(argc, argv, "hu:g:e:a:t:n:")) != -1) {
    switch (opt) {
    case 'h':
      printf("Usage:\n");
      printf("    # Only one option can be used at a time.\n"
             "    # If more than one option is chosen, the program chooses the "
             "last one to be declared.\n"
             "    # If no option is chosen, the program chooses `-u 1`.\n"
             "    # The default number of rounds is 1000, and the default "
             "number of prices per round is 10\n\n");
      printf("    priceGenerator [option] [-t <number of rounds>] [-n <prices "
             "per round>]\n");
      printf("    priceGenerator -h      # Display this help screen.\n\n");
      printf("Options:\n");
      printf("    -u <high>            Generate prices from the Uniform "
             "Distribution from 0 to <high>.\n");
      printf("    -g <mean>            Generate prices from the Gaussian "
             "Distribution with sigma 1.\n");
      printf("    -e <mean>          Generate prices from the Exponential "
             "Distribution.\n");
      printf("    -a <phi>             Generate prices from an Autoregressive "
             "Model of order 1.\n");
      return 0;
    case 'u':
      uniformFlag = 1;
      gaussianFlag = 0;
      exponentialFlag = 0;
      autoregressiveFlag = 0;

      uniformHigh = atof(optarg);
      break;
    case 'g':
      uniformFlag = 0;
      gaussianFlag = 1;
      exponentialFlag = 0;
      autoregressiveFlag = 0;

      gaussianMean = atof(optarg);
      break;
    case 'e':
      uniformFlag = 0;
      gaussianFlag = 0;
      exponentialFlag = 1;
      autoregressiveFlag = 0;

      exponentialMean = atof(optarg);
      break;
    case 'a':
      uniformFlag = 0;
      gaussianFlag = 0;
      exponentialFlag = 0;
      autoregressiveFlag = 1;

      autoregressivePhi = atof(optarg);
      break;
    case 't':
      roundsFlag = 1;

      totalRounds = atoll(optarg);
      break;
    case 'n':
      pricesFlag = 1;

      pricesPerRound = atoll(optarg);
      break;
    case '?':
      if (optopt == 'u' || optopt == 'g' || optopt == 'e' || optopt == 'a' ||
          optopt == 't' || optopt == 'n')
        fprintf(stderr, "Option -%c requires an argument.\n", optopt);
      break;
    default:
      abort();
    }
  }

  // Initialise gnu_rng variables for generating random values
  const gsl_rng_type *T;
  gsl_rng *r;
  gsl_rng_env_setup();
  T = gsl_rng_default;
  r = gsl_rng_alloc(T);
  gsl_rng_set(r, time(NULL));

  char filename[256];
  snprintf(filename, sizeof(filename), "prophetData/dataT%luN%lu.dat",
           totalRounds, pricesPerRound);

  FILE *file = fopen(filename, "wb");
  if (!file) {
    printf("Error opening file");
    return 1;
  }

  fwrite(&totalRounds, sizeof(totalRounds), 1, file);
  fwrite(&pricesPerRound, sizeof(pricesPerRound), 1, file);

  if (uniformFlag) {
    for (int i = 0; i < totalRounds * pricesPerRound; i++) {
      double u = gsl_rng_uniform(r) * uniformHigh;
      /* printf("%lf\n", u); */
      fwrite(&u, sizeof(u), 1, file);
    }
  }

  if (gaussianFlag) {
    for (int i = 0; i < totalRounds * pricesPerRound; i++) {
      double g = gsl_ran_gaussian(r, 1) + gaussianMean;
      /* printf("%lf\n", g); */
      fwrite(&g, sizeof(g), 1, file);
    }
  }

  if (exponentialFlag) {
    for (int i = 0; i < totalRounds * pricesPerRound; i++) {
      double e = gsl_ran_exponential(r, exponentialMean);
      /* printf("%lf\n", e); */
      fwrite(&e, sizeof(e), 1, file);
    }
  }

  if (autoregressiveFlag) {
    double prev = 0;
    for (int i = 0; i < totalRounds * pricesPerRound; i++) {
      // TODO: evaluate if this is sufficient
      double noise = gsl_ran_gaussian(r, 1);
      double a = prev * autoregressivePhi + noise;
      prev = a;
      printf("%lf\n", a);
      fwrite(&a, sizeof(a), 1, file);
    }
  }

  /*  TEST: For quick testing if the distributions are correct:
   *
   *  Uncomment the printf lines above.
   *  Run the command:
   *
   *  make && ./priceGenerator [flags] | gnuplot -p -e "plot '<cat' with lines"
   *
   */

  fclose(file);
  gsl_rng_free(r);
}
