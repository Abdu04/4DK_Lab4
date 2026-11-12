CC = gcc
CFLAGS = -O2 -Wall

SOURCES = main.c experiments.c sender.c receiver.c output.c cleanup.c simlib.c
HEADERS = main.h experiments.h sender.h receiver.h output.h cleanup.h \
	simlib.h simparameters.h trace.h

arq_sim: $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -o arq_sim $(SOURCES) -lm

# Run every experiment, then draw the plots.
run: arq_sim
	./arq_sim
	python3 plot_results.py

clean:
	rm -f arq_sim
