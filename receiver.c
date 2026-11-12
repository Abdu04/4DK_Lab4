
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
#include "output.h"
#include "sender.h"
#include "receiver.h"

/*******************************************************************************/

long int
schedule_data_arrival_event(Simulation_Run_Ptr simulation_run,
			    Time event_time,
			    void * packet)
{
  Event event;

  event.description = "Data Packet Arrival";
  event.function = data_arrival_event;
  event.attachment = packet;

  return simulation_run_schedule_event(simulation_run, event, event_time);
}

/*******************************************************************************/

/*
 * A data packet has arrived at the receiver. With probability pd it is
 * in error and is discarded without returning an ACK. Otherwise the
 * receiver processes it and returns an ACK, which is lost with
 * probability pa. ACK transmission time is neglected so the ACK reaches
 * the sender one propagation delay later.
 */

void
data_arrival_event(Simulation_Run_Ptr simulation_run, void * packet)
{
  Packet_Ptr this_packet;
  Ack_Ptr new_ack;
  long int seq, ack_seq;
  int * received;
  Simulation_Run_Data_Ptr data;

  data = (Simulation_Run_Data_Ptr) simulation_run_data(simulation_run);

  this_packet = (Packet_Ptr) packet;
  seq = this_packet->seq;
  xfree((void *) this_packet);

  /* Data packet error. */
  if(uniform_generator() < data->parameters.pd) {
    trace(simulation_run, "RECEIVER packet %ld is in ERROR, discarded, no ACK",
	  seq);
    return;
  }

  if(seq < data->rcv_base) {

    /* Duplicate of a packet that was already delivered. ACK it again. */
    ack_seq = seq;
    trace(simulation_run, "RECEIVER packet %ld is a duplicate, discarded",
	  seq);

  } else if(seq < data->rcv_base + data->parameters.wr) {

    /* The packet is inside the receive window. Accept it. */
    ack_seq = seq;
    received = data->rcv_window + (seq % data->parameters.wr);

    if(*received) {
      trace(simulation_run, "RECEIVER packet %ld is a duplicate, discarded",
	    seq);
    } else {
      *received = 1;
      trace(simulation_run, "RECEIVER packet %ld accepted "
	    "(receive window %ld to %ld)", seq, data->rcv_base,
	    data->rcv_base + data->parameters.wr - 1);
    }

    /* Deliver in-order packets and slide the receive window. */
    received = data->rcv_window + (data->rcv_base % data->parameters.wr);

    while(*received) {
      *received = 0;
      trace(simulation_run, "RECEIVER packet %ld delivered", data->rcv_base);

      data->rcv_base++;
      data->number_of_packets_delivered++;

      received = data->rcv_window + (data->rcv_base % data->parameters.wr);
    }

  } else {

    /* The packet is beyond the receive window. Discard it. */
    ack_seq = -1;
    trace(simulation_run, "RECEIVER packet %ld is outside the receive window "
	  "%ld to %ld, discarded", seq, data->rcv_base,
	  data->rcv_base + data->parameters.wr - 1);
  }

  /* ACK error. */
  if(uniform_generator() < data->parameters.pa) {
    trace(simulation_run, "RECEIVER sends ACK (RN=%ld, seq=%ld), ACK is LOST",
	  data->rcv_base, ack_seq);
    return;
  }

  new_ack = (Ack_Ptr) xmalloc(sizeof(Ack));
  new_ack->rn = data->rcv_base;
  new_ack->seq = ack_seq;

  trace(simulation_run, "RECEIVER sends ACK (RN=%ld, seq=%ld)",
	new_ack->rn, new_ack->seq);

  schedule_ack_arrival_event(simulation_run,
			     simulation_run_get_time(simulation_run) +
			     PROPAGATION_DELAY,
			     (void *) new_ack);
}
