#!/bin/bash

# Original file name
INPUT="$1"

# Desired durations in seconds
DUR_1H=$((60 * 60))       # 3600 seconds
DUR_2H=$((2 * 60 * 60))   # 7200 seconds
DUR_10H=$((10 * 60 * 60)) # 36000 seconds

# Function to create extended duration version
generate_looped_audio() {
    local duration=$1
    local output=$2
    ffmpeg -stream_loop -1 -i "$INPUT" -t "$duration" -c copy "$output"
}

# Create versions
generate_looped_audio "$DUR_1H" "exemples/audio_1h.wav"
generate_looped_audio "$DUR_2H" "exemples/audio_2h.wav"
generate_looped_audio "$DUR_10H" "exemples/audio_10h.wav"
