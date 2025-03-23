#include <getopt.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

void printHelp() {
  printf(
      "Usage:\n"
      "    # Only one option can be used at a time.\n"
      "    # If more than one option is chosen, the program chooses the "
      "last one to be declared.\n"
      "    # If no option is chosen, the program chooses the uniform "
      "distribution.\n\n"
      "    priceGenerator [options] [-t <number of rounds>] [-n <prices "
      "per round>]\n"
      "    priceGenerator -h      # Display this help screen.\n\n"
      "Options:\n"
      "    -r                   Randomize distibution parameters for "
      "options u, g, e, and b.\n"
      "    -u                   Generate prices from the Uniform "
      "Distribution from 0 to 1.\n"
      "    -g                   Generate prices from the Gaussian "
      "Distribution with mean 0 and sigma 1.\n"
      "    -e                   Generate prices from the Exponential "
      "Distribution with mean 1.\n"
      "    -b                   Generate prices from the Bernoulli "
      "Distribution with probability of 0.5.\n"
      "    -a <phi>             Generate prices from an Autoregressive "
      "Model of order 1:\n"
      "                             f(n) = phi * f(n - 1) + error\n"
      "    -m <theta>           Generate prices from a Moving Average Model "
      "of order 1:\n"
      "                             f(n) =  error(t) + theta * error(t - 1)\n"
      "    -s <frequency>       Generate prices from a wiggly sine wave:\n"
      "                             f(x) = 5 * sin(x * frequency * 2 * pi / "
      "(T * N)) + error\n"
      "    -c <frequency>       Generate prices from a wiggly sine wave "
      "with a steeper curve:\n"
      "                             f(x) = 5 * sin(x * frequency * 2 * pi / "
      "(T * N))^(1 / 3) + error\n");
}

int main(int argc, char *argv[]) {
  if (argc == 1) {
    printHelp();
    return 0;
  }

  char distLetter = 'u';
  uint8_t randomizeFlag = 0;

  double autoregressivePhi = 1;
  double movingAvgTheta = 1;
  double sineFrequency = 1;

  uint64_t totalRounds = 1000;
  uint64_t pricesPerRound = 10;

  int opt;

  opterr = 0;

  // TODO: import stock market prices
  //
  // TODO: make bash script to automatically create all possible data combos

  while ((opt = getopt(argc, argv, "hrugeba:m:s:c:t:n:")) != -1) {
    switch (opt) {
    case 'h':
      printHelp();
      return 0;
    case 'r':
      randomizeFlag = 1;
      break;
    case 'u':
      distLetter = 'u';
      break;
    case 'g':
      distLetter = 'g';
      break;
    case 'e':
      distLetter = 'e';
      break;
    case 'b':
      distLetter = 'b';
      break;
    case 'a':
      distLetter = 'a';
      autoregressivePhi = atof(optarg);
      break;
    case 'm':
      distLetter = 'm';
      movingAvgTheta = atof(optarg);
      break;
    case 's':
      distLetter = 's';
      sineFrequency = atof(optarg);
      break;
    case 'c':
      distLetter = 'c';
      sineFrequency = atof(optarg);
      break;
    case 't':
      totalRounds = atoll(optarg);
      break;
    case 'n':
      pricesPerRound = atoll(optarg);
      break;
    case '?':
      if (optopt == 'a' || optopt == 's' || optopt == 'c' || optopt == 't' ||
          optopt == 'n')
        fprintf(stderr, "Option -%c requires an argument.\n", optopt);
      break;
    default:
      abort();
    }
  }

  if (distLetter == 'a' || distLetter == 'm' || distLetter == 's' ||
      distLetter == 'c') {
    randomizeFlag = 0;
  }

  // Initialise gnu_rng variables for generating random values
  const gsl_rng_type *T;
  gsl_rng *r;
  gsl_rng_env_setup();
  T = gsl_rng_default;
  r = gsl_rng_alloc(T);
  gsl_rng_set(r, time(NULL));

  struct stat st = {0};
  if (stat("prophetData", &st) == -1) {
    mkdir("prophetData", 0700);
  }

  char filename[256];
  if (randomizeFlag) {
    snprintf(filename, sizeof(filename), "prophetData/%crdataT%luN%lu.dat",
             distLetter, totalRounds, pricesPerRound);
  } else if (distLetter == 'a') {
    snprintf(filename, sizeof(filename), "prophetData/%cdataP%gT%luN%lu.dat",
             distLetter, autoregressivePhi, totalRounds, pricesPerRound);
  } else if (distLetter == 's' || distLetter == 'c') {
    snprintf(filename, sizeof(filename), "prophetData/%cdataP%gT%luN%lu.dat",
             distLetter, sineFrequency, totalRounds, pricesPerRound);
  } else {
    snprintf(filename, sizeof(filename), "prophetData/%cdataT%luN%lu.dat",
             distLetter, totalRounds, pricesPerRound);
  }

  FILE *file = fopen(filename, "wb");
  if (!file) {
    printf("Error opening file");
    return 1;
  }

  fwrite(&totalRounds, sizeof(totalRounds), 1, file);
  fwrite(&pricesPerRound, sizeof(pricesPerRound), 1, file);

  if (distLetter == 'u') {
    double high = 1;
    double low = 0;
    for (uint64_t t = 0; t < totalRounds; t++) {
      for (uint64_t n = 0; n < pricesPerRound; n++) {
        double u = gsl_rng_uniform(r) * (high - low) + low;
        // printf("%lf\n", u);
        fwrite(&u, sizeof(u), 1, file);
      }
      if (randomizeFlag) {
        low += gsl_ran_gaussian(r, 1);
        high += gsl_ran_gaussian(r, 1);
        if (high < low) {
          // if low is larger than high then switch the their values
          double temp = low;
          low = high;
          high = temp;
        }
      }
    }
  }

  if (distLetter == 'g') {
    double mean = 0;
    double sigma = 1;
    for (uint64_t t = 0; t < totalRounds; t++) {
      for (uint64_t n = 0; n < pricesPerRound; n++) {
        double g = gsl_ran_gaussian(r, sigma) + mean;
        // printf("%lf\n", g);
        fwrite(&g, sizeof(g), 1, file);
      }
      if (randomizeFlag) {
        mean += gsl_ran_gaussian(r, 1);
        // make sure sigma is positive
        sigma = fabs(sigma + gsl_ran_gaussian(r, 1));
      }
    }
  }

  if (distLetter == 'e') {
    double mean = 1;
    for (uint64_t t = 0; t < totalRounds; t++) {
      for (uint64_t n = 0; n < pricesPerRound; n++) {
        double e = gsl_ran_exponential(r, mean);
        // printf("%lf\n", e);
        fwrite(&e, sizeof(e), 1, file);
      }
      if (randomizeFlag) {
        mean += gsl_ran_gaussian(r, 1);
      }
    }
  }

  if (distLetter == 'b') {
    double prob = 0.5;
    for (uint64_t t = 0; t < totalRounds; t++) {
      for (uint64_t n = 0; n < pricesPerRound; n++) {
        double b = (double)gsl_ran_bernoulli(r, prob);
        // printf("%lf\n", b);
        fwrite(&b, sizeof(b), 1, file);
      }
      if (randomizeFlag) {
        prob = gsl_rng_uniform(r);
      }
    }
  }

  if (distLetter == 'a') {
    double prev = gsl_ran_gaussian(r, 1);
    for (uint64_t i = 0; i < totalRounds * pricesPerRound; i++) {
      double noise = gsl_ran_gaussian(r, 1);
      double a = prev * autoregressivePhi + noise;
      prev = a;
      // printf("%lf\n", a);
      fwrite(&a, sizeof(a), 1, file);
    }
  }

  if (distLetter == 'm') {
    double prevNoise = 0;
    for (uint64_t i = 0; i < totalRounds * pricesPerRound; i++) {
      double noise = gsl_ran_gaussian(r, 1);
      double m = noise + movingAvgTheta * prevNoise;
      prevNoise = noise;
      // printf("%lf\n", m);
      fwrite(&m, sizeof(m), 1, file);
    }
  }

  if (distLetter == 's') {
    for (uint64_t i = 0; i < totalRounds * pricesPerRound; i++) {
      double noise = gsl_ran_gaussian(r, 1);
      // The sinewave will complete <frequency> cycles throughout all the rounds
      double angularFreq =
          2 * M_PI * sineFrequency / (totalRounds * pricesPerRound);
      double s = 5 * sin(i * angularFreq) + noise;
      // printf("%lf\n", s);
      fwrite(&s, sizeof(s), 1, file);
    }
  }

  if (distLetter == 'c') {
    for (uint64_t i = 0; i < totalRounds * pricesPerRound; i++) {
      double noise = gsl_ran_gaussian(r, 1);
      // The sinewave will complete <frequency> cycles throughout all the rounds
      double angularFreq =
          2 * M_PI * sineFrequency / (totalRounds * pricesPerRound);
      double c = 5 * cbrt(sin(i * angularFreq)) + noise;
      // printf("%lf\n", c);
      fwrite(&c, sizeof(c), 1, file);
    }
  }

  /*  INFO: For quick testing if the distributions are correct:
   *
   *  Uncomment the printf lines above.
   *  Run the command:
   *
   * ./priceGenerator [flags] | gnuplot -p -e "plot '<cat' frequency with lines"
   *
   */

  fclose(file);
  gsl_rng_free(r);
}
