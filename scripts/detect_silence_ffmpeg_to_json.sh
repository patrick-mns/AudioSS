#!/bin/bash

# Input audio file path
input_audio_path="$1"
output_json_path="$2"

# Check if the input parameters are provided correctly
if [[ -z "$input_audio_path" || -z "$output_json_path" ]]; then
  echo "Please provide the audio file path and the output JSON file path."
  exit 1
fi

# Create the JSON file and start the array if it doesn't exist
if [[ ! -f "$output_json_path" ]]; then
  echo "[" > "$output_json_path"  # Create the file with an empty JSON array
fi

# FFmpeg command to detect silences
ffmpeg -hide_banner -i "$input_audio_path" -af silencedetect=noise=-30dB:d=0.5 -f null - 2>&1 | \
while read -r line; do
    # Look for the start of silence
    if [[ "$line" =~ silence_start:\ ([0-9]+\.?[0-9]*) ]]; then
        silence_start="${BASH_REMATCH[1]}"
    fi
    
    # Look for the end of silence
    if [[ "$line" =~ silence_end:\ ([0-9]+\.?[0-9]*) ]]; then
        silence_end="${BASH_REMATCH[1]}"
        
        # Add new silence entry to the JSON file
        echo "  {" >> "$output_json_path"
        echo "    \"channel\": 0," >> "$output_json_path"
        echo "    \"silence_start\": $silence_start," >> "$output_json_path"
        echo "    \"silence_end\": $silence_end" >> "$output_json_path"
        echo "  }," >> "$output_json_path"
    fi
done

# Finalize the JSON file
# Remove the last comma from the array and close the JSON with
# We use a different method to ensure the last comma is removed
sed -i '$ s/,$//' "$output_json_path"
echo "]" >> "$output_json_path"
