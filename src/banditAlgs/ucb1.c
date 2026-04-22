#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <banditAlgs.h>
#include <util.h>

void ucb1(double *data, double *totalGain, double *avgThreshold, double *avgTrades, double *totalOpt,
          uint32_t totalThresholds, uint64_t totalRounds, uint64_t pricesPerRound) {
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

    Threshold *thres = malloc(totalThresholds * sizeof(Threshold));
    initThreshold(thres, totalThresholds);

    totalGain[0] = 0;
    uint8_t heldItems = 0;
    double heldItemValue;

    uint32_t chosenTh = 0;
    double *upperConfBound = malloc(totalThresholds * sizeof(double));

    double norm = -INFINITY;
    for (uint32_t t = 0; t < totalThresholds; t++) {
        double gain = runRound(thres, t, totalRounds, pricesPerRound, data, avgThreshold, avgTrades, totalGain, t,
                               &heldItems, &heldItemValue);
        if (norm < gain) {
            norm = gain;
        }
    }

    for (uint64_t t = totalThresholds; t < totalRounds; t++) {
        double maxUCB = -INFINITY;
        chosenTh = 0;

        for (uint32_t th = 0; th < totalThresholds; th++) {
            double average = fmax(thres[th].avgReward / norm, 0);
            double confRadius = sqrt(2 * log(pow(2.0, ceil(log2((double) t + 1.0)))) / (double) thres[th].timesChosen);
            // double confRadius = sqrt(2 * log(t + 1) / (double) thres[th].timesChosen);
            // double confRadius = sqrt(2 * log(totalRounds) / (double) thres[th].timesChosen);
            upperConfBound[th] = average + confRadius;

            if (upperConfBound[th] > maxUCB) {
                maxUCB = upperConfBound[th];
                chosenTh = th;
            }
        }
        double gain = runRound(thres, chosenTh, totalRounds, pricesPerRound, data, avgThreshold, avgTrades, totalGain,
                               t, &heldItems, &heldItemValue);

        if (norm < gain) {
            norm = gain;
        }
    }

    printf("\n");
    printf("--------------------------------------UCB1---------------------------"
           "-----------\n");
    printf("Threshold\tTotal Reward\tTimes Chosen\tAverage Reward\tUCB\n");
    for (int32_t th = 0; th < totalThresholds; th++) {
        printf("%-7.2lf\t\t%-10.2lf\t%-12lu\t%-8.6lf\t%-.5lf\n", thres[th].threshold, thres[th].rewardSum,
               thres[th].timesChosen, thres[th].avgReward, upperConfBound[th]);
    }

    printf("---------------------------------------------------------------------"
           "-----------\n");
    printf("Total Gain: %lf\n", totalGain[totalRounds - 1]);
    printf("Total OPT: %lf\n", totalOpt[totalRounds - 1]);
    printf("Total Regret: %lf\n", totalOpt[totalRounds - 1] - totalGain[totalRounds - 1]);
    printf("Average Gain: %lf\n", totalGain[totalRounds - 1] / (double) totalRounds);
    printf("Average OPT: %lf\n", totalOpt[totalRounds - 1] / (double) totalRounds);
    printf("Average Regret: %lf\n", (totalOpt[totalRounds - 1] - totalGain[totalRounds - 1]) / (double) totalRounds);
    printf("Competitive Ratio: %lf\n", totalGain[totalRounds - 1] / totalOpt[totalRounds - 1]);
    printf("---------------------------------------------------------------------"
           "-----------\n\n");

    free(upperConfBound);
    free(thres);
}
