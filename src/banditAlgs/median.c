#include <gsl/gsl_statistics_double.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <banditAlgs.h>
#include <util.h>

void median(double *data, double *totalGain, double *avgThreshold, double *avgTrades, double *totalOpt, Bandit b) {
    /**
     * INFO: The median algorithm in short:
     *
     * --------------------------------------------------
     * calculate the median of all the prices
     * for each round t
     *   buy and sell using the median as the threshold
     * --------------------------------------------------
     */

    double *dataCopy = malloc(b.T * b.N * sizeof(double));
    // GSL rearranges the array, so we need a copy
    memcpy(dataCopy, data, b.T * b.N * sizeof(double));
    // GSL my beloved <3
    double median = gsl_stats_median(dataCopy, 1, b.T * b.N);
    free(dataCopy);

    for (int t = 0; t < b.T; t++) {
        totalGain[t] = 0;
        avgThreshold[t] = median;
        avgTrades[t] = 0;
    }

    uint8_t heldItems = 0;
    double heldItemValue = 0;

    for (uint64_t i = 0; i < b.T * b.N; i++) {
        uint8_t lastPrice = (b.keepItems && i == (b.T * b.N - 1)) || (!b.keepItems && ((i + 1) % b.N) == 0);
        if ((lastPrice || data[i] >= median) && heldItems == 1) {
            totalGain[i / b.N] += data[i] - heldItemValue;
            heldItems = 0;
            avgTrades[i / b.N]++;
        } else if (!lastPrice && data[i] < median && heldItems == 0) {
            heldItemValue = data[i];
            heldItems = 1;
        }
    }

    for (uint64_t t = 1; t < b.T; t++) {
        totalGain[t] += totalGain[t - 1];
        avgTrades[t] += avgTrades[t - 1];
    }

    for (uint64_t t = 0; t < b.T; t++) {
        avgTrades[t] = avgTrades[t] / ((double) t + 1);
    }

    printf("\n");
    printf("-------------------------------------Median--------------------------"
           "-----------\n");
    printf("Median: %lf\n", median);
    printf("Total Gain: %lf\n", totalGain[b.T - 1]);
    if (!b.medianOpt) {
        printf("Total OPT: %lf\n", totalOpt[b.T - 1]);
        printf("Total Regret: %lf\n", totalOpt[b.T - 1] - totalGain[b.T - 1]);
    }
    printf("Average Gain: %lf\n", totalGain[b.T - 1] / (double) b.T);
    if (!b.medianOpt) {
        printf("Average OPT: %lf\n", totalOpt[b.T - 1] / (double) b.T);
        printf("Average Regret: %lf\n", (totalOpt[b.T - 1] - totalGain[b.T - 1]) / (double) b.T);
        printf("Competitive Ratio: %lf\n", totalGain[b.T - 1] / totalOpt[b.T - 1]);
    }
    printf("---------------------------------------------------------------------"
           "-----------\n\n");
}
