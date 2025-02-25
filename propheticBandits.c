#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  int maxItems = 1;
  int keepItemsFlag = 0;
  int epsilonGreedyFlag = 0;
  int usb1Flag = 0;
  int exp3Flag = 0;

  int opt;

  opterr = 0;

  while ((opt = getopt(argc, argv, ":hm:keux")) != -1) {
    switch (opt) {
    case 'h':
      printf("Usage:\n");
      printf("    propheticBandits [-keux] [-m <integer>] <file>\n");
      printf("    propheticBandits -h      #Display this help screen.\n\n");
      printf("Options:\n");
      printf("    -m <integer>    Set the number of maximum held items.\n");
      printf("    -k              Set the program to be able to keep items at "
             "the end of a round.\n");
      printf("    -e              Run the Epsilon Greedy algorithm.\n");
      printf("    -u              Run the UCB1 algorithm.\n");
      printf("    -x              Run the EXP3 algorithm.\n");
      return 0;
    case 'm':
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

  // Opens data file
  if (optind < argc) {
    // First non-option argument
    char *filename = argv[optind];
    FILE *file = fopen(filename, "r");
    if (!file) {
      printf("Error opening file\n");
      return 1;
    }
    printf("Processing file: %s\n", filename);
    fclose(file);
  } else {
    printf("Error: No filename provided\n");
    return 1;
  }

  return 0;
}
