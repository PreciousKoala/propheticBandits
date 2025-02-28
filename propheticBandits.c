#include <getopt.h>
#include <gsl/gsl_rng.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void normalizePrices(double min, double max, double *data, uint64_t totalRounds,
                     uint64_t pricesPerRound) {
  for (uint64_t i = 0; i < totalRounds * pricesPerRound; i++) {
    data[i] = (data[i] - min) / (max - min);

    /*
     * TEST:
     * uncomment this line to see if the data is normalized.
     * printf("%lf\n", data[i]);
     */
  }
}

int main(int argc, char **argv) {
  uint32_t maxItems = 1;
  uint8_t keepItemsFlag = 0;
  uint8_t epsilonGreedyFlag = 0;
  uint8_t usb1Flag = 0;
  uint8_t exp3Flag = 0;

  int opt;

  opterr = 0;

  while ((opt = getopt(argc, argv, ":hm:keux")) != -1) {
    switch (opt) {
    case 'h':
      printf("Usage:\n");
      printf("    propheticBandits [-keux] [-m <integer>] <file>\n");
      printf("    propheticBandits -h      # Display this help screen.\n\n");
      printf("Options:\n");
      printf("    -m <integer>    Set the number of maximum held items.\n");
      printf("    -k              Set the program to be able to keep items at "
             "the end of a round.\n");
      printf("    -e              Run the Epsilon Greedy algorithm.\n");
      printf("    -u              Run the UCB1 algorithm.\n");
      printf("    -x              Run the EXP3 algorithm.\n");
      return 0;
    case 'm':
      if (optind < argc)
        maxItems = atoi(optarg);
      break;
    case 'k':
      keepItemsFlag = 1;
      break;
    case 'e':
      epsilonGreedyFlag = 1;
      break;
    case 'u':
      usb1Flag = 1;
      break;
    case 'x':
      exp3Flag = 1;
      break;
    case '?':
      if (optopt == 'm')
        fprintf(stderr, "Option -%c requires an argument.\n", optopt);
      break;
    default:
      abort();
    }
  }

  /* INFO: Data array holds all of the prices found in the data file.
   * It's also a 1D array for values that are better suited in a 2D array, but
   * because of the sheer size of our data, it's better to save them like this.
   * Access the n'th price of the t'th round with data[pricesPerRound * t + n].
   *
   * WARN: Don't run this program for files close to your RAM
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

  normalizePrices(min, max, data, totalRounds, pricesPerRound);

  /*
   * TODO: optimal.c
   * findOpt(double *data, uint8_t keepItemsFlag, uint8_t maxItems, uint64_t
   * totalRounds, uint64_t pricesPerRound)
   */

  /*
   * TODO: epsilon greedy
   * epsilonGreedy(double *data, totalRounds, uint8_t keepItemsFlag, uint8_t
   * maxItems, uint64_t totalRounds, uint64_t pricesPerRound)
   */

  // TODO: plot regret

  /*
   * TODO: ucb1
   * ucb1(double *data, totalRounds, uint8_t keepItemsFlag, uint8_t
   * maxItems, uint64_t totalRounds, uint64_t pricesPerRound)
   */

  /*
   * TODO: exp3
   * exp3(double *data, totalRounds, uint8_t keepItemsFlag, uint8_t
   * maxItems, uint64_t totalRounds, uint64_t pricesPerRound)
   * exp3 requires prices normalized according to min and max round opt
   * maybe
   */

  free(data);
  return 0;
}
