#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.c"

void ucb2(double *reward, double *totalGain, double *totalOpt,
          uint32_t totalThresholds, uint32_t maxItems, uint64_t totalRounds,
          uint64_t pricesPerRound) {
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

  Threshold *thres = malloc(totalThresholds * sizeof(Threshold));
  initThreshold(thres, totalThresholds);

  totalGain[0] = 0;

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

  uint32_t chosenTh = 0;
  double *upperConfBound = malloc(totalThresholds * sizeof(double));
  // r_j
  uint32_t *epochsChosen = malloc(totalThresholds * sizeof(double));

  for (uint8_t t = 0; t < totalThresholds; t++) {
    epochsChosen[t] = 0;
    runRound(thres, chosenTh, totalThresholds, reward, totalGain, t);
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
          (thres[th].avgReward - rewardMin) / (rewardMax - rewardMin);
      if (upperConfBound[th] > max) {
        max = upperConfBound[th];
        chosenTh = th;
      }
    }

    epochsChosen[chosenTh]++;
    uint64_t repeat =
        (uint64_t)ceil(pow((1 + alpha), epochsChosen[chosenTh] + 1)) -
        (uint64_t)ceil(pow((1 + alpha), epochsChosen[chosenTh]));

    while (t < totalRounds && repeat > 0) {
      runRound(thres, chosenTh, totalThresholds, reward, totalGain, t);
      t++;
      repeat--;
    }
  }

  printf("\n");
  printf("--------------------------------------------------------------UCB2---"
         "-------------------------------------------------\n");
  printf("Threshold\tTotal Reward\tTimes Chosen\tAverage Reward\tUCB\t\t"
         "Epochs Chosen\tAverage Epoch Duration\n");
  for (int32_t th = 0; th < totalThresholds; th++) {
    double epochDuration;
    if (!epochsChosen[th]) {
      epochDuration = 0;
    } else {
      epochDuration = (double)thres[th].timesChosen / epochsChosen[th];
    }
    printf("%-7.2lf\t\t%-10.2lf\t%-12lu\t%-8.6lf\t%-10.5lf\t%-13u\t%-.5lf\n",
           thres[th].threshold, thres[th].rewardSum, thres[th].timesChosen,
           thres[th].avgReward, upperConfBound[th], epochsChosen[th],
           epochDuration);
  }

  printf("---------------------------------------------------------------------"
         "-------------------------------------------------\n");
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
         "-------------------------------------------------\n\n");

  free(upperConfBound);
  free(epochsChosen);
  free(thres);
}
