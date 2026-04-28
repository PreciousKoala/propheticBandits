#include <gsl/gsl_rng.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include <banditAlgs.h>
#include <util.h>

void exp3(double *data, double *totalGain, double *avgLowThreshold, double *avgHighThreshold, double *avgTrades,
          double *totalOpt, Bandit b) {
    /**
     * INFO: The exp3 algorithm in short:
     *
     * --------------------------------------------------
     * initialize w_i_1 = 1 for each threshold i
     * for each round t:
     *   p_i_t = (1-gamma) * (w_i_t / sum(i, K, w_i_t)) + gamma / K
     *   draw arm a according to the propabilities p_i_t
     *   recieve reward x_a_t in [0,1]
     *   for each arm k:
     *     x'_k_t = x_k_t / p_k_t   , if k = a
     *              0               , otherwise
     *     w_k_(t+1) = w_k_t * exp(gamma * x'_k_t / K)
     * --------------------------------------------------
     *
     * in this program, arm = threshold
     *
     * gamma is the exploration chance
     * K is the number of thresholds
     *
     * gamma = min{1, sqrt((K * log(K)) / ((e - 1) * g))}
     * e = euler's constant
     * g = the upper bound
     *
     * g is equal to the number of rounds, if it is known.
     * exp3.1 provides a solution in the case where we don't know
     * the number of rounds, but for now we will use g = t instead.
     *
     */

    const gsl_rng_type *T;
    gsl_rng *r;
    gsl_rng_env_setup();
    T = gsl_rng_default;
    r = gsl_rng_alloc(T);
    gsl_rng_set(r, time(nullptr));

    Threshold *thres = malloc(b.K * sizeof(Threshold));
    initThreshold(thres, b);

    totalGain[0] = 0;
    uint8_t heldItems = 0;
    double heldItemValue = 0;

    // threshold weights must be long double to prevent errors with very large
    // datasets probably doesn't work on windows but oh well
    long double *thresholdWeight = malloc(b.K * sizeof(long double));
    for (uint32_t th = 0; th < b.K; th++) {
        thresholdWeight[th] = 1;
    }

    // long double *estimTotalGain = malloc(b.K * sizeof(long double));
    // long double maxEstim = 0;
    // for (uint32_t th = 0; th < b.K; th++) {
    //     estimTotalGain[th] = 0;
    // }

    long double norm = 1;

    long double weightSum = 0;
    long double gamma = 1;
    // int rho = 0;
    for (uint64_t t = 0; t < b.T; t++) {
        // upper bound is variable for easier future changes

        double upperBound = (double) b.T;
        // double upperBound = (b.K * log(b.K) / (M_E - 1)) * pow(4, rho);
        // double upperBound = pow(2.0, ceil(log2((double) t + 1.0)));
        // double upperBound = t + 1;

        gamma = sqrt(b.K * log(b.K) / ((M_E - 1) * upperBound));
        gamma = fminl(gamma, 1);

        // if (maxEstim > upperBound - b.K / gamma) {
        // rho++;
        // gamma /= 2;
        // }

        weightSum = 0;
        for (uint32_t th = 0; th < b.K; th++) {
            weightSum += thresholdWeight[th];
        }

        // pick threshold according to probabilities (no need to calculate them all)
        // initialize chosenTh as the last threshold incase something goes wrong
        uint32_t chosenTh = b.K - 1;
        long double randomNumber = gsl_rng_uniform(r);
        long double thresholdProb = 0;
        for (uint32_t th = 0; th < b.K; th++) {
            thresholdProb = (1 - gamma) * (thresholdWeight[th] / weightSum) + gamma / b.K;
            if (randomNumber < thresholdProb) {
                chosenTh = th;
                break;
            }
            randomNumber -= thresholdProb;
        }

        // weight only changes for the chosen threshold
        long double gain = runRound(thres, chosenTh, b, data, avgLowThreshold, avgHighThreshold, avgTrades, totalGain,
                                    t, &heldItems, &heldItemValue);

        long double estimatedReward = fmaxl(gain, 0) / (norm * thresholdProb);
        thresholdWeight[chosenTh] *= expl(gamma * estimatedReward / b.K);

        if (t > 0) {
            if (norm < gain) {
                long double oldMaxOpt = norm;
                norm = gain;
                for (uint32_t th = 0; th < b.K; th++) {
                    thresholdWeight[th] = powl(thresholdWeight[th], oldMaxOpt / norm);
                }
            }
        }

        // maxEstim = totalOpt[t];

        // estimTotalGain[chosenTh] += estimatedReward;
        // if (maxEstim < estimTotalGain[chosenTh]) {
        // maxEstim = estimTotalGain[chosenTh];
        // }
    }


    printf("\n");
    // printf("%d\n", rho);
    printf("---------------------------------------------EXP3--------------------"
           "-----------------------\n");
    if (!b.dualThres) {
        printf("Threshold\tTotal Reward\tTimes Chosen\tAverage "
               "Reward\tFinal Weight\tProbability\n");

        for (int32_t th = 0; th < b.K; th++) {
            long double thresholdProb = (1 - gamma) * (thresholdWeight[th] / weightSum) + gamma / b.K;

            printf("%-7.2lf\t\t%-10.2lf\t%-12lu\t%-8.6lf\t%-6LE\t%-.6Lf%%\n", thres[th].low, thres[th].rewardSum,
                   thres[th].timesChosen, thres[th].avgReward, thresholdWeight[th], 100 * thresholdProb);
        }
    } else {
        printf("Low Thres\tHigh Thres\tTotal Reward\tTimes Chosen\tAverage "
               "Reward\tFinal Weight\tProbability\n");

        for (int32_t th = 0; th < b.K; th++) {
            long double thresholdProb = (1 - gamma) * (thresholdWeight[th] / weightSum) + gamma / b.K;

            printf("%-7.2lf\t\t%-7.2lf\t\t%-10.2lf\t%-12lu\t%-8.6lf\t%-6LE\t%-.6Lf%%\n", thres[th].low, thres[th].high,
                   thres[th].rewardSum, thres[th].timesChosen, thres[th].avgReward, thresholdWeight[th],
                   100 * thresholdProb);
        }
    }

    printf("---------------------------------------------------------------------"
           "-----------------------\n");
    printf("Final Gamma (Exploration Chance): %Lf%%\n", 100 * gamma);
    printf("Total Gain: %lf\n", totalGain[b.T - 1]);
    printf("Total OPT: %lf\n", totalOpt[b.T - 1]);
    printf("Total Regret: %lf\n", totalOpt[b.T - 1] - totalGain[b.T - 1]);
    printf("Average Gain: %lf\n", totalGain[b.T - 1] / (double) b.T);
    printf("Average OPT: %lf\n", totalOpt[b.T - 1] / (double) b.T);
    printf("Average Regret: %lf\n", (totalOpt[b.T - 1] - totalGain[b.T - 1]) / (double) b.T);
    printf("Competitive Ratio: %lf\n", totalGain[b.T - 1] / totalOpt[b.T - 1]);
    printf("---------------------------------------------------------------------"
           "-----------------------\n\n");

    gsl_rng_free(r);
    free(thresholdWeight);
    // free(estimTotalGain);
    free(thres);
}
