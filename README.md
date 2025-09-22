# ParaPipe Project

## Compilation

The program `parapipe` is compiled from two C source files, `parapipe.c` and `utils.c`, using the following command:

```bash
cd src/
gcc parapipe.c utils.c -o parapipe
```
## Project structure
```
.
├── src
│ ├── parapipe.c      # Main program with multithreading and pipeline
│ ├── utils.c         # Utility functions
│ ├── utils.h         # Utility function headers
│ └── largefile.txt   # Test data
└── README.md
```
## Test Case 1: Single Command
### Command:

The program reads input from largefile.txt and executes the command "grep abc" in parallel using 4 worker processes.

```Bash

cat largefile.txt | ./parapipe -n 4 -c "grep abc"
```
### Expected Behavior:

The parapipe program distributes the input lines from largefile.txt among 4 parallel processes. Each process runs the grep abc command on its portion of the data. The output lines are then collected and printed to standard output. Since the processing is done in parallel, the order of the output lines is not guaranteed to be the same as the order in the original file, as seen in the result.

### Result:
```
Result:
xSh dPPtVD StusBj sezTqG EERJtezxaO vAH RMNedFO WJutvH TnTGqpsbGg abc
tod4 NyRBUGyV abc 123 DBxtN RHbPWd UGZNDSaX kTg AKHe RMbqBpX iegXb tGP
tod8 NyRBUGyV abc 123 DBxtN RHbPWd UGZNDSaX kTg AKHe RMbqBpX iegXb tGP
1tod1 NyRBUGyV abc 123 DBxtN RHbPWd UGZNDSaX kTg AKHe RMbqBpX iegXb tGP
5tod5 NyRBUGyV abc 123 DBxtN RHbPWd UGZNDSaX kTg AKHe RMbqBpX iegXb tGP
9tod9 NyRBUGyV abc 123 DBxtN RHbPWd UGZNDSaX kTg AKHe RMbqBpX iegXb tGP
tod1 NyRBUGyV abc 123 DBxtN RHbPWd UGZNDSaX kTg AKHe RMbqBpX iegXb tGP
tod5 NyRBUGyV abc 123 DBxtN RHbPWd UGZNDSaX kTg AKHe RMbqBpX iegXb tGP
tod9 NyRBUGyV abc 123 DBxtN RHbPWd UGZNDSaX kTg AKHe RMbqBpX iegXb tGP
2tod2 NyRBUGyV abc 123 DBxtN RHbPWd UGZNDSaX kTg AKHe RMbqBpX iegXb tGP
6tod6 NyRBUGyV abc 123 DBxtN RHbPWd UGZNDSaX kTg AKHe RMbqBpX iegXb tGP
0tod0 NyRBUGyV abc 123 DBxtN RHbPWd UGZNDSaX kTg AKHe RMbqBpX iegXb tGP
IvTiKMHD dwFZqpY uADEXa abc 123 icG QqTMLnSl neus clH EmuU tsiryFp mM
tod3 NyRBUGyV abc 123 DBxtN RHbPWd UGZNDSaX kTg AKHe RMbqBpX iegXb tGP
tod7 NyRBUGyV abc 123 DBxtN RHbPWd UGZNDSaX kTg AKHe RMbqBpX iegXb tGP
4tod4 NyRBUGyV abc 123 DBxtN RHbPWd UGZNDSaX kTg AKHe RMbqBpX iegXb tGP
8tod8 NyRBUGyV abc 123 DBxtN RHbPWd UGZNDSaX kTg AKHe RMbqBpX iegXb tGP
tod2 NyRBUGyV abc 123 DBxtN RHbPWd UGZNDSaX kTg AKHe RMbqBpX iegXb tGP
tod6 NyRBUGyV abc 123 DBxtN RHbPWd UGZNDSaX kTg AKHe RMbqBpX iegXb tGP
tod0 NyRBUGyV abc 123 DBxtN RHbPWd UGZNDSaX kTg AKHe RMbqBpX iegXb tGP
3tod3 NyRBUGyV abc 123 DBxtN RHbPWd UGZNDSaX kTg AKHe RMbqBpX iegXb tGP
7tod7 NyRBUGyV abc 123 DBxtN RHbPWd UGZNDSaX kTg AKHe RMbqBpX iegXb tGP
```
## Test Case 2: Piped Commands
### Command:

The program executes a simple pipeline of commands, "grep abc -> grep 123", in parallel using 4 worker processes.

```Bash

cat largefile.txt | ./parapipe -n 4 -c "grep abc -> grep 123"
```
### Expected Behavior:

This test demonstrates the program's ability to handle multi-stage pipelines. The output of the first command (grep abc) is piped as input to the second command (grep 123). The parallel execution is likely managed by feeding the output of the first stage's parallel workers into the second stage, or by having each worker handle the entire pipeline for a chunk of data. The final output contains only lines that match both "abc" and "123". The ordering is also not guaranteed here due to the parallel nature.
```
### Result:
tod1 NyRBUGyV abc 123 DBxtN RHbPWd UGZNDSaX kTg AKHe RMbqBpX iegXb tGP
tod5 NyRBUGyV abc 123 DBxtN RHbPWd UGZNDSaX kTg AKHe RMbqBpX iegXb tGP
tod9 NyRBUGyV abc 123 DBxtN RHbPWd UGZNDSaX kTg AKHe RMbqBpX iegXb tGP
2tod2 NyRBUGyV abc 123 DBxtN RHbPWd UGZNDSaX kTg AKHe RMbqBpX iegXb tGP
6tod6 NyRBUGyV abc 123 DBxtN RHbPWd UGZNDSaX kTg AKHe RMbqBpX iegXb tGP
0tod0 NyRBUGyV abc 123 DBxtN RHbPWd UGZNDSaX kTg AKHe RMbqBpX iegXb tGP
tod4 NyRBUGyV abc 123 DBxtN RHbPWd UGZNDSaX kTg AKHe RMbqBpX iegXb tGP
tod8 NyRBUGyV abc 123 DBxtN RHbPWd UGZNDSaX kTg AKHe RMbqBpX iegXb tGP
1tod1 NyRBUGyV abc 123 DBxtN RHbPWd UGZNDSaX kTg AKHe RMbqBpX iegXb tGP
5tod5 NyRBUGyV abc 123 DBxtN RHbPWd UGZNDSaX kTg AKHe RMbqBpX iegXb tGP
9tod9 NyRBUGyV abc 123 DBxtN RHbPWd UGZNDSaX kTg AKHe RMbqBpX iegXb tGP
tod2 NyRBUGyV abc 123 DBxtN RHbPWd UGZNDSaX kTg AKHe RMbqBpX iegXb tGP
tod6 NyRBUGyV abc 123 DBxtN RHbPWd UGZNDSaX kTg AKHe RMbqBpX iegXb tGP
tod0 NyRBUGyV abc 123 DBxtN RHbPWd UGZNDSaX kTg AKHe RMbqBpX iegXb tGP
3tod3 NyRBUGyV abc 123 DBxtN RHbPWd UGZNDSaX kTg AKHe RMbqBpX iegXb tGP
7tod7 NyRBUGyV abc 123 DBxtN RHbPWd UGZNDSaX kTg AKHe RMbqBpX iegXb tGP
IvTiKMHD dwFZqpY uADEXa abc 123 icG QqTMLnSl neus clH EmuU tsiryFp mM
tod3 NyRBUGyV abc 123 DBxtN RHbPWd UGZNDSaX kTg AKHe RMbqBpX iegXb tGP
tod7 NyRBUGyV abc 123 DBxtN RHbPWd UGZNDSaX kTg AKHe RMbqBpX iegXb tGP
4tod4 NyRBUGyV abc 123 DBxtN RHbPWd UGZNDSaX kTg AKHe RMbqBpX iegXb tGP
8tod8 NyRBUGyV abc 123 DBxtN RHbPWd UGZNDSaX kTg AKHe RMbqBpX iegXb tGP
```
## Test Case 3: Complex Piped Commands
### Command:

This test runs a more complex pipeline of four commands: "grep abc -> grep 123 -> awk '{print $1}' -> sort -u -> wc -l".

```Bash

cat largefile.txt | ./parapipe -n 4 -c "grep abc -> grep 123 -> awk '{print $1}' -> sort -u -> wc -l"
```
### Expected Behavior:

The sort command requires all input before it can produce any output. This means that after the awk command, the outputs from all parallel workers must be collected and sorted globally before being counted. This suggests that the final two commands (sort -u and wc -l) are likely executed sequentially on the combined output of the parallel stages to ensure a correct and accurate final count. The parapipe program appears to handle this synchronization correctly.

### Result:

```
Result:
5
6
5
5
```
Analysis of the Result:

The result indicates that the final wc -l command is likely being executed on the output of each of the 4 parallel workers independently, rather than on the combined and sorted output of all workers. This leads to the program outputting the count of unique lines from each worker's data partition, instead of the final, single, global count. A correct implementation would need to merge the results of the sort -u command from all parallel processes and then run wc -l on the total combined result.
