#include <getopt.h>
#include <gsl/gsl_rng.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "epsilonGreedy.c"
#include "optimal.c"
#include "ucb1.c"

void normalizePrices(double min, double max, double *data, uint64_t totalRounds,
                     uint64_t pricesPerRound) {
  for (uint64_t t = 0; t < totalRounds * pricesPerRound; t++) {
    data[t] = (data[t] - min) / (max - min);
  }
}

void printHelp() {
  printf("Usage:\n"
         "    propheticBandits [-eux] [-m <integer>] [-t <integer>] "
         "<file>\n"
         "    propheticBandits -h      # Display this help screen.\n\n"
         "Options:\n"
         "    -t <integer>    Set the number of thresholds (default = 10).\n"
         "    -m <integer>    Set the number of maximum held items "
         "(default = 1).\n"
         /* "    -k      Set the program to be able to keep items at " */
         /*        "the end of a round.\n" */
         "    -e              Run the Epsilon Greedy algorithm.\n"
         "    -u              Run the UCB1 algorithm.\n"
         "    -x              Run the EXP3 algorithm.\n");
}

int main(int argc, char **argv) {
  if (argc == 1) {
    printHelp();
    return 0;
  }

  uint32_t totalThresholds = 10;
  uint32_t maxItems = 1;
  // uint8_t keepItemsFlag = 0;
  uint8_t epsilonGreedyFlag = 0;
  uint8_t ucb1Flag = 0;
  uint8_t exp3Flag = 0;

  int opt;

  opterr = 0;

  while ((opt = getopt(argc, argv, ":hm:euxt:")) != -1) {
    switch (opt) {
    case 'h':
      printHelp();
      return 0;
    case 't':
      totalThresholds = atoi(optarg);
      break;
    case 'm':
      maxItems = atoi(optarg);
      break;
    /* case 'k': */
    /*   keepItemsFlag = 1; */
    /*   break; */
    case 'e':
      epsilonGreedyFlag = 1;
      break;
    case 'u':
      ucb1Flag = 1;
      break;
    case 'x':
      exp3Flag = 1;
      break;
    case '?':
      if (optopt == 'm' || optopt == 't')
        fprintf(stderr, "Option -%c requires an argument.\n", optopt);
      break;
    default:
      abort();
    }
  }

  /* INFO: Data array holds all of the prices found in the data file.
   * It's also a 1D array for values that are better suited in a 2D array, but
   * because of the sheer size of our data, it's better to save them like
   * this. Access the n'th price of the t'th round with data[pricesPerRound *
   * t + n].
   *
   * WARN: Don't run this program for very big files (>50m)
   */
  double *data;
  uint64_t totalRounds, pricesPerRound;
  // opens binary data file
  if (optind < argc) {
    // small hack to get first non-option argument because getopt is a pain
    char *filename = argv[optind];
    FILE *file = fopen(filename, "rb");
    if (!file) {
      printf("Error opening file\n");
      return 1;
    }

    printf("Importing file...\n");

    // first 2 values are 64bit integers
    fread(&totalRounds, sizeof(uint64_t), 1, file);
    fread(&pricesPerRound, sizeof(uint64_t), 1, file);

    // allocate memory and get the actual data
    data = malloc(totalRounds * pricesPerRound * sizeof(double));
    fread(data, sizeof(double), totalRounds * pricesPerRound, file);

    fclose(file);
  } else {
    printf("Error: No filename provided\n");
    return 1;
  }

  double min = INFINITY;
  double max = -INFINITY;

  for (uint64_t i = 0; i < totalRounds * pricesPerRound; i++) {
    if (data[i] < min)
      min = data[i];
    if (data[i] > max)
      max = data[i];
  }

  // Normalize prices to [0, 1]
  printf("Normalizing prices to [0,1]...\n");
  normalizePrices(min, max, data, totalRounds, pricesPerRound);

  double *optAlg = malloc(totalRounds * sizeof(double));
  double *totalOpt = malloc(totalRounds * sizeof(double));

  printf("Calculating optimal result...\n");
  findOpt(data, optAlg, maxItems, totalRounds, pricesPerRound);
  findTotalOpt(optAlg, totalOpt, totalRounds);

  if (epsilonGreedyFlag) {
    printf("Calculating Epsilon-Greedy...\n");
    epsilonGreedy(data, totalThresholds, maxItems, totalRounds, pricesPerRound);
    // TODO: plot regret
  }

  if (ucb1Flag) {
    printf("Calculating UCB1...\n");
    ucb1(data, totalThresholds, maxItems, totalRounds, pricesPerRound);
  }

  if (exp3Flag) {
    min = INFINITY;
    max = -INFINITY;

    for (uint64_t i = 0; i < totalRounds * pricesPerRound; i++) {
      if (optAlg[i] < min)
        min = data[i];
      if (optAlg[i] > max)
        max = data[i];
    }

    normalizePrices(min, max, data, totalRounds, pricesPerRound);
    findOpt(data, optAlg, maxItems, totalRounds, pricesPerRound);
    findTotalOpt(optAlg, totalOpt, totalRounds);

    /*
     * TODO: exp3
     * exp3(double *data, totalRounds, uint8_t keepItemsFlag, uint8_t
     * maxItems, uint64_t totalRounds, uint64_t pricesPerRound)
     * exp3 requires prices normalized according to min and max round opt
     * maybe normalise again?
     */
  }

  free(data);
  free(optAlg);
  free(totalOpt);
  return 0;
}
