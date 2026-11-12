
/*
 * Simulation of ARQ Protocols (S&W, GBN and SR)
 *
 * COE 4DK4 Lab 4
 *
 * Built on the Simlib simulation library,
 * Copyright (C) 2014 Terence D. Todd Hamilton, Ontario, CANADA
 * todd@mcmaster.ca
 *
 */

/*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "simparameters.h"
#include "main.h"
#include "output.h"
#include "experiments.h"

/*******************************************************************************/

#define ARRAY_SIZE(a) ((int) (sizeof(a) / sizeof((a)[0])))

static FILE *
open_results_file(const char * name)
{
  FILE * file;

  if((file = fopen(name, "w")) == NULL) {
    printf("Error: cannot open %s (run from the directory that contains "
	   "the results directory)\n", name);
    exit(1);
  }
  printf("Writing %s\n", name);
  return file;
}

/*******************************************************************************/

/*
 * Simulate one data point of one curve. A new simulation run is done
 * for each random number generator seed defined in simparameters.h. For
 * a finite transfer, REPLICATIONS transfers are done for each seed and
 * their results are averaged.
 */

static void
run_data_point(FILE * file, const char * curve, int ws, int wr,
	       double pd, double pa, Time timeout, long int number_of_packets)
{
  unsigned random_seed;
  unsigned RANDOM_SEEDS[] = {RANDOM_SEED_LIST, 0};
  Arq_Parameters parameters;
  Arq_Results results, mean;
  int i, j = 0, replications;

  parameters.ws = ws;
  parameters.wr = wr;
  parameters.pd = pd;
  parameters.pa = pa;
  parameters.timeout = timeout;
  parameters.number_of_packets = number_of_packets;

  replications = (number_of_packets > 0) ? REPLICATIONS : 1;

  while ((random_seed = RANDOM_SEEDS[j++]) != 0) {

    /* Set the random generator seed. */
    random_generator_initialize(random_seed);

    mean.efficiency = 0.0;
    mean.attempts_per_packet = 0.0;
    mean.completion_time = 0.0;

    for(i=0; i<replications; i++) {
      results = arq_simulation_run(parameters, NULL);

      mean.efficiency += results.efficiency / replications;
      mean.attempts_per_packet += results.attempts_per_packet / replications;
      mean.completion_time += results.completion_time / replications;
    }

    output_results(file, curve, parameters, random_seed, mean);
  }
}

/*
 * Simulate one data point for S&W (Ws = Wr = 1).
 */

static void
run_stop_and_wait(FILE * file, double pd, double pa, Time timeout,
		  long int number_of_packets)
{
  run_data_point(file, "S&W", 1, 1, pd, pa, timeout, number_of_packets);
}

/*
 * Simulate one data point for GBN (Wr = 1) and for each of the SR
 * receive window sizes in SR_WR_LIST.
 */

static void
run_gbn_and_sr(FILE * file, int ws, double pd, double pa, Time timeout,
	       long int number_of_packets)
{
  int SR_WR[] = {SR_WR_LIST};
  char curve[32];
  int i;

  run_data_point(file, "GBN", ws, 1, pd, pa, timeout, number_of_packets);

  for(i=0; i<ARRAY_SIZE(SR_WR); i++) {
    if(SR_WR[i] == 0) {
      run_data_point(file, "SR Wr=Ws", ws, ws, pd, pa, timeout,
		     number_of_packets);
    } else {
      sprintf(curve, "SR Wr=%d", SR_WR[i]);
      run_data_point(file, curve, ws, SR_WR[i], pd, pa, timeout,
		     number_of_packets);
    }
  }
}

/*******************************************************************************/

/*
 * Part 1: Simulation verification. Short event traces of a small finite
 * transfer for S&W, GBN and SR, first without errors and then with
 * pd = pa = 0.5.
 */

static void
run_trace(FILE * file, const char * protocol, int ws, int wr, double p)
{
  unsigned RANDOM_SEEDS[] = {RANDOM_SEED_LIST, 0};
  Arq_Parameters parameters;
  Arq_Results results;

  parameters.ws = ws;
  parameters.wr = wr;
  parameters.pd = p;
  parameters.pa = p;
  parameters.timeout = TIMEOUT;
  parameters.number_of_packets = TRACE_NUMBER_OF_PACKETS;

  fprintf(file, "%s: Ws = %d, Wr = %d, pd = %g, pa = %g, timeout = %g ms, "
	  "%d packets, seed = %u\n\n", protocol, ws, wr, p, p, TIMEOUT,
	  TRACE_NUMBER_OF_PACKETS, RANDOM_SEEDS[0]);

  random_generator_initialize(RANDOM_SEEDS[0]);
  results = arq_simulation_run(parameters, file);

  fprintf(file, "\nCompletion time = %.3f ms, transmission attempts per "
	  "packet = %.3f\n\n\n", results.completion_time,
	  results.attempts_per_packet);
}

void
experiment_part1(void)
{
  double P[] = {0.0, TRACE_ERROR_PROBABILITY};
  FILE * file;
  int i;

  file = open_results_file("results/part1_trace.txt");

  for(i=0; i<ARRAY_SIZE(P); i++) {
    run_trace(file, "S&W", 1, 1, P[i]);
    run_trace(file, "GBN", TRACE_WS, 1, P[i]);
    run_trace(file, "SR", TRACE_WS, TRACE_WS, P[i]);
  }

  fclose(file);
}

/*******************************************************************************/

/*
 * Part 2: Backlogged source without errors. Efficiency versus Ws.
 */

void
experiment_part2(void)
{
  FILE * file;
  int ws;

  file = open_results_file("results/part2.csv");
  output_header(file);

  run_stop_and_wait(file, 0.0, 0.0, TIMEOUT, 0);

  for(ws=1; ws<=PART2_MAX_WS; ws++) {
    run_gbn_and_sr(file, ws, 0.0, 0.0, TIMEOUT, 0);
  }

  fclose(file);
}

/*******************************************************************************/

/*
 * Part 3: Backlogged source with packet errors and without ACK errors.
 * (a) Efficiency versus pd with Ws fixed. (b) Efficiency and attempts
 * per packet versus Ws with pd fixed.
 */

void
experiment_part3(void)
{
  double PD[] = {PART3A_PD_LIST};
  FILE * file;
  int i, ws;

  file = open_results_file("results/part3a.csv");
  output_header(file);

  for(i=0; i<ARRAY_SIZE(PD); i++) {
    run_stop_and_wait(file, PD[i], 0.0, TIMEOUT, 0);
    run_gbn_and_sr(file, FIXED_WS, PD[i], 0.0, TIMEOUT, 0);
  }

  fclose(file);

  file = open_results_file("results/part3b.csv");
  output_header(file);

  run_stop_and_wait(file, PART3B_PD, 0.0, TIMEOUT, 0);

  for(ws=1; ws<=PART3B_MAX_WS; ws++) {
    run_gbn_and_sr(file, ws, PART3B_PD, 0.0, TIMEOUT, 0);
  }

  fclose(file);
}

/*******************************************************************************/

/*
 * Part 4: Backlogged source with both packet and ACK errors. Efficiency
 * and attempts per packet versus pa with pd and Ws fixed.
 */

void
experiment_part4(void)
{
  double PA[] = {PART4_PA_LIST};
  FILE * file;
  int i;

  file = open_results_file("results/part4.csv");
  output_header(file);

  for(i=0; i<ARRAY_SIZE(PA); i++) {
    run_stop_and_wait(file, PART4_PD, PA[i], TIMEOUT, 0);
    run_gbn_and_sr(file, FIXED_WS, PART4_PD, PA[i], TIMEOUT, 0);
  }

  fclose(file);
}

/*******************************************************************************/

/*
 * Part 5: Effect of the retransmission timeout. Efficiency and attempts
 * per packet versus the timeout with pd, pa and Ws fixed.
 */

void
experiment_part5(void)
{
  double TIMEOUTS[] = {PART5_TIMEOUT_LIST};
  FILE * file;
  int i;

  file = open_results_file("results/part5.csv");
  output_header(file);

  for(i=0; i<ARRAY_SIZE(TIMEOUTS); i++) {
    run_stop_and_wait(file, PART5_PD, PART5_PA, TIMEOUTS[i], 0);
    run_gbn_and_sr(file, FIXED_WS, PART5_PD, PART5_PA, TIMEOUTS[i], 0);
  }

  fclose(file);
}

/*******************************************************************************/

/*
 * Parts 6 and 7: Finite packet transfer. Mean transfer completion time
 * versus Ws for a transfer of number_of_packets packets.
 */

static void
run_finite_transfer(const char * name, long int number_of_packets,
		    double pd, double pa)
{
  FILE * file;
  int ws;

  file = open_results_file(name);
  output_header(file);

  run_stop_and_wait(file, pd, pa, TIMEOUT, number_of_packets);

  for(ws=1; ws<=FINITE_MAX_WS; ws++) {
    run_gbn_and_sr(file, ws, pd, pa, TIMEOUT, number_of_packets);
  }

  fclose(file);
}

void
experiment_part6(void)
{
  double p = PART6_ERROR_PROBABILITY;
  long int n = PART6_NUMBER_OF_PACKETS;

  run_finite_transfer("results/part6_case1.csv", n, 0.0, 0.0);
  run_finite_transfer("results/part6_case2.csv", n, p, 0.0);
  run_finite_transfer("results/part6_case3.csv", n, p, p);
}

/*
 * Part 7: Repeat the error free case and the case with both packet and
 * ACK errors for a smaller and a larger transfer.
 */

void
experiment_part7(void)
{
  double p = PART6_ERROR_PROBABILITY;
  long int small = PART7_SMALL_NUMBER_OF_PACKETS;
  long int large = PART7_LARGE_NUMBER_OF_PACKETS;

  run_finite_transfer("results/part7_small_case1.csv", small, 0.0, 0.0);
  run_finite_transfer("results/part7_small_case3.csv", small, p, p);
  run_finite_transfer("results/part7_large_case1.csv", large, 0.0, 0.0);
  run_finite_transfer("results/part7_large_case3.csv", large, p, p);
}
