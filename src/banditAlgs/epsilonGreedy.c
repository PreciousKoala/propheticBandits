#include <gsl/gsl_rng.h>
#include <math.h>
#include <time.h>

#include <banditAlgs.h>
#include <util.h>

void epsilonGreedy(double *data, double *totalGain, double *avgLowThreshold, double *avgHighThreshold,
                   double *avgTrades, double *totalOpt, Bandit b) {
    /**
     * INFO: The epsilon greedy algorithm in short:
     *
     * --------------------------------------------------
     * for each round t:
     *   toss coin with success prob of e_t;
     *   if success then
     *     explore: pick arm uniformly at random
     *   else
     *     exploit: pick arm with highest average reward
     * --------------------------------------------------
     *
     * in this program, arm = threshold
     *
     * e_t  = (K * log(t) / t)^(1 / 3)
     * achieves a good regret bound,
     * where K is the number of arms/thresholds
     */

    const gsl_rng_type *T;
    gsl_rng *r;
    gsl_rng_env_setup();
    T = gsl_rng_default;
    r = gsl_rng_alloc(T);
    gsl_rng_set(r, time(nullptr));

    Threshold *thres = malloc(b.K * sizeof(Threshold));
    initThreshold(thres, b);

    uint64_t explore = 0;
    uint64_t exploit = 0;
    totalGain[0] = 0;
    uint8_t heldItems = 0;
    double heldItemValue = 0;
    double exploreProb = 1;

    for (uint64_t t = 0; t < b.T; t++) {
        // exploreProb = cbrt(b.K * log(pow(2.0, ceil(log2((double) t + 1.0)))) / (double) (t + 1));
        exploreProb = cbrt(b.K * log((double) t + 1) / (double) (t + 1));
        // exploreProb = cbrt(b.K * log(b.T) / (double) (t + 1));

        // this will be 0 in the first round, and will always explore

        uint32_t chosenTh;
        if (gsl_rng_uniform(r) < exploreProb) {
            chosenTh = gsl_rng_uniform_int(r, b.K);
            explore++;

        } else {
            chosenTh = 0;
            double max = thres[0].avgReward;

            for (uint32_t th = 0; th < b.K; th++) {
                if (thres[th].avgReward > max) {
                    max = thres[th].avgReward;
                    chosenTh = th;
                }
            }
            exploit++;
        }

        runRound(thres, chosenTh, b, data, avgLowThreshold, avgHighThreshold, avgTrades, totalGain, t, &heldItems,
                 &heldItemValue);
    }

    printf("\n");
    printf("---------------------------------EPSILON-GREEDY----------------------"
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
    printf("Final Exploration Chance: %lf%%\n", 100 * exploreProb);
    printf("Explored: %lu\n", explore);
    printf("Exploited: %lu\n", exploit);
    printf("Total Gain: %lf\n", totalGain[b.T - 1]);
    printf("Total OPT: %lf\n", totalOpt[b.T - 1]);
    printf("Total Regret: %lf\n", totalOpt[b.T - 1] - totalGain[b.T - 1]);
    printf("Average Gain: %lf\n", totalGain[b.T - 1] / (double) b.T);
    printf("Average OPT: %lf\n", totalOpt[b.T - 1] / (double) b.T);
    printf("Average Regret: %lf\n", (totalOpt[b.T - 1] - totalGain[b.T - 1]) / (double) b.T);
    printf("Competitive Ratio: %lf\n", totalGain[b.T - 1] / totalOpt[b.T - 1]);
    printf("---------------------------------------------------------------------"
           "-----------\n\n");

    gsl_rng_free(r);
    free(thres);
}
