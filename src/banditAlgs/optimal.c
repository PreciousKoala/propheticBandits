#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <banditAlgs.h>
#include <util.h>

void findOpt(double *data, double *totalOpt, double *avgTrades, double *avgTradeGain, uint64_t totalRounds,
             uint64_t pricesPerRound) {
    uint8_t rightAsc = data[0] <= data[1];
    uint8_t leftAsc;

    for (uint64_t t = 1; t < totalRounds; t++) {
        avgTrades[t] = 0;
    }

    totalOpt[0] = -data[0] * rightAsc;

    for (uint64_t i = 1; i < totalRounds * pricesPerRound - 1; i++) {
        leftAsc = (data[i] >= data[i - 1]);
        rightAsc = (data[i] < data[i + 1]);
        totalOpt[i / pricesPerRound] += data[i] * leftAsc - data[i] * rightAsc;

        if (leftAsc && !rightAsc) {
            avgTrades[i / pricesPerRound]++;
        }
    }

    leftAsc = data[totalRounds * pricesPerRound - 1] >= data[totalRounds * pricesPerRound - 2];
    totalOpt[totalRounds - 1] += data[totalRounds * pricesPerRound - 1] * leftAsc;

    for (uint64_t t = 1; t < totalRounds; t++) {
        totalOpt[t] += totalOpt[t - 1];
        avgTrades[t] += avgTrades[t - 1];
    }

    for (uint64_t t = 1; t < totalRounds; t++) {
        avgTrades[t] = avgTrades[t] / (double) (t + 1);
    }

    // printf("\n");
    // printf("---------------------------------LOCAL-EXTREMA-----------------------"
    //        "-----------\n");
    // printf("Total OPT: %lf\n", totalOpt[totalRounds - 1]);
    // printf("Average OPT: %lf\n", totalOpt[totalRounds - 1] / (double) totalRounds);
    // printf("---------------------------------------------------------------------"
    //        "-----------\n\n");
}
