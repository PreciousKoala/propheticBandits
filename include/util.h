#ifndef HDR_UTIL_H_
#define HDR_UTIL_H_

#include <stdint.h>

/**
 * @typedef banditStruct
 * @brief A struct that holds flags and information about the data
 *
 */
typedef struct banditStruct {
    uint64_t T;
    uint64_t N;
    uint32_t K;
    uint32_t thresholds;
    uint8_t dualThres;
    uint8_t medianOpt;
    double medianPrice;
    uint8_t bestHandOpt;
    uint8_t keepItems;
    uint8_t median;
    uint8_t greedy;
    uint8_t eGreedy;
    uint8_t succElim;
    uint8_t ucb1;
    uint8_t ucb2;
    uint8_t exp3;
} Bandit;

/**
 * @typedef thresholdStruct
 * @brief A struct that holds statistics for a threshold
 *
 */
typedef struct thresholdStruct {
    // the placement of the thresholds in [0,1]
    double low;
    double high;
    // how much money the threshold has made
    double rewardSum;
    // how many times the threshold has been picked
    uint64_t timesChosen;
    // how much money on average the threshold has made
    double avgReward;
} Threshold;

/**
 * @brief Initializes the array of threshold sturcts' values to 0
 *
 * @param thres The Threshold struct array
 * @param totalThresholds The size of the array
 */
void initThreshold(Threshold *thres, Bandit b);

/**
 * @brief Updates the threhold array's values for the specific threshold that
 * has been chosen
 *
 * @param thres Threshold array
 * @param th The chosen threshold
 * @param data The array with the prices
 * @param avgThreshold The array that holds the average chosen threshold of each round
 * @param avgTrades The array that holds the average number of trades (selling an item) up to the current round
 * @param totalGain The array that holds the total gain up to the current round
 * @param round The current round
 * @param heldItems True if the algorithm is holding an item from the previous round
 * @param heldItemValue Value  of the last purchased item
 *
 * @returns The reward of the round
 */
double runRound(Threshold *thres, uint32_t th, Bandit b, double *data, double *avgLowThreshold, double *avgHighThreshold, double *avgTrades,
                double *totalGain, uint64_t round, uint8_t *heldItems, double *heldItemValue);

/**
 * @brief Normalizes a 2D array represented in 1D in [0,1]
 *
 * @param min The minimum value of the array
 * @param max The maximum value of the array
 * @param data The array that is to be normalized
 * @param size The size of the array
 */
void normalizePrices(double min, double max, double *data, uint64_t size);

/**
 * @brief Calculates the average regret pre round for a specific algorithm
 *
 * @param totalRounds The total number of rounds
 * @param algAvgRegret The array that saves the average regret
 * @param totalOpt The array with the optimal gain an algorithm can achieve by
 * each round
 * @param algGain The array with the gain an algorithm has achieved by each
 * round
 */
void getAvgRegret(uint64_t totalRounds, double *algAvgRegret, double *totalOpt, double *algGain);

void getAvgGain(uint64_t size, double *avgGain, double *totalGain);

void getCompRatio(uint64_t totalRounds, double *algCompRatio, double *totalOpt, double *algGain);

void getAvgTradeGain(uint64_t totalRounds, double *algGain, double *algAvgTrades, double *algAvgTradeGain);

/**
 * @brief Plots the needed information per day for each algorithm using gnuplot
 *
 * @param ylabel The title that appears on the plot window
 * @param totalRounds The total number of rounds and the size of each array
 * @param [median, greedy, eGreedy, succElim, ucb1, ucb2, exp3] The array that holds the
 * information to be plotted for each algorithm for each round
 * @param flag The Flag struct that informs the program which algorithms have
 * been used
 * @param bounded True when the plotted values need to be bounded in [0,1]
 */
void plotAlgorithms(char *ylabel, Bandit b, double *opt, double *median, double *greedy, double *eGreedy,
                    double *succElim, double *ucb1, double *ucb2, double *exp3, uint8_t bounded);

void plotData(double *data, uint64_t size);

#endif
