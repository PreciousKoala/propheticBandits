#include <gsl/gsl_rng.h>
#include <math.h>
#include <time.h>

#include "banditAlgs.h"
#include "util.h"

void epsilonGreedy(double *reward, double *totalGain, double *totalOpt,
                   uint32_t totalThresholds, uint32_t maxItems,
                   uint64_t totalRounds, uint64_t pricesPerRound) {
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
  gsl_rng_set(r, time(NULL));

  Threshold *thres = malloc(totalThresholds * sizeof(Threshold));
  initThreshold(thres, totalThresholds);

  uint64_t explore = 0;
  uint64_t exploit = 0;

  totalGain[0] = 0;

  for (uint64_t t = 0; t < totalRounds; t++) {
    // TODO: find good exploration chance
    double exploreProb = cbrt(totalThresholds * log(t + 1) / (t + 1));
    // this will be 0 in the first round, and will always explore

    uint32_t chosenTh;
    if (gsl_rng_uniform(r) < exploreProb) {
      chosenTh = gsl_rng_uniform_int(r, totalThresholds);
      explore++;

    } else {
      chosenTh = 0;
      double max = thres[0].avgReward;

      for (uint32_t th = 0; th < totalThresholds; th++) {
        if (thres[th].avgReward > max) {
          max = thres[th].avgReward;
          chosenTh = th;
        }
      }
      exploit++;
    }

    runRound(thres, chosenTh, totalThresholds, reward, totalGain, t);
  }

  printf("\n");
  printf("---------------------------------EPSILON-GREEDY----------------------"
         "-----------\n");
  printf("Threshold\tTotal Reward\tTimes Chosen\tAverage Reward\n");
  for (int32_t th = 0; th < totalThresholds; th++) {
    printf("%-7.2lf\t\t%-10.2lf\t%-12lu\t%-.6lf\n", thres[th].threshold,
           thres[th].rewardSum, thres[th].timesChosen, thres[th].avgReward);
  }

  printf("---------------------------------------------------------------------"
         "-----------\n");
  printf("Final Exploration Chance: %lf%%\n",
         100 * cbrt(totalThresholds * log(totalRounds) / (totalRounds)));
  printf("Explored: %lu\n", explore);
  printf("Exploited: %lu\n", exploit);
  printf("Total Gain: %lf\n", totalGain[totalRounds - 1]);
  printf("Total OPT: %lf\n", totalOpt[totalRounds - 1]);
  printf("Total Regret: %lf\n",
         totalOpt[totalRounds - 1] - totalGain[totalRounds - 1]);
  printf("Average Gain: %lf\n", totalGain[totalRounds - 1] / totalRounds);
  printf("Average OPT: %lf\n", totalOpt[totalRounds - 1] / totalRounds);
  printf("Average Regret: %lf\n",
         (totalOpt[totalRounds - 1] - totalGain[totalRounds - 1]) /
             totalRounds);
  printf("---------------------------------------------------------------------"
         "-----------\n\n");

  gsl_rng_free(r);
  free(thres);
}
