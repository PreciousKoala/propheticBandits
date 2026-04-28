#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <banditAlgs.h>
#include <util.h>

void ucb1(double *data, double *totalGain, double *avgLowThreshold, double *avgHighThreshold, double *avgTrades,
          double *totalOpt, Bandit b) {
    /**
     * INFO: The ucb1 algorithm in short:
     *
     * --------------------------------------------------
     * try each arm once
     * for each round t
     *   pick arm which maximizes UCB_t
     * --------------------------------------------------
     *
     * in this program, arm = threshold
     *
     * UCB_t_a  = m_t_a + r_t_a
     * m_t_a = average reward of arm a in round t
     * r_t_a = sqrt(2 * log(t) / n_t_a)
     * n_t_a = number of rounds before t where arm a was chosen
     */

    Threshold *thres = malloc(b.K * sizeof(Threshold));
    initThreshold(thres, b);

    totalGain[0] = 0;
    uint8_t heldItems = 0;
    double heldItemValue = 0;

    uint32_t chosenTh = 0;
    double *upperConfBound = malloc(b.K * sizeof(double));

    double norm = -INFINITY;
    for (uint32_t t = 0; t < b.K; t++) {
        double gain = runRound(thres, t, b, data, avgLowThreshold, avgHighThreshold, avgTrades, totalGain, t,
                               &heldItems, &heldItemValue);
        if (norm < gain) {
            norm = gain;
        }
    }

    for (uint64_t t = b.K; t < b.T; t++) {
        double maxUCB = -INFINITY;
        chosenTh = 0;

        for (uint32_t th = 0; th < b.K; th++) {
            double average = fmax(thres[th].avgReward / norm, 0);
            // double confRadius = sqrt(2 * log(pow(2.0, ceil(log2((double) t + 1.0)))) / (double)
            // thres[th].timesChosen); double confRadius = sqrt(2 * log(t + 1) / (double) thres[th].timesChosen);
            double confRadius = sqrt(2 * log(b.T) / (double) thres[th].timesChosen);
            upperConfBound[th] = average + confRadius;

            if (upperConfBound[th] > maxUCB) {
                maxUCB = upperConfBound[th];
                chosenTh = th;
            }
        }
        double gain = runRound(thres, chosenTh, b, data, avgLowThreshold, avgHighThreshold, avgTrades, totalGain, t,
                               &heldItems, &heldItemValue);

        if (norm < gain) {
            norm = gain;
        }
    }

    printf("\n");
    printf("--------------------------------------UCB1---------------------------"
           "-----------\n");
    if (!b.dualThres) {
        printf("Threshold\tTotal Reward\tTimes Chosen\tAverage Reward\tUCB\n");
        for (int32_t th = 0; th < b.K; th++) {
            printf("%-7.2lf\t\t%-10.2lf\t%-12lu\t%-8.6lf\t%-.5lf\n", thres[th].low, thres[th].rewardSum,
                   thres[th].timesChosen, thres[th].avgReward, upperConfBound[th]);
        }
    } else {
        printf("Low Thres\tHigh Thres\tTotal Reward\tTimes Chosen\tAverage Reward\tUCB\n");
        for (int32_t th = 0; th < b.K; th++) {
            printf("%-7.2lf\t\t%-7.2lf\t\t%-10.2lf\t%-12lu\t%-8.6lf\t%-.5lf\n", thres[th].low, thres[th].high,
                   thres[th].rewardSum, thres[th].timesChosen, thres[th].avgReward, upperConfBound[th]);
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

    free(upperConfBound);
    free(thres);
}
