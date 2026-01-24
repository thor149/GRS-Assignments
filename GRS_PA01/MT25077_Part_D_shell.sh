#!/bin/bash

################################################################################
# MT25077_Part_D_shell.sh
#
# Bash script for Part D: Scaled experiments with varying worker counts
# Roll Number: MT25077
#
# This script automates the execution and measurement of programs with
# varying numbers of workers:
#   - Program A (process-based): 2, 3, 4, 5 processes
#   - Program B (thread-based): 2, 3, 4, 5, 6, 7, 8 threads
#   - Each configuration tested with: cpu, mem, io workers
#
# Total experiments: (4 × 3) + (7 × 3) = 12 + 21 = 33 experiments
#
# Measurements collected:
#   - CPU%: Average CPU utilization (using top)
#   - Memory: Peak memory usage in KB (using top and /usr/bin/time)
#   - I/O: Disk I/O statistics in KB/s (using iostat)
#   - Time: Total execution time in seconds (using time command)
#
# Output: MT25077_Part_D_CSV.csv
################################################################################

# Configuration
CSV_FILE="MT25077_Part_D_CSV.csv"
TEMP_DIR="/tmp/MT25077_measurements_partd"
CPU_SET="0-1"  # Pin to CPUs 0 and 1

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

################################################################################
# Utility Functions
################################################################################

echo_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

echo_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

echo_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

echo_progress() {
    echo -e "${BLUE}[PROGRESS]${NC} $1"
}

################################################################################
# Dependency Check
################################################################################

check_dependencies() {
    echo_info "Checking dependencies..."

    local all_ok=true

    # Check required programs
    if ! command -v taskset &> /dev/null; then
        echo_error "taskset not found. Please install util-linux package."
        all_ok=false
    fi

    if ! command -v top &> /dev/null; then
        echo_error "top not found. Please install procps package."
        all_ok=false
    fi

    if ! command -v /usr/bin/time &> /dev/null; then
        echo_error "/usr/bin/time not found. Please install time package."
        all_ok=false
    fi

    # Check iostat (optional but recommended)
    if ! command -v iostat &> /dev/null; then
        echo_warn "iostat not found. I/O measurements will be unavailable."
        echo_warn "To install: sudo apt-get install sysstat"
        IOSTAT_AVAILABLE=false
    else
        IOSTAT_AVAILABLE=true
    fi

    # Check if programs are built
    if [ ! -f "./program_a" ] || [ ! -f "./program_b" ]; then
        echo_error "Programs not found. Please run 'make all' first."
        all_ok=false
    fi

    if [ "$all_ok" = false ]; then
        echo_error "Dependency check failed. Exiting."
        exit 1
    fi

    echo_info "Dependency check passed."
}

################################################################################
# Measurement Function
################################################################################

# Function to run experiment and collect measurements
# Arguments: $1 = program_name (a or b), $2 = worker_type, $3 = num_workers
run_experiment() {
    local program=$1
    local worker=$2
    local num_workers=$3
    local program_name="program_${program}"

    echo_progress "Running: ${program_name} ${worker} ${num_workers}"

    # Create temporary files for measurements
    local temp_cpu="${TEMP_DIR}/${program}_${worker}_${num_workers}_cpu.txt"
    local temp_time="${TEMP_DIR}/${program}_${worker}_${num_workers}_time.txt"
    local temp_io="${TEMP_DIR}/${program}_${worker}_${num_workers}_io.txt"

    # Start the program in background with taskset
    /usr/bin/time -v taskset -c ${CPU_SET} ./${program_name} ${worker} ${num_workers} > /dev/null 2> ${temp_time} &
    local pid=$!

    # Wait a moment for process to start
    sleep 0.5

    # Monitor CPU usage using top (sample every 1 second)
    local cpu_samples=0
    local cpu_sum=0

    # Collect samples while process is running
    while kill -0 $pid 2>/dev/null; do
        # For process-based, we need to sum CPU% of all child processes
        # For thread-based, top shows total CPU% for the process
        if [ "$program" = "a" ]; then
            # Get all processes with same parent
            local cpu_line=$(ps -o pid,ppid,%cpu --no-headers | awk -v pid=$pid '$2 == pid || $1 == pid {sum += $3} END {print sum}')
        else
            # Get CPU% for the main process (includes all threads)
            local cpu_line=$(top -b -n 1 -p $pid 2>/dev/null | tail -n 1 | awk '{print $9}')
        fi

        if [[ $cpu_line =~ ^[0-9.]+$ ]]; then
            cpu_sum=$(echo "$cpu_sum + $cpu_line" | bc)
            cpu_samples=$((cpu_samples + 1))
        fi
        sleep 1
    done

    # Wait for process to complete
    wait $pid
    local exit_code=$?

    # Calculate average CPU%
    local avg_cpu=0
    if [ $cpu_samples -gt 0 ]; then
        avg_cpu=$(echo "scale=2; $cpu_sum / $cpu_samples" | bc)
    fi

    # Extract memory usage from /usr/bin/time output (Maximum resident set size in KB)
    local max_memory=$(grep "Maximum resident set size" ${temp_time} | awk '{print $6}')
    if [ -z "$max_memory" ]; then
        max_memory=0
    fi

    # Extract execution time (real time in seconds)
    local exec_time=$(grep "Elapsed (wall clock) time" ${temp_time} | awk '{print $8}' | tr -d '()' | awk -F: '{if (NF==3) print ($1*3600 + $2*60 + $3); else if (NF==2) print ($1*60 + $2); else print $1}')
    if [ -z "$exec_time" ]; then
        exec_time=0
    fi

    # Measure I/O (if iostat available)
    local io_kbs=0
    if [ "$IOSTAT_AVAILABLE" = true ]; then
        # Run iostat for a short period to get I/O stats
        iostat -d -x 1 2 > ${temp_io} 2>/dev/null
        # Extract average kB/s (read + write)
        io_kbs=$(awk '/^[hs]d/ {sum += $4 + $5; count++} END {if (count > 0) print sum/count; else print 0}' ${temp_io})
        if [ -z "$io_kbs" ]; then
            io_kbs=0
        fi
    fi

    # Write result to CSV
    local csv_line="${program},${worker},${num_workers},${avg_cpu},${max_memory},${io_kbs},${exec_time}"
    echo "$csv_line" >> ${CSV_FILE}

    # Display compact results
    echo "  → CPU: ${avg_cpu}%, Mem: ${max_memory} KB, I/O: ${io_kbs} KB/s, Time: ${exec_time}s"
}

################################################################################
# Main Script
################################################################################

main() {
    echo "==================================================================="
    echo "Part D: Scaled Experiments with Varying Worker Counts"
    echo "Roll Number: MT25077"
    echo "==================================================================="
    echo ""

    # Check dependencies
    check_dependencies

    # Create temporary directory
    mkdir -p ${TEMP_DIR}

    # Initialize CSV file with header
    echo "Program,Function,Workers,CPU%,Memory(KB),IO(KB/s),Time(s)" > ${CSV_FILE}
    echo_info "Initialized CSV file: ${CSV_FILE}"
    echo ""

    # Calculate total experiments
    local total_experiments=0
    total_experiments=$((4 * 3 + 7 * 3))  # (2,3,4,5) × 3 workers + (2,3,4,5,6,7,8) × 3 workers

    echo "==================================================================="
    echo "Running experiments (${total_experiments} total)..."
    echo "==================================================================="
    echo ""

    local experiment_count=0

    # Program A experiments: 2, 3, 4, 5 processes
    echo_info "Program A (Process-based) Experiments:"
    echo "-------------------------------------------------------------------"
    for num_procs in 2 3 4 5; do
        for worker in cpu mem io; do
            experiment_count=$((experiment_count + 1))
            echo "[${experiment_count}/${total_experiments}]"
            run_experiment "a" "$worker" "$num_procs"
        done
    done
    echo ""

    # Program B experiments: 2, 3, 4, 5, 6, 7, 8 threads
    echo_info "Program B (Thread-based) Experiments:"
    echo "-------------------------------------------------------------------"
    for num_threads in 2 3 4 5 6 7 8; do
        for worker in cpu mem io; do
            experiment_count=$((experiment_count + 1))
            echo "[${experiment_count}/${total_experiments}]"
            run_experiment "b" "$worker" "$num_threads"
        done
    done
    echo ""

    # Cleanup temporary directory
    rm -rf ${TEMP_DIR}

    # Display final results
    echo "==================================================================="
    echo "Part D Measurement Collection Complete!"
    echo "==================================================================="
    echo ""
    echo_info "Results saved to: ${CSV_FILE}"
    echo ""
    echo "CSV Summary (first 10 lines):"
    echo "-------------------------------------------------------------------"
    head -10 ${CSV_FILE}
    echo "..."
    echo "-------------------------------------------------------------------"
    echo ""
    echo "==================================================================="
    echo "Summary:"
    echo "  Total experiments:  ${experiment_count}"
    echo "  Output file:        ${CSV_FILE}"
    echo "  Lines in CSV:       $(wc -l < ${CSV_FILE})"
    echo "==================================================================="
    echo ""
    echo_info "Next step: Generate plots using 'make plots' or 'python3 MT25077_Part_D_plot.py'"
}

# Run main function
main
