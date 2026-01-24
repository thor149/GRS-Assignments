#!/usr/bin/env python3

"""
MT25077_Part_D_plot.py

Python script for Part D: Automated plot generation from measurement data
Roll Number: MT25077

This script reads the CSV data from Part D experiments and generates
comprehensive plots for analysis:
  1. CPU% vs Number of Workers
  2. Memory Usage vs Number of Workers
  3. Execution Time vs Number of Workers
  4. I/O Usage vs Number of Workers
  5. Side-by-side comparison of Processes vs Threads

Input: MT25077_Part_D_CSV.csv
Output: MT25077_Part_D_Plots.pdf (multi-page PDF with all plots)
"""

import sys
import os
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.backends.backend_pdf as pdf_backend
import numpy as np

# Configuration
CSV_FILE = "MT25077_Part_D_CSV.csv"
OUTPUT_PDF = "MT25077_Part_D_Plots.pdf"

# Plot styling
plt.style.use('seaborn-v0_8-darkgrid')
COLORS = {
    'cpu': '#FF6B6B',  # Red
    'mem': '#4ECDC4',  # Teal
    'io': '#45B7D1',   # Blue
}
MARKERS = {
    'a': 'o',  # Circle for processes
    'b': 's',  # Square for threads
}
LINE_STYLES = {
    'a': '-',   # Solid line for processes
    'b': '--',  # Dashed line for threads
}

def load_data(csv_file):
    """Load and validate CSV data."""
    try:
        df = pd.read_csv(csv_file)
        print(f"[INFO] Loaded data from {csv_file}")
        print(f"[INFO] Shape: {df.shape[0]} rows, {df.shape[1]} columns")
        print(f"[INFO] Columns: {', '.join(df.columns)}")

        # Validate required columns
        required_columns = ['Program', 'Function', 'Workers', 'CPU%', 'Memory(KB)', 'IO(KB/s)', 'Time(s)']
        missing_columns = [col for col in required_columns if col not in df.columns]
        if missing_columns:
            print(f"[ERROR] Missing required columns: {', '.join(missing_columns)}")
            sys.exit(1)

        return df
    except FileNotFoundError:
        print(f"[ERROR] File not found: {csv_file}")
        print("[ERROR] Please run Part D measurement script first.")
        sys.exit(1)
    except Exception as e:
        print(f"[ERROR] Failed to load CSV: {e}")
        sys.exit(1)

def create_plots(df):
    """Create all plots and return figure list."""
    figures = []

    # Plot 1: CPU% vs Workers (grouped by function, separate lines for programs)
    print("[INFO] Creating Plot 1: CPU% vs Workers")
    fig1, ax1 = plt.subplots(figsize=(12, 8))
    for program in ['a', 'b']:
        for function in ['cpu', 'mem', 'io']:
            data = df[(df['Program'] == program) & (df['Function'] == function)]
            label = f"Program {program.upper()} - {function.upper()}"
            ax1.plot(data['Workers'], data['CPU%'],
                    marker=MARKERS[program],
                    linestyle=LINE_STYLES[program],
                    color=COLORS[function],
                    linewidth=2,
                    markersize=8,
                    label=label)

    ax1.set_xlabel('Number of Workers', fontsize=12, fontweight='bold')
    ax1.set_ylabel('CPU Utilization (%)', fontsize=12, fontweight='bold')
    ax1.set_title('CPU% vs Number of Workers\n(MT25077)', fontsize=14, fontweight='bold')
    ax1.legend(loc='best', fontsize=10)
    ax1.grid(True, alpha=0.3)
    figures.append(fig1)

    # Plot 2: Memory Usage vs Workers
    print("[INFO] Creating Plot 2: Memory Usage vs Workers")
    fig2, ax2 = plt.subplots(figsize=(12, 8))
    for program in ['a', 'b']:
        for function in ['cpu', 'mem', 'io']:
            data = df[(df['Program'] == program) & (df['Function'] == function)]
            label = f"Program {program.upper()} - {function.upper()}"
            ax2.plot(data['Workers'], data['Memory(KB)'] / 1024,  # Convert to MB
                    marker=MARKERS[program],
                    linestyle=LINE_STYLES[program],
                    color=COLORS[function],
                    linewidth=2,
                    markersize=8,
                    label=label)

    ax2.set_xlabel('Number of Workers', fontsize=12, fontweight='bold')
    ax2.set_ylabel('Peak Memory Usage (MB)', fontsize=12, fontweight='bold')
    ax2.set_title('Memory Usage vs Number of Workers\n(MT25077)', fontsize=14, fontweight='bold')
    ax2.legend(loc='best', fontsize=10)
    ax2.grid(True, alpha=0.3)
    figures.append(fig2)

    # Plot 3: Execution Time vs Workers
    print("[INFO] Creating Plot 3: Execution Time vs Workers")
    fig3, ax3 = plt.subplots(figsize=(12, 8))
    for program in ['a', 'b']:
        for function in ['cpu', 'mem', 'io']:
            data = df[(df['Program'] == program) & (df['Function'] == function)]
            label = f"Program {program.upper()} - {function.upper()}"
            ax3.plot(data['Workers'], data['Time(s)'],
                    marker=MARKERS[program],
                    linestyle=LINE_STYLES[program],
                    color=COLORS[function],
                    linewidth=2,
                    markersize=8,
                    label=label)

    ax3.set_xlabel('Number of Workers', fontsize=12, fontweight='bold')
    ax3.set_ylabel('Execution Time (seconds)', fontsize=12, fontweight='bold')
    ax3.set_title('Execution Time vs Number of Workers\n(MT25077)', fontsize=14, fontweight='bold')
    ax3.legend(loc='best', fontsize=10)
    ax3.grid(True, alpha=0.3)
    figures.append(fig3)

    # Plot 4: I/O Usage vs Workers
    print("[INFO] Creating Plot 4: I/O Usage vs Workers")
    fig4, ax4 = plt.subplots(figsize=(12, 8))
    for program in ['a', 'b']:
        for function in ['cpu', 'mem', 'io']:
            data = df[(df['Program'] == program) & (df['Function'] == function)]
            label = f"Program {program.upper()} - {function.upper()}"
            ax4.plot(data['Workers'], data['IO(KB/s)'],
                    marker=MARKERS[program],
                    linestyle=LINE_STYLES[program],
                    color=COLORS[function],
                    linewidth=2,
                    markersize=8,
                    label=label)

    ax4.set_xlabel('Number of Workers', fontsize=12, fontweight='bold')
    ax4.set_ylabel('I/O Throughput (KB/s)', fontsize=12, fontweight='bold')
    ax4.set_title('I/O Usage vs Number of Workers\n(MT25077)', fontsize=14, fontweight='bold')
    ax4.legend(loc='best', fontsize=10)
    ax4.grid(True, alpha=0.3)
    figures.append(fig4)

    # Plot 5: Comparison - Processes vs Threads (2x2 subplots for each metric)
    print("[INFO] Creating Plot 5: Processes vs Threads Comparison")
    fig5, axes = plt.subplots(2, 2, figsize=(16, 12))
    fig5.suptitle('Processes vs Threads Comparison by Worker Type\n(MT25077)',
                  fontsize=16, fontweight='bold')

    # Define metrics and their properties
    metrics = [
        ('CPU%', 'CPU Utilization (%)'),
        ('Memory(KB)', 'Memory Usage (MB)'),
        ('Time(s)', 'Execution Time (s)'),
        ('IO(KB/s)', 'I/O Throughput (KB/s)')
    ]

    for idx, (metric, ylabel) in enumerate(metrics):
        ax = axes[idx // 2, idx % 2]

        for function in ['cpu', 'mem', 'io']:
            # Program A (processes)
            data_a = df[(df['Program'] == 'a') & (df['Function'] == function)]
            # Program B (threads) - filter to match Program A's worker counts
            data_b = df[(df['Program'] == 'b') & (df['Function'] == function) &
                       (df['Workers'].isin([2, 3, 4, 5]))]

            # Convert memory to MB if needed
            if metric == 'Memory(KB)':
                ax.plot(data_a['Workers'], data_a[metric] / 1024,
                       marker='o', linestyle='-', linewidth=2, markersize=8,
                       color=COLORS[function], label=f'{function.upper()} (Processes)')
                ax.plot(data_b['Workers'], data_b[metric] / 1024,
                       marker='s', linestyle='--', linewidth=2, markersize=8,
                       color=COLORS[function], label=f'{function.upper()} (Threads)')
            else:
                ax.plot(data_a['Workers'], data_a[metric],
                       marker='o', linestyle='-', linewidth=2, markersize=8,
                       color=COLORS[function], label=f'{function.upper()} (Processes)')
                ax.plot(data_b['Workers'], data_b[metric],
                       marker='s', linestyle='--', linewidth=2, markersize=8,
                       color=COLORS[function], label=f'{function.upper()} (Threads)')

        ax.set_xlabel('Number of Workers', fontsize=11, fontweight='bold')
        ax.set_ylabel(ylabel, fontsize=11, fontweight='bold')
        ax.legend(loc='best', fontsize=9)
        ax.grid(True, alpha=0.3)

    plt.tight_layout()
    figures.append(fig5)

    # Plot 6: Scalability Analysis - Speedup for each worker type
    print("[INFO] Creating Plot 6: Scalability Analysis")
    fig6, axes = plt.subplots(1, 3, figsize=(18, 6))
    fig6.suptitle('Scalability Analysis: Execution Time Speedup\n(MT25077)',
                  fontsize=16, fontweight='bold')

    for idx, function in enumerate(['cpu', 'mem', 'io']):
        ax = axes[idx]

        for program in ['a', 'b']:
            data = df[(df['Program'] == program) & (df['Function'] == function)].sort_values('Workers')
            if len(data) > 0:
                baseline_time = data.iloc[0]['Time(s)']  # Time with minimum workers
                speedup = baseline_time / data['Time(s)']

                label = f"Program {program.upper()}"
                ax.plot(data['Workers'], speedup,
                       marker=MARKERS[program],
                       linestyle=LINE_STYLES[program],
                       linewidth=2,
                       markersize=8,
                       label=label)

        # Add ideal linear speedup line
        max_workers = df['Workers'].max()
        ax.plot([2, max_workers], [1, max_workers/2],
               'k:', linewidth=1, label='Ideal Linear Speedup')

        ax.set_xlabel('Number of Workers', fontsize=11, fontweight='bold')
        ax.set_ylabel('Speedup', fontsize=11, fontweight='bold')
        ax.set_title(f'{function.upper()} Worker', fontsize=12, fontweight='bold')
        ax.legend(loc='best', fontsize=9)
        ax.grid(True, alpha=0.3)

    plt.tight_layout()
    figures.append(fig6)

    return figures

def save_plots(figures, output_file):
    """Save all figures to a multi-page PDF."""
    print(f"[INFO] Saving plots to {output_file}...")

    with pdf_backend.PdfPages(output_file) as pdf:
        for fig in figures:
            pdf.savefig(fig, bbox_inches='tight')
            plt.close(fig)

    print(f"[INFO] Successfully saved {len(figures)} plots to {output_file}")

def print_statistics(df):
    """Print summary statistics."""
    print("\n" + "="*70)
    print("Summary Statistics (MT25077)")
    print("="*70)

    for program in ['a', 'b']:
        program_name = "Process-based (A)" if program == 'a' else "Thread-based (B)"
        print(f"\n{program_name}:")
        print("-"*70)

        for function in ['cpu', 'mem', 'io']:
            data = df[(df['Program'] == program) & (df['Function'] == function)]

            if len(data) > 0:
                print(f"\n  {function.upper()} Worker:")
                print(f"    Workers range:  {data['Workers'].min()} - {data['Workers'].max()}")
                print(f"    Avg CPU%:       {data['CPU%'].mean():.2f}%")
                print(f"    Avg Memory:     {data['Memory(KB)'].mean() / 1024:.2f} MB")
                print(f"    Avg Time:       {data['Time(s)'].mean():.2f} seconds")
                print(f"    Avg I/O:        {data['IO(KB/s)'].mean():.2f} KB/s")

    print("\n" + "="*70)

def main():
    """Main function."""
    print("="*70)
    print("Part D: Automated Plot Generation")
    print("Roll Number: MT25077")
    print("="*70)
    print()

    # Load data
    df = load_data(CSV_FILE)

    # Print statistics
    print_statistics(df)

    # Create plots
    print("\n" + "="*70)
    print("Generating Plots...")
    print("="*70)
    figures = create_plots(df)

    # Save plots
    save_plots(figures, OUTPUT_PDF)

    # Summary
    print("\n" + "="*70)
    print("Plot Generation Complete!")
    print("="*70)
    print(f"Input file:  {CSV_FILE}")
    print(f"Output file: {OUTPUT_PDF}")
    print(f"Total plots: {len(figures)}")
    print("="*70)

if __name__ == "__main__":
    main()
