#ifndef HDR_BANDITALGS_H_
#define HDR_BANDITALGS_H_

#include <util.h>

void findOpt(double *data, double *totalOpt, double *avgTrades, Bandit b);

void bestHand(double *data, double *totalOpt, double *avgTrades, Bandit b);

void median(double *data, double *totalGain, double *avgThreshold, double *avgTrades, double *totalOpt, Bandit b);

void greedy(double *data, double *totalGain, double *avgLowThreshold, double *avgHighThreshold, double *avgTrades,
            double *totalOpt, Bandit b);

void epsilonGreedy(double *data, double *totalGain, double *avgLowThreshold, double *avgHighThreshold,
                   double *avgTrades, double *totalOpt, Bandit b);

void succElim(double *data, double *totalGain, double *avgLowThreshold, double *avgHighThreshold, double *avgTrades,
              double *totalOpt, Bandit b);

void ucb1(double *data, double *totalGain, double *avgLowThreshold, double *avgHighThreshold, double *avgTrades,
          double *totalOpt, Bandit b);

void ucb2(double *data, double *totalGain, double *avgLowThreshold, double *avgHighThreshold, double *avgTrades,
          double *totalOpt, Bandit b);

void exp3(double *data, double *totalGain, double *avgLowThreshold, double *avgHighThreshold, double *avgTrades,
          double *totalOpt, Bandit b);

#endif
