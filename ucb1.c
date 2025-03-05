#include <gsl/gsl_rng.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

void ucb1(double *data, uint32_t totalThresholds, uint8_t maxItems,
          uint64_t totalRounds, uint64_t pricesPerRound) {
  const gsl_rng_type *T;
  gsl_rng *r;
  gsl_rng_env_setup();
  T = gsl_rng_default;
  r = gsl_rng_alloc(T);
  gsl_rng_set(r, time(NULL));

  // uniformly distributed thresholds in [0,1)
  double *threshold = malloc(totalThresholds * sizeof(double));
  // how much money a threshold has made
  double *rewardSum = malloc(totalThresholds * sizeof(double));
  // how many times a threshold has been picked
  uint64_t *timesChosen = malloc(totalThresholds * sizeof(uint64_t));
  // how much money on average a threshold has made
  double *avgReward = malloc(totalThresholds * sizeof(double));

  for (uint32_t th = 0; th < totalThresholds; th++) {
    threshold[th] = (double)th / totalThresholds;
    //+ 1/(double)(2*totalThreshold);
    rewardSum[th] = 0;
    timesChosen[th] = 0;
    avgReward[th] = 0;
  }

  uint32_t heldItems = 0;
  double *roundGain = malloc(totalRounds * sizeof(double));
  double *totalRoundGain = malloc(totalRounds * sizeof(double));

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

  uint32_t chosenTh;

  for (uint8_t t = 0; t < totalThresholds; t++) {
    chosenTh = threshold[t];
    double gain = 0;
    for (uint32_t n = 0; n < pricesPerRound; n++) {
      if ((pricesPerRound - n == heldItems ||
           data[pricesPerRound * t + n] >= threshold[chosenTh]) &&
          heldItems > 0) {
        gain += data[pricesPerRound * t + n];
        heldItems--;
      } else if (pricesPerRound - n - 1 != heldItems &&
                 data[pricesPerRound * t + n] < threshold[chosenTh] &&
                 heldItems < maxItems) {
        gain -= data[pricesPerRound * t + n];
        heldItems++;
      }
    }

    rewardSum[chosenTh] += gain;
    roundGain[t] = gain;
    timesChosen[chosenTh]++;
    avgReward[chosenTh] = rewardSum[chosenTh] / timesChosen[chosenTh];
  }

  for (uint64_t t = totalThresholds; t < totalRounds; t++) {
    double *upperConfBound = malloc(totalThresholds * sizeof(double));
    double max = -INFINITY;
    chosenTh = 0;

    for (uint32_t th = 0; th < totalThresholds; th++) {
      upperConfBound[th] =
          sqrt(2 * log(t + 1) / timesChosen[th]) + avgReward[th];
      if (upperConfBound[th] > max) {
        max = upperConfBound[th];
        chosenTh = th;
      }
    }

    free(upperConfBound);

    double gain = 0;
    for (uint32_t n = 0; n < pricesPerRound; n++) {
      if ((pricesPerRound - n == heldItems ||
           data[pricesPerRound * t + n] >= threshold[chosenTh]) &&
          heldItems > 0) {
        gain += data[pricesPerRound * t + n];
        heldItems--;
      } else if (pricesPerRound - n - 1 != heldItems &&
                 data[pricesPerRound * t + n] < threshold[chosenTh] &&
                 heldItems < maxItems) {
        gain -= data[pricesPerRound * t + n];
        heldItems++;
      }
    }

    rewardSum[chosenTh] += gain;
    roundGain[t] = gain;
    timesChosen[chosenTh]++;
    avgReward[chosenTh] = rewardSum[chosenTh] / timesChosen[chosenTh];
  }

  totalRoundGain[0] = roundGain[0];
  for (uint64_t t = 1; t < totalRounds; t++) {
    totalRoundGain[t] = totalRoundGain[t - 1] + roundGain[t];
    printf("%lf\n", totalRoundGain[t]);
  }

  gsl_rng_free(r);
  free(threshold);
  free(rewardSum);
  free(timesChosen);
  free(avgReward);
  free(roundGain);
  free(totalRoundGain);
}
