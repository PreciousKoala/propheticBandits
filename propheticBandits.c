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
#include "ucb2.c"

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

// FIXME: make this faster
void plotRegret(uint64_t totalRounds, double *opt, double *eGreedyGain,
                double *ucb1Gain, double *ucb2Gain, double *exp3Gain,
                uint8_t eGreedyFlag, uint8_t ucb1Flag, uint8_t ucb2Flag,
                uint8_t exp3Flag) {
  FILE *gnuplot = popen("gnuplot -persistent", "w");
  if (!gnuplot) {
    exit(EXIT_FAILURE);
  }
  fprintf(gnuplot, "set title 'Regret Plot'\n");
  fprintf(gnuplot, "set xlabel 'Rounds'\n");
  fprintf(gnuplot, "set ylabel 'Regret'\n");
  fprintf(gnuplot, "set grid\n");
  fprintf(gnuplot, "set key left top\n");

  fprintf(gnuplot, "plot ");

  if (eGreedyFlag) {
    fprintf(gnuplot, "'-' using 1:2 with lines lt rgb 'red' lw 2 title "
                     "'eGreedy Regret'");
  }

  if (ucb1Flag) {
    if (eGreedyFlag) {
      fprintf(gnuplot, ", ");
    }
    fprintf(gnuplot,
            "'-' using 1:2 with lines lt rgb 'blue' lw 2 title 'UCB1 Regret'");
  }

  if (ucb2Flag) {
    if (eGreedyFlag || ucb1Flag) {
      fprintf(gnuplot, ", ");
    }
    fprintf(
        gnuplot,
        "'-' using 1:2 with lines lt rgb 'purple' lw 2 title 'UCB2 Regret'");
  }

  if (exp3Flag) {
    if (eGreedyFlag || ucb1Flag || ucb2Flag) {
      fprintf(gnuplot, ", ");
    }
    fprintf(gnuplot,
            "'-' using 1:2 with lines lt rgb 'green' lw 2 title 'EXP3 Regret'");
  }

  fprintf(gnuplot, "\n");

  if (eGreedyFlag) {
    for (int t = 0; t < totalRounds; t++) {
      fprintf(gnuplot, "%d %lf\n", t, opt[t] - eGreedyGain[t]);
    }
    fprintf(gnuplot, "e\n");
  }

  if (ucb1Flag) {
    for (int t = 0; t < totalRounds; t++) {
      fprintf(gnuplot, "%d %lf\n", t, opt[t] - ucb1Gain[t]);
    }
    fprintf(gnuplot, "e\n");
  }

  if (ucb2Flag) {
    for (int t = 0; t < totalRounds; t++) {
      fprintf(gnuplot, "%d %lf\n", t, opt[t] - ucb2Gain[t]);
    }
    fprintf(gnuplot, "e\n");
  }

  if (exp3Flag) {
    for (int t = 0; t < totalRounds; t++) {
      fprintf(gnuplot, "%d %lf\n", t, opt[t] - exp3Gain[t]);
    }
    fprintf(gnuplot, "e\n");
  }
  fflush(gnuplot);
  pclose(gnuplot);
}

void printHelp() {
  printf("Usage:\n"
         "    propheticBandits [-euUx] [-m <integer>] [-t <integer>] "
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
  uint8_t eGreedyFlag = 0;
  uint8_t ucb1Flag = 0;
  uint8_t ucb2Flag = 0;
  uint8_t exp3Flag = 0;

  int opt;

  opterr = 0;

  while ((opt = getopt(argc, argv, ":hm:euUxt:")) != -1) {
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

    /* printf("Importing file...\n"); */

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
  /* printf("Normalizing prices to [0,1]...\n"); */
  normalizePrices(min, max, data, totalRounds, pricesPerRound);

  /* printf("Calculating rewards...\n"); */
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

  double *eGreedyGain = malloc(totalRounds * sizeof(double));
  if (eGreedyFlag) {
    printf("Calculating Epsilon-Greedy...\n");
    epsilonGreedy(reward, eGreedyGain, totalOpt, totalBestHand, totalThresholds,
                  maxItems, totalRounds, pricesPerRound);
  }

  double *ucb1Gain = malloc(totalRounds * sizeof(double));
  if (ucb1Flag) {
    printf("Calculating UCB1...\n");
    ucb1(reward, ucb1Gain, totalOpt, totalBestHand, totalThresholds, maxItems,
         totalRounds, pricesPerRound);
  }

  double *ucb2Gain = malloc(totalRounds * sizeof(double));
  if (ucb2Flag) {
    printf("Calculating UCB2...\n");
    ucb2(reward, ucb2Gain, totalOpt, totalBestHand, totalThresholds, maxItems,
         totalRounds, pricesPerRound);
  }

  double *exp3Gain = malloc(totalRounds * sizeof(double));
  if (exp3Flag) {
    printf("Calculating EXP3...\n");
    exp3(reward, exp3Gain, totalOpt, totalBestHand, totalThresholds, maxItems,
         totalRounds, pricesPerRound);
  }

  free(reward);
  free(optAlg);
  free(bestHand);

  printf("Plotting best hand regret...\n");
  plotRegret(totalRounds, totalBestHand, eGreedyGain, ucb1Gain, ucb2Gain,
             exp3Gain, eGreedyFlag, ucb1Flag, ucb2Flag, exp3Flag);

  printf("Plotting optimal regret...\n");
  plotRegret(totalRounds, totalOpt, eGreedyGain, ucb1Gain, ucb2Gain, exp3Gain,
             eGreedyFlag, ucb1Flag, ucb2Flag, exp3Flag);

  free(totalOpt);
  free(totalBestHand);
  free(eGreedyGain);
  free(ucb1Gain);
  free(ucb2Gain);
  free(exp3Gain);
  return 0;
}
