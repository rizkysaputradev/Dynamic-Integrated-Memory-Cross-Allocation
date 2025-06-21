# allocator_visual/visualizer.py
import json
import time
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import os

OUTPUT_FILE = "allocator_output.json"

def load_data():
    try:
        with open(OUTPUT_FILE, "r") as f:
            return json.load(f)
    except Exception:
        return []

def update(frame, scatter, ax):
    data = load_data()
    ax.clear()

    # Filter only ALLOC entries
    alloc_data = [entry for entry in data if entry.get("type") == "ALLOC"]

    if not alloc_data:
        ax.set_title("Waiting for allocator ALLOC data...")
        return

    x = [entry["preferred_level"] for entry in alloc_data]
    y = [entry["time_cost"] for entry in alloc_data]
    sizes = [entry["size"] for entry in alloc_data]
    ids = [entry["id"] for entry in alloc_data]

    scatter = ax.scatter(x, y, s=[s / 2 for s in sizes], c="skyblue", alpha=0.6, edgecolors="black")
    for i, txt in enumerate(ids):
        ax.annotate(f"ID:{txt}", (x[i] + 0.05, y[i]), fontsize=8)

    ax.set_title("DIMCA Memory Allocation Visualizer")
    ax.set_xlabel("Preferred Level")
    ax.set_ylabel("Time Cost")
    ax.set_xlim(0.5, 3.5)
    ax.set_ylim(0, max(y + [1]) * 1.2)
    ax.grid(True)

def run_visualizer():
    fig, ax = plt.subplots(figsize=(8, 6))
    scatter = ax.scatter([], [])
    ani = animation.FuncAnimation(fig, update, fargs=(scatter, ax), interval=1000)
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    run_visualizer()

