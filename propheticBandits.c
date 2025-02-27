#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  uint32_t maxItems = 1;
  uint8_t keepItemsFlag = 0;
  uint8_t epsilonGreedyFlag = 0;
  uint8_t usb1Flag = 0;
  uint8_t exp3Flag = 0;

  int opt;

  opterr = 0;

  while ((opt = getopt(argc, argv, ":h:m:keux")) != -1) {
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

  free(data);
  return 0;
}
