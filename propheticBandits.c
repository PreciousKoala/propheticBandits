#include <getopt.h>
#include <gsl/gsl_rng.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.c"

#include "epsilonGreedy.c"
#include "exp3.c"
#include "greedy.c"
#include "optimal.c"
#include "ucb1.c"
#include "ucb2.c"

void printHelp() {
  printf("Usage:\n"
         "    propheticBandits [-geuUx] [-m <integer>] [-t <integer>] "
         "<file>\n"
         "    propheticBandits -h      # Display this help screen.\n\n"
         "Options:\n"
         "    -t <integer>    Set the number of thresholds (default = 10).\n"
         "    -m <integer>    Set the number of maximum held items "
         "(default = 1).\n"
         /* "    -k      Set the program to be able to keep items at " */
         /*        "the end of a round.\n" */
         "    -g              Run the Greedy algorithm.\n"
         "    -e              Run the Epsilon Greedy algorithm.\n"
         "    -u              Run the UCB1 algorithm.\n"
         "    -U              Run the UCB2 algorithm.\n"
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
  uint8_t greedyFlag = 0;
  uint8_t eGreedyFlag = 0;
  uint8_t ucb1Flag = 0;
  uint8_t ucb2Flag = 0;
  uint8_t exp3Flag = 0;

  int opt;

  opterr = 0;

  while ((opt = getopt(argc, argv, ":hm:geuUxt:")) != -1) {
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
    case 'g':
      greedyFlag = 1;
      break;
    case 'e':
      eGreedyFlag = 1;
      break;
    case 'u':
      ucb1Flag = 1;
      break;
    case 'U':
      ucb2Flag = 1;
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
   * WARN: Don't run this program for very big files
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

  printf("Calculating rewards...\n");
  double *reward = malloc(totalRounds * totalThresholds * sizeof(double));
  calculateRewards(reward, data, totalRounds, pricesPerRound, totalThresholds,
                   maxItems);

  /* printf("Calculating optimal result...\n"); */
  /* double *optAlg = malloc(totalRounds * sizeof(double)); */
  /* double *totalOpt = malloc(totalRounds * sizeof(double)); */
  /* findOpt(data, optAlg, totalOpt, maxItems, totalRounds, pricesPerRound); */

  printf("Calculating best hand...\n");
  double *totalBestHand = malloc(totalRounds * sizeof(double));
  findBestHand(reward, totalBestHand, totalRounds, totalThresholds);

  free(data);

  double *greedyGain = malloc(totalRounds * sizeof(double));
  if (greedyFlag) {
    printf("Calculating Greedy...\n");
    greedy(reward, greedyGain, totalBestHand, totalThresholds, maxItems,
           totalRounds, pricesPerRound);
  }

  double *eGreedyGain = malloc(totalRounds * sizeof(double));
  if (eGreedyFlag) {
    printf("Calculating Epsilon-Greedy...\n");
    epsilonGreedy(reward, eGreedyGain, totalBestHand, totalThresholds, maxItems,
                  totalRounds, pricesPerRound);
  }

  double *ucb1Gain = malloc(totalRounds * sizeof(double));
  if (ucb1Flag) {
    printf("Calculating UCB1...\n");
    ucb1(reward, ucb1Gain, totalBestHand, totalThresholds, maxItems,
         totalRounds, pricesPerRound);
  }

  double *ucb2Gain = malloc(totalRounds * sizeof(double));
  if (ucb2Flag) {
    printf("Calculating UCB2...\n");
    ucb2(reward, ucb2Gain, totalBestHand, totalThresholds, maxItems,
         totalRounds, pricesPerRound);
  }

  double *exp3Gain = malloc(totalRounds * sizeof(double));
  if (exp3Flag) {
    printf("Calculating EXP3...\n");
    exp3(reward, exp3Gain, totalBestHand, totalThresholds, maxItems,
         totalRounds, pricesPerRound);
  }

  free(reward);
  /* free(optAlg); */

  printf("Plotting best hand regret...\n");
  plotRegret(totalRounds, totalBestHand, greedyGain, eGreedyGain, ucb1Gain,
             ucb2Gain, exp3Gain, greedyFlag, eGreedyFlag, ucb1Flag, ucb2Flag,
             exp3Flag);

  /* printf("Plotting optimal regret...\n"); */
  /* plotRegret(totalRounds, totalOpt, greedyGain, eGreedyGain, ucb1Gain,
   * ucb2Gain, */
  /*            exp3Gain, greedyFlag, eGreedyFlag, ucb1Flag, ucb2Flag,
   * exp3Flag); */

  free(totalBestHand);
  free(greedyGain);
  free(eGreedyGain);
  free(ucb1Gain);
  free(ucb2Gain);
  free(exp3Gain);
  return 0;
}
