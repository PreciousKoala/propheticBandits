#ifndef UTIL_C
#define UTIL_C

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  uint8_t greedyFlag;
  uint8_t eGreedyFlag;
  uint8_t ucb1Flag;
  uint8_t ucb2Flag;
  uint8_t exp3Flag;
} Flag;

typedef struct {
  // the placement of the threshold in [0,1]
  double threshold;
  // how much money the threshold has made
  double rewardSum;
  // how many times the threshold has been picked
  uint64_t timesChosen;
  // how much money on average the threshold has made
  double avgReward;
} Threshold;

void initThreshold(Threshold *thres, uint32_t totalThresholds) {
  for (uint32_t th = 0; th < totalThresholds; th++) {
    thres[th].threshold = (double)th / totalThresholds;
    thres[th].rewardSum = 0;
    thres[th].timesChosen = 0;
    thres[th].avgReward = 0;
  }
}

void runRound(Threshold *thres, uint32_t th, uint32_t totalThresholds,
              double *reward, double *totalGain, uint64_t round) {
  double gain = reward[totalThresholds * round + th];

  thres[th].rewardSum += gain;
  thres[th].timesChosen++;
  if (thres[th].timesChosen != 0) {
    thres[th].avgReward = thres[th].rewardSum / thres[th].timesChosen;
  }
  if (round == 0) {
    totalGain[round] = gain;
  } else {
    totalGain[round] = totalGain[round - 1] + gain;
  }
}

void normalizePrices(double min, double max, double *data, uint64_t totalRounds,
                     uint64_t pricesPerRound) {
  for (uint64_t t = 0; t < totalRounds * pricesPerRound; t++) {
    data[t] = (data[t] - min) / (max - min);
  }
}

void calculateRewards(double *reward, double *data, uint64_t totalRounds,
                      uint64_t pricesPerRound, uint32_t totalThresholds,
                      uint32_t maxItems) {
  uint32_t heldItems = 0;
  for (uint32_t th = 0; th < totalThresholds; th++) {
    double threshold = (double)th / totalThresholds;

    for (uint64_t t = 0; t < totalRounds; t++) {
      double gain = 0;

      for (uint32_t n = 0; n < pricesPerRound; n++) {
        if ((pricesPerRound - n == heldItems ||
             data[pricesPerRound * t + n] >= threshold) &&
            heldItems > 0) {
          gain += data[pricesPerRound * t + n];
          heldItems--;
        } else if (pricesPerRound - n - 1 != heldItems &&
                   data[pricesPerRound * t + n] < threshold &&
                   heldItems < maxItems) {
          gain -= data[pricesPerRound * t + n];
          heldItems++;
        }
      }
      reward[totalThresholds * t + th] = gain;
    }
  }
}

void plotRegret(uint64_t totalRounds, double *totalOpt, double *greedyGain,
                double *eGreedyGain, double *ucb1Gain, double *ucb2Gain,
                double *exp3Gain, Flag flag) {
  uint32_t step = 1;
  // bigger step if the dataset is bigger, makes plot way faster
  if (totalRounds > 10000) {
    step = totalRounds / 10000;
  }

  FILE *gnuplot = popen("gnuplot -persistent", "w");
  if (!gnuplot) {
    exit(1);
  }
  fprintf(gnuplot, "set title 'Regret Plot'\n");
  fprintf(gnuplot, "set xlabel 'Rounds'\n");
  fprintf(gnuplot, "set ylabel 'Regret'\n");
  fprintf(gnuplot, "set grid\n");
  fprintf(gnuplot, "set key left top\n");

  fprintf(gnuplot, "plot ");

  if (flag.greedyFlag) {
    fprintf(gnuplot, "'-' using 1:2 with lines lt rgb 'orange' lw 2 title "
                     "'Greedy Regret', ");
  }

  if (flag.eGreedyFlag) {
    fprintf(gnuplot, "'-' using 1:2 with lines lt rgb 'red' lw 2 title "
                     "'eGreedy Regret', ");
  }

  if (flag.ucb1Flag) {
    fprintf(
        gnuplot,
        "'-' using 1:2 with lines lt rgb 'blue' lw 2 title 'UCB1 Regret', ");
  }

  if (flag.ucb2Flag) {
    fprintf(
        gnuplot,
        "'-' using 1:2 with lines lt rgb 'purple' lw 2 title 'UCB2 Regret', ");
  }

  if (flag.exp3Flag) {
    fprintf(
        gnuplot,
        "'-' using 1:2 with lines lt rgb 'green' lw 2 title 'EXP3 Regret', ");
  }

  fprintf(gnuplot, "\n");

  if (flag.greedyFlag) {
    for (int t = 0; t < totalRounds; t += step) {
      fprintf(gnuplot, "%d %lf\n", t, (totalOpt[t] - greedyGain[t]) / (t + 1));
    }
    fprintf(gnuplot, "e\n");
  }

  if (flag.eGreedyFlag) {
    for (int t = 0; t < totalRounds; t += step) {
      fprintf(gnuplot, "%d %lf\n", t, (totalOpt[t] - eGreedyGain[t]) / (t + 1));
    }
    fprintf(gnuplot, "e\n");
  }

  if (flag.ucb1Flag) {
    for (int t = 0; t < totalRounds; t += step) {
      fprintf(gnuplot, "%d %lf\n", t, (totalOpt[t] - ucb1Gain[t]) / (t + 1));
    }
    fprintf(gnuplot, "e\n");
  }

  if (flag.ucb2Flag) {
    for (int t = 0; t < totalRounds; t += step) {
      fprintf(gnuplot, "%d %lf\n", t, (totalOpt[t] - ucb2Gain[t]) / (t + 1));
    }
    fprintf(gnuplot, "e\n");
  }

  if (flag.exp3Flag) {
    for (int t = 0; t < totalRounds; t += step) {
      fprintf(gnuplot, "%d %lf\n", t, (totalOpt[t] - exp3Gain[t]) / (t + 1));
    }
    fprintf(gnuplot, "e\n");
  }

  fflush(gnuplot);
  pclose(gnuplot);
}

#endif
