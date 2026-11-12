
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

#ifndef _RECEIVER_H_
#define _RECEIVER_H_

/*******************************************************************************/

#include "main.h"

/*******************************************************************************/

/*
 * Function prototypes
 */

long int
schedule_data_arrival_event(Simulation_Run_Ptr, Time, void *);

void
data_arrival_event(Simulation_Run_Ptr, void *);

/*******************************************************************************/

#endif /* receiver.h */
