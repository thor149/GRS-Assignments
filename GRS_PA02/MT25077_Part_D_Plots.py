"""
MT25077_Part_D_Plots.py
Plotting script for PA02: Analysis of Network I/O primitives.

Generates 4 plots:
  1. Throughput (Gbps) vs Message Size
  2. Latency (us) vs Thread Count
  3. Cache Misses vs Message Size
  4. CPU Cycles per Byte Transferred vs Message Size

All values are hardcoded from experimental measurements
(MT25077_Part_B_Results.csv).
Run this script to regenerate the plots:
    python3 MT25077_Part_D_Plots.py

Author: MT25077
"""

import matplotlib
matplotlib.use("Agg")  # Non-interactive backend (no display needed)
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import numpy as np

# ============================================================
# System Configuration
# ============================================================

SYSTEM_INFO = (
    "System: Linux 6.17.0-14-generic | "
    "CPU: AMD Ryzen 7 7840HS (8C/16T @ 3.8GHz) | "
    "RAM: 14 GB"
)

# ============================================================
# Experiment Parameters
# ============================================================

MSG_SIZES = [1024, 4096, 65536, 1048576]          # Bytes
MSG_LABELS = ["1 KB", "4 KB", "64 KB", "1 MB"]
THREAD_COUNTS = [1, 2, 4, 8]

# ============================================================
# HARDCODED EXPERIMENTAL DATA
# Values taken from MT25077_Part_B_Results.csv
# ============================================================

# --- Plot 1: Throughput (Gbps) vs Message Size ---
# Rows: msg_sizes [1KB, 4KB, 64KB, 1MB]
# Measured with thread_count = 4

throughput_two_copy  = [9.3742, 14.4076, 38.9824, 44.0402]
throughput_one_copy  = [7.0840, 13.6835, 42.9614, 48.4065]
throughput_zero_copy = [4.6127, 10.2657, 30.1563, 35.3253]

# --- Plot 2: Latency (us) vs Thread Count ---
# Rows: thread_counts [1, 2, 4, 8]
# Measured with msg_size = 4096

latency_two_copy  = [8.26, 9.10, 9.04, 6.98]
latency_one_copy  = [9.57, 9.29, 9.53, 8.64]
latency_zero_copy = [12.25, 12.25, 12.71, 16.45]

# --- Plot 3: Cache Misses vs Message Size ---
# L1 data cache misses; rows: msg_sizes [1KB, 4KB, 64KB, 1MB]
# Measured with thread_count = 4

l1_misses_two_copy  = [3963320820, 3375278746, 2943379236, 3123572797]
l1_misses_one_copy  = [3237920369, 3187780795, 3218773154, 3461652149]
l1_misses_zero_copy = [2993598361, 3140791657, 3077558012, 3131315716]

# Context switches (used instead of LLC misses which reported 0 on AMD Zen 4 veth)
ctx_sw_two_copy  = [1895182, 2509493, 774042, 853238]
ctx_sw_one_copy  = [1627504, 3328111, 889593, 931450]
ctx_sw_zero_copy = [3209825, 3075449, 1678570, 714696]

# --- Plot 4: CPU Cycles per Byte Transferred ---
# Rows: msg_sizes [1KB, 4KB, 64KB, 1MB]
# Calculated as: total_cycles / total_bytes
# Measured with thread_count = 4

cycles_per_byte_two_copy  = [11.99, 6.52, 2.06, 1.95]
cycles_per_byte_one_copy  = [14.30, 5.96, 1.87, 1.84]
cycles_per_byte_zero_copy = [17.98, 7.99, 2.96, 2.60]


# ============================================================
# Plotting Configuration
# ============================================================

COLORS = {
    "two_copy":  "#e74c3c",  # Red
    "one_copy":  "#2ecc71",  # Green
    "zero_copy": "#3498db",  # Blue
}
MARKERS = {
    "two_copy":  "o",
    "one_copy":  "s",
    "zero_copy": "^",
}
LABELS = {
    "two_copy":  "Two-Copy (send/recv)",
    "one_copy":  "One-Copy (sendmsg+iovec)",
    "zero_copy": "Zero-Copy (MSG_ZEROCOPY)",
}

plt.rcParams.update({
    "font.size": 11,
    "figure.figsize": (9, 6),
    "figure.dpi": 150,
    "axes.grid": True,
    "grid.alpha": 0.3,
})


def plot_throughput_vs_msgsize():
    """Plot 1: Throughput (Gbps) vs Message Size."""
    fig, ax = plt.subplots()

    x = np.arange(len(MSG_SIZES))
    width = 0.25

    ax.bar(x - width, throughput_two_copy,  width, label=LABELS["two_copy"],
           color=COLORS["two_copy"],  edgecolor="black", linewidth=0.5)
    ax.bar(x,         throughput_one_copy,  width, label=LABELS["one_copy"],
           color=COLORS["one_copy"],  edgecolor="black", linewidth=0.5)
    ax.bar(x + width, throughput_zero_copy, width, label=LABELS["zero_copy"],
           color=COLORS["zero_copy"], edgecolor="black", linewidth=0.5)

    ax.set_xlabel("Message Size")
    ax.set_ylabel("Throughput (Gbps)")
    ax.set_title("Throughput vs Message Size\n" + SYSTEM_INFO, fontsize=10)
    ax.set_xticks(x)
    ax.set_xticklabels(MSG_LABELS)
    ax.legend()
    ax.set_ylim(bottom=0)

    plt.tight_layout()
    plt.savefig("MT25077_Plot_Throughput_vs_MsgSize.png", bbox_inches="tight")
    plt.close(fig)


def plot_latency_vs_threads():
    """Plot 2: Latency (us) vs Thread Count."""
    fig, ax = plt.subplots()

    for impl in ["two_copy", "one_copy", "zero_copy"]:
        data = {
            "two_copy":  latency_two_copy,
            "one_copy":  latency_one_copy,
            "zero_copy": latency_zero_copy,
        }[impl]

        ax.plot(THREAD_COUNTS, data,
                marker=MARKERS[impl], color=COLORS[impl],
                label=LABELS[impl], linewidth=2, markersize=8)

    ax.set_xlabel("Thread Count")
    ax.set_ylabel("Average Latency (\u00b5s)")
    ax.set_title("Latency vs Thread Count\n" + SYSTEM_INFO, fontsize=10)
    ax.set_xticks(THREAD_COUNTS)
    ax.legend()
    ax.set_ylim(bottom=0)

    plt.tight_layout()
    plt.savefig("MT25077_Plot_Latency_vs_Threads.png", bbox_inches="tight")
    plt.close(fig)


def plot_cache_misses_vs_msgsize():
    """Plot 3: Cache Misses (L1) and Context Switches vs Message Size."""
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6))

    x = np.arange(len(MSG_SIZES))
    width = 0.25

    # L1 Cache Misses
    ax1.bar(x - width, l1_misses_two_copy,  width, label=LABELS["two_copy"],
            color=COLORS["two_copy"],  edgecolor="black", linewidth=0.5)
    ax1.bar(x,         l1_misses_one_copy,  width, label=LABELS["one_copy"],
            color=COLORS["one_copy"],  edgecolor="black", linewidth=0.5)
    ax1.bar(x + width, l1_misses_zero_copy, width, label=LABELS["zero_copy"],
            color=COLORS["zero_copy"], edgecolor="black", linewidth=0.5)

    ax1.set_xlabel("Message Size")
    ax1.set_ylabel("L1 Data Cache Misses")
    ax1.set_title("L1 Cache Misses vs Message Size")
    ax1.set_xticks(x)
    ax1.set_xticklabels(MSG_LABELS)
    ax1.legend()

    # Context Switches (LLC-load-misses reported 0 on AMD Zen 4 veth)
    ax2.bar(x - width, ctx_sw_two_copy,  width, label=LABELS["two_copy"],
            color=COLORS["two_copy"],  edgecolor="black", linewidth=0.5)
    ax2.bar(x,         ctx_sw_one_copy,  width, label=LABELS["one_copy"],
            color=COLORS["one_copy"],  edgecolor="black", linewidth=0.5)
    ax2.bar(x + width, ctx_sw_zero_copy, width, label=LABELS["zero_copy"],
            color=COLORS["zero_copy"], edgecolor="black", linewidth=0.5)

    ax2.set_xlabel("Message Size")
    ax2.set_ylabel("Context Switches")
    ax2.set_title("Context Switches vs Message Size")
    ax2.set_xticks(x)
    ax2.set_xticklabels(MSG_LABELS)
    ax2.legend()

    fig.suptitle("Cache Misses & Context Switches vs Message Size\n" + SYSTEM_INFO,
                 fontsize=10)
    plt.tight_layout()
    plt.savefig("MT25077_Plot_CacheMisses_vs_MsgSize.png", bbox_inches="tight")
    plt.close(fig)


def plot_cycles_per_byte():
    """Plot 4: CPU Cycles per Byte Transferred vs Message Size."""
    fig, ax = plt.subplots()

    for impl in ["two_copy", "one_copy", "zero_copy"]:
        data = {
            "two_copy":  cycles_per_byte_two_copy,
            "one_copy":  cycles_per_byte_one_copy,
            "zero_copy": cycles_per_byte_zero_copy,
        }[impl]

        ax.plot(MSG_SIZES, data,
                marker=MARKERS[impl], color=COLORS[impl],
                label=LABELS[impl], linewidth=2, markersize=8)

    ax.set_xlabel("Message Size (bytes)")
    ax.set_ylabel("CPU Cycles per Byte")
    ax.set_title("CPU Cycles per Byte vs Message Size\n" + SYSTEM_INFO,
                 fontsize=10)
    ax.set_xscale("log", base=2)
    ax.xaxis.set_major_formatter(ticker.FuncFormatter(
        lambda x, _: f"{int(x)}" if x < 1024
        else f"{int(x/1024)}KB" if x < 1048576
        else f"{int(x/1048576)}MB"
    ))
    ax.legend()
    ax.set_ylim(bottom=0)

    plt.tight_layout()
    plt.savefig("MT25077_Plot_CyclesPerByte_vs_MsgSize.png",
                bbox_inches="tight")
    plt.close(fig)


# ============================================================
# Main: generate all 4 plots
# ============================================================

if __name__ == "__main__":
    print("Generating Plot 1: Throughput vs Message Size...")
    plot_throughput_vs_msgsize()

    print("Generating Plot 2: Latency vs Thread Count...")
    plot_latency_vs_threads()

    print("Generating Plot 3: Cache Misses vs Message Size...")
    plot_cache_misses_vs_msgsize()

    print("Generating Plot 4: CPU Cycles per Byte...")
    plot_cycles_per_byte()

    print("\nAll plots generated successfully.")
    print("PNG files saved in current directory.")
