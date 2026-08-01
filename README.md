# Real-Time Dynamic Memory Cache Simulator

An architectural-level, cycle-accurate memory cache simulator built in **C++** utilizing **Intel Pin** for Dynamic Binary Instrumentation (DBI). Unlike traditional trace-driven simulators that rely on static log files, this tool models live interaction between a processor's Load/Store execution units and a configurable cache controller at runtime.

---

## 🚀 Key Features
* **Configurable Architecture:** Supports adjustable cache sizes, block sizes, and associativity levels (Direct-Mapped to Set-Associative).
* **Dynamic Binary Instrumentation (Intel Pin):** Intercepts memory reads and writes directly from target binaries at runtime without requiring static trace generation.
* **LRU Replacement Policy:** Implements Least Recently Used (LRU) eviction logic using high-precision timestamp tracking (`LLONG_MAX` safe initialization).
* **Write-Back & Dirty Bit Logic:** Enforces write-back consistency policies by tracking line modification flags and logging total write-back evictions.
* **Windowed Miss-Rate Analysis:** Periodically captures access and miss telemetry over fixed execution windows (every 1000 accesses) and exports data to CSV for performance visualization.

---

## 🛠️ Project Structure
* `MyPinTool.cpp` – Core simulation engine, cache controller logic, and Intel Pin instrumentation hooks.
* `ploy.py` – Python analytics script utilizing `pandas` and `matplotlib` to parse telemetry and plot temporal locality curves.
* `miss.csv` – Auto-generated output file capturing windowed access and miss metrics.

---

## 📋 Step-by-Step Installation, Compilation, and Execution Guide

### Step 1: Open Your Workspace in VS Code
Open your WSL terminal and launch VS Code directly inside your tool directory:
```bash
go to ~/pin_kit/source/tools/MyPinTool
make obj-intel64/MyPinTool.so
gcc test.c -o test
 $PIN_ROOT/pin -t obj-intel64/MyPinTool.so -- ./test
source venv/bin/activate
python3 ploy.py
