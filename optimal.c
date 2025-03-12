#include "util.c"
#include <math.h>
#include <stdint.h>
#include <stdio.h>

/*
void findOpt(double *data, double *optAlg, double *totalOpt, uint8_t maxItems,
             uint64_t totalRounds, uint64_t pricesPerRound) {
  for (uint64_t t = 0; t < totalRounds; t++) {
    uint8_t localMin = data[pricesPerRound * t] <= data[pricesPerRound * t + 1];
    uint8_t localMax;

    optAlg[t] = -data[pricesPerRound * t] * localMin;

    for (uint64_t n = 1; n < pricesPerRound - 1; n++) {
      localMax =
          data[pricesPerRound * t + n] >= data[pricesPerRound * t + n - 1];

      localMin =
          data[pricesPerRound * t + n] <= data[pricesPerRound * t + n + 1];

      optAlg[t] += data[pricesPerRound * t + n] * localMax -
                   data[pricesPerRound * t + n] * localMin;
    }
    localMax = (data[pricesPerRound * (t + 1) - 1] >=
                data[pricesPerRound * (t + 1) - 2]);
    optAlg[t] += data[pricesPerRound * (t + 1) - 1] * localMax;
  }
  totalOpt[0] = optAlg[0];
  for (uint64_t t = 1; t < totalRounds; t++) {
    totalOpt[t] = totalOpt[t - 1] + optAlg[t];
  }
}
*/

void findBestHand(double *reward, double *bestHand, double *totalBestHand,
                  uint64_t totalRounds, uint32_t totalThresholds) {
  Threshold *thres = malloc(totalThresholds * sizeof(Threshold));
  initThreshold(thres, totalThresholds);

  for (uint64_t t = 0; t < totalRounds; t++) {
    double max = -INFINITY;
    uint32_t chosenTh = 0;

    for (uint32_t th = 0; th < totalThresholds; th++) {
      if (reward[totalThresholds * t + th] > max) {
        max = reward[totalThresholds * t + th];
        chosenTh = th;
      }
    }
    runRound(thres, chosenTh, totalThresholds, reward, bestHand, t);
  }

  totalBestHand[0] = bestHand[0];
  for (uint64_t t = 1; t < totalRounds; t++) {
    totalBestHand[t] = totalBestHand[t - 1] + bestHand[t];
    /* printf("%lf\n", totalBestHand[t]); */
  }

  printf("\n");
  printf("-----------------------------------BEST-HAND-------------------------"
         "-----------\n");
  printf("Threshold\tTotal Reward\tTimes Chosen\tAverage Reward\n");
  for (int32_t th = 0; th < totalThresholds; th++) {
    printf("%-7.2lf\t\t%-10.2lf\t%-12lu\t%-.6lf\n", thres[th].threshold,
           thres[th].rewardSum, thres[th].timesChosen, thres[th].avgReward);
  }

  printf("---------------------------------------------------------------------"
         "-----------\n");
  printf("OPT: %lf (picking best hand every round)\n",
         totalBestHand[totalRounds - 1]);
  printf("---------------------------------------------------------------------"
         "-----------\n\n");

  free(thres);
}
