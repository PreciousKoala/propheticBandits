#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <util.h>

void initThreshold(Threshold *thres, Bandit b) {
    if (!b.dualThres) {
        for (uint32_t th = 0; th < b.K; th++) {
            thres[th].low = (th + 1.0) / (b.K + 1.0);
            thres[th].high = (th + 1.0) / (b.K + 1.0);
            thres[th].rewardSum = 0;
            thres[th].timesChosen = 0;
            thres[th].avgReward = 0;
        }
    } else {
        uint32_t th = 0;

        for (uint32_t l = 0; l < b.thresholds; l++) {
            for (uint32_t h = l; h < b.thresholds; h++) {
                thres[th].low = (l + 1.0) / (b.thresholds + 1.0);
                thres[th].high = (h + 1.0) / (b.thresholds + 1.0);
                thres[th].rewardSum = 0;
                thres[th].timesChosen = 0;
                thres[th].avgReward = 0;
                th++;
            }
        }
    }
}

double runRound(Threshold *thres, uint32_t th, Bandit b, double *data, double *avgLowThreshold,
                double *avgHighThreshold, double *avgTrades, double *totalGain, uint64_t round, uint8_t *heldItems,
                double *heldItemValue) {
    double low = thres[th].low;
    double high = thres[th].high;
    uint32_t trades = 0;

    if (!b.keepItems) { // extra check
        *heldItems = 0;
        *heldItemValue = 0;
    }

    double gain = runThreshold(low, high, b, data, &trades, round, heldItems, heldItemValue);

    if (round > 0) {
        avgTrades[round] = (avgTrades[round - 1] * (double) round + trades) / ((double) round + 1);
        avgLowThreshold[round] = (avgLowThreshold[round - 1] * (double) round + thres[th].low) / ((double) round + 1);
        avgHighThreshold[round] =
                (avgHighThreshold[round - 1] * (double) round + thres[th].high) / ((double) round + 1);
    } else {
        avgTrades[round] = trades;
        avgLowThreshold[round] = thres[th].low;
        avgHighThreshold[round] = thres[th].high;
    }

    thres[th].rewardSum += gain;
    thres[th].timesChosen++;
    if (thres[th].timesChosen != 0) {
        thres[th].avgReward = thres[th].rewardSum / (double) thres[th].timesChosen;
    }

    if (round == 0) {
        totalGain[0] = gain;
    } else {
        totalGain[round] = totalGain[round - 1] + gain;
    }

    return gain;
}

double runThreshold(double low, double high, Bandit b, double *data, uint32_t *trades, uint64_t round,
                    uint8_t *heldItems, double *heldItemValue) {
    double gain = 0;
    *trades = 0;
    for (uint64_t i = round * b.N; i < (round + 1) * b.N; i++) {
        uint8_t lastPrice = (b.keepItems && i == (b.T * b.N - 1)) || (!b.keepItems && ((i + 1) % b.N) == 0);
        if ((lastPrice || data[i] >= high) && *heldItems == 1) {
            gain += data[i] - *heldItemValue;
            *heldItems = 0;
            trades++;
        } else if (!lastPrice && data[i] < low && *heldItems == 0) {
            *heldItemValue = data[i];
            *heldItems = 1;
        }
    }

    return gain;
}

void normalizePrices(double min, double max, double *data, uint64_t size) {
    for (uint64_t t = 0; t < size; t++) {
        data[t] = (data[t] - min) / (max - min);
    }
}

void getAvgGain(uint64_t size, double *avgGain, double *totalGain) {
    for (uint64_t i = 0; i < size; i++) {
        avgGain[i] = totalGain[i] / (double) (i + 1);
    }
}

void getAvgRegret(uint64_t totalRounds, double *algAvgRegret, double *totalOpt, double *algGain) {
    for (uint64_t t = 0; t < totalRounds; t++) {
        algAvgRegret[t] = (totalOpt[t] - algGain[t]) / (double) (t + 1);
    }
}

void getCompRatio(uint64_t totalRounds, double *algCompRatio, double *totalOpt, double *algGain) {
    for (uint64_t t = 0; t < totalRounds; t++) {
        algCompRatio[t] = algGain[t] / totalOpt[t];
    }
}

void getAvgTradeGain(uint64_t totalRounds, double *algGain, double *algAvgTrades, double *algAvgTradeGain) {
    for (uint64_t t = 0; t < totalRounds; t++) {
        if (algAvgTrades[t] != 0) {
            algAvgTradeGain[t] = algGain[t] / (algAvgTrades[t] * (double) (t + 1));
        } else {
            algAvgTradeGain[t] = 0;
        }
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
    fprintf(gnuplot, "set yrange [0:1]\n");
    fprintf(gnuplot, "plot '-' using 1:2 with lines lc rgb 'black' lw 0.5 title 'Price'\n");

    for (int i = 0; i < size; i += step) {
        fprintf(gnuplot, "%d %lf\n", i, data[i]);
    }
    fprintf(gnuplot, "e\n");
    fflush(gnuplot);
    pclose(gnuplot);
}

void plotAlgorithms(char *ylabel, Bandit b, double *opt, double *median, double *greedy, double *eGreedy,
                    double *succElim, double *ucb1, double *ucb2, double *exp3, uint8_t bounded) {
    uint32_t step = 1;
    // bigger step if the dataset is bigger, makes plot way faster
    if (b.T > 10000) {
        step = b.T / 10000;
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
    else
        fprintf(gnuplot, "set yrange [0:]\n");

    fprintf(gnuplot, "set grid\n");
    fprintf(gnuplot, "set key outside\n");

    fprintf(gnuplot, "plot ");

    if (opt != NULL) {
        char *optTitle = "Best Threshold";
        if (!b.medianOpt && !b.bestHandOpt)
            optTitle = "OPT";
        else if (b.medianOpt)
            optTitle = "Median";

        fprintf(gnuplot, "'-' using 1:2 with lines lc rgb 'black' lw 1.5 title '%s', ", optTitle);
    }

    if (b.median) {
        fprintf(gnuplot, "'-' using 1:2 with linespoints lc rgb 'black' pt 7 ps 1 pn 20 lw 1.5 title 'Median', ");
    }

    if (b.greedy) {
        fprintf(gnuplot, "'-' using 1:2 with linespoints lc rgb 'orange' pt 1 ps 1 pn 20 lw 1.5 title 'Greedy', ");
    }

    if (b.eGreedy) {
        fprintf(gnuplot, "'-' using 1:2 with linespoints lc rgb 'red' pt 2 ps 1 pn 20 lw 1.5 title 'eGreedy', ");
    }

    if (b.succElim) {
        fprintf(gnuplot, "'-' using 1:2 with linespoints lc rgb 'cyan' pt 3 ps 1 pn 20 lw 1.5 title "
                         "'Successive Elimination', ");
    }

    if (b.ucb1) {
        fprintf(gnuplot, "'-' using 1:2 with linespoints lc rgb 'blue' pt 4 ps 1 pn 20 lw 1.5 title 'UCB1', ");
    }

    if (b.ucb2) {
        fprintf(gnuplot, "'-' using 1:2 with linespoints lc rgb 'purple' pt 5 ps 1 pn 20 lw 1.5 title 'UCB2', ");
    }

    if (b.exp3) {
        fprintf(gnuplot, "'-' using 1:2 with linespoints lc rgb 'green' pt 6 ps 1 pn 20 lw 1.5 title 'EXP3', ");
    }

    fprintf(gnuplot, "\n");

    if (opt != NULL) {
        for (uint64_t t = 0; t < b.T; t += step) {
            fprintf(gnuplot, "%lu %lf\n", t, opt[t]);
        }
        fprintf(gnuplot, "e\n");
    }

    if (b.median) {
        for (uint64_t t = 0; t < b.T; t += step) {
            fprintf(gnuplot, "%lu %lf\n", t, median[t]);
        }
        fprintf(gnuplot, "e\n");
    }

    if (b.greedy) {
        for (uint64_t t = 0; t < b.T; t += step) {
            fprintf(gnuplot, "%lu %lf\n", t, greedy[t]);
        }
        fprintf(gnuplot, "e\n");
    }

    if (b.eGreedy) {
        for (uint64_t t = 0; t < b.T; t += step) {
            fprintf(gnuplot, "%lu %lf\n", t, eGreedy[t]);
        }
        fprintf(gnuplot, "e\n");
    }

    if (b.succElim) {
        for (uint64_t t = 0; t < b.T; t += step) {
            fprintf(gnuplot, "%lu %lf\n", t, succElim[t]);
        }
        fprintf(gnuplot, "e\n");
    }

    if (b.ucb1) {
        for (uint64_t t = 0; t < b.T; t += step) {
            fprintf(gnuplot, "%lu %lf\n", t, ucb1[t]);
        }
        fprintf(gnuplot, "e\n");
    }

    if (b.ucb2) {
        for (uint64_t t = 0; t < b.T; t += step) {
            fprintf(gnuplot, "%lu %lf\n", t, ucb2[t]);
        }
        fprintf(gnuplot, "e\n");
    }

    if (b.exp3) {
        for (uint64_t t = 0; t < b.T; t += step) {
            fprintf(gnuplot, "%lu %lf\n", t, exp3[t]);
        }
        fprintf(gnuplot, "e\n");
    }

    fflush(gnuplot);
    pclose(gnuplot);
}

void plotThresholds(Bandit b, double *optLow, double *optHigh, double *median, double *greedyLow, double *greedyHigh,
                    double *eGreedyLow, double *eGreedyHigh, double *succElimLow, double *succElimHigh, double *ucb1Low,
                    double *ucb1High, double *ucb2Low, double *ucb2High, double *exp3Low, double *exp3High) {
    uint32_t step = 1;
    // bigger step if the dataset is bigger, makes plot way faster
    if (b.T > 10000) {
        step = b.T / 10000;
    }

    FILE *gnuplot = popen("gnuplot -persistent", "w");
    if (!gnuplot) {
        exit(1);
    }
    fprintf(gnuplot, "set xlabel 'Rounds'\n");
    fprintf(gnuplot, "set ylabel 'Average Threshold'\n");
    fprintf(gnuplot, "set yrange [0:1]\n");
    fprintf(gnuplot, "set grid\n");
    fprintf(gnuplot, "set key outside\n");

    fprintf(gnuplot, "plot ");

    if (b.medianOpt || b.bestHandOpt) {
        char *optTitle = "Best Threshold";
        if (b.medianOpt)
            optTitle = "Median";

        fprintf(gnuplot, "'-' using 1:2 with lines lc rgb 'black' lw 1.5 title '%s', ", optTitle);
        if (b.dualThres && !b.medianOpt)
            fprintf(gnuplot, "'' using 1:2 with lines lc rgb 'black' lw 1.5 notitle, ");
    }

    if (b.median && !b.medianOpt) {
        fprintf(gnuplot, "'' using 1:2 with linespoints lc rgb 'black' pt 7 ps 1 pn 20 lw 1.5 title 'Median', ");
    }

    if (b.greedy) {
        fprintf(gnuplot, "'-' using 1:2 with linespoints lc rgb 'orange' pt 1 ps 1 pn 20 lw 1.5 title 'Greedy', ");
        if (b.dualThres)
            fprintf(gnuplot, "'' using 1:2 with linespoints lc rgb 'orange' pt 1 ps 1 pn 20 lw 1.5 notitle, ");
    }

    if (b.eGreedy) {
        fprintf(gnuplot, "'-' using 1:2 with linespoints lc rgb 'red' pt 2 ps 1 pn 20 lw 1.5 title 'eGreedy', ");
        if (b.dualThres)
            fprintf(gnuplot, "'' using 1:2 with linespoints lc rgb 'red' pt 2 ps 1 pn 20 lw 1.5 notitle, ");
    }

    if (b.succElim) {
        fprintf(gnuplot, "'-' using 1:2 with linespoints lc rgb 'cyan' pt 3 ps 1 pn 20 lw 1.5 title "
                         "'Successive Elimination', ");
        if (b.dualThres)
            fprintf(gnuplot, "'' using 1:2 with linespoints lc rgb 'cyan' pt 3 ps 1 pn 20 lw 1.5 notitle, ");
    }

    if (b.ucb1) {
        fprintf(gnuplot, "'-' using 1:2 with linespoints lc rgb 'blue' pt 4 ps 1 pn 20 lw 1.5 title 'UCB1', ");
        if (b.dualThres)
            fprintf(gnuplot, "'' using 1:2 with linespoints lc rgb 'blue' pt 4 ps 1 pn 20 lw 1.5 notitle, ");
    }

    if (b.ucb2) {
        fprintf(gnuplot, "'-' using 1:2 with linespoints lc rgb 'purple' pt 5 ps 1 pn 20 lw 1.5 title 'UCB2', ");
        if (b.dualThres)
            fprintf(gnuplot, "'' using 1:2 with linespoints lc rgb 'purple' pt 5 ps 1 pn 20 lw 1.5 notitle, ");
    }

    if (b.exp3) {
        fprintf(gnuplot, "'-' using 1:2 with linespoints lc rgb 'green' pt 6 ps 1 pn 20 lw 1.5 title 'EXP3', ");
        if (b.dualThres)
            fprintf(gnuplot, "'' using 1:2 with linespoints lc rgb 'green' pt 6 ps 1 pn 20 lw 1.5 notitle, ");
    }

    fprintf(gnuplot, "\n");

    if (b.medianOpt || b.bestHandOpt) {
        for (uint64_t t = 0; t < b.T; t += step) {
            fprintf(gnuplot, "%lu %lf\n", t, optLow[t]);
        }
        fprintf(gnuplot, "e\n");
        if (b.dualThres && !b.medianOpt) {
            for (uint64_t t = 0; t < b.T; t += step) {
                fprintf(gnuplot, "%lu %lf\n", t, optHigh[t]);
            }
            fprintf(gnuplot, "e\n");
        }
    }

    if (b.median && !b.medianOpt) {
        for (uint64_t t = 0; t < b.T; t += step) {
            fprintf(gnuplot, "%lu %lf\n", t, median[t]);
        }
        fprintf(gnuplot, "e\n");
    }

    if (b.greedy) {
        for (uint64_t t = 0; t < b.T; t += step) {
            fprintf(gnuplot, "%lu %lf\n", t, greedyLow[t]);
        }
        fprintf(gnuplot, "e\n");
        if (b.dualThres) {
            for (uint64_t t = 0; t < b.T; t += step) {
                fprintf(gnuplot, "%lu %lf\n", t, greedyHigh[t]);
            }
            fprintf(gnuplot, "e\n");
        }
    }

    if (b.eGreedy) {
        for (uint64_t t = 0; t < b.T; t += step) {
            fprintf(gnuplot, "%lu %lf\n", t, eGreedyLow[t]);
        }
        fprintf(gnuplot, "e\n");
        if (b.dualThres) {
            for (uint64_t t = 0; t < b.T; t += step) {
                fprintf(gnuplot, "%lu %lf\n", t, eGreedyHigh[t]);
            }
            fprintf(gnuplot, "e\n");
        }
    }

    if (b.succElim) {
        for (uint64_t t = 0; t < b.T; t += step) {
            fprintf(gnuplot, "%lu %lf\n", t, succElimLow[t]);
        }
        fprintf(gnuplot, "e\n");
        if (b.dualThres) {
            for (uint64_t t = 0; t < b.T; t += step) {
                fprintf(gnuplot, "%lu %lf\n", t, succElimHigh[t]);
            }
            fprintf(gnuplot, "e\n");
        }
    }

    if (b.ucb1) {
        for (uint64_t t = 0; t < b.T; t += step) {
            fprintf(gnuplot, "%lu %lf\n", t, ucb1Low[t]);
        }
        fprintf(gnuplot, "e\n");
        if (b.dualThres) {
            for (uint64_t t = 0; t < b.T; t += step) {
                fprintf(gnuplot, "%lu %lf\n", t, ucb1High[t]);
            }
            fprintf(gnuplot, "e\n");
        }
    }

    if (b.ucb2) {
        for (uint64_t t = 0; t < b.T; t += step) {
            fprintf(gnuplot, "%lu %lf\n", t, ucb2Low[t]);
        }
        fprintf(gnuplot, "e\n");
        if (b.dualThres) {
            for (uint64_t t = 0; t < b.T; t += step) {
                fprintf(gnuplot, "%lu %lf\n", t, ucb2High[t]);
            }
            fprintf(gnuplot, "e\n");
        }
    }

    if (b.exp3) {
        for (uint64_t t = 0; t < b.T; t += step) {
            fprintf(gnuplot, "%lu %lf\n", t, exp3Low[t]);
        }
        fprintf(gnuplot, "e\n");
        if (b.dualThres) {
            for (uint64_t t = 0; t < b.T; t += step) {
                fprintf(gnuplot, "%lu %lf\n", t, exp3High[t]);
            }
            fprintf(gnuplot, "e\n");
        }
    }

    fflush(gnuplot);
    pclose(gnuplot);
}
