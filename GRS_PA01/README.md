# GRS Programming Assignment 01
**Roll Number:** MT25077
**Course:** Graduate Operating Systems
**Assignment:** Process and Thread Performance Analysis

---

## Table of Contents
- [Overview](#overview)
- [Assignment Structure](#assignment-structure)
- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Building the Programs](#building-the-programs)
- [Running the Programs](#running-the-programs)
- [Measurement Scripts](#measurement-scripts)
- [Generating Plots](#generating-plots)
- [File Descriptions](#file-descriptions)
- [Expected Output](#expected-output)
- [Troubleshooting](#troubleshooting)
- [References](#references)

---

## Overview

This assignment implements and analyzes the performance characteristics of process-based and thread-based parallel execution with three types of workloads:
- **CPU-intensive**: Heavy mathematical computations
- **Memory-intensive**: Large memory allocations and operations
- **I/O-intensive**: Substantial disk read/write operations

### Key Components:
1. **Program A**: Process-based parallel execution using `fork()`
2. **Program B**: Thread-based parallel execution using `pthread`
3. **Worker Functions**: Three workload types (cpu, mem, io) with 7000 iterations each
4. **Measurement Scripts**: Automated collection of CPU%, Memory, I/O, and Time metrics
5. **Plotting Scripts**: Visualization of performance trends

---

## Assignment Structure

### Part A: Programs
- **Program A**: Creates multiple child processes using `fork()`
- **Program B**: Creates multiple threads using `pthread`

### Part B: Worker Functions
Three worker functions with 7000 iterations each (based on roll number last digit: 7):
- `worker_cpu()`: CPU-intensive mathematical operations
- `worker_mem()`: Memory-intensive allocations and sorting
- `worker_io()`: I/O-intensive disk operations

### Part C: Basic Measurements
6 combinations tested (2 workers each):
- Program A: cpu, mem, io
- Program B: cpu, mem, io

Metrics collected: CPU%, Memory (KB), I/O (KB/s), Time (s)

### Part D: Scaled Experiments
Varying worker counts:
- Program A: 2, 3, 4, 5 processes × 3 worker types = 12 experiments
- Program B: 2, 3, 4, 5, 6, 7, 8 threads × 3 worker types = 21 experiments
- **Total**: 33 experiments

Comprehensive plots generated for analysis.

---

## Prerequisites

### Required Software
- **GCC Compiler**: Version 7.0 or higher
- **GNU Make**: Build automation
- **GNU Time**: Resource usage measurement (`/usr/bin/time`)
- **Taskset**: CPU affinity control
- **Top**: Process monitoring
- **Python 3**: Version 3.6 or higher (for plotting)

### Optional but Recommended
- **iostat** (from sysstat package): Disk I/O statistics

### Python Packages
```bash
pip3 install matplotlib pandas numpy
```

### System Requirements
- Linux operating system (tested on Ubuntu/Debian)
- At least 2 CPU cores
- At least 4 GB RAM
- At least 10 GB free disk space (for I/O tests)

---

## Installation

### 1. Install System Dependencies

```bash
# Update package list
sudo apt-get update

# Install build essentials
sudo apt-get install build-essential

# Install required tools
sudo apt-get install time procps util-linux

# Install iostat (optional but recommended)
sudo apt-get install sysstat

# Install Python 3 and pip
sudo apt-get install python3 python3-pip
```

### 2. Install Python Packages

```bash
pip3 install matplotlib pandas numpy
```

### 3. Clone or Extract Assignment Files

```bash
# If using Git
git clone <repository-url>
cd MT25077_PA01

# Or extract the ZIP file
unzip MT25077_PA01.zip
cd MT25077_PA01
```

---

## Building the Programs

### Build All Programs
```bash
make
# or
make all
```

This will compile:
- `program_a` - Process-based program
- `program_b` - Thread-based program

### Build Individual Programs
```bash
make program_a    # Build process-based program only
make program_b    # Build thread-based program only
```

### Clean Build Artifacts
```bash
make clean
```

### Run Quick Tests
```bash
make test
```

---

## Running the Programs

### Program A (Process-based)

**Basic Usage:**
```bash
./program_a <cpu|mem|io> [num_processes]
```

**Examples:**
```bash
# Run with 2 child processes (default), CPU worker
./program_a cpu

# Run with 4 child processes, Memory worker
./program_a mem 4

# Run with 5 child processes, I/O worker
./program_a io 5
```

### Program B (Thread-based)

**Basic Usage:**
```bash
./program_b <cpu|mem|io> [num_threads]
```

**Examples:**
```bash
# Run with 2 threads (default), CPU worker
./program_b cpu

# Run with 4 threads, Memory worker
./program_b mem 4

# Run with 8 threads, I/O worker
./program_b io 8
```

---

## Measurement Scripts

### Part C: Basic Measurements

Runs 6 experiments (Program A and B with cpu, mem, io workers, 2 workers each):

```bash
# Using make
make run_part_c

# Or directly
bash MT25077_Part_C_shell.sh
```

**Output:** `MT25077_Part_C_CSV.csv`

### Part D: Scaled Experiments

Runs 33 experiments (varying worker counts):

```bash
# Using make
make run_part_d

# Or directly
bash MT25077_Part_D_shell.sh
```

**Output:** `MT25077_Part_D_CSV.csv`

### Measurement Details

Both scripts collect:
- **CPU%**: Average CPU utilization during execution
- **Memory**: Peak memory usage in KB
- **I/O**: Disk I/O throughput in KB/s (if iostat available)
- **Time**: Total execution time in seconds

**CPU Affinity:** Programs are pinned to CPUs 0-1 using `taskset -c 0-1`

---

## Generating Plots

### Generate Plots from Part D Data

```bash
# Using make
make plots

# Or directly
python3 MT25077_Part_D_plot.py
```

**Output:** `MT25077_Part_D_Plots.pdf` (multi-page PDF with 6 plots)

### Generated Plots:
1. **CPU% vs Workers**: Shows CPU utilization trends
2. **Memory vs Workers**: Shows memory usage patterns
3. **Execution Time vs Workers**: Shows performance scaling
4. **I/O vs Workers**: Shows I/O throughput trends
5. **Processes vs Threads Comparison**: Side-by-side comparison
6. **Scalability Analysis**: Speedup analysis

---

## File Descriptions

### Source Files
| File | Description |
|------|-------------|
| `MT25077_Part_A_Program_A.c` | Process-based program using fork() |
| `MT25077_Part_A_Program_B.c` | Thread-based program using pthread |
| `MT25077_Part_B_workers.c` | Worker function implementations |
| `MT25077_Part_B_workers.h` | Worker function declarations |

### Build Files
| File | Description |
|------|-------------|
| `Makefile` | Build automation with targets for compilation and testing |

### Measurement Scripts
| File | Description |
|------|-------------|
| `MT25077_Part_C_shell.sh` | Part C measurement automation (6 experiments) |
| `MT25077_Part_D_shell.sh` | Part D measurement automation (33 experiments) |
| `MT25077_Part_D_plot.py` | Python script for plot generation |

### Output Files (Generated)
| File | Description |
|------|-------------|
| `MT25077_Part_C_CSV.csv` | Part C measurement results |
| `MT25077_Part_D_CSV.csv` | Part D measurement results |
| `MT25077_Part_D_Plots.pdf` | Generated plots (multi-page PDF) |

### Documentation
| File | Description |
|------|-------------|
| `README.md` | This documentation file |
| `MT25077_Report.pdf` | Final report with analysis (to be created manually) |

### Configuration
| File | Description |
|------|-------------|
| `.gitignore` | Git ignore rules for binaries and temporary files |

---

## Expected Output

### Part C CSV Format
```csv
Program+Function,CPU%,Memory(KB),IO(KB/s),Time(s)
a+cpu,198.5,2048,12.3,15.67
a+mem,145.2,524288,234.5,28.34
a+io,87.3,4096,8934.2,45.12
b+cpu,201.2,2056,10.8,14.89
b+mem,142.8,518144,198.7,27.56
b+io,89.1,4120,8756.4,44.87
```

### Part D CSV Format
```csv
Program,Function,Workers,CPU%,Memory(KB),IO(KB/s),Time(s)
a,cpu,2,198.5,2048,12.3,15.67
a,cpu,3,287.3,3072,18.9,11.23
...
b,io,8,356.7,32768,71234.5,98.34
```

### Program Output Example
```
=================================================================
Program A: Process-based execution using fork()
Roll Number: MT25077
=================================================================
Configuration:
  Worker Type:       cpu
  Number of Processes: 2 child processes
  Loop Count per Worker: 7000 iterations
  Parent PID:        12345
=================================================================

Creating 2 child processes...
  Child 1: PID = 12346, executing cpu worker
  Child 2: PID = 12347, executing cpu worker

Parent (PID 12345): Waiting for all 2 child processes to complete...
  Child PID 12346 completed successfully
  Child PID 12347 completed successfully

=================================================================
Execution Summary:
  Total child processes: 2
  Successful completions: 2
  Failed processes: 0
=================================================================
```

---

## Troubleshooting

### Issue: "iostat not found"
**Solution:**
```bash
sudo apt-get install sysstat
```
**Alternative:** Scripts will continue without I/O measurements if iostat is unavailable.

### Issue: "Compilation errors with pthread"
**Solution:** Ensure `-lpthread` flag is used. The Makefile already includes this.

### Issue: "Permission denied" when running scripts
**Solution:**
```bash
chmod +x MT25077_Part_C_shell.sh
chmod +x MT25077_Part_D_shell.sh
chmod +x MT25077_Part_D_plot.py
```

### Issue: "Python module not found"
**Solution:**
```bash
pip3 install --user matplotlib pandas numpy
# or
sudo pip3 install matplotlib pandas numpy
```

### Issue: "Temporary files filling up /tmp"
**Solution:** Scripts automatically clean up temporary files. If needed:
```bash
rm -rf /tmp/MT25077_measurements*
rm -f /tmp/io_test_*.dat
```

### Issue: "Top/taskset command not found"
**Solution:**
```bash
sudo apt-get install procps util-linux
```

---

## References

### Man Pages
- `man fork` - Process creation
- `man pthread_create` - Thread creation
- `man top` - Process monitoring
- `man iostat` - I/O statistics
- `man taskset` - CPU affinity

### Documentation
- POSIX Threads Programming: https://computing.llnl.gov/tutorials/pthreads/
- Linux System Programming: Process creation and management
- GNU Make Manual: https://www.gnu.org/software/make/manual/

---

## Notes

1. **Loop Count**: Based on roll number last digit (7), each worker function executes 7000 iterations.

2. **CPU Affinity**: All experiments pin execution to CPUs 0-1 using `taskset -c 0-1` for consistent measurements.

3. **Measurement Accuracy**: Each metric is sampled multiple times during execution, and averages are calculated.

4. **Resource Cleanup**: All programs properly free allocated memory, close files, and wait for child processes/threads.

5. **Code Quality**: Source code follows modular design with consistent indentation (4 spaces), comprehensive comments, and error handling.

---

## Contact

**Roll Number:** MT25077
**Course:** Graduate Operating Systems
**Institution:** IIIT Delhi

For questions or issues, please refer to the course materials or contact the instructor.

---

## License

This code is submitted as part of an academic assignment. Please adhere to your institution's academic integrity policies.

---

**Last Updated:** January 22, 2026
**Version:** 1.0
