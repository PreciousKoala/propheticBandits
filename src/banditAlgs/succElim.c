#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <banditAlgs.h>
#include <util.h>

void succElim(double *reward, double *totalGain, double *totalOpt,
              uint32_t totalThresholds, uint32_t maxItems, uint64_t totalRounds,
              uint64_t pricesPerRound) {
  /**
   * INFO: The Successive Elimination algorithm in short:
   *
   * --------------------------------------------------
   * initialize all arms as active
   * loop:
   *   try each arm once
   *   deactivate all arms that satisfy: UCB_t_a < max(LCB_t_a)
   * --------------------------------------------------
   *
   * in this program, arm = threshold
   *
   * UCB_t_a  = m_t_a + r_t_a
   * LCB_t_a  = m_t_a - r_t_a
   * m_t_a = average reward of arm a in round t
   * r_t_a = sqrt(2 * log(t) / n_t_a)
   * n_t_a = number of rounds before t where arm a was chosen
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

  double *upperConfBound = malloc(totalThresholds * sizeof(double));
  double *lowerConfBound = malloc(totalThresholds * sizeof(double));
  uint8_t *thresActive = malloc(totalThresholds * sizeof(uint8_t));

  for (uint32_t th = 0; th < totalThresholds; th++) {
    thresActive[th] = 1;
  }

  uint64_t t = 0;
  while (t < totalRounds) {
    for (uint32_t th = 0; th < totalThresholds && t < totalRounds; th++) {
      if (thresActive[th]) {
        runRound(thres, th, totalThresholds, reward, totalGain, t);
        t++;
      }
    }

    double maxLCB = -INFINITY;
    for (uint32_t th = 0; th < totalThresholds; th++) {
      if (thresActive[th]) {
        double normalizedAvg =
            (thres[th].avgReward - rewardMin) / (rewardMax - rewardMin);
        double confRadius = sqrt(2 * log(t + 1) / thres[th].timesChosen);

        upperConfBound[th] = normalizedAvg + confRadius;
        lowerConfBound[th] = normalizedAvg - confRadius;

        if (lowerConfBound[th] > maxLCB) {
          maxLCB = lowerConfBound[th];
        }
      }
    }

    for (uint32_t th = 0; th < totalThresholds; th++) {
      if (upperConfBound[th] < maxLCB) {
        thresActive[th] = 0;
      }
    }
  }

  printf("\n");
  printf("----------------------------------------SUCCESSIVE-ELIMINATION-------"
         "---------------------------------\n");
  printf("Threshold\tTotal Reward\tTimes Chosen\tAverage "
         "Reward\tFinal UCB\tFinal LCB\tActive\n");
  for (int32_t th = 0; th < totalThresholds; th++) {
    printf("%-7.2lf\t\t%-10.2lf\t%-12lu\t%-8.6lf\t%-10.5lf\t%-10.5lf\t%d\n",
           thres[th].threshold, thres[th].rewardSum, thres[th].timesChosen,
           thres[th].avgReward, upperConfBound[th], lowerConfBound[th],
           thresActive[th]);
  }

  printf("---------------------------------------------------------------------"
         "---------------------------------\n");
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
         "---------------------------------\n\n");

  free(thresActive);
  free(upperConfBound);
  free(lowerConfBound);
  free(thres);
}
