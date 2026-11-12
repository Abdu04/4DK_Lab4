
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
#include <stdarg.h>
#include "simparameters.h"
#include "main.h"
#include "output.h"

/*******************************************************************************/

/*
 * Write one line of the event trace, stamped with the current time.
 * Nothing is written unless the run has a trace file.
 */

void
trace(Simulation_Run_Ptr simulation_run, const char * format, ...)
{
  va_list arguments;
  Simulation_Run_Data_Ptr data;

  data = (Simulation_Run_Data_Ptr) simulation_run_data(simulation_run);

  if(data->trace_file == NULL) return;

  fprintf(data->trace_file, "%8.3f ms  ",
	  simulation_run_get_time(simulation_run));

  va_start(arguments, format);
  vfprintf(data->trace_file, format, arguments);
  va_end(arguments);

  fprintf(data->trace_file, "\n");
}

/*******************************************************************************/

/*
 * The results are written as one CSV line per curve, data point and
 * random seed.
 */

void
output_header(FILE * file)
{
  fprintf(file, "curve,ws,wr,pd,pa,timeout,number_of_packets,seed,"
	  "efficiency,attempts_per_packet,completion_time\n");
}

void
output_results(FILE * file, const char * curve, Arq_Parameters parameters,
	       unsigned random_seed, Arq_Results results)
{
  fprintf(file, "%s,%d,%d,%g,%g,%g,%ld,%u,%.6f,%.6f,%.4f\n",
	  curve, parameters.ws, parameters.wr, parameters.pd, parameters.pa,
	  parameters.timeout, parameters.number_of_packets, random_seed,
	  results.efficiency, results.attempts_per_packet,
	  results.completion_time);
}
