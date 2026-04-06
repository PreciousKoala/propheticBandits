#ifndef HDR_BANDITALGS_H_
#define HDR_BANDITALGS_H_

#include <stdint.h>

void findOpt(double *data, double *totalOpt, uint64_t totalRounds, uint64_t pricesPerRound);

void findBestHand(double *reward, double *totalOpt, double *bestThreshold, uint64_t totalRounds,
                  uint32_t totalThresholds);

void greedy(double *reward, double *totalGain, double *avgThreshold, double *totalOpt, uint32_t totalThresholds,
            uint64_t totalRounds);

void epsilonGreedy(double *reward, double *totalGain, double *avgThreshold, double *totalOpt, uint32_t totalThresholds,
                   uint64_t totalRounds);

void succElim(double *reward, double *totalGain, double *avgThreshold, double *totalOpt, uint32_t totalThresholds,
              uint64_t totalRounds);

void ucb1(double *reward, double *totalGain, double *avgThreshold, double *totalOpt, uint32_t totalThresholds,
          uint64_t totalRounds);

void ucb2(double *reward, double *totalGain, double *avgThreshold, double *totalOpt, uint32_t totalThresholds,
          uint64_t totalRounds);

void exp3(double *reward, double *totalGain, double *avgThreshold, double *totalOpt, uint32_t totalThresholds,
          uint64_t totalRounds);

#endif
