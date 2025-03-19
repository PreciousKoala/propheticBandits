#include <getopt.h>
#include <gsl/gsl_rng.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <banditAlgs.h>
#include <util.h>

void printHelp() {
  printf("Usage:\n"
         "    propheticBandits [-geuUx] [-m <integer>] [-t <integer>] "
         "<file>\n"
         "    propheticBandits -h      # Display this help screen.\n\n"
         "Options:\n"
         "    -t <integer>    Set the number of thresholds (default = 10).\n"
         "    -m <integer>    Set the number of maximum held items "
         "(default = 1).\n"
         "    -a              Run all the available algorithms.\n"
         "    -g              Run the Greedy algorithm.\n"
         "    -e              Run the Epsilon Greedy algorithm.\n"
         "    -s              Run the Successive Elimination algorithm.\n"
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
  Flag flag = {0, 0, 0, 0, 0, 0};

  int opt;

  opterr = 0;

  while ((opt = getopt(argc, argv, ":hm:agesuUxt:")) != -1) {
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
    case 'a':
      flag = (Flag){1, 1, 1, 1, 1, 1};
    case 'g':
      flag.greedyFlag = 1;
      break;
    case 'e':
      flag.eGreedyFlag = 1;
      break;
    case 's':
      flag.succElimFlag = 1;
      break;
    case 'u':
      flag.ucb1Flag = 1;
      break;
    case 'U':
      flag.ucb2Flag = 1;
      break;
    case 'x':
      flag.exp3Flag = 1;
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
   * this. Access the n'th price of the t'th round with
   * data[pricesPerRound * t + n].
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

  double dataMin = INFINITY;
  double dataMax = -INFINITY;

  for (uint64_t i = 0; i < totalRounds * pricesPerRound; i++) {
    if (data[i] < dataMin)
      dataMin = data[i];
    if (data[i] > dataMax)
      dataMax = data[i];
  }

  // Normalize prices to [0, 1]
  printf("Normalizing prices to [0,1]...\n");
  normalizePrices(dataMin, dataMax, data, totalRounds, pricesPerRound);

  printf("Calculating rewards...\n");
  double *reward = malloc(totalRounds * totalThresholds * sizeof(double));
  calculateRewards(reward, data, totalRounds, pricesPerRound, totalThresholds,
                   maxItems);

  double rewardMin = INFINITY;
  double rewardMax = -INFINITY;

  for (uint64_t i = 0; i < totalRounds * totalThresholds; i++) {
    if (reward[i] < rewardMin)
      rewardMin = reward[i];
    if (reward[i] > rewardMax)
      rewardMax = reward[i];
  }

  // Normalize prices to [0, 1]
  printf("Normalizing rewards to [0,1]...\n");
  normalizePrices(rewardMin, rewardMax, reward, totalRounds, totalThresholds);

  printf("Calculating optimal result (best hand)...\n");
  double *totalOpt = malloc(totalRounds * sizeof(double));
  findBestHand(reward, totalOpt, totalRounds, totalThresholds);

  // printf("Calculating optimal result (local extrema)...\n");
  // double *totalExtremaOpt = malloc(totalRounds * sizeof(double));
  // findOpt(data, totalExtremaOpt, maxItems, totalRounds, pricesPerRound);

  free(data);

  double *greedyAvgRegret = malloc(totalRounds * sizeof(double));
  double *greedyBestHand = malloc(totalRounds * sizeof(double));
  if (flag.greedyFlag) {
    double *greedyGain = malloc(totalRounds * sizeof(double));

    printf("Calculating Greedy...\n");
    greedy(reward, greedyGain, totalOpt, totalThresholds, maxItems, totalRounds,
           pricesPerRound);

    getAvgRegret(totalRounds, greedyAvgRegret, totalOpt, greedyGain);
    getBestHandPerc(totalRounds, greedyBestHand, totalOpt, greedyGain);
    free(greedyGain);
  }

  double *eGreedyAvgRegret = malloc(totalRounds * sizeof(double));
  double *eGreedyBestHand = malloc(totalRounds * sizeof(double));
  if (flag.eGreedyFlag) {
    double *eGreedyGain = malloc(totalRounds * sizeof(double));

    printf("Calculating Epsilon-Greedy...\n");
    epsilonGreedy(reward, eGreedyGain, totalOpt, totalThresholds, maxItems,
                  totalRounds, pricesPerRound);

    getAvgRegret(totalRounds, eGreedyAvgRegret, totalOpt, eGreedyGain);
    getBestHandPerc(totalRounds, eGreedyBestHand, totalOpt, eGreedyGain);
    free(eGreedyGain);
  }

  double *succElimAvgRegret = malloc(totalRounds * sizeof(double));
  double *succElimBestHand = malloc(totalRounds * sizeof(double));
  if (flag.succElimFlag) {
    double *succElimGain = malloc(totalRounds * sizeof(double));

    printf("Calculating Successive Elimination...\n");
    succElim(reward, succElimGain, totalOpt, totalThresholds, maxItems,
             totalRounds, pricesPerRound);

    getAvgRegret(totalRounds, succElimAvgRegret, totalOpt, succElimGain);
    getBestHandPerc(totalRounds, succElimBestHand, totalOpt, succElimGain);
    free(succElimGain);
  }

  double *ucb1AvgRegret = malloc(totalRounds * sizeof(double));
  double *ucb1BestHand = malloc(totalRounds * sizeof(double));
  if (flag.ucb1Flag) {
    double *ucb1Gain = malloc(totalRounds * sizeof(double));

    printf("Calculating UCB1...\n");
    ucb1(reward, ucb1Gain, totalOpt, totalThresholds, maxItems, totalRounds,
         pricesPerRound);

    getAvgRegret(totalRounds, ucb1AvgRegret, totalOpt, ucb1Gain);
    getBestHandPerc(totalRounds, ucb1BestHand, totalOpt, ucb1Gain);
    free(ucb1Gain);
  }

  double *ucb2AvgRegret = malloc(totalRounds * sizeof(double));
  double *ucb2BestHand = malloc(totalRounds * sizeof(double));
  if (flag.ucb2Flag) {
    double *ucb2Gain = malloc(totalRounds * sizeof(double));

    printf("Calculating UCB2...\n");
    ucb2(reward, ucb2Gain, totalOpt, totalThresholds, maxItems, totalRounds,
         pricesPerRound);

    getAvgRegret(totalRounds, ucb2AvgRegret, totalOpt, ucb2Gain);
    getBestHandPerc(totalRounds, ucb2BestHand, totalOpt, ucb2Gain);
    free(ucb2Gain);
  }

  double *exp3AvgRegret = malloc(totalRounds * sizeof(double));
  double *exp3BestHand = malloc(totalRounds * sizeof(double));
  if (flag.exp3Flag) {
    double *exp3Gain = malloc(totalRounds * sizeof(double));

    printf("Calculating EXP3...\n");
    exp3(reward, exp3Gain, totalOpt, totalThresholds, maxItems, totalRounds,
         pricesPerRound);

    getAvgRegret(totalRounds, exp3AvgRegret, totalOpt, exp3Gain);
    getBestHandPerc(totalRounds, exp3BestHand, totalOpt, exp3Gain);
    free(exp3Gain);
  }

  free(reward);
  free(totalOpt);

  printf("Plotting best hand regret...\n");
  plotAlgorithms("Average Regret", totalRounds, totalOpt, greedyAvgRegret,
                 eGreedyAvgRegret, succElimAvgRegret, ucb1AvgRegret,
                 ucb2AvgRegret, exp3AvgRegret, flag);

  free(greedyAvgRegret);
  free(eGreedyAvgRegret);
  free(succElimAvgRegret);
  free(ucb1AvgRegret);
  free(ucb2AvgRegret);
  free(exp3AvgRegret);

  plotAlgorithms("Percentage of Best Hand Played", totalRounds, totalOpt,
                 greedyBestHand, eGreedyBestHand, succElimBestHand,
                 ucb1BestHand, ucb2BestHand, exp3BestHand, flag);

  free(greedyBestHand);
  free(eGreedyBestHand);
  free(succElimBestHand);
  free(ucb1BestHand);
  free(ucb2BestHand);
  free(exp3BestHand);

  // printf("Plotting optimal regret...\n");
  // plotAlgorithms(totalRounds, totalOpt, greedyGain, eGreedyGain,
  // succElimGain, ucb1Gain, ucb2Gain, exp3Gain, flag);

  // free(totalExtremaOpt);
  return 0;
}
