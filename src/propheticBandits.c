#include <getopt.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_statistics_double.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <banditAlgs.h>
#include <util.h>

void printHelp() {
    printf("Usage:\n"
           "    propheticBandits [options] [-t <integer>] "
           "<file>\n"
           "    propheticBandits -h      # Display this help screen.\n\n"
           "Options:\n"
           "    -t <integer>    Set the number of thresholds (default = 10).\n"
           "    -n              Don't plot any statistics.\n"
           "    -a              Run all the available algorithms.\n"
           "    -m              Run the Median algorithm.\n"
           "    -g              Run the Greedy algorithm.\n"
           "    -e              Run the Epsilon Greedy algorithm.\n"
           "    -s              Run the Successive Elimination algorithm.\n"
           "    -u              Run the UCB1 algorithm.\n"
           "    -U              Run the UCB2 algorithm.\n"
           "    -x              Run the EXP3 algorithm.\n");
}

int main(int argc, char **argv) {
    if (argc == 1) {
        printHelp();
        return 0;
    }

    uint32_t totalThresholds = 10;
    Flag flag = {0, 0, 0, 0, 0, 0, 0};
    uint8_t plot = 1;

    int opt;

    opterr = 0;

    while ((opt = getopt(argc, argv, ":h:namgesuUxt:")) != -1) {
        switch (opt) {
            case 'h':
                printHelp();
                return 0;
            case 't':
                totalThresholds = atoi(optarg);
                break;
            case 'n':
                plot = 0;
                break;
            case 'a':
                flag = (Flag) {1, 1, 1, 1, 1, 1, 1};
            case 'm':
                flag.median = 1;
                break;
            case 'g':
                flag.greedy = 1;
                break;
            case 'e':
                flag.eGreedy = 1;
                break;
            case 's':
                flag.succElim = 1;
                break;
            case 'u':
                flag.ucb1 = 1;
                break;
            case 'U':
                flag.ucb2 = 1;
                break;
            case 'x':
                flag.exp3 = 1;
                break;
            case '?':
                if (optopt == 't')
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                break;
            default:
                abort();
        }
    }

    /* INFO: Data array holds all of the prices found in the data file.
     * It's also a 1D array for values that are better suited in a 2D array, but
     * because of the sheer size of our data, it's better to save them like
     * this. Access the n'th price of the t'th round with
     * data[pricesPerRound * t + n].
     *
     * WARN: Don't run this program for very big files
     */
    double *data;
    uint64_t totalRounds, pricesPerRound;
    // opens binary data file
    if (optind < argc) {
        // small hack to get first non-option argument because getopt is a pain
        char *filename = argv[optind];
        FILE *file = fopen(filename, "rb");
        if (!file) {
            printf("Error opening file\n");
            return 1;
        }

        printf("Importing file...\n");

        // first 2 values are 64bit integers
        if (!fread(&totalRounds, sizeof(uint64_t), 1, file)) {
            printf("Error while importing file");
            return 1;
        }
        if (!fread(&pricesPerRound, sizeof(uint64_t), 1, file)) {
            printf("Error while importing file");
            return 1;
        }

        if (pricesPerRound <= 2) {
            printf("Error: Program does not support 2 prices per round\n");
            return 1;
        }

        // allocate memory and get the actual data
        data = malloc(totalRounds * pricesPerRound * sizeof(double));
        if (!fread(data, sizeof(double), totalRounds * pricesPerRound, file)) {
            printf("Error while importing file");
            free(data);
            return 1;
        }

        fclose(file);
    } else {
        printf("Error: No filename provided\n");
        return 1;
    }

    double dataMin, dataMax;
    gsl_stats_minmax(&dataMin, &dataMax, data, 1, totalRounds * pricesPerRound);

    // Normalize prices to [0, 1]
    printf("Normalizing prices to [0,1]...\n");
    normalizePrices(dataMin, dataMax, data, totalRounds * pricesPerRound);

    printf("Calculating optimal result (local extrema)...\n");
    double *totalOpt = malloc(totalRounds * sizeof(double));
    double *avgOpt = malloc(totalRounds * sizeof(double));
    double *optAvgTrades = malloc(totalRounds * sizeof(double));
    double *optAvgTradeGain = malloc(totalRounds * sizeof(double));

    findOpt(data, totalOpt, optAvgTrades, totalRounds, pricesPerRound);
    getAvgGain(totalRounds, avgOpt, totalOpt);
    getAvgTradeGain(totalRounds, totalOpt, optAvgTrades, optAvgTradeGain);

    /*
     * Both bandit algorithms and trading prophets benefit greatly when X_i belongs in [0,1], especially when analyzing
     * regret. Combining these 2 problems requires further normalization. Choices include:
     *
     * -Dividing by the maximum possible reward of a day.
     *
     * -Dividing by the average reward of Extrema OPT. Good for regret analysis, but some rewards may be
     * higher than 1.
     *
     * -Normalizing with the min and max reward a threshold can achieve. Regret analysis and normalization for Extrema
     * OPT is difficult.
     *
     * -Dividing by the maximum theoretical reward of a day. Assuming prices are {0,1,0,1,...}, the maximum
     * reward is N/2. This is easy to implement, but it probably affects algorithm performance.
     *
     * Any algorithms that require rewards to be bounded in [0,1] (SuccElim, UCB1, UCB2, EXP3) will use normalized
     * rewards.
     */

    // Normalize prices to [0, 1]
    // printf("Further normalization according to OPT...\n");
    //
    // double optMax = -INFINITY;
    // for (uint64_t t = 1; t < totalRounds; t++) {
    //     double roundOpt = totalOpt[t] - totalOpt[t - 1];
    //     if (roundOpt > optMax)
    //         optMax = roundOpt;
    // }
    // double norm = optMax;
    // double norm = ceil((double) pricesPerRound / 2);

    // for (uint64_t t = 0; t < totalRounds; t++) {
    //     totalOpt[t] = totalOpt[t] / norm;
    // }
    // for (int i = 0; i < totalRounds * pricesPerRound; i++) {
    //     data[i] = data[i] / norm;
    // }

    double *medianAvgGain = malloc(totalRounds * sizeof(double));
    double *medianAvgRegret = malloc(totalRounds * sizeof(double));
    double *medianCompRatio = malloc(totalRounds * sizeof(double));
    double *medianAvgTrades = malloc(totalRounds * sizeof(double));
    double *medianAvgThreshold = malloc(totalRounds * sizeof(double));
    double *medianAvgTradeGain = malloc(totalRounds * sizeof(double));
    if (flag.median) {
        double *medianGain = malloc(totalRounds * sizeof(double));
        printf("Calculating Median...\n");
        median(data, medianGain, medianAvgThreshold, medianAvgTrades, totalOpt, totalRounds, pricesPerRound);

        getAvgGain(totalRounds, medianAvgGain, medianGain);
        getAvgRegret(totalRounds, medianAvgRegret, totalOpt, medianGain);
        getCompRatio(totalRounds, medianCompRatio, totalOpt, medianGain);
        getAvgTradeGain(totalRounds, medianGain, medianAvgTrades, medianAvgTradeGain);
        free(medianGain);
    }

    double *greedyAvgGain = malloc(totalRounds * sizeof(double));
    double *greedyAvgRegret = malloc(totalRounds * sizeof(double));
    double *greedyCompRatio = malloc(totalRounds * sizeof(double));
    double *greedyAvgTrades = malloc(totalRounds * sizeof(double));
    double *greedyAvgThreshold = malloc(totalRounds * sizeof(double));
    double *greedyAvgTradeGain = malloc(totalRounds * sizeof(double));
    if (flag.greedy) {
        double *greedyGain = malloc(totalRounds * sizeof(double));
        printf("Calculating Greedy...\n");
        greedy(data, greedyGain, greedyAvgThreshold, greedyAvgTrades, totalOpt, totalThresholds, totalRounds,
               pricesPerRound);

        getAvgGain(totalRounds, greedyAvgGain, greedyGain);
        getAvgRegret(totalRounds, greedyAvgRegret, totalOpt, greedyGain);
        getCompRatio(totalRounds, greedyCompRatio, totalOpt, greedyGain);
        getAvgTradeGain(totalRounds, greedyGain, greedyAvgTrades, greedyAvgTradeGain);
        free(greedyGain);
    }

    double *eGreedyAvgGain = malloc(totalRounds * sizeof(double));
    double *eGreedyAvgRegret = malloc(totalRounds * sizeof(double));
    double *eGreedyCompRatio = malloc(totalRounds * sizeof(double));
    double *eGreedyAvgTrades = malloc(totalRounds * sizeof(double));
    double *eGreedyAvgThreshold = malloc(totalRounds * sizeof(double));
    double *eGreedyAvgTradeGain = malloc(totalRounds * sizeof(double));
    if (flag.eGreedy) {
        double *eGreedyGain = malloc(totalRounds * sizeof(double));
        printf("Calculating Epsilon-Greedy...\n");
        epsilonGreedy(data, eGreedyGain, eGreedyAvgThreshold, eGreedyAvgTrades, totalOpt, totalThresholds, totalRounds,
                      pricesPerRound);

        getAvgGain(totalRounds, eGreedyAvgGain, eGreedyGain);
        getAvgRegret(totalRounds, eGreedyAvgRegret, totalOpt, eGreedyGain);
        getCompRatio(totalRounds, eGreedyCompRatio, totalOpt, eGreedyGain);
        getAvgTradeGain(totalRounds, eGreedyGain, eGreedyAvgTrades, eGreedyAvgTradeGain);
        free(eGreedyGain);
    }

    double *succElimAvgGain = malloc(totalRounds * sizeof(double));
    double *succElimAvgRegret = malloc(totalRounds * sizeof(double));
    double *succElimCompRatio = malloc(totalRounds * sizeof(double));
    double *succElimAvgTrades = malloc(totalRounds * sizeof(double));
    double *succElimAvgThreshold = malloc(totalRounds * sizeof(double));
    double *succElimAvgTradeGain = malloc(totalRounds * sizeof(double));
    if (flag.succElim) {
        double *succElimGain = malloc(totalRounds * sizeof(double));
        printf("Calculating Successive Elimination...\n");
        succElim(data, succElimGain, succElimAvgThreshold, succElimAvgTrades, totalOpt, totalThresholds, totalRounds,
                 pricesPerRound);

        getAvgGain(totalRounds, succElimAvgGain, succElimGain);
        getAvgRegret(totalRounds, succElimAvgRegret, totalOpt, succElimGain);
        getCompRatio(totalRounds, succElimCompRatio, totalOpt, succElimGain);
        getAvgTradeGain(totalRounds, succElimGain, succElimAvgTrades, succElimAvgTradeGain);
        free(succElimGain);
    }

    double *ucb1AvgGain = malloc(totalRounds * sizeof(double));
    double *ucb1AvgRegret = malloc(totalRounds * sizeof(double));
    double *ucb1CompRatio = malloc(totalRounds * sizeof(double));
    double *ucb1AvgTrades = malloc(totalRounds * sizeof(double));
    double *ucb1AvgThreshold = malloc(totalRounds * sizeof(double));
    double *ucb1AvgTradeGain = malloc(totalRounds * sizeof(double));
    if (flag.ucb1) {
        double *ucb1Gain = malloc(totalRounds * sizeof(double));
        printf("Calculating UCB1...\n");
        ucb1(data, ucb1Gain, ucb1AvgThreshold, ucb1AvgTrades, totalOpt, totalThresholds, totalRounds, pricesPerRound);

        getAvgGain(totalRounds, ucb1AvgGain, ucb1Gain);
        getAvgRegret(totalRounds, ucb1AvgRegret, totalOpt, ucb1Gain);
        getCompRatio(totalRounds, ucb1CompRatio, totalOpt, ucb1Gain);
        getAvgTradeGain(totalRounds, ucb1Gain, ucb1AvgTrades, ucb1AvgTradeGain);
        free(ucb1Gain);
    }

    double *ucb2AvgGain = malloc(totalRounds * sizeof(double));
    double *ucb2AvgRegret = malloc(totalRounds * sizeof(double));
    double *ucb2CompRatio = malloc(totalRounds * sizeof(double));
    double *ucb2AvgTrades = malloc(totalRounds * sizeof(double));
    double *ucb2AvgThreshold = malloc(totalRounds * sizeof(double));
    double *ucb2AvgTradeGain = malloc(totalRounds * sizeof(double));
    if (flag.ucb2) {
        double *ucb2Gain = malloc(totalRounds * sizeof(double));
        printf("Calculating UCB2...\n");
        ucb2(data, ucb2Gain, ucb2AvgThreshold, ucb2AvgTrades, totalOpt, totalThresholds, totalRounds, pricesPerRound);

        getAvgGain(totalRounds, ucb2AvgGain, ucb2Gain);
        getAvgRegret(totalRounds, ucb2AvgRegret, totalOpt, ucb2Gain);
        getCompRatio(totalRounds, ucb2CompRatio, totalOpt, ucb2Gain);
        getAvgTradeGain(totalRounds, ucb2Gain, ucb2AvgTrades, ucb2AvgTradeGain);
        free(ucb2Gain);
    }

    double *exp3AvgGain = malloc(totalRounds * sizeof(double));
    double *exp3AvgRegret = malloc(totalRounds * sizeof(double));
    double *exp3CompRatio = malloc(totalRounds * sizeof(double));
    double *exp3AvgTrades = malloc(totalRounds * sizeof(double));
    double *exp3AvgThreshold = malloc(totalRounds * sizeof(double));
    double *exp3AvgTradeGain = malloc(totalRounds * sizeof(double));
    if (flag.exp3) {
        double *exp3Gain = malloc(totalRounds * sizeof(double));
        printf("Calculating EXP3...\n");
        exp3(data, exp3Gain, exp3AvgThreshold, exp3AvgTrades, totalOpt, totalThresholds, totalRounds, pricesPerRound);

        getAvgGain(totalRounds, exp3AvgGain, exp3Gain);
        getAvgRegret(totalRounds, exp3AvgRegret, totalOpt, exp3Gain);
        getCompRatio(totalRounds, exp3CompRatio, totalOpt, exp3Gain);
        getAvgTradeGain(totalRounds, exp3Gain, exp3AvgTrades, exp3AvgTradeGain);
        free(exp3Gain);
    }

    if (plot) {
        printf("Plotting prices...\n");
        plotData(data, totalRounds * pricesPerRound);
    }

    free(data);
    free(totalOpt);

    if (plot) {
        printf("Plotting gains...\n");
        plotAlgorithms("Average Gain", totalRounds, avgOpt, medianAvgGain, greedyAvgGain, eGreedyAvgGain,
                       succElimAvgGain, ucb1AvgGain, ucb2AvgGain, exp3AvgGain, flag, 0);
    }

    free(avgOpt);
    free(medianAvgGain);
    free(greedyAvgGain);
    free(eGreedyAvgGain);
    free(succElimAvgGain);
    free(ucb1AvgGain);
    free(ucb2AvgGain);
    free(exp3AvgGain);

    uint8_t noAlgs =
            !(flag.median || flag.greedy || flag.eGreedy || flag.succElim || flag.ucb1 || flag.ucb2 || flag.exp3);

    if (!noAlgs && plot) {
        printf("Plotting regret...\n");
        plotAlgorithms("Average Regret", totalRounds, nullptr, medianAvgRegret, greedyAvgRegret, eGreedyAvgRegret,
                       succElimAvgRegret, ucb1AvgRegret, ucb2AvgRegret, exp3AvgRegret, flag, 0);
    }

    free(medianAvgRegret);
    free(greedyAvgRegret);
    free(eGreedyAvgRegret);
    free(succElimAvgRegret);
    free(ucb1AvgRegret);
    free(ucb2AvgRegret);
    free(exp3AvgRegret);

    if (!noAlgs && plot) {
        printf("Plotting competitive ratio...\n");
        plotAlgorithms("Competitive Ratio", totalRounds, nullptr, medianCompRatio, greedyCompRatio, eGreedyCompRatio,
                       succElimCompRatio, ucb1CompRatio, ucb2CompRatio, exp3CompRatio, flag, 1);
    }

    free(medianCompRatio);
    free(greedyCompRatio);
    free(eGreedyCompRatio);
    free(succElimCompRatio);
    free(ucb1CompRatio);
    free(ucb2CompRatio);
    free(exp3CompRatio);

    if (!noAlgs && plot) {
        printf("Plotting average threshold...\n");
        plotAlgorithms("Average Threshold", totalRounds, nullptr, medianAvgThreshold, greedyAvgThreshold,
                       eGreedyAvgThreshold, succElimAvgThreshold, ucb1AvgThreshold, ucb2AvgThreshold, exp3AvgThreshold,
                       flag, 1);
    }

    free(medianAvgThreshold);
    free(greedyAvgThreshold);
    free(eGreedyAvgThreshold);
    free(succElimAvgThreshold);
    free(ucb1AvgThreshold);
    free(ucb2AvgThreshold);
    free(exp3AvgThreshold);

    if (plot) {
        printf("Plotting average number of trades...\n");
        plotAlgorithms("Average Number of Trades", totalRounds, optAvgTrades, medianAvgTrades, greedyAvgTrades,
                       eGreedyAvgTrades, succElimAvgTrades, ucb1AvgTrades, ucb2AvgTrades, exp3AvgTrades, flag, 0);
    }

    free(optAvgTrades);
    free(medianAvgTrades);
    free(greedyAvgTrades);
    free(eGreedyAvgTrades);
    free(succElimAvgTrades);
    free(ucb1AvgTrades);
    free(ucb2AvgTrades);
    free(exp3AvgTrades);

    if (plot) {
        printf("Plotting average gain per trade...\n");
        plotAlgorithms("Average Gain per Trade", totalRounds, optAvgTradeGain, medianAvgTradeGain, greedyAvgTradeGain,
                       eGreedyAvgTradeGain, succElimAvgTradeGain, ucb1AvgTradeGain, ucb2AvgTradeGain, exp3AvgTradeGain,
                       flag, 0);
    }

    free(optAvgTradeGain);
    free(medianAvgTradeGain);
    free(greedyAvgTradeGain);
    free(eGreedyAvgTradeGain);
    free(succElimAvgTradeGain);
    free(ucb1AvgTradeGain);
    free(ucb2AvgTradeGain);
    free(exp3AvgTradeGain);

    return 0;
}
