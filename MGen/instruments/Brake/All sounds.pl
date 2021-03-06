# Main
library = "True Strike Cinematic Orchestral Percussion" # For which library algorithm is optimized
Type = 0 # Instrument type
poly = 100 # Maximum number of simultaneous voices
single_stage = 1 # Collapse all tracks for this instrument into a single stage
Pan_default = 54
Reverb_fixed = 1 # 0 - reverb can be changed; 1 - reverb cannot be changed (hard coded)

# Instrument parameters
n_min = "C3" # Lowest note
n_max = "B4" # Highest note
Volume_default = 80 # (direct CC volume, should not be changed by user)
Volume = 85
db_max = 0 # Maximum controlled loudness in db when volume CC is 127
db_coef = 3 # Slope of CC to db function (3 for Kontakt, 1 for SWAM)

