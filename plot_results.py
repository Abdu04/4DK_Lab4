"""
Simulation of ARQ Protocols (S&W, GBN and SR) -- COE 4DK4 Lab 4

Draws the plots requested in Section 2 of the lab from the CSV files that
arq_sim writes to the results directory. Each marker is the mean over the
random seeds and the error bar is the standard deviation over the seeds.
Only markers are drawn for simulation results (no interpolation). Dashed
lines are analytical results.

Usage: python3 plot_results.py
"""

import csv
import statistics
from collections import defaultdict

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

RESULTS = "results/"

# Must match simparameters.h (ms).
T_XMT = 1.0
ROUND_TRIP = 10.0
TIMEOUT = 15.0

STYLE = {
    "S&W":      dict(marker="x", color="black"),
    "GBN":      dict(marker="s", color="tab:red"),
    "SR Wr=2":  dict(marker="^", color="tab:green"),
    "SR Wr=8":  dict(marker="D", color="tab:orange"),
    "SR Wr=16": dict(marker="o", color="tab:blue"),
}

EFFICIENCY = ("efficiency", "Link efficiency")
ATTEMPTS = ("attempts_per_packet",
            "Mean transmission attempts per delivered packet")
COMPLETION = ("completion_time",
              "Mean transfer completion time (ms, log scale)")


def load(name):
    """Return the rows of one results file."""
    with open(RESULTS + name) as f:
        return list(csv.DictReader(f))


def curves(rows, x, y):
    """Group rows into {curve: (xs, means, stds)} averaging over seeds."""
    groups = defaultdict(lambda: defaultdict(list))
    for row in rows:
        groups[row["curve"]][float(row[x])].append(float(row[y]))
    out = {}
    for curve, points in groups.items():
        xs = sorted(points)
        out[curve] = (xs,
                      [statistics.mean(points[v]) for v in xs],
                      [statistics.pstdev(points[v]) for v in xs])
    return out


def plot(name, x, xlabel, y, title, output, sw_reference=False, analysis=None,
         logx=False, logy=False):
    """One figure. sw_reference draws S&W (a single point) as a level."""
    ylabel = y[1]
    data = curves(load(name), x, y[0])
    fig, ax = plt.subplots(figsize=(10, 5.5))

    for curve, (xs, means, stds) in data.items():
        style = STYLE[curve]
        if curve == "S&W" and sw_reference:
            ax.axhline(means[0], color=style["color"], linestyle=":",
                       label="S&W (Ws = Wr = 1) reference = %.4g" % means[0])
            continue
        label = "GBN (Wr = 1)" if curve == "GBN" else curve
        ax.errorbar(xs, means, yerr=stds, linestyle="none", markersize=6,
                    markerfacecolor="none", capsize=2, label=label, **style)

    if analysis:
        analysis(ax)
    if logx:
        ax.set_xscale("log")
    if logy:
        ax.set_yscale("log")

    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)
    ax.set_title(title, fontsize=10)
    ax.grid(True, which="both", alpha=0.3)
    ax.legend(fontsize=8, loc="upper left", bbox_to_anchor=(1.01, 1.0))
    fig.tight_layout()
    fig.savefig(RESULTS + output, dpi=150)
    plt.close(fig)
    print("Wrote", RESULTS + output)


def part2_analysis(ax):
    ws = list(range(1, 21))
    ax.plot(ws, [min(1.0, w * T_XMT / (T_XMT + ROUND_TRIP)) for w in ws],
            "--", color="gray", linewidth=1,
            label="Analysis: min(1, Ws Tx / (Tx + round trip))")


def part3a_analysis(ax):
    pd = [i / 200 for i in range(101)]
    ax.plot(pd, [1 - p for p in pd], "--", color="tab:blue", linewidth=1,
            label="Analysis, ideal SR (Wr = Ws): 1 - pd")
    ax.plot(pd, [(1 - p) / (1 + p * TIMEOUT / T_XMT) for p in pd], "--",
            color="tab:red", linewidth=1,
            label="Analysis, GBN: (1 - pd) / (1 + pd Tout / Tx)")
    ax.plot(pd, [(1 - p) * T_XMT /
                 ((1 - p) * (T_XMT + ROUND_TRIP) + p * (T_XMT + TIMEOUT))
                 for p in pd], "--", color="black", linewidth=1,
            label="Analysis, S&W")


COMMON = "packet Tx time 1 ms, round trip 10 ms"

plot("part2.csv", "ws", "Send window size Ws (packets)", EFFICIENCY,
     "Part 2: backlogged source, pd = pa = 0, timeout 15 ms, " + COMMON,
     "part2_efficiency.png", sw_reference=True, analysis=part2_analysis)

plot("part3a.csv", "pd", "Data packet error probability pd", EFFICIENCY,
     "Part 3(a): backlogged source, Ws = 32, pa = 0, timeout 15 ms, " + COMMON,
     "part3a_efficiency.png", analysis=part3a_analysis)

for y, tag in ((EFFICIENCY, "efficiency"), (ATTEMPTS, "attempts")):
    plot("part3b.csv", "ws", "Send window size Ws (packets)", y,
         "Part 3(b): backlogged source, pd = 0.1, pa = 0, timeout 15 ms, "
         + COMMON, "part3b_%s.png" % tag, sw_reference=True)

    plot("part4.csv", "pa", "ACK error probability pa", y,
         "Part 4: backlogged source, Ws = 32, pd = 0.1, timeout 15 ms, "
         + COMMON, "part4_%s.png" % tag)

    plot("part5.csv", "timeout",
         "Retransmission timeout Tout (ms, log scale)", y,
         "Part 5: backlogged source, Ws = 32, pd = pa = 0.1, " + COMMON,
         "part5_%s.png" % tag, logx=True)

FINITE = [
    ("part6_case1", "Part 6", 100, "pd = pa = 0"),
    ("part6_case2", "Part 6", 100, "pd = 0.05, pa = 0"),
    ("part6_case3", "Part 6", 100, "pd = pa = 0.05"),
    ("part7_small_case1", "Part 7", 10, "pd = pa = 0"),
    ("part7_small_case3", "Part 7", 10, "pd = pa = 0.05"),
    ("part7_large_case1", "Part 7", 1000, "pd = pa = 0"),
    ("part7_large_case3", "Part 7", 1000, "pd = pa = 0.05"),
]

for name, part, n, errors in FINITE:
    plot(name + ".csv", "ws", "Send window size Ws (packets)", COMPLETION,
         "%s: finite transfer, N = %d packets, %s, timeout 15 ms, %s"
         % (part, n, errors, COMMON), name + ".png", sw_reference=True,
         logy=True)
