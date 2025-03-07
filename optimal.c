#include <math.h>
#include <stdint.h>
#include <stdio.h>

void findOpt(double *data, double *optAlg, double *totalOpt, uint8_t maxItems,
             uint64_t totalRounds, uint64_t pricesPerRound) {
  // TODO: do the same but for maxItems>1 (what the fuck)

  /*
   * INFO: Based on "Trading Prophets" section 3 lemma 1 by Jose Correa, Andres
   * Cristi, Paul Dutting, Mohammad Hajighayi, Jan Olkowski, and Kevin Schewior
   */

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
    /* printf("%lf\n", totalOpt[t]); */
  }
}

void findBestHand(double *reward, double *bestHand, double *totalBestHand,
                  uint64_t totalRounds, uint32_t totalThresholds) {
  // Works for maxItems>1 (yay)
  for (uint64_t t = 0; t < totalRounds; t++) {
    double max = -INFINITY;
    for (uint32_t th = 0; th < totalThresholds; th++) {
      if (reward[totalThresholds * t + th] > max) {
        max = reward[totalThresholds * t + th];
      }
    }
    bestHand[t] = max;
  }

  totalBestHand[0] = bestHand[0];
  for (uint64_t t = 1; t < totalRounds; t++) {
    totalBestHand[t] = totalBestHand[t - 1] + bestHand[t];
    /* printf("%lf\n", totalBestHand[t]); */
  }
}
