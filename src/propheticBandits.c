#include <getopt.h>
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
           "    -p              Plot more statistics.\n"
           "    -d              Use two thresholds.\n"
           "    -o              Use median algorithm as OPT.\n"
           "    -O              Use best hand as OPT.\n"
           "    -k              Keep items between rounds.\n\n"
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

    Bandit b = {0, 0, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t plot = 1;
    uint8_t morePlot = 0;

    int opt;
    opterr = 0;

    while ((opt = getopt(argc, argv, ":h:npkdoOamgesuUxt:")) != -1) {
        switch (opt) {
            case 'h':
                printHelp();
                return 0;
            case 't':
                b.K = atoi(optarg);
                b.thresholds = b.K;
                break;
            case 'n':
                plot = 0;
                break;
            case 'p':
                morePlot = 1;
                break;
            case 'k':
                b.keepItems = 1;
                break;
            case 'd':
                b.dualThres = 1;
                break;
            case 'o':
                b.medianOpt = 1;
                b.bestHandOpt = 0;
                break;
            case 'O':
                b.bestHandOpt = 1;
                b.medianOpt = 0;
                b.keepItems = 0;
                break;
            case 'a':
                b.median = 1;
                b.greedy = 1;
                b.eGreedy = 1;
                b.succElim = 1;
                b.ucb1 = 1;
                b.ucb2 = 1;
                b.exp3 = 1;
                break;
            case 'm':
                b.median = 1;
                break;
            case 'g':
                b.greedy = 1;
                break;
            case 'e':
                b.eGreedy = 1;
                break;
            case 's':
                b.succElim = 1;
                break;
            case 'u':
                b.ucb1 = 1;
                break;
            case 'U':
                b.ucb2 = 1;
                break;
            case 'x':
                b.exp3 = 1;
                break;
            case '?':
                if (optopt == 't')
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                break;
            default:
                abort();
        }
    }

    /* INFO: Data array holds all the prices found in the data file.
     * It's also a 1D array for values that are better suited in a 2D array, but
     * because of the sheer size of our data, it's better to save them like
     * this. Access the nth price of the tth round with
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

    b.T = totalRounds;
    b.N = pricesPerRound;

    if (b.dualThres && b.K <= 2) {
        printf("Error: Too few thresholds");
        return 1;
    }

    if (b.dualThres) {
        b.thresholds = b.K;
        b.K = b.K * (b.K + 1) / 2;
    }

    if (b.medianOpt) {
        b.median = 0;
    }

    double dataMin, dataMax;
    gsl_stats_minmax(&dataMin, &dataMax, data, 1, b.T * b.N);

    // Normalize prices to [0, 1]
    printf("Normalizing prices to [0,1]...\n");
    normalizePrices(dataMin, dataMax, data, b.T * b.N);

    printf("Calculating optimal result...\n");
    double *totalOpt = malloc(b.T * sizeof(double));
    double *avgOpt = malloc(b.T * sizeof(double));
    double *optAvgTrades = malloc(b.T * sizeof(double));
    double *optAvgLowThres = malloc(b.T * sizeof(double));
    double *optAvgHighThres = malloc(b.T * sizeof(double));
    double *optAvgTradeGain = malloc(b.T * sizeof(double));

    if (!b.medianOpt && !b.bestHandOpt) {
        findOpt(data, totalOpt, optAvgTrades, b);
    } else if (b.medianOpt) {
        median(data, totalOpt, optAvgLowThres, optAvgTrades, totalOpt, b);
    } else if (b.bestHandOpt) {
        bestHand(data, totalOpt, optAvgLowThres, optAvgHighThres, optAvgTrades, b);
    }
    getAvgGain(b.T, avgOpt, totalOpt);
    getAvgTradeGain(b.T, totalOpt, optAvgTrades, optAvgTradeGain);


    double *medianAvgGain = malloc(b.T * sizeof(double));
    double *medianAvgRegret = malloc(b.T * sizeof(double));
    double *medianCompRatio = malloc(b.T * sizeof(double));
    double *medianAvgTrades = malloc(b.T * sizeof(double));
    double *medianAvgThreshold = malloc(b.T * sizeof(double));
    double *medianAvgTradeGain = malloc(b.T * sizeof(double));
    if (b.median) {
        double *medianGain = malloc(b.T * sizeof(double));
        printf("Calculating Median...\n");
        median(data, medianGain, medianAvgThreshold, medianAvgTrades, totalOpt, b);

        getAvgGain(b.T, medianAvgGain, medianGain);
        getAvgRegret(b.T, medianAvgRegret, totalOpt, medianGain);
        getCompRatio(b.T, medianCompRatio, totalOpt, medianGain);
        getAvgTradeGain(b.T, medianGain, medianAvgTrades, medianAvgTradeGain);
        free(medianGain);
    }

    double *greedyAvgGain = malloc(b.T * sizeof(double));
    double *greedyAvgRegret = malloc(b.T * sizeof(double));
    double *greedyCompRatio = malloc(b.T * sizeof(double));
    double *greedyAvgTrades = malloc(b.T * sizeof(double));
    double *greedyAvgLowThres = malloc(b.T * sizeof(double));
    double *greedyAvgHighThres = malloc(b.T * sizeof(double));
    double *greedyAvgTradeGain = malloc(b.T * sizeof(double));
    if (b.greedy) {
        double *greedyGain = malloc(b.T * sizeof(double));
        printf("Calculating Greedy...\n");
        greedy(data, greedyGain, greedyAvgLowThres, greedyAvgHighThres, greedyAvgTrades, totalOpt, b);

        getAvgGain(b.T, greedyAvgGain, greedyGain);
        getAvgRegret(b.T, greedyAvgRegret, totalOpt, greedyGain);
        getCompRatio(b.T, greedyCompRatio, totalOpt, greedyGain);
        getAvgTradeGain(b.T, greedyGain, greedyAvgTrades, greedyAvgTradeGain);
        free(greedyGain);
    }

    double *eGreedyAvgGain = malloc(b.T * sizeof(double));
    double *eGreedyAvgRegret = malloc(b.T * sizeof(double));
    double *eGreedyCompRatio = malloc(b.T * sizeof(double));
    double *eGreedyAvgTrades = malloc(b.T * sizeof(double));
    double *eGreedyAvgLowThres = malloc(b.T * sizeof(double));
    double *eGreedyAvgHighThres = malloc(b.T * sizeof(double));
    double *eGreedyAvgTradeGain = malloc(b.T * sizeof(double));
    if (b.eGreedy) {
        double *eGreedyGain = malloc(b.T * sizeof(double));
        printf("Calculating Epsilon-Greedy...\n");
        epsilonGreedy(data, eGreedyGain, eGreedyAvgLowThres, eGreedyAvgHighThres, eGreedyAvgTrades, totalOpt, b);

        getAvgGain(b.T, eGreedyAvgGain, eGreedyGain);
        getAvgRegret(b.T, eGreedyAvgRegret, totalOpt, eGreedyGain);
        getCompRatio(b.T, eGreedyCompRatio, totalOpt, eGreedyGain);
        getAvgTradeGain(b.T, eGreedyGain, eGreedyAvgTrades, eGreedyAvgTradeGain);
        free(eGreedyGain);
    }

    double *succElimAvgGain = malloc(b.T * sizeof(double));
    double *succElimAvgRegret = malloc(b.T * sizeof(double));
    double *succElimCompRatio = malloc(b.T * sizeof(double));
    double *succElimAvgTrades = malloc(b.T * sizeof(double));
    double *succElimAvgLowThres = malloc(b.T * sizeof(double));
    double *succElimAvgHighThres = malloc(b.T * sizeof(double));
    double *succElimAvgTradeGain = malloc(b.T * sizeof(double));
    if (b.succElim) {
        double *succElimGain = malloc(b.T * sizeof(double));
        printf("Calculating Successive Elimination...\n");
        succElim(data, succElimGain, succElimAvgLowThres, succElimAvgHighThres, succElimAvgTrades, totalOpt, b);

        getAvgGain(b.T, succElimAvgGain, succElimGain);
        getAvgRegret(b.T, succElimAvgRegret, totalOpt, succElimGain);
        getCompRatio(b.T, succElimCompRatio, totalOpt, succElimGain);
        getAvgTradeGain(b.T, succElimGain, succElimAvgTrades, succElimAvgTradeGain);
        free(succElimGain);
    }

    double *ucb1AvgGain = malloc(b.T * sizeof(double));
    double *ucb1AvgRegret = malloc(b.T * sizeof(double));
    double *ucb1CompRatio = malloc(b.T * sizeof(double));
    double *ucb1AvgTrades = malloc(b.T * sizeof(double));
    double *ucb1AvgLowThres = malloc(b.T * sizeof(double));
    double *ucb1AvgHighThres = malloc(b.T * sizeof(double));
    double *ucb1AvgTradeGain = malloc(b.T * sizeof(double));
    if (b.ucb1) {
        double *ucb1Gain = malloc(b.T * sizeof(double));
        printf("Calculating UCB1...\n");
        ucb1(data, ucb1Gain, ucb1AvgLowThres, ucb1AvgHighThres, ucb1AvgTrades, totalOpt, b);

        getAvgGain(b.T, ucb1AvgGain, ucb1Gain);
        getAvgRegret(b.T, ucb1AvgRegret, totalOpt, ucb1Gain);
        getCompRatio(b.T, ucb1CompRatio, totalOpt, ucb1Gain);
        getAvgTradeGain(b.T, ucb1Gain, ucb1AvgTrades, ucb1AvgTradeGain);
        free(ucb1Gain);
    }

    double *ucb2AvgGain = malloc(b.T * sizeof(double));
    double *ucb2AvgRegret = malloc(b.T * sizeof(double));
    double *ucb2CompRatio = malloc(b.T * sizeof(double));
    double *ucb2AvgTrades = malloc(b.T * sizeof(double));
    double *ucb2AvgLowThres = malloc(b.T * sizeof(double));
    double *ucb2AvgHighThres = malloc(b.T * sizeof(double));
    double *ucb2AvgTradeGain = malloc(b.T * sizeof(double));
    if (b.ucb2) {
        double *ucb2Gain = malloc(b.T * sizeof(double));
        printf("Calculating UCB2...\n");
        ucb2(data, ucb2Gain, ucb2AvgLowThres, ucb2AvgHighThres, ucb2AvgTrades, totalOpt, b);

        getAvgGain(b.T, ucb2AvgGain, ucb2Gain);
        getAvgRegret(b.T, ucb2AvgRegret, totalOpt, ucb2Gain);
        getCompRatio(b.T, ucb2CompRatio, totalOpt, ucb2Gain);
        getAvgTradeGain(b.T, ucb2Gain, ucb2AvgTrades, ucb2AvgTradeGain);
        free(ucb2Gain);
    }

    double *exp3AvgGain = malloc(b.T * sizeof(double));
    double *exp3AvgRegret = malloc(b.T * sizeof(double));
    double *exp3CompRatio = malloc(b.T * sizeof(double));
    double *exp3AvgTrades = malloc(b.T * sizeof(double));
    double *exp3AvgLowThres = malloc(b.T * sizeof(double));
    double *exp3AvgHighThres = malloc(b.T * sizeof(double));
    double *exp3AvgTradeGain = malloc(b.T * sizeof(double));
    if (b.exp3) {
        double *exp3Gain = malloc(b.T * sizeof(double));
        printf("Calculating EXP3...\n");
        exp3(data, exp3Gain, exp3AvgLowThres, exp3AvgHighThres, exp3AvgTrades, totalOpt, b);

        getAvgGain(b.T, exp3AvgGain, exp3Gain);
        getAvgRegret(b.T, exp3AvgRegret, totalOpt, exp3Gain);
        getCompRatio(b.T, exp3CompRatio, totalOpt, exp3Gain);
        getAvgTradeGain(b.T, exp3Gain, exp3AvgTrades, exp3AvgTradeGain);
        free(exp3Gain);
    }

    if (plot && morePlot) {
        printf("Plotting prices...\n");
        plotData(data, b.T * b.N);
    }

    free(data);
    free(totalOpt);

    if (plot && morePlot) {
        printf("Plotting gains...\n");
        plotAlgorithms("Average Gain", b, avgOpt, medianAvgGain, greedyAvgGain, eGreedyAvgGain, succElimAvgGain,
                       ucb1AvgGain, ucb2AvgGain, exp3AvgGain, 0);
    }

    free(avgOpt);
    free(medianAvgGain);
    free(greedyAvgGain);
    free(eGreedyAvgGain);
    free(succElimAvgGain);
    free(ucb1AvgGain);
    free(ucb2AvgGain);
    free(exp3AvgGain);

    uint8_t noAlgs = !(b.median || b.greedy || b.eGreedy || b.succElim || b.ucb1 || b.ucb2 || b.exp3);

    if (!noAlgs && plot) {
        printf("Plotting regret...\n");
        plotAlgorithms("Average Regret", b, nullptr, medianAvgRegret, greedyAvgRegret, eGreedyAvgRegret,
                       succElimAvgRegret, ucb1AvgRegret, ucb2AvgRegret, exp3AvgRegret, 0);
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
        plotAlgorithms("Competitive Ratio", b, nullptr, medianCompRatio, greedyCompRatio, eGreedyCompRatio,
                       succElimCompRatio, ucb1CompRatio, ucb2CompRatio, exp3CompRatio, 0);
    }

    free(medianCompRatio);
    free(greedyCompRatio);
    free(eGreedyCompRatio);
    free(succElimCompRatio);
    free(ucb1CompRatio);
    free(ucb2CompRatio);
    free(exp3CompRatio);

    // if (!noAlgs && plot && !b.dualThres) {
    //     printf("Plotting average threshold...\n");
    //     plotAlgorithms("Average Threshold", b, optAvgLowThres, medianAvgThreshold, greedyAvgLowThres,
    //     eGreedyAvgLowThres,
    //                    succElimAvgLowThres, ucb1AvgLowThres, ucb2AvgLowThres, exp3AvgLowThres, 1);
    // } else if (!noAlgs && plot) {
    //     printf("Plotting average lower threshold...\n");
    //     plotAlgorithms("Average Lower Threshold", b, optAvgLowThres, medianAvgThreshold, greedyAvgLowThres,
    //     eGreedyAvgLowThres,
    //                    succElimAvgLowThres, ucb1AvgLowThres, ucb2AvgLowThres, exp3AvgLowThres, 1);
    //
    //     printf("Plotting average higher threshold...\n");
    //     plotAlgorithms("Average Higher Threshold", b, optAvgHighThres, medianAvgThreshold, greedyAvgHighThres,
    //                    eGreedyAvgHighThres, succElimAvgHighThres, ucb1AvgHighThres, ucb2AvgHighThres,
    //                    exp3AvgHighThres, 1);
    // }
    if (!noAlgs && plot) {
        printf("Plotting average thresholds...\n");
        plotThresholds(b, optAvgLowThres, optAvgHighThres, medianAvgThreshold, greedyAvgLowThres, greedyAvgHighThres,
                       eGreedyAvgLowThres, eGreedyAvgHighThres, succElimAvgLowThres, succElimAvgHighThres,
                       ucb1AvgLowThres, ucb1AvgHighThres, ucb2AvgLowThres, ucb2AvgHighThres, exp3AvgLowThres,
                       exp3AvgHighThres);
    }

    free(medianAvgThreshold);

    free(optAvgLowThres);
    free(greedyAvgLowThres);
    free(eGreedyAvgLowThres);
    free(succElimAvgLowThres);
    free(ucb1AvgLowThres);
    free(ucb2AvgLowThres);
    free(exp3AvgLowThres);

    free(optAvgHighThres);
    free(greedyAvgHighThres);
    free(eGreedyAvgHighThres);
    free(succElimAvgHighThres);
    free(ucb1AvgHighThres);
    free(ucb2AvgHighThres);
    free(exp3AvgHighThres);

    if (plot && morePlot) {
        printf("Plotting average number of trades...\n");
        plotAlgorithms("Average Number of Trades", b, optAvgTrades, medianAvgTrades, greedyAvgTrades, eGreedyAvgTrades,
                       succElimAvgTrades, ucb1AvgTrades, ucb2AvgTrades, exp3AvgTrades, 0);
    }

    free(optAvgTrades);
    free(medianAvgTrades);
    free(greedyAvgTrades);
    free(eGreedyAvgTrades);
    free(succElimAvgTrades);
    free(ucb1AvgTrades);
    free(ucb2AvgTrades);
    free(exp3AvgTrades);

    if (plot && morePlot) {
        printf("Plotting average gain per trade...\n");
        plotAlgorithms("Average Gain per Trade", b, optAvgTradeGain, medianAvgTradeGain, greedyAvgTradeGain,
                       eGreedyAvgTradeGain, succElimAvgTradeGain, ucb1AvgTradeGain, ucb2AvgTradeGain, exp3AvgTradeGain,
                       0);
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
