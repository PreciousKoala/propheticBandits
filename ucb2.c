#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void ucb2(double *reward, double *totalRoundGain, double *totalOpt,
          double *totalBestHand, uint32_t totalThresholds, uint8_t maxItems,
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

  /**
   * INFO: The ucb2 algorithm in short:
   *
   * --------------------------------------------------
   * initialize r_j = 0 for each threshold
   *
   * try each arm once
   * for each round t
   *   pick arm which maximizes UCB_t
   *   play arm exactly tau(r_j + 1) - tau(r_j) times
   *   set r_j = r_j + 1
   * --------------------------------------------------
   *
   * in this program, arm = threshold
   *
   * UCB_t_a  = m_t_a + a_t_j
   * m_t_a = average reward of arm a in round t
   * a_t_j = sqrt((1 + a) * log(e * t / tau(r)) / (2 * tau(r)))
   * a = a small number, probably 1 / t
   * tau(r) = ceil((1 + a)^r)
   */

  uint32_t chosenTh;
  double *upperConfBound = malloc(totalThresholds * sizeof(double));
  // r_j
  uint32_t *epochsChosen = malloc(totalThresholds * sizeof(double));

  for (uint8_t t = 0; t < totalThresholds; t++) {
    epochsChosen[t] = 0;
    chosenTh = t;
    double gain = reward[totalThresholds * t + chosenTh];

    rewardSum[chosenTh] += gain;
    roundGain[t] = gain;
    timesChosen[chosenTh]++;
    avgReward[chosenTh] = rewardSum[chosenTh] / timesChosen[chosenTh];
  }

  uint64_t t = totalThresholds;
  while (t < totalRounds) {
    // TODO: find alpha that minimizes regret
    double alpha = 0.3; //(double)1 / (t + 1);
    double max = -INFINITY;
    chosenTh = 0;

    for (uint32_t th = 0; th < totalThresholds; th++) {
      uint64_t tau = (uint64_t)ceil(pow((1 + alpha), epochsChosen[th]));
      upperConfBound[th] =
          sqrt((1 + alpha) * log(M_E * (t + 1) / tau) / (2 * tau)) +
          avgReward[th];
      if (upperConfBound[th] > max) {
        max = upperConfBound[th];
        chosenTh = th;
      }
    }

    epochsChosen[chosenTh]++;
    uint64_t repeat =
        (uint64_t)ceil(pow((1 + alpha), epochsChosen[chosenTh] + 1)) -
        (uint64_t)ceil(pow((1 + alpha), epochsChosen[chosenTh]));
    /* printf("%lu %lf %lf\n", repeat, */
    /*        ceil(pow((1 + alpha), epochsChosen[chosenTh] + 1)), */
    /*        ceil(pow((1 + alpha), epochsChosen[chosenTh]))); */

    while (t < totalRounds && repeat > 0) {
      double gain = reward[totalThresholds * t + chosenTh];

      rewardSum[chosenTh] += gain;
      roundGain[t] = gain;
      timesChosen[chosenTh]++;
      avgReward[chosenTh] = rewardSum[chosenTh] / timesChosen[chosenTh];

      t++;
      repeat--;
    }
  }

  totalRoundGain[0] = roundGain[0];
  for (uint64_t t = 1; t < totalRounds; t++) {
    totalRoundGain[t] = totalRoundGain[t - 1] + roundGain[t];
    /* printf("%lf\n", totalRoundGain[t]); */
  }

  printf("\n");
  printf("--------------------------------------------------------------UCB2---"
         "-------------------------------------------------\n");
  printf("Threshold\tTotal Reward\tTimes Chosen\tAverage Reward\tUCB\t"
         "\tEpochs Chosen\tAverage Epoch Duration\n");
  for (int32_t th = 0; th < totalThresholds; th++) {
    double epochDuration;
    if (!epochsChosen[th]) {
      epochDuration = 0;
    } else {
      epochDuration = (double)timesChosen[th] / epochsChosen[th];
    }
    printf("%-7.2lf\t\t%-10.2lf\t%-12lu\t%-8.6lf\t%-5.5lf\t\t%-13u\t%-.5lf\n",
           (double)th / totalThresholds, rewardSum[th], timesChosen[th],
           avgReward[th], upperConfBound[th], epochsChosen[th], epochDuration);
  }

  printf("---------------------------------------------------------------------"
         "-------------------------------------------------\n");
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
         "-------------------------------------------------\n\n");

  free(upperConfBound);
  free(epochsChosen);
  free(rewardSum);
  free(timesChosen);
  free(avgReward);
  free(roundGain);
}
