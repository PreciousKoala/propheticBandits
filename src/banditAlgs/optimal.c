#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <banditAlgs.h>
#include <util.h>

void findOpt(double *data, double *totalOpt, uint64_t totalRounds, uint64_t pricesPerRound) {
    uint8_t localMin = data[0] <= data[1];
    uint8_t localMax;

    totalOpt[0] = -data[0] * localMin;

    for (uint64_t i = 1; i < totalRounds * pricesPerRound - 1; i++) {
        localMax = (data[i] >= data[i - 1]);
        localMin = (data[i] <= data[i + 1]);
        totalOpt[i / pricesPerRound] += data[i] * localMax - data[i] * localMin;
    }

    localMax = data[totalRounds * pricesPerRound - 1] >= data[totalRounds * pricesPerRound - 2];
    totalOpt[totalRounds - 1] += data[totalRounds * pricesPerRound - 1] * localMax;

    for (uint64_t t = 1; t < totalRounds; t++) {
        totalOpt[t] += totalOpt[t - 1];
    }

    // printf("\n");
    // printf("---------------------------------LOCAL-EXTREMA-----------------------"
    //        "-----------\n");
    // printf("Total OPT: %lf\n", totalOpt[totalRounds - 1]);
    // printf("Average OPT: %lf\n", totalOpt[totalRounds - 1] / (double) totalRounds);
    // printf("---------------------------------------------------------------------"
    //        "-----------\n\n");
}

void findBestHand(double *reward, double *totalOpt, double *bestThreshold, uint64_t totalRounds,
                  uint32_t totalThresholds) {
    Threshold *thres = malloc(totalThresholds * sizeof(Threshold));
    initThreshold(thres, totalThresholds);

    double *avgThreshold = malloc(totalRounds * sizeof(double));

    for (uint64_t t = 0; t < totalRounds; t++) {
        double max = -INFINITY;
        uint32_t chosenTh = 0;

        for (uint32_t th = 0; th < totalThresholds; th++) {
            if (reward[totalThresholds * t + th] > max) {
                max = reward[totalThresholds * t + th];
                chosenTh = th;
            }
        }
        bestThreshold[t] = thres[chosenTh].threshold;
        if (t > 0)
            bestThreshold[t] += bestThreshold[t - 1];

        runRound(thres, chosenTh, totalThresholds, reward, avgThreshold, totalOpt, t);
    }

    for (uint64_t t = 0; t < totalRounds; t++) {
        bestThreshold[t] = bestThreshold[t] / (t + 1);
    }
    free(avgThreshold);

    printf("\n");
    printf("-----------------------------------BEST-HAND-------------------------"
           "-----------\n");
    printf("Threshold\tTotal Reward\tTimes Chosen\tAverage Reward\n");
    for (int32_t th = 0; th < totalThresholds; th++) {
        printf("%-7.2lf\t\t%-10.2lf\t%-12lu\t%-.6lf\n", thres[th].threshold, thres[th].rewardSum, thres[th].timesChosen,
               thres[th].avgReward);
    }

    printf("---------------------------------------------------------------------"
           "-----------\n");
    printf("Total OPT: %lf\n", totalOpt[totalRounds - 1]);
    printf("Average OPT: %lf\n", totalOpt[totalRounds - 1] / totalRounds);
    printf("---------------------------------------------------------------------"
           "-----------\n\n");

    free(thres);
}
