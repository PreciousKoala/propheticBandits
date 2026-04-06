#include <getopt.h>
#include <gsl/gsl_rng.h>
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
           "    -a              Run all the available algorithms.\n"
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
    Flag flag = {0, 0, 0, 0, 0, 0};

    int opt;

    opterr = 0;

    while ((opt = getopt(argc, argv, ":h:agesuUxt:")) != -1) {
        switch (opt) {
            case 'h':
                printHelp();
                return 0;
            case 't':
                totalThresholds = atoi(optarg);
                break;
            case 'a':
                flag = (Flag) {1, 1, 1, 1, 1, 1};
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
                if (optopt == 'm' || optopt == 't')
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

    double dataMin = INFINITY;
    double dataMax = -INFINITY;

    for (uint64_t i = 0; i < totalRounds * pricesPerRound; i++) {
        if (data[i] < dataMin)
            dataMin = data[i];
        if (data[i] > dataMax)
            dataMax = data[i];
    }

    // Normalize prices to [0, 1]
    printf("Normalizing prices to [0,1]...\n");
    normalizePrices(dataMin, dataMax, data, totalRounds, pricesPerRound);

    printf("Calculating rewards...\n");
    double *reward = malloc(totalRounds * totalThresholds * sizeof(double));
    calculateRewards(reward, data, totalRounds, pricesPerRound, totalThresholds);

    double rewardMin = INFINITY;
    double rewardMax = -INFINITY;

    for (uint64_t i = 0; i < totalRounds * totalThresholds; i++) {
        if (reward[i] < rewardMin)
            rewardMin = reward[i];
        if (reward[i] > rewardMax)
            rewardMax = reward[i];
    }

    printf("Calculating optimal result (local extrema)...\n");
    double *totalExtremaOpt = malloc(totalRounds * sizeof(double));
    findOpt(data, totalExtremaOpt, totalRounds, pricesPerRound);

    double optMax = totalExtremaOpt[0];

    for (uint64_t t = 1; t < totalRounds; t++) {
        double roundOpt = totalExtremaOpt[t] - totalExtremaOpt[t - 1];
        if (roundOpt > optMax)
            optMax = roundOpt;
    }

    for (uint64_t t = 0; t < totalRounds; t++) {
        totalExtremaOpt[t] = (totalExtremaOpt[t] - (double)(t + 1) * rewardMin) / (optMax - rewardMin);
    }

    // Normalize prices to [0, 1]
    printf("Normalizing rewards to [0,1]...\n");
    normalizePrices(rewardMin, optMax, reward, totalRounds, totalThresholds);
    normalizePrices(rewardMin, optMax, data, totalRounds, pricesPerRound);

    printf("\n");
    printf("---------------------------------LOCAL-EXTREMA-----------------------"
           "-----------\n");
    printf("Total OPT: %lf\n", totalExtremaOpt[totalRounds - 1]);
    printf("Average OPT: %lf\n", totalExtremaOpt[totalRounds - 1] / (double) totalRounds);
    printf("---------------------------------------------------------------------"
           "-----------\n\n");



    printf("Calculating optimal result (best hand)...\n");
    double *totalOpt = malloc(totalRounds * sizeof(double));
    double *bestAvgThreshold = malloc(totalRounds * sizeof(double));
    findBestHand(reward, totalOpt, bestAvgThreshold, totalRounds, totalThresholds);

    free(data);

    double *greedyAvgRegret = malloc(totalRounds * sizeof(double));
    double *greedyExtrAvgRegret = malloc(totalRounds * sizeof(double));
    double *greedyThresholdDist = malloc(totalRounds * sizeof(double));
    double *greedyAvgThreshold = malloc(totalRounds * sizeof(double));
    if (flag.greedy) {
        double *greedyGain = malloc(totalRounds * sizeof(double));

        printf("Calculating Greedy...\n");
        greedy(reward, greedyGain, greedyAvgThreshold, totalOpt, totalThresholds, totalRounds);

        getAvgRegret(totalRounds, greedyAvgRegret, totalOpt, greedyGain);
        getAvgRegret(totalRounds, greedyExtrAvgRegret, totalExtremaOpt, greedyGain);
        getBestHandDistance(totalRounds, greedyThresholdDist, bestAvgThreshold, greedyAvgThreshold);
        free(greedyGain);
    }

    double *eGreedyAvgRegret = malloc(totalRounds * sizeof(double));
    double *eGreedyExtrAvgRegret = malloc(totalRounds * sizeof(double));
    double *eGreedyThresholdDist = malloc(totalRounds * sizeof(double));
    double *eGreedyAvgThreshold = malloc(totalRounds * sizeof(double));
    if (flag.eGreedy) {
        double *eGreedyGain = malloc(totalRounds * sizeof(double));

        printf("Calculating Epsilon-Greedy...\n");
        epsilonGreedy(reward, eGreedyGain, eGreedyAvgThreshold, totalOpt, totalThresholds, totalRounds);

        getAvgRegret(totalRounds, eGreedyAvgRegret, totalOpt, eGreedyGain);
        getAvgRegret(totalRounds, eGreedyExtrAvgRegret, totalExtremaOpt, eGreedyGain);
        getBestHandDistance(totalRounds, eGreedyThresholdDist, bestAvgThreshold, eGreedyAvgThreshold);
        free(eGreedyGain);
    }

    double *succElimAvgRegret = malloc(totalRounds * sizeof(double));
    double *succElimExtrAvgRegret = malloc(totalRounds * sizeof(double));
    double *succElimThresholdDist = malloc(totalRounds * sizeof(double));
    double *succElimAvgThreshold = malloc(totalRounds * sizeof(double));
    if (flag.succElim) {
        double *succElimGain = malloc(totalRounds * sizeof(double));

        printf("Calculating Successive Elimination...\n");
        succElim(reward, succElimGain, succElimAvgThreshold, totalOpt, totalThresholds, totalRounds);

        getAvgRegret(totalRounds, succElimAvgRegret, totalOpt, succElimGain);
        getAvgRegret(totalRounds, succElimExtrAvgRegret, totalExtremaOpt, succElimGain);
        getBestHandDistance(totalRounds, succElimThresholdDist, bestAvgThreshold, succElimAvgThreshold);
        free(succElimGain);
    }

    double *ucb1AvgRegret = malloc(totalRounds * sizeof(double));
    double *ucb1ExtrAvgRegret = malloc(totalRounds * sizeof(double));
    double *ucb1ThresholdDist = malloc(totalRounds * sizeof(double));
    double *ucb1AvgThreshold = malloc(totalRounds * sizeof(double));
    if (flag.ucb1) {
        double *ucb1Gain = malloc(totalRounds * sizeof(double));

        printf("Calculating UCB1...\n");
        ucb1(reward, ucb1Gain, ucb1AvgThreshold, totalOpt, totalThresholds, totalRounds);

        getAvgRegret(totalRounds, ucb1AvgRegret, totalOpt, ucb1Gain);
        getAvgRegret(totalRounds, ucb1ExtrAvgRegret, totalExtremaOpt, ucb1Gain);
        getBestHandDistance(totalRounds, ucb1ThresholdDist, bestAvgThreshold, ucb1AvgThreshold);
        free(ucb1Gain);
    }

    double *ucb2AvgRegret = malloc(totalRounds * sizeof(double));
    double *ucb2ExtrAvgRegret = malloc(totalRounds * sizeof(double));
    double *ucb2ThresholdDist = malloc(totalRounds * sizeof(double));
    double *ucb2AvgThreshold = malloc(totalRounds * sizeof(double));
    if (flag.ucb2) {
        double *ucb2Gain = malloc(totalRounds * sizeof(double));

        printf("Calculating UCB2...\n");
        ucb2(reward, ucb2Gain, ucb2AvgThreshold, totalOpt, totalThresholds, totalRounds);

        getAvgRegret(totalRounds, ucb2AvgRegret, totalOpt, ucb2Gain);
        getAvgRegret(totalRounds, ucb2ExtrAvgRegret, totalExtremaOpt, ucb2Gain);
        getBestHandDistance(totalRounds, ucb2ThresholdDist, bestAvgThreshold, ucb2AvgThreshold);
        free(ucb2Gain);
    }

    double *exp3AvgRegret = malloc(totalRounds * sizeof(double));
    double *exp3ExtrAvgRegret = malloc(totalRounds * sizeof(double));
    double *exp3ThresholdDist = malloc(totalRounds * sizeof(double));
    double *exp3AvgThreshold = malloc(totalRounds * sizeof(double));
    if (flag.exp3) {
        double *exp3Gain = malloc(totalRounds * sizeof(double));

        printf("Calculating EXP3...\n");
        exp3(reward, exp3Gain, exp3AvgThreshold, totalOpt, totalThresholds, totalRounds);

        getAvgRegret(totalRounds, exp3AvgRegret, totalOpt, exp3Gain);
        getAvgRegret(totalRounds, exp3ExtrAvgRegret, totalExtremaOpt, exp3Gain);
        getBestHandDistance(totalRounds, exp3ThresholdDist, bestAvgThreshold, exp3AvgThreshold);
        free(exp3Gain);
    }

    free(reward);
    free(totalOpt);
    free(totalExtremaOpt);
    free(bestAvgThreshold);

    uint8_t noAlgs = !(flag.greedy || flag.eGreedy || flag.succElim || flag.ucb1 || flag.ucb2 || flag.exp3);

    if (!noAlgs) {
        printf("Plotting best hand regret...\n");
        plotAlgorithms("Average Regret (Best Hand)", totalRounds, greedyAvgRegret, eGreedyAvgRegret, succElimAvgRegret,
                       ucb1AvgRegret, ucb2AvgRegret, exp3AvgRegret, flag);
    }

    free(greedyAvgRegret);
    free(eGreedyAvgRegret);
    free(succElimAvgRegret);
    free(ucb1AvgRegret);
    free(ucb2AvgRegret);
    free(exp3AvgRegret);

    if (!noAlgs) {
        printf("Plotting percentage of best hands played...\n");
        plotAlgorithms("Average distance from best threshold", totalRounds, greedyThresholdDist, eGreedyThresholdDist,
                       succElimThresholdDist, ucb1ThresholdDist, ucb2ThresholdDist, exp3ThresholdDist, flag);
    }

    free(greedyThresholdDist);
    free(eGreedyThresholdDist);
    free(succElimThresholdDist);
    free(ucb1ThresholdDist);
    free(ucb2ThresholdDist);
    free(exp3ThresholdDist);

    if (!noAlgs) {
        printf("Plotting average threshold...\n");
        plotAlgorithms("Average Hand", totalRounds, greedyAvgThreshold, eGreedyAvgThreshold, succElimAvgThreshold,
                       ucb1AvgThreshold, ucb2AvgThreshold, exp3AvgThreshold, flag);
    }

    free(greedyAvgThreshold);
    free(eGreedyAvgThreshold);
    free(succElimAvgThreshold);
    free(ucb1AvgThreshold);
    free(ucb2AvgThreshold);
    free(exp3AvgThreshold);

    if (!noAlgs) {
        printf("Plotting extrema regret...\n");
        plotAlgorithms("Average Regret (Extrema)", totalRounds, greedyExtrAvgRegret, eGreedyExtrAvgRegret,
                       succElimExtrAvgRegret, ucb1ExtrAvgRegret, ucb2ExtrAvgRegret, exp3ExtrAvgRegret, flag);
    }

    free(greedyExtrAvgRegret);
    free(eGreedyExtrAvgRegret);
    free(succElimExtrAvgRegret);
    free(ucb1ExtrAvgRegret);
    free(ucb2ExtrAvgRegret);
    free(exp3ExtrAvgRegret);

    return 0;
}
