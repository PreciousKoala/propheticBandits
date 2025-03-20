#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <util.h>

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

void getAvgRegret(uint64_t totalRounds, double *algAvgRegret, double *totalOpt,
                  double *algGain) {
  for (uint64_t t = 0; t < totalRounds; t++) {
    algAvgRegret[t] = (totalOpt[t] - algGain[t]) / (t + 1);
  }
}

void getBestHandPerc(uint64_t totalRounds, double *algBestHand,
                     double *totalOpt, double *algGain) {
  algBestHand[0] = 0;
  if (algBestHand[0] == totalOpt[0]) {
    algBestHand[0] = 1.0;
  }

  for (uint64_t t = 1; t < totalRounds; t++) {
    algBestHand[t] = algBestHand[t - 1];

    if (algGain[t] - algGain[t - 1] == totalOpt[t] - totalOpt[t - 1]) {
      algBestHand[t] += 1.0;
    }
  }

  for (uint64_t t = 1; t < totalRounds; t++) {
    algBestHand[t] = 100 * algBestHand[t] / (t + 1);
  }
}

void plotAlgorithms(char *title, uint64_t totalRounds, double *greedy,
                    double *eGreedy, double *succElim, double *ucb1,
                    double *ucb2, double *exp3, Flag flag) {
  uint32_t step = 1;
  // bigger step if the dataset is bigger, makes plot way faster
  if (totalRounds > 10000) {
    step = totalRounds / 10000;
  }

  FILE *gnuplot = popen("gnuplot -persistent", "w");
  if (!gnuplot) {
    exit(1);
  }
  fprintf(gnuplot, "set title '");
  fprintf(gnuplot, "%s", title);
  fprintf(gnuplot, "'\n");
  fprintf(gnuplot, "set xlabel 'Rounds'\n");
  fprintf(gnuplot, "set ylabel 'Regret'\n");
  fprintf(gnuplot, "set grid\n");
  fprintf(gnuplot, "set key left top\n");

  fprintf(gnuplot, "plot ");

  if (flag.greedy) {
    fprintf(gnuplot, "'-' using 1:2 with lines lt rgb 'orange' lw 2 title "
                     "'Greedy', ");
  }

  if (flag.eGreedy) {
    fprintf(gnuplot, "'-' using 1:2 with lines lt rgb 'red' lw 2 title "
                     "'eGreedy', ");
  }

  if (flag.succElim) {
    fprintf(gnuplot, "'-' using 1:2 with lines lt rgb 'cyan' lw 2 title "
                     "'Successive Elimination', ");
  }

  if (flag.ucb1) {
    fprintf(gnuplot,
            "'-' using 1:2 with lines lt rgb 'blue' lw 2 title 'UCB1', ");
  }

  if (flag.ucb2) {
    fprintf(gnuplot,
            "'-' using 1:2 with lines lt rgb 'purple' lw 2 title 'UCB2', ");
  }

  if (flag.exp3) {
    fprintf(gnuplot,
            "'-' using 1:2 with lines lt rgb 'green' lw 2 title 'EXP3', ");
  }

  fprintf(gnuplot, "\n");

  if (flag.greedy) {
    for (int t = 0; t < totalRounds; t += step) {
      fprintf(gnuplot, "%d %lf\n", t, greedy[t]);
    }
    fprintf(gnuplot, "e\n");
  }

  if (flag.eGreedy) {
    for (int t = 0; t < totalRounds; t += step) {
      fprintf(gnuplot, "%d %lf\n", t, eGreedy[t]);
    }
    fprintf(gnuplot, "e\n");
  }

  if (flag.succElim) {
    for (int t = 0; t < totalRounds; t += step) {
      fprintf(gnuplot, "%d %lf\n", t, succElim[t]);
    }
    fprintf(gnuplot, "e\n");
  }

  if (flag.ucb1) {
    for (int t = 0; t < totalRounds; t += step) {
      fprintf(gnuplot, "%d %lf\n", t, ucb1[t]);
    }
    fprintf(gnuplot, "e\n");
  }

  if (flag.ucb2) {
    for (int t = 0; t < totalRounds; t += step) {
      fprintf(gnuplot, "%d %lf\n", t, ucb2[t]);
    }
    fprintf(gnuplot, "e\n");
  }

  if (flag.exp3) {
    for (int t = 0; t < totalRounds; t += step) {
      fprintf(gnuplot, "%d %lf\n", t, exp3[t]);
    }
    fprintf(gnuplot, "e\n");
  }

  fflush(gnuplot);
  pclose(gnuplot);
}
