#Main
Library = "Soundiron Voices of Rapture 1.0 - 2017-03-23" # For which library algorithm is optimized
Type = 3 # Instrument type
poly = 1 # Maximum number of simultaneous voices

# Automation parameters
CC_dynamics = 72
CC_ma = 9 # Number of CC steps to moving average (please use only odd numbers)
CC_steps = 30 # Number of CC steps in one second

# Controls
Volume_default = 127 # default 127 (direct CC volume, should not be changed by user)
db_max = 0 # Maximum controlled loudness in db when volume CC is 127
db_coef = 3 # Slope of CC to db function (3 for Kontakt, 1 for SWAM)
CC_Name = 72: "Dynamics"
CC_Name = 74: "Attack"
CC_Name = 76: "Release time"
CC_Name = 78: "Offset"
CC_Name = 80: "Vibrato intensity"
CC_Name = 90: "Release volume"

# Controls to map manually
CC_Name = 2: "Legato transition speed"
CC_Name = 3: "Legato on"
CC_Name = 4: "Chord auto-pan on"
CC_Name = 5: "Release samples on"

# Initial setup
Attack = 0
Offset = 0
Release time = 127
Release volume = 127
Legato transition speed = 0
Legato on = 101
Chord auto-pan on = 0
Release samples on = 101
Volume = 100 # default 100 (relative volume from 0 to 100)

# Randomization
rand_pos = 0-0 # Randomize note starts-ends not greater than percent of note length 
rand_pos_max = 0-0 # Maximum shift in ms (start-end)
rnd_vel = 8 # Randomize note velocity not greater than this percent
rnd_dyn = 10 # Randomize step dynamics not greater than this percent
rnd_dyn_slow = 3 # Slow down random dynamics function by this value. Can be only integer: 1 and above

# Nonlegato adaptor
nonlegato_mingap = 100 # Minimum gap between notes (in ms) introduced by automatic nonlegato 

# Legato adaptor
all_ahead = 125 # Time in ms to stretch all notes (sustains and legato) back to cope with slow attack

# Nonlegato adaptor
nonlegato_freq = 20 # Frequency (in percent) when legato can be replaced with non-legato by moving note end to the left
nonlegato_minlen = 400 # Minimum note length (in ms) allowed to convert to nonlegato
nonlegato_maxgap = 300 # Maximum gap between notes (in ms) introduced by automatic nonlegato 

# Automatic crescendo for long notes without attack
cresc_mindur = 700 # Minimum note duration (ms) that can have automatic crescendo
cresc_mul = 20 # Multiply dynamics by this percent at crescendo start (0.2 creates smooth slope)
cresc_len = 30 # Percent of note length to use for crescendo
cresc_maxvel = 119 # Maximum velocity to still trigger automatic crescendo
cresc_minpause = 100 # Minimum pause length required before automatic crescendo (ms)

# Automatic diminuendo for long notes
dim_mindur = 200 # Minimum note duration (ms) that can have automatic diminuendo
dim_mul = 20 # Multiply dynamics by this percent at diminuendo end (0.2 creates smooth slope)
dim_len = 30 # Percent of note length to use for diminuendo
dim_minpause = 100 # Minimum pause length required after automatic diminuendo (ms)

# Reverse bell adaptor
rbell_freq = 0 # Frequency to apply reverse bell when all conditions met
rbell_dur = 300-1000 # Minimum note duration (ms) that can have a reverse bell - that will have deepest reverse bell
vib_sbell_freq = 100 # Frequency to apply vibrato bell when all conditions met
rbell_mul = 20-80 # Multiply dynamics by this parameter at bell center with longer-shorter duration
rbell_pos = 20-80 # Leftmost-rightmost minimum reverse bell position inside window (percent of window duration)

# Vibrato adaptor
CC_vib = 80 # CC number for vibrato intensity
vib_bell_top = 5-95 # Leftmost-rightmost maximum vibrato intensity in note (percent of note duration)
vib_bell_exp = 2 # Exponent to create non-linear bell shape (left)
vib_bell_rexp = 2 # Exponent to create non-linear bell shape (right)
vib_bell_freq = 100 # Frequency to apply vibrato bell when all conditions met
vib_bell_dur = 200-600 # Minimum note duration (ms) that can have a vibrato bell - that can have highest vibrato bell
vib_bell = 50-126 # Maximum vibrato intensity in vibrato bell (for minimum and highest dynamics)
rnd_vib = 10 # Randomize vibrato intensity not greater than this percent
rnd_vib_slow = 2 # Slow down random vibrato function by this value. Can be only integer: 1 and above
vib_dyn = 30-110 # Min-max dynamics. Below min dynamics there is no vibrato. Above max dynamics vibrato is max vibrato

# Vibrato adaptor - short notes
vib_sbell_top = 5-95 # Leftmost-rightmost maximum vibrato intensity in note (percent of note duration)
vib_sbell_exp = 0.25 # Exponent to create non-linear bell shape (left)
vib_sbell_rexp = 0.25 # Exponent to create non-linear bell shape (right)
vib_sbell_freq = 100 # Frequency to apply vibrato bell when all conditions met
vib_sdyn = 30-110 # Min-max dynamics. Below min dynamics there is no vibrato. Above max dynamics vibrato is max vibrato
vib_sbell = 0-0 # Maximum vibrato intensity in vibrato bell (for minimum and highest dynamics)
