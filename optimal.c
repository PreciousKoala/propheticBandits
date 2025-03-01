#include <stdint.h>

void findOpt(double *data, double *optAlg, uint8_t keepItemsFlag,
             uint8_t maxItems, uint64_t totalRounds, uint64_t pricesPerRound) {
  // TODO: do the same but for maxItems>1 and keepItemsFlag=1 (what the fuck)

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
}

void findTotalOpt(double *data, double *optAlg, double *totalOpt,
                  uint64_t totalRounds) {
  totalOpt[0] = optAlg[0];
  for (uint64_t t = 1; t < totalRounds; t++) {
    totalOpt[t] = totalOpt[t - 1] + optAlg[t];
    /* printf("%lf\n", totalOpt[t]); */
  }
}
