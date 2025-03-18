#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <banditAlgs.h>
#include <util.h>

void greedy(double *reward, double *totalGain, double *totalOpt,
            uint32_t totalThresholds, uint32_t maxItems, uint64_t totalRounds,
            uint64_t pricesPerRound) {
  /**
   * INFO: The greedy algorithm in short:
   *
   * --------------------------------------------------
   * try each arm once
   * for each round t
   *   pick the arm with the highest average reward
   * --------------------------------------------------
   *
   * in this program, arm = threshold
   *
   */

  Threshold *thres = malloc(totalThresholds * sizeof(Threshold));
  initThreshold(thres, totalThresholds);

  uint32_t chosenTh = 0;
  totalGain[0] = 0;

  for (uint32_t t = 0; t < totalThresholds; t++) {
    runRound(thres, t, totalThresholds, reward, totalGain, t);
  }

  for (uint64_t t = totalThresholds; t < totalRounds; t++) {
    double max = -INFINITY;
    chosenTh = 0;

    for (uint32_t th = 0; th < totalThresholds; th++) {
      if (thres[th].avgReward > max) {
        max = thres[th].avgReward;
        chosenTh = th;
      }
    }
    runRound(thres, chosenTh, totalThresholds, reward, totalGain, t);
  }

  printf("\n");
  printf("-------------------------------------Greedy--------------------------"
         "-----------\n");
  printf("Threshold\tTotal Reward\tTimes Chosen\tAverage Reward\n");
  for (int32_t th = 0; th < totalThresholds; th++) {
    printf("%-7.2lf\t\t%-10.2lf\t%-12lu\t%-.6lf\n", thres[th].threshold,
           thres[th].rewardSum, thres[th].timesChosen, thres[th].avgReward);
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

  free(thres);
}
