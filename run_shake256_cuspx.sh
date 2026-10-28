#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"

ARCH=$(nvidia-smi --query-gpu=compute_cap --format=csv,noheader | head -1 | tr -d '.')
if [[ -z "$ARCH" ]]; then
  echo "Could not detect GPU architecture. Run this from an allocated GPU session." >&2
  exit 1
fi

variants=(128s 128f 192s 192f 256s 256f)
targets=()
for variant in "${variants[@]}"; do
  targets+=("bin/bm-shake256-$variant")
done

echo "Building baseline SHAKE256 benchmarks for sm_$ARCH (cuPQC disabled)."
make -B ARCH="$ARCH" CUPQC_DIR= "${targets[@]}"

OUT="shake256-cuspx-results.txt"
for variant in "${variants[@]}"; do
  binary="./bin/bm-shake256-$variant"
  heading="===== SHAKE256-$variant CUSPX (no cuPQC) | $(date -u) ====="
  printf '\n%s\n' "$heading" | tee -a "$OUT"

  if ! log=$("$binary" 2>&1); then
    printf '%s\n' "$log" | tee -a "$OUT" >&2
    echo "Benchmark failed: $binary" >&2
    exit 1
  fi

  printf '%s\n' "$log" | tee -a "$OUT"
  if ! grep -Fxq "build: cuspx" <<< "$log" ||
     ! grep -Fxq "parallel GPU verify of last signature: OK" <<< "$log" ||
     ! grep -Fxq "CPU reference verify of last signature: OK" <<< "$log"; then
    echo "Expected baseline build label or successful verification missing for $binary." >&2
    exit 1
  fi
done

echo "All six baseline runs completed. Results appended to $OUT."
