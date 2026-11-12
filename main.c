
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

#include <stdlib.h>
#include <stdio.h>
#include "output.h"
#include "simparameters.h"
#include "cleanup.h"
#include "sender.h"
#include "experiments.h"
#include "main.h"

/*******************************************************************************/

/*
 * Do one simulation run of the general SR ARQ protocol and return the
 * results. The random number generator must already be seeded. If
 * trace_file is not NULL an event trace is written to it.
 */

Arq_Results
arq_simulation_run(Arq_Parameters parameters, FILE * trace_file)
{
  Simulation_Run_Ptr simulation_run;
  Simulation_Run_Data data;
  Arq_Results results;

  /* Create a new simulation_run. This gives a clock and
     eventlist. Clock time is set to zero. */
  simulation_run = (Simulation_Run_Ptr) simulation_run_new();

  /* Add our data definitions to the simulation_run. */
  simulation_run_set_data(simulation_run, (void *) & data);

  /* Initialize the sender, the receiver and the statistics. */
  data.parameters = parameters;
  data.send_base = 0;
  data.next_seq = 0;
  data.send_window = (Sender_Slot_Ptr) xcalloc((unsigned int) parameters.ws,
					       sizeof(Sender_Slot));
  data.transmitter_busy = 0;
  data.rcv_base = 0;
  data.rcv_window = (int *) xcalloc((unsigned int) parameters.wr, sizeof(int));
  data.number_of_transmissions = 0;
  data.number_of_packets_delivered = 0;
  data.completion_time = -1.0;
  data.trace_file = trace_file;

  /* Start transmitting the first packet at time 0. */
  sender_transmit_next(simulation_run);

  /* Execute events until we are finished. */
  if(parameters.number_of_packets > 0) {
    /* Finite transfer: run until the sender knows that all of the
       packets have been delivered. */
    while(data.completion_time < 0) {
      simulation_run_execute_event(simulation_run);
    }
  } else {
    /* Continuously backlogged source. */
    while(data.number_of_packets_delivered < RUNLENGTH) {
      simulation_run_execute_event(simulation_run);
    }
  }

  /* Collect the results. */
  results.efficiency = data.number_of_packets_delivered * PACKET_XMT_TIME /
    simulation_run_get_time(simulation_run);
  results.attempts_per_packet = (double) data.number_of_transmissions /
    data.number_of_packets_delivered;
  results.completion_time = data.completion_time;

  /* Clean up memory. */
  cleanup(simulation_run);

  return results;
}

/*******************************************************************************/

/*
 * Usage: arq_sim [part]
 *
 * part is the experiment number in Section 2 of the lab (1 to 7). With
 * no argument all of the experiments are run. The results are written
 * to the results directory.
 */

int
main(int argc, char * argv[])
{
  int part = 0;

  if(argc > 1) part = atoi(argv[1]);

  if(part == 0 || part == 1) experiment_part1();
  if(part == 0 || part == 2) experiment_part2();
  if(part == 0 || part == 3) experiment_part3();
  if(part == 0 || part == 4) experiment_part4();
  if(part == 0 || part == 5) experiment_part5();
  if(part == 0 || part == 6) experiment_part6();
  if(part == 0 || part == 7) experiment_part7();

  return 0;
}
