import subprocess
import matplotlib.pyplot as plt
import numpy as np

# Define audio files of different durations
AUDIO_PATHS = {
    "1h": "exemples/audio_1h.wav",
    "2h": "exemples/audio_2h.wav",
    "10h": "exemples/audio_10h.wav"
}

def run_command(cmd):
    result = subprocess.run(["/usr/bin/time", "-p", "bash", "-c", cmd],
                            stderr=subprocess.PIPE,
                            stdout=subprocess.DEVNULL,
                            text=True)
    times = {"real": 0.0, "user": 0.0, "sys": 0.0}
    for line in result.stderr.splitlines():
        if line.startswith("real") or line.startswith("user") or line.startswith("sys"):
            key, value = line.split()
            times[key] = float(value)
    return times

# Results will be stored as: results[tool][duration] = timing
results = {tool: {} for tool in ["Audio§", "FFmpeg", "SoX"]}

for duration, path in AUDIO_PATHS.items():
    print(f"\n--- Processing audio of duration: {duration} ---")
    commands = {
        "Audio§": f"./dist/audio§ {path} -threshold 1000 -min_duration 0.4 -verbose 0",
        "FFmpeg": f"ffmpeg -hide_banner -loglevel error -i {path} -af silencedetect=noise=-30dB:d=0.5  -ar 8000 -f null -",
        "SoX": f"sox -V0 {path} -n silence 1 0.5 -30d -1 0.5 -30d"
    }

    for name, cmd in commands.items():
        print(f"Running: {name} on {duration}")
        times = run_command(cmd)
        results[name][duration] = times

# Print results numerically
print("\nBenchmark Results (in seconds):")
for tool, durations in results.items():
    for duration, times in durations.items():
        print(f"{tool:<10} [{duration}] => real: {times['real']:.2f}, user: {times['user']:.2f}, sys: {times['sys']:.2f}")


def plot_horizontal_grouped_bar_chart(results, durations, tools, metric="real", filename="images/benchmark_horizontal.png"):
    """
    Generate a grouped horizontal bar chart as image file.
    
    - results: dict {tool: {duration: {metric: float}}}
    - durations: list of durations e.g. ["1h", "2h", "10h"]
    - tools: list of tools e.g. ["Audio§", "FFmpeg", "SoX"]
    - metric: which metric to plot (default "real")
    - filename: output image filename
    """

    # Organize data: for each tool, array of times per duration
    data = []
    for tool in tools:
        data.append([results[tool][dur][metric] for dur in durations])

    data = np.array(data)  # shape: (num_tools, num_durations)

    y_pos = np.arange(len(durations))
    bar_height = 0.2

    fig, ax = plt.subplots(figsize=(12, 7))

    # Plot each tool's bars side by side
    for i, tool in enumerate(tools):
        ax.barh(y_pos + i * bar_height, data[i], height=bar_height, label=tool)

    # Set yticks in the middle of grouped bars
    ax.set_yticks(y_pos + bar_height * (len(tools) - 1) / 2)
    ax.set_yticklabels(durations, fontsize=14)

    ax.invert_yaxis()  # So 1h is on top

    ax.set_xlabel("Time (seconds)", fontsize=16)
    ax.set_title("Benchmarking silence detection: comparing performance with popular tools. [WAV 16-bit PCM]", fontsize=16)
    ax.legend(fontsize=20)

    plt.tight_layout()
    plt.savefig(filename, dpi=200)
    

durations = list(AUDIO_PATHS.keys())
tools = list(results.keys())
plot_horizontal_grouped_bar_chart(results, durations, tools, metric="real")