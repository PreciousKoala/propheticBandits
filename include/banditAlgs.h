#ifndef HDR_BANDITALGS_H_
#define HDR_BANDITALGS_H_

#include <stdint.h>

void findOpt(double *data, double *totalOpt, uint8_t maxItems,
             uint64_t totalRounds, uint64_t pricesPerRound);

void findBestHand(double *reward, double *totalOpt, uint64_t totalRounds,
                  uint32_t totalThresholds);

void greedy(double *reward, double *totalGain, double *totalOpt,
            uint32_t totalThresholds, uint32_t maxItems, uint64_t totalRounds,
            uint64_t pricesPerRound);

void epsilonGreedy(double *reward, double *totalGain, double *totalOpt,
                   uint32_t totalThresholds, uint32_t maxItems,
                   uint64_t totalRounds, uint64_t pricesPerRound);

void succElim(double *reward, double *totalGain, double *totalOpt,
              uint32_t totalThresholds, uint32_t maxItems, uint64_t totalRounds,
              uint64_t pricesPerRound);

void ucb1(double *reward, double *totalGain, double *totalOpt,
          uint32_t totalThresholds, uint32_t maxItems, uint64_t totalRounds,
          uint64_t pricesPerRound);

void ucb2(double *reward, double *totalGain, double *totalOpt,
          uint32_t totalThresholds, uint32_t maxItems, uint64_t totalRounds,
          uint64_t pricesPerRound);

void exp3(double *reward, double *totalGain, double *totalOpt,
          uint32_t totalThresholds, uint32_t maxItems, uint64_t totalRounds,
          uint64_t pricesPerRound);

#endif
