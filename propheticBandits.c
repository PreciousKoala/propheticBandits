#include <getopt.h>
#include <gsl/gsl_rng.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "epsilonGreedy.c"
#include "exp3.c"
#include "optimal.c"
#include "ucb1.c"

void normalizePrices(double min, double max, double *data, uint64_t totalRounds,
                     uint64_t pricesPerRound) {
  for (uint64_t t = 0; t < totalRounds * pricesPerRound; t++) {
    data[t] = (data[t] - min) / (max - min);
  }
}

void calculateRewards(double *reward, double *data, uint64_t totalRounds,
                      uint64_t pricesPerRound, uint32_t totalThresholds,
                      uint32_t maxItems) {
  uint32_t heldItems = 0;
  for (uint32_t th = 0; th < totalThresholds; th++) {
    double threshold = (double)th / totalThresholds;

    for (uint64_t t = 0; t < totalRounds; t++) {
      double gain = 0;

      for (uint32_t n = 0; n < pricesPerRound; n++) {
        if ((pricesPerRound - n == heldItems ||
             data[pricesPerRound * t + n] >= threshold) &&
            heldItems > 0) {
          gain += data[pricesPerRound * t + n];
          heldItems--;
        } else if (pricesPerRound - n - 1 != heldItems &&
                   data[pricesPerRound * t + n] < threshold &&
                   heldItems < maxItems) {
          gain -= data[pricesPerRound * t + n];
          heldItems++;
        }
      }
      reward[totalThresholds * t + th] = gain;
    }
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

  printf("Calculating rewards...\n");
  double *reward = malloc(totalRounds * totalThresholds * sizeof(double));
  calculateRewards(reward, data, totalRounds, pricesPerRound, totalThresholds,
                   maxItems);

  printf("Calculating optimal result...\n");
  double *optAlg = malloc(totalRounds * sizeof(double));
  double *totalOpt = malloc(totalRounds * sizeof(double));
  findOpt(data, optAlg, totalOpt, maxItems, totalRounds, pricesPerRound);

  printf("Calculating best hand...\n");
  double *bestHand = malloc(totalRounds * sizeof(double));
  double *totalBestHand = malloc(totalRounds * sizeof(double));
  findBestHand(reward, bestHand, totalBestHand, totalRounds, totalThresholds);

  free(data);

  if (epsilonGreedyFlag) {
    printf("Calculating Epsilon-Greedy...\n");
    epsilonGreedy(reward, totalThresholds, maxItems, totalRounds,
                  pricesPerRound);
    // TODO: plot regret
  }

  if (ucb1Flag) {
    printf("Calculating UCB1...\n");
    ucb1(reward, totalThresholds, maxItems, totalRounds, pricesPerRound);
  }

  if (exp3Flag) {
    /* INFO: The exp3 algorithm requires the feedback to be in range [0,1].
     * However, our feedback (reward after each round) can be >1.
     *
     * A few solutions:
     *
     * 1) calculate each reward for each arm and normalize prices according to
     * the min and max reward of all the arms and rounds.
     *    pros:
     *      -accurate normalization (the entire range of 0,1 is reached)
     *    cons:
     *      -computational power needed is multiplied by the number of arms
     *      -cannot compare exp3 with other algorithms easily
     *
     * 2) divide all numbers in the data by the number of prices per round / 2,
     * since each round has N prices and each price is in [0,1], the absolute
     * best outcome is buying at 0 and selling at 1, therefore the feedback is
     * at most N/2.
     *    pros:
     *      -much faster than option 1
     *      -can easily revert back, and compare with eGreedy & ucb1
     *    cons:
     *      -less accurate
     *
     *
     * It turns out that option 1 is significantly better for exp3
     */

    printf("Calculating EXP3...\n");
    exp3(reward, totalThresholds, maxItems, totalRounds, pricesPerRound);
  }

  free(reward);
  free(optAlg);
  free(totalOpt);
  free(bestHand);
  free(totalBestHand);
  return 0;
}
