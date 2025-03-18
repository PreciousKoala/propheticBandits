#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <banditAlgs.h>
#include <util.h>

void ucb1(double *reward, double *totalGain, double *totalOpt,
          uint32_t totalThresholds, uint32_t maxItems, uint64_t totalRounds,
          uint64_t pricesPerRound) {
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

  Threshold *thres = malloc(totalThresholds * sizeof(Threshold));
  initThreshold(thres, totalThresholds);

  totalGain[0] = 0;

  uint32_t chosenTh = 0;
  double *upperConfBound = malloc(totalThresholds * sizeof(double));

  for (uint32_t t = 0; t < totalThresholds; t++) {
    runRound(thres, chosenTh, totalThresholds, reward, totalGain, t);
  }

  for (uint64_t t = totalThresholds; t < totalRounds; t++) {
    double max = -INFINITY;
    chosenTh = 0;

    for (uint32_t th = 0; th < totalThresholds; th++) {
      double average = thres[th].avgReward;
      double confRadius = sqrt(2 * log(t + 1) / thres[th].timesChosen);
      upperConfBound[th] = average + confRadius;

      if (upperConfBound[th] > max) {
        max = upperConfBound[th];
        chosenTh = th;
      }
    }
    runRound(thres, chosenTh, totalThresholds, reward, totalGain, t);
  }

  printf("\n");
  printf("--------------------------------------UCB1---------------------------"
         "-----------\n");
  printf("Threshold\tTotal Reward\tTimes Chosen\tAverage Reward\tUCB\n");
  for (int32_t th = 0; th < totalThresholds; th++) {
    printf("%-7.2lf\t\t%-10.2lf\t%-12lu\t%-8.6lf\t%-.5lf\n",
           thres[th].threshold, thres[th].rewardSum, thres[th].timesChosen,
           thres[th].avgReward, upperConfBound[th]);
  }

  printf("---------------------------------------------------------------------"
         "-----------\n");
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

  free(upperConfBound);
  free(thres);
}
