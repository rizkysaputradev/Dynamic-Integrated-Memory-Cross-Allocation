# allocator_visual/src/simulator.py

import subprocess
import json
import re

OUTPUT_FILE = "allocator_output.json"
EXECUTABLE = "./main_advanced"  # Adjust path if needed

def parse_line(line):
    if "[ALLOC]" in line:
        match = re.search(r"id=(\d+) size=(\d+) preferred=(\d+) actual=(\d+) addr=(0x[0-9a-fA-F]+) cost=([0-9.]+)", line)
        if match:
            return {
                "type": "ALLOC",
                "id": int(match.group(1)),
                "size": int(match.group(2)),
                "preferred_level": int(match.group(3)),
                "actual_level": int(match.group(4)),
                "address": match.group(5),
                "time_cost": float(match.group(6))
            }
    elif "[FREE]" in line:
        match = re.search(r"addr=(0x[0-9a-fA-F]+) level=(\d+)", line)
        if match:
            return {
                "type": "FREE",
                "address": match.group(1),
                "level": int(match.group(2))
            }
    elif "[INIT]" in line:
        match = re.search(r"Level (\d+) base=(0x[0-9a-fA-F]+) size=(\d+)", line)
        if match:
            return {
                "type": "INIT",
                "level": int(match.group(1)),
                "base": match.group(2),
                "size": int(match.group(3))
            }
    elif "[SHUTDOWN]" in line:
        return {"type": "SHUTDOWN"}
    return None

def run_allocator_simulation():
    # Clear or initialize output file
    with open(OUTPUT_FILE, "w") as f:
        json.dump([], f)

    process = subprocess.Popen(
        [EXECUTABLE],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        universal_newlines=True,
        bufsize=1
    )

    json_data = []

    for line in process.stdout:
        print(line.strip())  # Optional: echo for debug
        parsed = parse_line(line)
        if parsed:
            json_data.append(parsed)
            with open(OUTPUT_FILE, "w") as f:
                json.dump(json_data, f, indent=2)

    process.wait()

if __name__ == "__main__":
    run_allocator_simulation()
