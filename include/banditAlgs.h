#ifndef HDR_BANDITALGS_H_
#define HDR_BANDITALGS_H_

#include <stdint.h>

void findOpt(double *data, double *totalOpt, double *avgTrades, double *avgTradeGain, uint64_t totalRounds,
             uint64_t pricesPerRound);

void median(double *data, double *totalGain, double *avgThreshold, double *avgTrades, double *totalOpt,
            uint64_t totalRounds, uint64_t pricesPerRound, double norm);

void greedy(double *data, double *totalGain, double *avgThreshold, double *avgTrades, double *totalOpt,
            uint32_t totalThresholds, uint64_t totalRounds, uint64_t pricesPerRound, double norm);

void epsilonGreedy(double *data, double *totalGain, double *avgThreshold, double *avgTrades, double *totalOpt,
                   uint32_t totalThresholds, uint64_t totalRounds, uint64_t pricesPerRound, double norm);

void succElim(double *data, double *totalGain, double *avgThreshold, double *avgTrades, double *totalOpt,
              uint32_t totalThresholds, uint64_t totalRounds, uint64_t pricesPerRound, double norm);

void ucb1(double *data, double *totalGain, double *avgThreshold, double *avgTrades, double *totalOpt,
          uint32_t totalThresholds, uint64_t totalRounds, uint64_t pricesPerRound, double norm);

void ucb2(double *data, double *totalGain, double *avgThreshold, double *avgTrades, double *totalOpt,
          uint32_t totalThresholds, uint64_t totalRounds, uint64_t pricesPerRound, double norm);

void exp3(double *data, double *totalGain, double *avgThreshold, double *avgTrades, double *totalOpt,
          uint32_t totalThresholds, uint64_t totalRounds, uint64_t pricesPerRound, double norm);

#endif
