#include <math.h>
#include <stdint.h>
#include <stdlib.h>

void ucb1(double *reward, uint32_t totalThresholds, uint8_t maxItems,
          uint64_t totalRounds, uint64_t pricesPerRound) {
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
    chosenTh = t;
    double gain = reward[totalThresholds * t + chosenTh];

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

    double gain = reward[totalThresholds * t + chosenTh];

    rewardSum[chosenTh] += gain;
    roundGain[t] = gain;
    timesChosen[chosenTh]++;
    avgReward[chosenTh] = rewardSum[chosenTh] / timesChosen[chosenTh];
  }

  totalRoundGain[0] = roundGain[0];
  for (uint64_t t = 1; t < totalRounds; t++) {
    totalRoundGain[t] = totalRoundGain[t - 1] + roundGain[t];
    /* printf("%lf\n", totalRoundGain[t]); */
  }

  free(rewardSum);
  free(timesChosen);
  free(avgReward);
  free(roundGain);
  free(totalRoundGain);
}
