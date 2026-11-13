
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

#ifndef _SIMPARAMETERS_H_
#define _SIMPARAMETERS_H_

/*******************************************************************************/

/*
 * Default system parameters (Section 1 of the lab). All times in the
 * simulation are in milliseconds.
 */

#define PACKET_LENGTH 1000          /* bits */
#define LINK_BIT_RATE 1e6           /* bits per second */
#define PACKET_XMT_TIME (1e3 * PACKET_LENGTH / LINK_BIT_RATE)   /* = 1 ms */
#define PROPAGATION_DELAY 5.0       /* ms, in each direction */
#define ROUND_TRIP_DELAY (2 * PROPAGATION_DELAY)                /* = 10 ms */
#define TIMEOUT 15.0                /* ms, default retransmission timeout */

/* Smallest send window that fully utilizes the error free link (= 11). */
#define FULL_UTILIZATION_WS ((PACKET_XMT_TIME + ROUND_TRIP_DELAY) / PACKET_XMT_TIME)

/*
 * Run lengths. A backlogged run ends after RUNLENGTH packets have been
 * delivered. A finite transfer is repeated REPLICATIONS times for each
 * random seed and the completion times are averaged.
 */

#define RUNLENGTH 100000
#define REPLICATIONS 200

/*
 * Comma separated list of random seeds to run. The same seeds are used for
 * every protocol. STUDENT_ID is the student number of a group member.
 */

#define STUDENT_ID 400435085
#define RANDOM_SEED_LIST STUDENT_ID, 333333, 222222, 44444

/*******************************************************************************/

/*
 * Experiment parameters (Section 2 of the lab).
 */

/* Receive window sizes used for the SR curves. 0 means Wr = Ws. */
#define SR_WR_LIST 2, 8, 0

/* Send window used when Ws is held fixed (Parts 3(a), 4 and 5). It is
   larger than FULL_UTILIZATION_WS, and also large enough that the SR send
   window does not stall while one lost packet is being retransmitted
   (Tx time + timeout + Tx time + round trip = 27 packet times). */
#define FIXED_WS 32

/* Part 1: short traces of a small finite transfer. */
#define TRACE_NUMBER_OF_PACKETS 6
#define TRACE_WS 4                  /* GBN: Wr = 1, SR: Wr = TRACE_WS */
#define TRACE_ERROR_PROBABILITY 0.5

/* Part 2: Ws = 1, ..., PART2_MAX_WS with pd = pa = 0. */
#define PART2_MAX_WS 20

/* Part 3(a): efficiency versus pd. */
#define PART3A_PD_LIST 0, 0.01, 0.02, 0.05, 0.1, 0.15, 0.2, 0.25, 0.3, 0.4, 0.5

/* Part 3(b): Ws = 1, ..., PART3B_MAX_WS with pd fixed. */
#define PART3B_PD 0.1
#define PART3B_MAX_WS 40

/* Part 4: efficiency versus pa with pd fixed. */
#define PART4_PD 0.1
#define PART4_PA_LIST 0, 0.05, 0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5

/* Part 5: timeout sweep (ms). The round-trip time is 10 ms. */
#define PART5_PD 0.1
#define PART5_PA 0.1
#define PART5_TIMEOUT_LIST 2, 4, 6, 8, 9, 11, 12, 13, 15, 20, 25, 30, 40, 50, 75, 100

/* Parts 6 and 7: finite transfers, Ws = 1, ..., FINITE_MAX_WS. */
#define FINITE_MAX_WS 40
#define PART6_NUMBER_OF_PACKETS 100
#define PART6_ERROR_PROBABILITY 0.05
#define PART7_SMALL_NUMBER_OF_PACKETS 10
#define PART7_LARGE_NUMBER_OF_PACKETS 1000

/*******************************************************************************/

#endif /* simparameters.h */
