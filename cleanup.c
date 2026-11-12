
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

#include "simlib.h"
#include "main.h"
#include "sender.h"
#include "cleanup.h"

/*******************************************************************************/

void
cleanup (Simulation_Run_Ptr simulation_run)
{
  Simulation_Run_Data_Ptr data;
  Event_Container_Ptr container;

  data = (Simulation_Run_Data_Ptr) simulation_run_data(simulation_run);

  /* Clean out the packets and ACKs that are still on the link. Timeout
     events point into the send window, which is freed below. */
  while (simulation_run->eventlist->size > 0) {
    container = simulation_run->eventlist->front_ptr;

    if (container->event.function != timeout_event) {
      xfree(container->event.attachment);
    }
    simulation_run_deschedule_event(simulation_run, container->event_id);
  }

  /* Clean out the sender and the receiver. */
  xfree(data->send_window);
  xfree(data->rcv_window);

  /* Clean up the simulation_run. */
  simulation_run_free_memory(simulation_run);
}
