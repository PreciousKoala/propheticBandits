#include <gsl/gsl_rng.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

void exp3(double *reward, double *totalRoundGain, uint32_t totalThresholds,
          uint8_t maxItems, uint64_t totalRounds, uint64_t pricesPerRound) {
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

  double *roundGain = malloc(totalRounds * sizeof(double));

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

  double *thresholdWeight = malloc(totalThresholds * sizeof(double));
  for (uint32_t th = 0; th < totalThresholds; th++) {
    thresholdWeight[th] = 1;
  }

  double min = INFINITY;
  double max = -INFINITY;
  for (uint64_t i = 0; i < totalRounds * totalThresholds; i++) {
    if (reward[i] < min) {
      min = reward[i];
    }
    if (reward[i] > max) {
      max = reward[i];
    }
  }

  for (uint64_t t = 0; t < totalRounds; t++) {
    // upper bound is variable for easier future changes
    uint64_t upperBound = t + 1;
    double gamma = sqrt((totalThresholds * log(totalThresholds)) /
                        ((M_E - 1) * upperBound));
    if (gamma > 1) {
      gamma = 1;
    }

    double totalWeight = 0;
    for (uint32_t th = 0; th < totalThresholds; th++) {
      totalWeight += thresholdWeight[th];
    }

    // pick threshold according to probabilities (no need to calculate them all)
    // initialize chosenTh as the last threshold incase something goes wrong
    uint32_t chosenTh = totalThresholds - 1;
    double randomNumber = gsl_rng_uniform(r);
    double thresholdProb = 0;
    for (uint32_t th = 0; th < totalThresholds; th++) {
      thresholdProb = (1 - gamma) * (thresholdWeight[th] / totalWeight) +
                      gamma / totalThresholds;
      if (randomNumber < thresholdProb) {
        chosenTh = th;
        break;
      } else {
        randomNumber -= thresholdProb;
      }
    }

    double gain = reward[totalThresholds * t + chosenTh];

    rewardSum[chosenTh] += gain;
    roundGain[t] = gain;
    timesChosen[chosenTh]++;
    if (timesChosen[chosenTh] != 0) {
      avgReward[chosenTh] = rewardSum[chosenTh] / timesChosen[chosenTh];
    }

    // weight only changes for the chosen threshold
    double normalizedGain = (gain - min) / (max - min);
    thresholdWeight[chosenTh] =
        thresholdWeight[chosenTh] *
        exp(gamma * normalizedGain / (thresholdProb * totalThresholds));
  }

  totalRoundGain[0] = roundGain[0];
  for (uint64_t t = 1; t < totalRounds; t++) {
    totalRoundGain[t] = totalRoundGain[t - 1] + roundGain[t];
    /* printf("%lf\n", totalRoundGain[t]); */
  }

  gsl_rng_free(r);
  free(thresholdWeight);
  free(rewardSum);
  free(timesChosen);
  free(avgReward);
  free(roundGain);
}
