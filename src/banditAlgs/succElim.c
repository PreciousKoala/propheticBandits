#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <banditAlgs.h>
#include <util.h>

void succElim(double *data, double *totalGain, double *avgLowThreshold, double *avgHighThreshold, double *avgTrades,
              double *totalOpt, Bandit b) {
    /**
     * INFO: The Successive Elimination algorithm in short:
     *
     * --------------------------------------------------
     * initialize all arms as active
     * loop:
     *   try each arm once
     *   deactivate all arms that satisfy: UCB_t_a < max(LCB_t_a)
     * --------------------------------------------------
     *
     * in this program, arm = threshold
     *
     * UCB_t_a  = m_t_a + r_t_a
     * LCB_t_a  = m_t_a - r_t_a
     * m_t_a = average reward of arm a in round t
     * r_t_a = sqrt(2 * log(t) / n_t_a)
     * n_t_a = number of rounds before t where arm a was chosen
     */

    Threshold *thres = malloc(b.K * sizeof(Threshold));
    initThreshold(thres, b);

    totalGain[0] = 0;
    uint8_t heldItems = 0;
    double heldItemValue = 0;

    double *upperConfBound = malloc(b.K * sizeof(double));
    double *lowerConfBound = malloc(b.K * sizeof(double));
    uint8_t *thresActive = malloc(b.K * sizeof(uint8_t));

    for (uint32_t th = 0; th < b.K; th++) {
        thresActive[th] = 1;
    }

    double norm = -INFINITY;

    uint64_t t = 0;
    while (t < b.T) {
        for (uint32_t th = 0; th < b.K && t < b.T; th++) {
            if (thresActive[th]) {
                double gain = runRound(thres, th, b, data, avgLowThreshold, avgHighThreshold, avgTrades, totalGain, t,
                                       &heldItems, &heldItemValue);
                if (norm < gain) {
                    norm = gain;
                }
                t++;
            }
        }

        double maxLCB = -INFINITY;
        for (uint32_t th = 0; th < b.K; th++) {
            if (thresActive[th]) {
                double average = fmax(thres[th].avgReward / norm, 0);
                double confRadius = sqrt(2 * log((double) b.T) / (double) thres[th].timesChosen);

                upperConfBound[th] = average + confRadius;
                lowerConfBound[th] = average - confRadius;

                if (lowerConfBound[th] > maxLCB) {
                    maxLCB = lowerConfBound[th];
                }
            }
        }

        for (uint32_t th = 0; th < b.K; th++) {
            if (upperConfBound[th] < maxLCB) {
                thresActive[th] = 0;
            }
        }
    }

    printf("\n");
    printf("----------------------------------------SUCCESSIVE-ELIMINATION-------"
           "---------------------------------\n");
    if (!b.dualThres) {
        printf("Threshold\tTotal Reward\tTimes Chosen\tAverage "
               "Reward\tFinal UCB\tFinal LCB\tActive\n");
        for (int32_t th = 0; th < b.K; th++) {
            printf("%-7.2lf\t\t%-10.2lf\t%-12lu\t%-8.6lf\t%-10.5lf\t%-10.5lf\t%d\n", thres[th].low, thres[th].rewardSum,
                   thres[th].timesChosen, thres[th].avgReward, upperConfBound[th], lowerConfBound[th], thresActive[th]);
        }
    } else {
        printf("Low Thres\tHigh Thres\tTotal Reward\tTimes Chosen\tAverage "
               "Reward\tFinal UCB\tFinal LCB\tActive\n");
        for (int32_t th = 0; th < b.K; th++) {
            printf("%-7.2lf\t\t%-7.2lf\t\t%-10.2lf\t%-12lu\t%-8.6lf\t%-10.5lf\t%-10.5lf\t%d\n", thres[th].low,
                   thres[th].high, thres[th].rewardSum, thres[th].timesChosen, thres[th].avgReward, upperConfBound[th],
                   lowerConfBound[th], thresActive[th]);
        }
    }

    printf("---------------------------------------------------------------------"
           "---------------------------------\n");
    printf("Total Gain: %lf\n", totalGain[b.T - 1]);
    printf("Total OPT: %lf\n", totalOpt[b.T - 1]);
    printf("Total Regret: %lf\n", totalOpt[b.T - 1] - totalGain[b.T - 1]);
    printf("Average Gain: %lf\n", totalGain[b.T - 1] / (double) b.T);
    printf("Average OPT: %lf\n", totalOpt[b.T - 1] / (double) b.T);
    printf("Average Regret: %lf\n", (totalOpt[b.T - 1] - totalGain[b.T - 1]) / (double) b.T);
    printf("Competitive Ratio: %lf\n", totalGain[b.T - 1] / totalOpt[b.T - 1]);
    printf("---------------------------------------------------------------------"
           "---------------------------------\n\n");

    free(thresActive);
    free(upperConfBound);
    free(lowerConfBound);
    free(thres);
}
