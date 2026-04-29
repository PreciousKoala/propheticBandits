#include <banditAlgs.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <util.h>

void findOpt(double *data, double *totalOpt, double *avgTrades, Bandit b) {
    uint8_t rightAsc;
    uint8_t leftAsc;

    for (uint64_t t = 1; t < b.T; t++) {
        avgTrades[t] = 0;
        totalOpt[t] = 0;
    }

    if (b.keepItems) {
        rightAsc = data[0] <= data[1];
        totalOpt[0] = -data[0] * rightAsc;

        for (uint64_t i = 1; i < b.T * b.N - 1; i++) {
            leftAsc = (data[i] >= data[i - 1]);
            rightAsc = (data[i] <= data[i + 1]);
            totalOpt[i / b.N] += data[i] * leftAsc - data[i] * rightAsc;

            if (leftAsc && !rightAsc) {
                avgTrades[i / b.N]++;
            }
        }

        leftAsc = data[b.T * b.N - 1] >= data[b.T * b.N - 2];
        totalOpt[b.T - 1] += data[b.T * b.N - 1] * leftAsc;
    } else {
        for (uint64_t t = 0; t < b.T; t++) {
            rightAsc = data[b.N * t] <= data[b.N * t + 1];
            totalOpt[t] = -data[b.N * t] * rightAsc;

            for (uint64_t n = 1; n < b.N - 1; n++) {
                leftAsc = (data[b.N * t + n] >= data[b.N * t + n - 1]);
                rightAsc = (data[b.N * t + n] <= data[b.N * t + n + 1]);
                totalOpt[t] += data[b.N * t + n] * leftAsc - data[b.N * t + n] * rightAsc;

                if (leftAsc && !rightAsc) {
                    avgTrades[t]++;
                }
            }

            leftAsc = data[(t + 1) * b.N - 1] >= data[(t + 1) * b.N - 2];
            totalOpt[t] += data[(t + 1) * b.N - 1] * leftAsc;
        }
    }


    for (uint64_t t = 1; t < b.T; t++) {
        totalOpt[t] += totalOpt[t - 1];
        avgTrades[t] += avgTrades[t - 1];
    }

    for (uint64_t t = 1; t < b.T; t++) {
        avgTrades[t] = avgTrades[t] / (double) (t + 1);
    }

    printf("\n");
    printf("---------------------------------LOCAL-EXTREMA-----------------------"
           "-----------\n");
    printf("Total OPT: %lf\n", totalOpt[b.T - 1]);
    printf("Average OPT: %lf\n", totalOpt[b.T - 1] / (double) b.T);
    printf("---------------------------------------------------------------------"
           "-----------\n\n");
}

void bestHand(double *data, double *totalOpt, double *avgLowThreshold, double *avgHighThreshold, double *avgTrades,
              Bandit b) {
    Threshold *thres = malloc(b.K * sizeof(Threshold));
    initThreshold(thres, b);

    double *totalGain = malloc(b.T * sizeof(double));
    double *buffer = malloc(b.T * sizeof(double));

    for (uint64_t t = 0; t < b.T; t++) {
        double maxGain = -INFINITY;
        uint32_t chosenTh = 0;
        double gain = 0;
        buffer[t] = 0;
        uint8_t heldItems = 0;
        double heldItemValue = 0;

        for (uint32_t th = 0; th < b.K; th++) {
            uint32_t trades = 0;
            gain = runThreshold(thres[th].low, thres[th].high, b, data, &trades, t, &heldItems, &heldItemValue);
            if (gain >= maxGain) {
                maxGain = gain;
                chosenTh = th;
            }
        }

        runRound(thres, chosenTh, b, data, avgLowThreshold, avgHighThreshold, avgTrades, totalOpt, t, &heldItems, &heldItemValue);
    }

    free(totalGain);
    free(buffer);

    printf("\n");
    printf("-----------------------------------BEST-HAND-------------------------"
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
    printf("OPT: %lf\n", totalOpt[b.T - 1]);
    printf("Average OPT: %lf\n", totalOpt[b.T - 1] / (double) b.T);
    printf("---------------------------------------------------------------------"
           "-----------\n\n");

    free(thres);
}
