#include <gsl/gsl_rng.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

void epsilonGreedy(double *reward, double *totalRoundGain, double *totalOpt,
                   double *totalBestHand, uint32_t totalThresholds,
                   uint8_t maxItems, uint64_t totalRounds,
                   uint64_t pricesPerRound) {
  const gsl_rng_type *T;
  gsl_rng *r;
  gsl_rng_env_setup();
  T = gsl_rng_default;
  r = gsl_rng_alloc(T);
  gsl_rng_set(r, time(NULL));

  // how much money a threshold has made
  double *rewardSum = malloc(totalThresholds * sizeof(double));
  // how many times a threshold has been picked
  uint64_t *timesChosen = malloc(totalThresholds * sizeof(uint64_t));
  // how much money on average a threshold has made
  double *avgReward = malloc(totalThresholds * sizeof(double));

  for (uint32_t th = 0; th < totalThresholds; th++) {
    rewardSum[th] = 0;
    timesChosen[th] = 0;
    avgReward[th] = 0;
  }

  uint64_t explore = 0;
  uint64_t exploit = 0;
  double *roundGain = malloc(totalRounds * sizeof(double));

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

  for (uint64_t t = 0; t < totalRounds; t++) {
    double exploreProb = cbrt(totalThresholds * log(t + 1) / (t + 1));
    // this will be 0 in the first round, and will always explore

    uint32_t chosenTh;
    if (gsl_rng_uniform(r) < exploreProb) {
      chosenTh = gsl_rng_uniform_int(r, totalThresholds);
      explore++;
    } else {
      chosenTh = 0;
      double max = avgReward[0];

      for (uint32_t th = 0; th < totalThresholds; th++) {
        if (avgReward[th] > max) {
          max = avgReward[th];
          chosenTh = th;
        }
      }
      exploit++;
    }

    double gain = reward[totalThresholds * t + chosenTh];

    rewardSum[chosenTh] += gain;
    roundGain[t] = gain;
    timesChosen[chosenTh]++;
    if (timesChosen[chosenTh] != 0) {
      avgReward[chosenTh] = rewardSum[chosenTh] / timesChosen[chosenTh];
    }
  }

  totalRoundGain[0] = roundGain[0];
  for (uint64_t t = 1; t < totalRounds; t++) {
    totalRoundGain[t] = totalRoundGain[t - 1] + roundGain[t];
    /* printf("%lf\n", totalRoundGain[t]); */
  }

  printf("\n");
  printf("---------------------------------EPSILON-GREEDY----------------------"
         "-----------\n");
  printf("Threshold\tTotal Reward\tTimes Chosen\tAverage Reward\n");
  for (int32_t th = 0; th < totalThresholds; th++) {
    printf("%-7.2lf\t\t%-10.2lf\t%-12lu\t%-.6lf\n",
           (double)th / totalThresholds, rewardSum[th], timesChosen[th],
           avgReward[th]);
  }

  printf("---------------------------------------------------------------------"
         "-----------\n");
  printf("Final Exploration Chance: %lf%%\n",
         100 * cbrt(totalThresholds * log(totalRounds) / (totalRounds)));
  printf("Explored: %lu\n", explore);
  printf("Exploited: %lu\n", exploit);
  printf("Total Gain: %lf\n", totalRoundGain[totalRounds - 1]);
  printf("OPT: %lf (buying & selling at local extrema)\n",
         totalOpt[totalRounds - 1]);
  printf("Regret: %lf\n",
         totalOpt[totalRounds - 1] - totalRoundGain[totalRounds - 1]);
  printf("OPT: %lf (picking best hand every round)\n",
         totalBestHand[totalRounds - 1]);
  printf("Regret: %lf\n",
         totalBestHand[totalRounds - 1] - totalRoundGain[totalRounds - 1]);
  printf("---------------------------------------------------------------------"
         "-----------\n\n");

  gsl_rng_free(r);
  free(rewardSum);
  free(timesChosen);
  free(avgReward);
  free(roundGain);
}
