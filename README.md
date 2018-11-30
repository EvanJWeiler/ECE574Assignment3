# ECE574Assignment3 [Due date: 11/29 10% EC on it, 10% EC for FOR Loops]

# Program Name: hlsyn (High-level Synthesizer)
  # Tasks: 
  #   Brendan -> Fix up text parsing and prepare objects for scheduling algorithm, prep all for scheduling, sense for If-Else/For
  #   Diego -> Compute Time frames and operation probabilities 
  #   Evan -> Compute self-force, pred-force, total force, schedule the op 
  #   Logan -> Write adjustments for output file.  Always Blocks and case statements.  HLSM and state machine.
  #


1. Members
Brendan Cassidy (brendancassidy)
Diego Alantera (dalcantra7)
Evan Weiler (evanweiler)
Logan Knott (lbknott)

2.
Brendan Cassidy - 474
Diego Alantera - 574
Evan Weiler - 574
Logan Knott - 474

3. 
The purpose of this program is to schedule a list of operations using the Force Directed Scheduling (FDS) technique. The program should use FDS to create a High Level State Machine (HLSM) written in Verilog. The program should detect if statements within an input file and write proper output to handle that case

4. 
Brendan Cassidy - Parsing input files, creating objects for manipulation, detect if/else statements error correction
Diego Alantera - ASAP and ALAP scheduling, computing probabilities, and computing type distributions
Evan Weiler - Computing Self, Predecessor, and Successor Forces and Scheduling of Nodes, error correction
Logan Knott - writing to output file (HLSM, always blocks, case statements), testing
