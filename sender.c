
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
#include "receiver.h"
#include "sender.h"

/*******************************************************************************/

/*
 * Sender helper functions.
 */

static Sender_Slot_Ptr
get_sender_slot(Simulation_Run_Data_Ptr data, long int seq)
{
  return data->send_window + (seq % data->parameters.ws);
}

/* Put one copy of packet seq onto the link. */

static void
start_transmission(Simulation_Run_Ptr simulation_run, long int seq)
{
  Simulation_Run_Data_Ptr data;
  Packet_Ptr new_packet;

  data = (Simulation_Run_Data_Ptr) simulation_run_data(simulation_run);

  data->transmitter_busy = 1;
  data->number_of_transmissions++;

  new_packet = (Packet_Ptr) xmalloc(sizeof(Packet));
  new_packet->seq = seq;

  schedule_transmission_end_event(simulation_run,
				  simulation_run_get_time(simulation_run) +
				  PACKET_XMT_TIME,
				  (void *) new_packet);
}

/* A packet has been acknowledged. Stop its timer. */

static void
acknowledge_packet(Simulation_Run_Ptr simulation_run, Sender_Slot_Ptr slot)
{
  slot->acked = 1;
  slot->retransmit = 0;

  if(slot->timer_id != 0) {
    simulation_run_deschedule_event(simulation_run, slot->timer_id);
    slot->timer_id = 0;
  }
}

/*******************************************************************************/

/*
 * If the transmitter is free, transmit the next packet. Packets that
 * have timed out are retransmitted first, lowest sequence number
 * first. Otherwise a new packet is transmitted if the send window is
 * not full and the source has a packet available.
 */

void
sender_transmit_next(Simulation_Run_Ptr simulation_run)
{
  Simulation_Run_Data_Ptr data;
  Sender_Slot_Ptr slot;
  long int seq;

  data = (Simulation_Run_Data_Ptr) simulation_run_data(simulation_run);

  if(data->transmitter_busy) return;

  /* Look for a packet that needs to be retransmitted. */
  for(seq = data->send_base; seq < data->next_seq; seq++) {
    slot = get_sender_slot(data, seq);

    if(slot->retransmit) {
      slot->retransmit = 0;
      trace(simulation_run, "SENDER   retransmits packet %ld", seq);
      start_transmission(simulation_run, seq);
      return;
    }
  }

  /* Otherwise send a new packet if the window and the source allow it. */
  if(data->next_seq < data->send_base + data->parameters.ws &&
     (data->parameters.number_of_packets == 0 ||
      data->next_seq < data->parameters.number_of_packets)) {

    seq = data->next_seq++;

    slot = get_sender_slot(data, seq);
    slot->seq = seq;
    slot->acked = 0;
    slot->retransmit = 0;
    slot->timer_id = 0;

    trace(simulation_run, "SENDER   transmits new packet %ld "
	  "(send window %ld to %ld)", seq, data->send_base,
	  data->send_base + data->parameters.ws - 1);
    start_transmission(simulation_run, seq);
  }
}

/*******************************************************************************/

long int
schedule_transmission_end_event(Simulation_Run_Ptr simulation_run,
				Time event_time,
				void * packet)
{
  Event event;

  event.description = "End of Packet";
  event.function = transmission_end_event;
  event.attachment = packet;

  return simulation_run_schedule_event(simulation_run, event, event_time);
}

/*******************************************************************************/

/*
 * The last bit of a data packet has left the sender. The packet reaches
 * the receiver one propagation delay later. The retransmission timer of
 * the packet is started now.
 */

void
transmission_end_event(Simulation_Run_Ptr simulation_run, void * packet)
{
  Packet_Ptr this_packet;
  Sender_Slot_Ptr slot;
  Time now;
  Simulation_Run_Data_Ptr data;

  data = (Simulation_Run_Data_Ptr) simulation_run_data(simulation_run);
  now = simulation_run_get_time(simulation_run);

  this_packet = (Packet_Ptr) packet;

  data->transmitter_busy = 0;

  /* Start the timer, unless the packet was ACKed, or was marked to be
     sent again, while this copy was being transmitted. */
  slot = get_sender_slot(data, this_packet->seq);

  if(this_packet->seq >= data->send_base && !slot->acked &&
     !slot->retransmit) {
    slot->timer_id = schedule_timeout_event(simulation_run,
					    now + data->parameters.timeout,
					    (void *) slot);
  }

  /* The packet propagates to the receiver. */
  schedule_data_arrival_event(simulation_run,
			      now + PROPAGATION_DELAY,
			      (void *) this_packet);

  /* The transmitter is free. See if there is something else to send. */
  sender_transmit_next(simulation_run);
}

/*******************************************************************************/

long int
schedule_timeout_event(Simulation_Run_Ptr simulation_run,
		       Time event_time,
		       void * slot)
{
  Event event;

  event.description = "Timeout";
  event.function = timeout_event;
  event.attachment = slot;

  return simulation_run_schedule_event(simulation_run, event, event_time);
}

/*******************************************************************************/

/*
 * The timer of an unacknowledged packet has expired. Mark the packet
 * for retransmission. It is sent as soon as the transmitter is free.
 *
 * The receiver only accepts packets inside its window of Wr packets.
 * While packet seq is missing at the receiver, packets seq + Wr and
 * beyond are outside the receive window and are discarded. These
 * outstanding packets are therefore marked for retransmission as well
 * and are resent in order after packet seq. With Wr = 1 this is GBN
 * (go back and resend everything from the timed out packet). With
 * Wr = Ws only the timed out packet is resent (pure SR).
 */

void
timeout_event(Simulation_Run_Ptr simulation_run, void * ptr)
{
  Sender_Slot_Ptr slot, other_slot;
  long int seq, go_back_count = 0;
  Simulation_Run_Data_Ptr data;

  data = (Simulation_Run_Data_Ptr) simulation_run_data(simulation_run);

  slot = (Sender_Slot_Ptr) ptr;

  slot->timer_id = 0;
  slot->retransmit = 1;

  for(seq = slot->seq + data->parameters.wr; seq < data->next_seq; seq++) {
    other_slot = get_sender_slot(data, seq);

    if(!other_slot->acked && !other_slot->retransmit) {
      other_slot->retransmit = 1;
      go_back_count++;

      if(other_slot->timer_id != 0) {
	simulation_run_deschedule_event(simulation_run, other_slot->timer_id);
	other_slot->timer_id = 0;
      }
    }
  }

  trace(simulation_run, "SENDER   timeout of packet %ld (%ld outstanding "
	"packets beyond the receive window will also be resent)",
	slot->seq, go_back_count);

  sender_transmit_next(simulation_run);
}

/*******************************************************************************/

long int
schedule_ack_arrival_event(Simulation_Run_Ptr simulation_run,
			   Time event_time,
			   void * ack)
{
  Event event;

  event.description = "ACK Arrival";
  event.function = ack_arrival_event;
  event.attachment = ack;

  return simulation_run_schedule_event(simulation_run, event, event_time);
}

/*******************************************************************************/

/*
 * An error free ACK has arrived at the sender. Mark the packets that
 * it acknowledges, slide the send window and transmit new packets.
 */

void
ack_arrival_event(Simulation_Run_Ptr simulation_run, void * ptr)
{
  Ack_Ptr this_ack;
  long int seq;
  Simulation_Run_Data_Ptr data;

  data = (Simulation_Run_Data_Ptr) simulation_run_data(simulation_run);

  this_ack = (Ack_Ptr) ptr;

  /* Cumulative part: all packets below rn have been received. */
  for(seq = data->send_base; seq < this_ack->rn && seq < data->next_seq;
      seq++) {
    acknowledge_packet(simulation_run, get_sender_slot(data, seq));
  }

  /* Selective part: packet seq has been received. */
  if(this_ack->seq >= data->send_base && this_ack->seq < data->next_seq) {
    acknowledge_packet(simulation_run, get_sender_slot(data, this_ack->seq));
  }

  /* Slide the send window past the acknowledged packets. */
  while(data->send_base < data->next_seq &&
	get_sender_slot(data, data->send_base)->acked) {
    data->send_base++;
  }

  trace(simulation_run, "SENDER   receives ACK (RN=%ld, seq=%ld), "
	"send window now %ld to %ld", this_ack->rn, this_ack->seq,
	data->send_base, data->send_base + data->parameters.ws - 1);

  xfree((void *) this_ack);

  /* A finite transfer is complete when every packet has been ACKed. */
  if(data->parameters.number_of_packets > 0 &&
     data->send_base == data->parameters.number_of_packets &&
     data->completion_time < 0) {
    data->completion_time = simulation_run_get_time(simulation_run);
    trace(simulation_run, "SENDER   transfer complete");
  }

  sender_transmit_next(simulation_run);
}
