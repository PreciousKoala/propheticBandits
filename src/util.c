#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <util.h>

#include <math.h>

void initThreshold(Threshold *thres, uint32_t totalThresholds) {
    for (uint32_t th = 0; th < totalThresholds; th++) {
        thres[th].threshold = (double) (th + 0.5) / totalThresholds;
        thres[th].rewardSum = 0;
        thres[th].timesChosen = 0;
        thres[th].avgReward = 0;
    }
}

void runRound(Threshold *thres, uint32_t th, uint32_t totalThresholds, double *reward, double *avgThreshold,
              double *totalGain, uint64_t round) {
    double gain = reward[totalThresholds * round + th];

    if (round > 0)
        avgThreshold[round] = (avgThreshold[round - 1] * round + thres[th].threshold) / (round + 1);
    else
        avgThreshold[round] = thres[th].threshold;

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

void normalizePrices(double min, double max, double *data, uint64_t totalRounds, uint64_t pricesPerRound) {
    for (uint64_t t = 0; t < totalRounds * pricesPerRound; t++) {
        data[t] = (data[t] - min) / (max - min);
    }
}

void calculateRewards(double *reward, double *data, uint64_t totalRounds, uint64_t pricesPerRound,
                      uint32_t totalThresholds) {
    uint8_t heldItems = 0;
    for (uint32_t th = 0; th < totalThresholds; th++) {
        double threshold = (th + 0.5) / totalThresholds;

        for (uint64_t t = 0; t < totalRounds; t++) {
            double gain = 0;

            for (uint32_t n = 0; n < pricesPerRound; n++) {
                if ((n == pricesPerRound - 1 || data[pricesPerRound * t + n] >= threshold) && heldItems == 1) {
                    gain += data[pricesPerRound * t + n];
                    heldItems = 0;
                } else if (n != pricesPerRound - 1 && data[pricesPerRound * t + n] < threshold &&
                           heldItems == 0) {
                    gain -= data[pricesPerRound * t + n];
                    heldItems = 1;
                }
            }
            reward[totalThresholds * t + th] = gain;
        }
    }
}

void getAvgRegret(uint64_t totalRounds, double *algAvgRegret, double *totalOpt, double *algGain) {
    for (uint64_t t = 0; t < totalRounds; t++) {
        algAvgRegret[t] = (totalOpt[t] - algGain[t]) / (t + 1);
    }
}

void getBestHandDistance(uint64_t totalRounds, double *algThresholdDist, double *bestAvgThreshold, double *algAvgThreshold) {
    for (uint64_t t = 1; t < totalRounds; t++) {
        algThresholdDist[t] = fabs(bestAvgThreshold[t] - algAvgThreshold[t]);
    }
}

void plotAlgorithms(char *ylabel, uint64_t totalRounds, double *greedy, double *eGreedy, double *succElim, double *ucb1,
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
    fprintf(gnuplot, "set xlabel 'Rounds'\n");
    fprintf(gnuplot, "set ylabel '");
    fprintf(gnuplot, "%s", ylabel);
    fprintf(gnuplot, "'\n");
    fprintf(gnuplot, "set yrange [0:1]\n");
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
        fprintf(gnuplot, "'-' using 1:2 with lines lt rgb 'blue' lw 2 title 'UCB1', ");
    }

    if (flag.ucb2) {
        fprintf(gnuplot, "'-' using 1:2 with lines lt rgb 'purple' lw 2 title 'UCB2', ");
    }

    if (flag.exp3) {
        fprintf(gnuplot, "'-' using 1:2 with lines lt rgb 'green' lw 2 title 'EXP3', ");
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
