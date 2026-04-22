#include <gsl/gsl_statistics_double.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void median(double *data, double *totalGain, double *avgThreshold, double *avgTrades, double *totalOpt,
            uint64_t totalRounds, uint64_t pricesPerRound) {
    /**
     * INFO: The median algorithm in short:
     *
     * --------------------------------------------------
     * calculate the median of all the prices
     * for each round t
     *   buy and sell using the median as the threshold
     * --------------------------------------------------
     */

    uint64_t arrSize = totalRounds * pricesPerRound;
    double *dataCopy = malloc(arrSize * sizeof(double));
    // GSL rearranges the array, so we need a copy
    memcpy(dataCopy, data, arrSize * sizeof(double));
    // GSL my beloved <3
    const double median = gsl_stats_median(dataCopy, 1, arrSize);
    free(dataCopy);

    for (int t = 0; t < totalRounds; t++) {
        totalGain[t] = 0;
        avgThreshold[t] = median;
        avgTrades[t] = 0;
    }

    uint8_t heldItems = 0;
    double heldItemValue = 0;

    for (uint64_t i = 0; i < arrSize - 1; i++) {
        if (data[i] >= median && heldItems == 1) {
            totalGain[i / pricesPerRound] += data[i] - heldItemValue;
            heldItems = 0;
            avgTrades[i / pricesPerRound]++;
        } else if (data[i] < median && heldItems == 0) {
            heldItemValue = data[i];
            heldItems = 1;
        }
    }

    if (heldItems == 1) {
        totalGain[totalRounds - 1] += data[arrSize - 1];
    }

    for (uint64_t t = 1; t < totalRounds; t++) {
        totalGain[t] += totalGain[t - 1];
        avgTrades[t] += avgTrades[t - 1];
    }

    for (uint64_t t = 0; t < totalRounds; t++) {
        avgTrades[t] = avgTrades[t] / (t + 1);
    }

    printf("\n");
    printf("-------------------------------------Median--------------------------"
           "-----------\n");
    printf("Median: %lf\n", median);
    printf("Total Gain: %lf\n", totalGain[totalRounds - 1]);
    printf("Total OPT: %lf\n", totalOpt[totalRounds - 1]);
    printf("Total Regret: %lf\n", totalOpt[totalRounds - 1] - totalGain[totalRounds - 1]);
    printf("Average Gain: %lf\n", totalGain[totalRounds - 1] / (double) totalRounds);
    printf("Average OPT: %lf\n", totalOpt[totalRounds - 1] / (double) totalRounds);
    printf("Average Regret: %lf\n", (totalOpt[totalRounds - 1] - totalGain[totalRounds - 1]) / (double) totalRounds);
    printf("Competitive Ratio: %lf\n", totalGain[totalRounds - 1] / totalOpt[totalRounds - 1]);
    printf("---------------------------------------------------------------------"
           "-----------\n\n");
}
