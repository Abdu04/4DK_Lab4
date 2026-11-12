
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

#ifndef _SENDER_H_
#define _SENDER_H_

/*******************************************************************************/

#include "main.h"

/*******************************************************************************/

/*
 * Function prototypes
 */

void
sender_transmit_next(Simulation_Run_Ptr);

long int
schedule_transmission_end_event(Simulation_Run_Ptr, Time, void *);

void
transmission_end_event(Simulation_Run_Ptr, void *);

long int
schedule_timeout_event(Simulation_Run_Ptr, Time, void *);

void
timeout_event(Simulation_Run_Ptr, void *);

long int
schedule_ack_arrival_event(Simulation_Run_Ptr, Time, void *);

void
ack_arrival_event(Simulation_Run_Ptr, void *);

/*******************************************************************************/

#endif /* sender.h */
