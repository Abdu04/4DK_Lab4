
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

/**********************************************************************/

#ifndef _MAIN_H_
#define _MAIN_H_

/**********************************************************************/

#include <stdio.h>
#include "simlib.h"
#include "simparameters.h"

/**********************************************************************/

typedef double Time;

/**********************************************************************/

/*
 * Parameters of one simulation run. The general SR ARQ protocol is
 * simulated. Wr = 1 gives GBN and Ws = Wr = 1 gives S&W.
 */

typedef struct _arq_parameters_
{
  int ws;                 /* send window size */
  int wr;                 /* receive window size */
  double pd;              /* data packet error probability */
  double pa;              /* ACK error probability */
  Time timeout;           /* retransmission timeout (ms) */
  long int number_of_packets; /* 0 = continuously backlogged source */
} Arq_Parameters, * Arq_Parameters_Ptr;

/* Results of one simulation run. */

typedef struct _arq_results_
{
  double efficiency;          /* delivered pkts * Tx time / elapsed time */
  double attempts_per_packet; /* data transmissions / delivered packets */
  Time completion_time;       /* finite transfers only (ms) */
} Arq_Results, * Arq_Results_Ptr;

/**********************************************************************/

/* One copy of a data packet travelling on the forward link. */

typedef struct _packet_
{
  long int seq;
} Packet, * Packet_Ptr;

/*
 * An ACK travelling on the reverse link. rn is the next in-order
 * packet that the receiver expects, i.e., all packets below rn are
 * acknowledged (cumulative). seq is the packet that the receiver has
 * just accepted, or -1 if the data packet was outside the receive
 * window and was discarded (selective).
 */

typedef struct _ack_
{
  long int rn;
  long int seq;
} Ack, * Ack_Ptr;

/* The state of one packet in the send window. */

typedef struct _sender_slot_
{
  long int seq;
  int acked;              /* an ACK has been received for this packet */
  int retransmit;         /* timed out, waiting for the transmitter */
  long int timer_id;      /* timeout event id, 0 if timer not running */
} Sender_Slot, * Sender_Slot_Ptr;

/**********************************************************************/

typedef struct _simulation_run_data_
{
  Arq_Parameters parameters;

  /* Sender state. Outstanding packets are send_base ... next_seq-1. */
  long int send_base;
  long int next_seq;
  Sender_Slot_Ptr send_window;  /* ws slots, packet seq is in seq % ws */
  int transmitter_busy;

  /* Receiver state. The receive window is rcv_base ... rcv_base+wr-1. */
  long int rcv_base;
  int * rcv_window;             /* wr flags, packet seq is in seq % wr */

  /* Statistics. */
  long int number_of_transmissions;
  long int number_of_packets_delivered;
  Time completion_time;         /* < 0 until a finite transfer is done */

  FILE * trace_file;            /* NULL = no event trace */
} Simulation_Run_Data, * Simulation_Run_Data_Ptr;

/**********************************************************************/

/*
 * Function prototypes
 */

Arq_Results
arq_simulation_run(Arq_Parameters, FILE *);

int
main(int, char * []);

/**********************************************************************/

#endif /* main.h */
