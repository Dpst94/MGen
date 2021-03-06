# Main
library = "True Strike Cinematic Orchestral Percussion" # For which library algorithm is optimized
Type = 0 # Instrument type
poly = 100 # Maximum number of simultaneous voices
Reverb_fixed = 1 # 0 - reverb can be changed; 1 - reverb cannot be changed (hard coded)

# Instrument parameters
n_min = "C2" # Lowest note
n_max = "F5" # Highest note
Volume_default = 50 # (direct CC volume, should not be changed by user)

# Tremolo adaptor
trem_maxlen = 100 # Maximum length of one note inside tremolo for detection
trem_min_repeats = 3 # Minimum number of note repeats to detect tremolo
trem_dyn_range = 0-100 # (0-100) Percent of tremolo dynamics compared to sustain notes
trem_replace = "E2" # Replace tremolo notes with this note
trem_maxint = 127 # Maximum interval between notes of a trill in semitones to be considered a tremolo

# Tremolo is time-limited
# Tremolo dynamics is velocity-controlled
