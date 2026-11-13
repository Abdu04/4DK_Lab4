# COE 4DK4 Lab 4 — Simulation of ARQ Protocols

Discrete-event simulation of a general Selective-Repeat ARQ link with send
window `Ws` and receive window `Wr`, written on the same `simlib` library as
the earlier labs. `Wr = 1` gives GBN and `Ws = Wr = 1` gives S&W.

## Running

```bash
make            # builds ./arq_sim
./arq_sim       # all experiments (about 2 minutes)
./arq_sim 3     # only Part 3 (any part 1 to 7)
python3 plot_results.py   # needs matplotlib
```

Everything is written to `results/`: `part1_trace.txt` (Part 1 event traces),
one CSV per experiment (one line per curve, data point and seed) and one PNG
per requested plot.

## Files

| File | Contents |
| --- | --- |
| `simparameters.h` | Link parameters, run lengths, seeds, and every experiment's parameter values |
| `main.c/h` | Data structures, one simulation run (`arq_simulation_run`), `main` |
| `sender.c/h` | Send window, transmission end, timeout and ACK arrival events |
| `receiver.c/h` | Data packet arrival event: errors, receive window, ACK generation |
| `experiments.c/h` | The parameter sweeps for Parts 1 to 7 |
| `output.c/h` | Event trace and CSV output |
| `cleanup.c/h` | Frees a run's memory |
| `simlib.c/h`, `trace.h` | Unchanged from Lab 3 |
| `plot_results.py` | Draws the plots |

## Model and assumptions (for the report)

- Times are in ms: packet transmission 1 ms, propagation 5 ms each way,
  timeout 15 ms. ACK transmission and processing times are zero.
- Events: end of data packet transmission, data packet arrival at the
  receiver, ACK arrival at the sender, and timeout.
- Each packet has its own timer, started when its last bit leaves the sender
  (so an ACK returns 10 ms after the timer starts).
- The sender has one transmitter. When it is free, timed-out packets are sent
  first (lowest sequence number first), otherwise a new packet is sent if the
  send window allows.
- A data packet is in error with probability `pd` (drawn when it reaches the
  receiver); it is discarded and no ACK is sent. Every error-free arrival
  produces an ACK, lost with probability `pa`.
- An ACK carries `RN` (next packet expected in order: cumulative) and the
  sequence number just accepted (selective). Duplicates are ACKed again;
  packets beyond the receive window are discarded and ACKed with `RN` only.
- On a timeout of packet `i` the sender resends `i` and also every
  outstanding un-ACKed packet from `i + Wr` up, because those were outside
  the receive window while `i` was missing. For `Wr = 1` this is go-back-N;
  for `Wr = Ws` only `i` is resent.
- Link efficiency = packets delivered in order x 1 ms / elapsed time.
  Attempts per packet = data transmissions / packets delivered.
- Backlogged runs stop after 100000 delivered packets, for each of 4 seeds.
  Finite transfers are repeated 200 times per seed and averaged. Plot markers
  are the mean over seeds; error bars are the standard deviation over seeds.
