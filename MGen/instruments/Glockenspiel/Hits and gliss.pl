# Main
library = "True Strike Cinematic Orchestral Percussion" # For which library algorithm is optimized
Type = 0 # Instrument type
poly = 100 # Maximum number of simultaneous voices
single_stage = 1 # Collapse all tracks for this instrument into a single stage
Pan_default = 57

# Instrument parameters
n_min = "C2" # Lowest note
n_max = "C6" # Highest note
Volume_default = 90 # (direct CC volume, should not be changed by user)
Volume = 85
db_max = 12 # Maximum controlled loudness in db when volume CC is 127
db_coef = 3 # Slope of CC to db function (3 for Kontakt, 1 for SWAM)

# Randomization
rnd_vel = 8 # Randomize note velocity not greater than this percent
rand_pos = 0-0 # Randomize note starts-ends not greater than percent of note length 
rand_pos_max = 0-0 # Maximum shift in ms (start-end)

