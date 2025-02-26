all: propheticBandits priceGenerator

propheticBandits: propheticBandits.c
	@gcc propheticBandits.c -o propheticBandits

priceGenerator: priceGenerator.c
	@gcc priceGenerator.c -o priceGenerator -lgsl -lgslcblas -lm -fsanitize=address

clean:
	@rm propheticBandits
