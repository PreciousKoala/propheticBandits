#ifndef HDR_UTIL_H_
#define HDR_UTIL_H_

#include <stdint.h>

typedef struct flagStruct {
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
 * @param totalThresholds The size of the threshold array
 * @param reward The array with all the possible rewards
 * @param totalGain The array that holds the total gain up to the current round
 * @param round The current round
 */
void runRound(Threshold *thres, uint32_t th, uint32_t totalThresholds,
              double *reward, double *totalGain, uint64_t round);

/**
 * @brief Normalizes a 2D array represented in 1D in [0,1]
 *
 * @param min The minimum value of the array
 * @param max The maximum value of the array
 * @param data The array that is to be normalized
 * @param totalRounds The total number of rounds
 * @param pricesPerRound The total number of prices each round
 */
void normalizePrices(double min, double max, double *data, uint64_t totalRounds,
                     uint64_t pricesPerRound);

/**
 * @brief Calculates the reward for each threshold, for each day
 *
 * @param reward The TxK array that the calculated values are stored in
 * @param data The TxN array that holds the prices
 * @param totalRounds The total number of rounds T
 * @param pricesPerRound The total number of prices per round N
 * @param totalThresholds The total number of thresholds K
 * @param maxItems The maximum number of items the algorithm can hold
 */
void calculateRewards(double *reward, double *data, uint64_t totalRounds,
                      uint64_t pricesPerRound, uint32_t totalThresholds,
                      uint32_t maxItems);

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
void getAvgRegret(uint64_t totalRounds, double *algAvgRegret, double *totalOpt,
                  double *algGain);

/**
 * @brief Calculates per day the percentage of the best hands a specific
 * algorithm has chosen
 *
 * @param totalRounds The total number of rounds
 * @param algAvgRegret The array that saves the percentage each round
 * @param totalOpt The array with the optimal gain an algorithm can achieve by
 * each round
 * @param algGain The array with the gain an algorithm has achieved by each
 * round
 */
void getBestHandPerc(uint64_t totalRounds, double *algBestHand,
                     double *totalOpt, double *algGain);

/**
 * @brief Plots the needed information per day for each algorithm using gnuplot
 *
 * @param title The title that appears on the plot window
 * @param totalRounds The total number of rounds and the size of each array
 * @param [greedy, eGreedy, succElim, ucb1, ucb2, exp3] The array that holds the
 * information to be plotted for each algorithms for each round
 * @param flag The Flag struct that informs the program which algorithms have
 * been used
 */
void plotAlgorithms(char *title, uint64_t totalRounds, double *greedy,
                    double *eGreedy, double *succElim, double *ucb1,
                    double *ucb2, double *exp3, Flag flag);

#endif
