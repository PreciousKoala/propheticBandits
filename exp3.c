#include "util.c"
#include <gsl/gsl_rng.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

void exp3(double *reward, double *totalGain, double *totalOpt,
          uint32_t totalThresholds, uint32_t maxItems, uint64_t totalRounds,
          uint64_t pricesPerRound) {
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
  gsl_rng_set(r, time(NULL));

  Threshold *thres = malloc(totalThresholds * sizeof(Threshold));
  initThreshold(thres, totalThresholds);

  totalGain[0] = 0;

  // threshold weights must be long double to prevent errors with very large
  // datasets probably doesn't work on windows but oh well
  long double *thresholdWeight = malloc(totalThresholds * sizeof(long double));
  for (uint32_t th = 0; th < totalThresholds; th++) {
    thresholdWeight[th] = 1;
  }

  double rewardMin = INFINITY;
  double rewardMax = -INFINITY;
  for (uint64_t i = 0; i < totalRounds * totalThresholds; i++) {
    if (reward[i] < rewardMin) {
      rewardMin = reward[i];
    }
    if (reward[i] > rewardMax) {
      rewardMax = reward[i];
    }
  }

  long double weightSum = 0;
  for (uint64_t t = 0; t < totalRounds; t++) {
    // upper bound is variable for easier future changes
    uint64_t upperBound = t + 1;
    long double gamma = sqrt((totalThresholds * log(totalThresholds)) /
                             ((M_E - 1) * upperBound));
    if (gamma > 1) {
      gamma = 1;
    }

    weightSum = 0;
    for (uint32_t th = 0; th < totalThresholds; th++) {
      weightSum += thresholdWeight[th];
    }

    // pick threshold according to probabilities (no need to calculate them all)
    // initialize chosenTh as the last threshold incase something goes wrong
    uint32_t chosenTh = totalThresholds - 1;
    double randomNumber = gsl_rng_uniform(r);
    long double thresholdProb = 0;
    for (uint32_t th = 0; th < totalThresholds; th++) {
      thresholdProb = (1 - gamma) * (thresholdWeight[th] / weightSum) +
                      gamma / totalThresholds;
      if (randomNumber < thresholdProb) {
        chosenTh = th;
        break;
      } else {
        randomNumber -= thresholdProb;
      }
    }

    runRound(thres, chosenTh, totalThresholds, reward, totalGain, t);

    // weight only changes for the chosen threshold
    double normalizedGain =
        (reward[totalThresholds * t + chosenTh] - rewardMin) /
        (rewardMax - rewardMin);
    double estimatedReward = normalizedGain / thresholdProb;
    thresholdWeight[chosenTh] *=
        expl(gamma * estimatedReward / totalThresholds);
  }

  printf("\n");
  printf("---------------------------------------------EXP3--------------------"
         "-----------------------\n");
  printf("Threshold\tTotal Reward\tTimes Chosen\tAverage "
         "Reward\tFinal Weight\tProbability\n");

  long double gamma = sqrt((totalThresholds * log(totalThresholds)) /
                           ((M_E - 1) * totalRounds));

  for (int32_t th = 0; th < totalThresholds; th++) {
    long double thresholdProb =
        (1 - gamma) * (thresholdWeight[th] / weightSum) +
        gamma / totalThresholds;

    printf("%-7.2lf\t\t%-10.2lf\t%-12lu\t%-8.6lf\t%-6LE\t%-.6Lf%%\n",
           thres[th].threshold, thres[th].rewardSum, thres[th].timesChosen,
           thres[th].avgReward, thresholdWeight[th], 100 * thresholdProb);
  }

  printf("---------------------------------------------------------------------"
         "-----------------------\n");
  printf("Final Gamma (Exploration Chance): %Lf%%\n", 100 * gamma);
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
         "-----------------------\n\n");

  gsl_rng_free(r);
  free(thresholdWeight);
  free(thres);
}
