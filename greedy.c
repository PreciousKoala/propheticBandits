#include "util.c"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void greedy(double *reward, double *totalRoundGain, double *totalOpt,
            double *totalBestHand, uint32_t totalThresholds, uint32_t maxItems,
            uint64_t totalRounds, uint64_t pricesPerRound) {
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

  double *roundGain = malloc(totalRounds * sizeof(double));
  uint32_t chosenTh;

  for (uint32_t t = 0; t < totalThresholds; t++) {
    runRound(thres, t, totalThresholds, reward, roundGain, t);
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
    runRound(thres, chosenTh, totalThresholds, reward, roundGain, t);
  }

  totalRoundGain[0] = roundGain[0];
  for (uint64_t t = 1; t < totalRounds; t++) {
    totalRoundGain[t] = totalRoundGain[t - 1] + roundGain[t];
    /* printf("%lf\n", totalRoundGain[t]); */
  }

  printf("\n");
  printf("-------------------------------------Greedy--------------------------"
         "-----------\n");
  printf("Threshold\tTotal Reward\tTimes Chosen\tAverage Reward\n");
  for (int32_t th = 0; th < totalThresholds; th++) {
    printf("%-7.2lf\t\t%-10.2lf\t%-12lu\t%-.6lf\n",
           thres[th].threshold, thres[th].rewardSum,
           thres[th].timesChosen, thres[th].avgReward);
  }

  printf("---------------------------------------------------------------------"
         "-----------\n");
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
         "-----------\n\n");

  free(thres);
  free(roundGain);
}
