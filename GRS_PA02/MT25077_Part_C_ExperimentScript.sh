#!/usr/bin/env bash
# MT25077
# MT25077_Part_C_ExperimentScript.sh
# Automated experiment script for PA02.
#
# This script:
#   1. Compiles all implementations via make
#   2. Sets up network namespaces (ns_server, ns_client) with a veth pair
#   3. Runs experiments across message sizes and thread counts
#   4. Collects application-level metrics (throughput, latency) and
#      perf stat metrics (CPU cycles, cache misses, context switches)
#   5. Stores all results in CSV format
#   6. Cleans up namespaces on exit
#
# Must be run as root (sudo) for namespace and perf access.
#
# Usage: sudo bash MT25077_Part_C_ExperimentScript.sh
#
# Author: MT25077

set -euo pipefail

# ============================================================
# Configuration
# ============================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Experiment parameters
MSG_SIZES=(1024 4096 65536 1048576)    # 1KB, 4KB, 64KB, 1MB
THREAD_COUNTS=(1 2 4 8)
DURATION=10                             # seconds per experiment

# Network namespace configuration
NS_SERVER="ns_server"
NS_CLIENT="ns_client"
VETH_SERVER="veth-srv"
VETH_CLIENT="veth-cli"
SERVER_IP="10.0.0.1"
CLIENT_IP="10.0.0.2"
PORT=9000

# Implementation map: name -> (server_binary, client_binary)
declare -A SERVER_BINS=(
    ["two_copy"]="a1_server"
    ["one_copy"]="a2_server"
    ["zero_copy"]="a3_server"
)
declare -A CLIENT_BINS=(
    ["two_copy"]="a1_client"
    ["one_copy"]="a2_client"
    ["zero_copy"]="a3_client"
)

# Output CSV file
CSV_FILE="MT25077_Part_B_Results.csv"
PERF_EVENTS="cycles,L1-dcache-load-misses,LLC-load-misses,context-switches"

# Temporary files directory
TMP_DIR=$(mktemp -d /tmp/pa02_exp_XXXXXX)

# ============================================================
# Cleanup function (runs on EXIT)
# ============================================================

cleanup() {
    echo "[INFO] Cleaning up..."

    # Kill any leftover server processes
    ip netns exec "$NS_SERVER" killall -9 a1_server a2_server a3_server 2>/dev/null || true

    # Delete namespaces (also removes veth pairs)
    ip netns del "$NS_SERVER" 2>/dev/null || true
    ip netns del "$NS_CLIENT" 2>/dev/null || true

    # Remove temp files
    rm -rf "$TMP_DIR"

    echo "[INFO] Cleanup complete."
}

trap cleanup EXIT

# ============================================================
# Check prerequisites
# ============================================================

if [[ $EUID -ne 0 ]]; then
    echo "[ERROR] This script must be run as root (sudo)." >&2
    exit 1
fi

if ! command -v perf &>/dev/null; then
    echo "[ERROR] 'perf' tool not found. Install linux-tools-$(uname -r)." >&2
    exit 1
fi

# Allow perf for all users (needed inside namespaces)
sysctl -w kernel.perf_event_paranoid=-1 >/dev/null 2>&1 || true

# ============================================================
# Step 1: Compile all implementations
# ============================================================

echo "============================================================"
echo " Step 1: Compiling all implementations"
echo "============================================================"

make -C "$SCRIPT_DIR" clean
make -C "$SCRIPT_DIR" all

echo "[OK] Compilation successful."

# ============================================================
# Step 2: Set up network namespaces
# ============================================================

echo "============================================================"
echo " Step 2: Setting up network namespaces"
echo "============================================================"

# Remove stale namespaces if they exist
ip netns del "$NS_SERVER" 2>/dev/null || true
ip netns del "$NS_CLIENT" 2>/dev/null || true

# Create namespaces
ip netns add "$NS_SERVER"
ip netns add "$NS_CLIENT"

# Create veth pair
ip link add "$VETH_SERVER" type veth peer name "$VETH_CLIENT"

# Assign veth endpoints to namespaces
ip link set "$VETH_SERVER" netns "$NS_SERVER"
ip link set "$VETH_CLIENT" netns "$NS_CLIENT"

# Configure IP addresses
ip netns exec "$NS_SERVER" ip addr add "${SERVER_IP}/24" dev "$VETH_SERVER"
ip netns exec "$NS_CLIENT" ip addr add "${CLIENT_IP}/24" dev "$VETH_CLIENT"

# Bring interfaces up
ip netns exec "$NS_SERVER" ip link set lo up
ip netns exec "$NS_SERVER" ip link set "$VETH_SERVER" up
ip netns exec "$NS_CLIENT" ip link set lo up
ip netns exec "$NS_CLIENT" ip link set "$VETH_CLIENT" up

# Disable TCP segmentation offload for consistent measurements
ip netns exec "$NS_SERVER" ethtool -K "$VETH_SERVER" tso off gso off gro off 2>/dev/null || true
ip netns exec "$NS_CLIENT" ethtool -K "$VETH_CLIENT" tso off gso off gro off 2>/dev/null || true

# Verify connectivity
if ! ip netns exec "$NS_CLIENT" ping -c 1 -W 2 "$SERVER_IP" >/dev/null 2>&1; then
    echo "[ERROR] Cannot reach server namespace. Network setup failed." >&2
    exit 1
fi
echo "[OK] Namespaces configured. Server=${SERVER_IP}, Client=${CLIENT_IP}"

# ============================================================
# Step 3: Collect system information
# ============================================================

echo "============================================================"
echo " Step 3: System Information"
echo "============================================================"

SYSINFO_FILE="${SCRIPT_DIR}/MT25077_Part_B_SystemInfo.txt"
{
    echo "=== System Configuration ==="
    echo "Date: $(date)"
    echo "Kernel: $(uname -r)"
    echo "CPU:"
    lscpu | grep -E "Model name|Socket|Core|Thread|CPU\(s\)|MHz|Cache"
    echo "Memory:"
    free -h | head -2
    echo "NIC (veth):"
    ip netns exec "$NS_SERVER" ip link show "$VETH_SERVER"
} > "$SYSINFO_FILE"

cat "$SYSINFO_FILE"

# ============================================================
# Step 4: Run experiments
# ============================================================

echo "============================================================"
echo " Step 4: Running experiments"
echo "============================================================"

# Write CSV header
echo "implementation,msg_size,thread_count,throughput_gbps,latency_us,total_bytes,total_msgs,cycles,l1_cache_misses,llc_cache_misses,context_switches" \
    > "$CSV_FILE"

IMPLEMENTATIONS=(two_copy one_copy zero_copy)
TOTAL_EXPS=$(( ${#IMPLEMENTATIONS[@]} * ${#MSG_SIZES[@]} * ${#THREAD_COUNTS[@]} ))
EXP_NUM=0
FAILED=0

for IMPL in "${IMPLEMENTATIONS[@]}"; do
    for MSG_SIZE in "${MSG_SIZES[@]}"; do
        for THREADS in "${THREAD_COUNTS[@]}"; do
            EXP_NUM=$((EXP_NUM + 1))
            echo ""
            echo "--- Experiment ${EXP_NUM}/${TOTAL_EXPS}: ${IMPL} | msg_size=${MSG_SIZE} | threads=${THREADS} ---"

            SERVER_BIN="${SERVER_BINS[$IMPL]}"
            CLIENT_BIN="${CLIENT_BINS[$IMPL]}"

            # Temp files for this experiment
            CLIENT_OUT="${TMP_DIR}/client_out.txt"
            CLIENT_ERR="${TMP_DIR}/client_err.txt"
            PERF_OUT="${TMP_DIR}/perf_out.txt"

            # Clear temp files
            > "$CLIENT_OUT"
            > "$CLIENT_ERR"
            > "$PERF_OUT"

            # ---- Start server in ns_server ----
            ip netns exec "$NS_SERVER" "${SCRIPT_DIR}/${SERVER_BIN}" "$PORT" "$MSG_SIZE" \
                > /dev/null 2>&1 &
            SERVER_PID=$!
            sleep 2  # Give server time to bind and listen

            # ---- Run client in ns_client with perf stat ----
            # perf stat wraps the client process; output goes to PERF_OUT.
            # Client stdout (CSV result) -> CLIENT_OUT
            # Client stderr (debug info) -> CLIENT_ERR
            echo "  Running ${IMPL} client for ${DURATION}s (msg=${MSG_SIZE}, threads=${THREADS})..."
            ip netns exec "$NS_CLIENT" \
                perf stat -e "$PERF_EVENTS" -o "$PERF_OUT" -- \
                "${SCRIPT_DIR}/${CLIENT_BIN}" "$SERVER_IP" "$PORT" "$MSG_SIZE" "$THREADS" "$DURATION" \
                > "$CLIENT_OUT" 2> "$CLIENT_ERR" || true

            # ---- Stop server ----
            kill "$SERVER_PID" 2>/dev/null || true
            sleep 0.5
            kill -9 "$SERVER_PID" 2>/dev/null || true  # Force kill if still alive
            wait "$SERVER_PID" 2>/dev/null || true
            sleep 1  # Let port release

            # ---- Parse client output ----
            # Expected format: RESULT,<impl>,<msg_size>,<threads>,<throughput>,<latency>,<bytes>,<msgs>
            RESULT_LINE=$(grep "^RESULT," "$CLIENT_OUT" 2>/dev/null || echo "")

            if [[ -z "$RESULT_LINE" ]]; then
                echo "[WARN] No result from client. Skipping."
                cat "$CLIENT_ERR" 2>/dev/null | head -10 || true
                FAILED=$((FAILED + 1))
                continue
            fi

            # Extract fields from client result
            THROUGHPUT=$(echo "$RESULT_LINE" | cut -d',' -f5)
            LATENCY=$(echo "$RESULT_LINE" | cut -d',' -f6)
            TOTAL_BYTES=$(echo "$RESULT_LINE" | cut -d',' -f7)
            TOTAL_MSGS=$(echo "$RESULT_LINE" | cut -d',' -f8)

            # ---- Parse perf stat output ----
            # Extract numeric values from perf stat. Use subshell to avoid pipefail issues.
            CYCLES=$(set +o pipefail; grep -oP '[\d,]+(?=\s+cycles)' "$PERF_OUT" 2>/dev/null | tr -d ',' | head -1)
            L1_MISSES=$(set +o pipefail; grep -oP '[\d,]+(?=\s+L1-dcache-load-misses)' "$PERF_OUT" 2>/dev/null | tr -d ',' | head -1)
            LLC_MISSES=$(set +o pipefail; grep -oP '[\d,]+(?=\s+LLC-load-misses)' "$PERF_OUT" 2>/dev/null | tr -d ',' | head -1)
            CTX_SW=$(set +o pipefail; grep -oP '[\d,]+(?=\s+context-switches)' "$PERF_OUT" 2>/dev/null | tr -d ',' | head -1)

            # Default missing/empty values to 0
            [[ -z "$CYCLES" ]]     && CYCLES=0
            [[ -z "$L1_MISSES" ]]  && L1_MISSES=0
            [[ -z "$LLC_MISSES" ]] && LLC_MISSES=0
            [[ -z "$CTX_SW" ]]     && CTX_SW=0

            # ---- Append to CSV ----
            echo "${IMPL},${MSG_SIZE},${THREADS},${THROUGHPUT},${LATENCY},${TOTAL_BYTES},${TOTAL_MSGS},${CYCLES},${L1_MISSES},${LLC_MISSES},${CTX_SW}" \
                >> "$CSV_FILE"

            echo "  Throughput=${THROUGHPUT} Gbps, Latency=${LATENCY} us"
            echo "  Cycles=${CYCLES}, L1_misses=${L1_MISSES}, LLC_misses=${LLC_MISSES}, ctx_sw=${CTX_SW}"
        done
    done
done

# ============================================================
# Step 5: Summary
# ============================================================

echo ""
echo "============================================================"
echo " Experiments complete!"
echo "============================================================"
echo "Results saved to: ${SCRIPT_DIR}/${CSV_FILE}"
echo "System info saved to: ${SYSINFO_FILE}"
echo ""
echo "CSV preview:"
head -5 "$CSV_FILE"
echo "..."
echo "Total experiments: ${TOTAL_EXPS} (${FAILED} failed)"
