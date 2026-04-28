#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <banditAlgs.h>
#include <util.h>

void greedy(double *data, double *totalGain, double *avgLowThreshold, double *avgHighThreshold, double *avgTrades, double *totalOpt, Bandit b) {
    /**
     * INFO: The greedy algorithm in short:
     *
     * --------------------------------------------------
     * try each arm once
     * for each round t
     *   pick the arm with the highest average reward
     * --------------------------------------------------
     *
     * in this program, arm = threshold
     *
     */

    Threshold *thres = malloc(b.K * sizeof(Threshold));
    initThreshold(thres, b);

    uint32_t chosenTh = 0;
    totalGain[0] = 0;
    uint8_t heldItems = 0;
    double heldItemValue = 0;

    for (uint32_t t = 0; t < b.K; t++) {
        runRound(thres, t, b, data, avgLowThreshold, avgHighThreshold, avgTrades, totalGain, t, &heldItems, &heldItemValue);
    }
    double max = -INFINITY;
    chosenTh = 0;

    for (uint32_t th = 0; th < b.K; th++) {
        if (thres[th].avgReward > max) {
            max = thres[th].avgReward;
            chosenTh = th;
        }
    }

    for (uint64_t t = b.K; t < b.T; t++) {
        runRound(thres, chosenTh, b, data, avgLowThreshold, avgHighThreshold, avgTrades, totalGain, t, &heldItems, &heldItemValue);
    }

    printf("\n");
    printf("-------------------------------------Greedy--------------------------"
           "-----------\n");
    if (!b.dualThres) {
        printf("Threshold\tTotal Reward\tTimes Chosen\tAverage Reward\n");
        for (int32_t th = 0; th < b.K; th++) {
            printf("%-7.2lf\t\t%-10.2lf\t%-12lu\t%-.6lf\n", thres[th].low, thres[th].rewardSum, thres[th].timesChosen,
                   thres[th].avgReward);
        }
    } else {
        printf("Low Thres\tHigh Thres\tTotal Reward\tTimes Chosen\tAverage Reward\n");
        for (int32_t th = 0; th < b.K; th++) {
            printf("%-7.2lf\t\t%-7.2lf\t\t%-10.2lf\t%-12lu\t%-.6lf\n", thres[th].low, thres[th].high,
                   thres[th].rewardSum, thres[th].timesChosen, thres[th].avgReward);
        }
    }

    printf("---------------------------------------------------------------------"
           "-----------\n");
    printf("Total Gain: %lf\n", totalGain[b.T - 1]);
    printf("Total OPT: %lf\n", totalOpt[b.T - 1]);
    printf("Total Regret: %lf\n", totalOpt[b.T - 1] - totalGain[b.T - 1]);
    printf("Average Gain: %lf\n", totalGain[b.T - 1] / (double) b.T);
    printf("Average OPT: %lf\n", totalOpt[b.T - 1] / (double) b.T);
    printf("Average Regret: %lf\n", (totalOpt[b.T - 1] - totalGain[b.T - 1]) / (double) b.T);
    printf("Competitive Ratio: %lf\n", totalGain[b.T - 1] / totalOpt[b.T - 1]);
    printf("---------------------------------------------------------------------"
           "-----------\n\n");

    free(thres);
}
