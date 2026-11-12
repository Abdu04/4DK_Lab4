
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

#ifndef _OUTPUT_H_
#define _OUTPUT_H_

/*******************************************************************************/

#include "main.h"

/*******************************************************************************/

/*
 * Function prototypes
 */

void
trace(Simulation_Run_Ptr, const char *, ...);

void
output_header(FILE *);

void
output_results(FILE *, const char *, Arq_Parameters, unsigned, Arq_Results);

/*******************************************************************************/

#endif /* output.h */
