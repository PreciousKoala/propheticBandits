FLAGS = -Wall -O3 -fsanitize=address
LIBS = -lgsl -lgslcblas -lm
SRC = propheticBandits.c util.c optimal.c greedy.c epsilonGreedy.c ucb1.c ucb2.c exp3.c

all: propheticBandits priceGenerator

propheticBandits: $(SRC)
	@gcc propheticBandits.c $(FLAGS) $(LIBS) -o propheticBandits

priceGenerator: priceGenerator.c
	@gcc priceGenerator.c $(FLAGS) $(LIBS) -o priceGenerator
	@mkdir -p prophetData

clean:
	@rm propheticBandits
	@rm priceGenerator
