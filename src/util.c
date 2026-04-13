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

double runRound(Threshold *thres, uint32_t th, uint64_t totalRounds, uint64_t pricesPerRound, double *data,
                double *avgThreshold, double *avgTrades, double *totalGain, uint64_t round, uint8_t *heldItems,
                double norm) {
    double gain = 0;
    double threshold = thres[th].threshold / norm;
    avgTrades[round] = 0;

    for (uint64_t i = round * pricesPerRound; i < (round + 1) * pricesPerRound; i++) {
        if ((i == totalRounds * pricesPerRound - 1 || data[i] >= threshold) && *heldItems == 1) {
            gain += data[i];
            *heldItems = 0;
            avgTrades[round]++;
        } else if (data[i] < threshold && *heldItems == 0) {
            gain -= data[i];
            *heldItems = 1;
        }
    }

    if (round > 0) {
        avgTrades[round] = (avgTrades[round - 1] * round + avgTrades[round]) / (round + 1);
        avgThreshold[round] = (avgThreshold[round - 1] * round + thres[th].threshold) / (round + 1);
    } else {
        avgThreshold[round] = thres[th].threshold;
    }

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

    return gain;
}

void normalizePrices(double min, double max, double *data, uint64_t size) {
    for (uint64_t t = 0; t < size; t++) {
        data[t] = (data[t] - min) / (max - min);
    }
}

void getAvgRegret(uint64_t totalRounds, double *algAvgRegret, double *totalOpt, double *algGain) {
    for (uint64_t t = 0; t < totalRounds; t++) {
        algAvgRegret[t] = (totalOpt[t] - algGain[t]) / (t + 1);
    }
}

void getCompRatio(uint64_t totalRounds, double *algCompRatio, double *totalOpt, double *algGain) {
    for (uint64_t t = 0; t < totalRounds; t++) {
        algCompRatio[t] = algGain[t] / totalOpt[t];
    }
}

void getAvgTradeGain(uint64_t totalRounds, double *algGain, double *algAvgTrades, double *algAvgTradeGain) {
    for (uint64_t t = 0; t < totalRounds; t++) {
        algAvgTradeGain[t] = algGain[t] / (algAvgTrades[t] * (t + 1));
    }
}

void plotData(double *data, uint64_t size) {
    uint32_t step = 1;
    // bigger step if the dataset is bigger, makes plot way faster
    if (size > 10000) {
        step = size / 10000;
    }

    FILE *gnuplot = popen("gnuplot -persistent", "w");
    if (!gnuplot) {
        exit(1);
    }

    fprintf(gnuplot, "set ylabel 'Price'\n");
    fprintf(gnuplot, "set grid\n");
    fprintf(gnuplot, "plot '-' using 1:2 with lines lc rgb 'black' lw 0.5 title 'Price'\n");

    for (int i = 0; i < size; i += step) {
        fprintf(gnuplot, "%d %lf\n", i, data[i]);
    }
    fprintf(gnuplot, "e\n");
    fflush(gnuplot);
    pclose(gnuplot);
}

void plotAlgorithms(char *ylabel, uint64_t totalRounds, double *opt, double *median, double *greedy, double *eGreedy,
                    double *succElim, double *ucb1, double *ucb2, double *exp3, Flag flag, uint8_t bounded) {
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
    if (bounded)
        fprintf(gnuplot, "set yrange [0:1]\n");
    fprintf(gnuplot, "set grid\n");
    fprintf(gnuplot, "set key right top spacing .5 font ',8'\n");

    fprintf(gnuplot, "plot ");

    if (opt != NULL) {
        fprintf(gnuplot, "'-' using 1:2 with lines lc rgb 'black' lw 1.5 title 'OPT', ");
    }

    if (flag.median) {
        fprintf(gnuplot, "'-' using 1:2 with linespoints lc rgb 'black' pt 7 ps 1 pn 20 lw 1.5 title 'Median', ");
    }

    if (flag.greedy) {
        fprintf(gnuplot, "'-' using 1:2 with linespoints lc rgb 'orange' pt 1 ps 1 pn 20 lw 1.5 title 'Greedy', ");
    }

    if (flag.eGreedy) {
        fprintf(gnuplot, "'-' using 1:2 with linespoints lc rgb 'red' pt 2 ps 1 pn 20 lw 1.5 title 'eGreedy', ");
    }

    if (flag.succElim) {
        fprintf(gnuplot, "'-' using 1:2 with linespoints lc rgb 'cyan' pt 3 ps 1 pn 20 lw 1.5 title "
                         "'Successive Elimination', ");
    }

    if (flag.ucb1) {
        fprintf(gnuplot, "'-' using 1:2 with linespoints lc rgb 'blue' pt 4 ps 1 pn 20 lw 1.5 title 'UCB1', ");
    }

    if (flag.ucb2) {
        fprintf(gnuplot, "'-' using 1:2 with linespoints lc rgb 'purple' pt 5 ps 1 pn 20 lw 1.5 title 'UCB2', ");
    }

    if (flag.exp3) {
        fprintf(gnuplot, "'-' using 1:2 with linespoints lc rgb 'green' pt 6 ps 1 pn 20 lw 1.5 title 'EXP3', ");
    }

    fprintf(gnuplot, "\n");

    if (opt != NULL) {
        for (uint64_t t = 0; t < totalRounds; t += step) {
            fprintf(gnuplot, "%lu %lf\n", t, opt[t]);
        }
        fprintf(gnuplot, "e\n");
    }

    if (flag.median) {
        for (uint64_t t = 0; t < totalRounds; t += step) {
            fprintf(gnuplot, "%lu %lf\n", t, median[t]);
        }
        fprintf(gnuplot, "e\n");
    }

    if (flag.greedy) {
        for (uint64_t t = 0; t < totalRounds; t += step) {
            fprintf(gnuplot, "%lu %lf\n", t, greedy[t]);
        }
        fprintf(gnuplot, "e\n");
    }

    if (flag.eGreedy) {
        for (uint64_t t = 0; t < totalRounds; t += step) {
            fprintf(gnuplot, "%lu %lf\n", t, eGreedy[t]);
        }
        fprintf(gnuplot, "e\n");
    }

    if (flag.succElim) {
        for (uint64_t t = 0; t < totalRounds; t += step) {
            fprintf(gnuplot, "%lu %lf\n", t, succElim[t]);
        }
        fprintf(gnuplot, "e\n");
    }

    if (flag.ucb1) {
        for (uint64_t t = 0; t < totalRounds; t += step) {
            fprintf(gnuplot, "%lu %lf\n", t, ucb1[t]);
        }
        fprintf(gnuplot, "e\n");
    }

    if (flag.ucb2) {
        for (uint64_t t = 0; t < totalRounds; t += step) {
            fprintf(gnuplot, "%lu %lf\n", t, ucb2[t]);
        }
        fprintf(gnuplot, "e\n");
    }

    if (flag.exp3) {
        for (uint64_t t = 0; t < totalRounds; t += step) {
            fprintf(gnuplot, "%lu %lf\n", t, exp3[t]);
        }
        fprintf(gnuplot, "e\n");
    }

    fflush(gnuplot);
    pclose(gnuplot);
}
