include "..\include\SI VoR.pl"

# Controls
KswGroup = "C0: Ah", "C#0: Oh", "D0: Oo" # Syllable

# Initial setup
InitCommand = "Ah"

# Instrument parameters
n_min = A1 # Lowest note
n_max = D4 # Highest note
t_min = 100 # Shortest note in ms
t_max = 12000 # Longest melody withot pauses in ms (0 = no limit). Decreases with dynamics
#leap_t_min = 100 # Shortest note after leap in ms

