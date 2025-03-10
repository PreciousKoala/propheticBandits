#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void greedy(double *reward, double *totalRoundGain, double *totalOpt,
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
    double max = -INFINITY;
    chosenTh = 0;

    for (uint32_t th = 0; th < totalThresholds; th++) {
      if (avgReward[th] > max) {
        max = avgReward[th];
        chosenTh = th;
      }
    }

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

  printf("\n");
  printf("-------------------------------------Greedy--------------------------"
         "-----------\n");
  printf("Threshold\tTotal Reward\tTimes Chosen\tAverage Reward\n");
  for (int32_t th = 0; th < totalThresholds; th++) {
    printf("%-7.2lf\t\t%-10.2lf\t%-12lu\t%-.6lf\n",
           (double)th / totalThresholds, rewardSum[th], timesChosen[th],
           avgReward[th]);
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

  free(rewardSum);
  free(timesChosen);
  free(avgReward);
  free(roundGain);
}
