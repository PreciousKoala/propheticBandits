#ifndef HDR_UTIL_H_
#define HDR_UTIL_H_

#include <stdint.h>

typedef struct flagStruct {
    uint8_t median;
    uint8_t greedy;
    uint8_t eGreedy;
    uint8_t succElim;
    uint8_t ucb1;
    uint8_t ucb2;
    uint8_t exp3;
} Flag;

/**
 * @typedef thresholdStruct
 * @brief A struct that holds statistics for a threshold
 *
 */
typedef struct thresholdStruct {
    // the placement of the threshold in [0,1]
    double threshold;
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
void initThreshold(Threshold *thres, uint32_t totalThresholds);

/**
 * @brief Updates the threhold array's values for the specific threshold that
 * has been chosen
 *
 * @param thres Threshold array
 * @param th The chosen threshold
 * @param totalRounds The number of rounds
 * @param pricesPerRound The number of prices per round
 * @param data The array with the prices
 * @param avgThreshold The array that holds the average chosen threshold of each round
 * @param avgTrades The array that holds the average number of trades (selling an item) up to the current round
 * @param totalGain The array that holds the total gain up to the current round
 * @param round The current round
 * @param heldItems True if the algorithm is holding an item from the previous round
 * @param norm Variable that normalized data previously
 *
 * @returns The reward of the round
 */
double runRound(Threshold *thres, uint32_t th, uint64_t totalRounds, uint64_t pricesPerRound, double *data,
              double *avgThreshold, double *avgTrades, double *totalGain, uint64_t round, uint8_t *heldItems,
              double norm);

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
 * @brief Calculates the reward for each threshold, for each day
 *
 * @param reward The TxK array that the calculated values are stored in
 * @param data The TxN array that holds the prices
 * @param totalRounds The total number of rounds T
 * @param pricesPerRound The total number of prices per round N
 * @param totalThresholds The total number of thresholds K
 */
void calculateRewards(double *reward, double *data, uint64_t totalRounds, uint64_t pricesPerRound,
                      uint32_t totalThresholds);

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

/**
 * @brief Calculates per day the average distance from the best threshold
 *
 * @param totalRounds The total number of rounds
 * @param algThresholdDist The array that saves the distance each round
 * @param bestAvgThreshold The array with the best average threshold of each round
 * @param algAvgThreshold The array with the average threshold chosen each round
 */
void getBestHandDistance(uint64_t totalRounds, double *algThresholdDist, double *bestAvgThreshold,
                         double *algAvgThreshold);

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
void plotAlgorithms(char *ylabel, uint64_t totalRounds, double *median, double *greedy, double *eGreedy,
                    double *succElim, double *ucb1, double *ucb2, double *exp3, Flag flag, uint8_t bounded);

#endif
