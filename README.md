# MPI-Wattmeter

MPI-Wattmeter measures the power consumption of MPI programs.

## Running an application


```bash
mpirun mpi_wattmeter [options] myappli
```


The following options can be passed to MPI Interceptor:

- `-v` or `--verbose`
  + Set verbosity level (default level: 0)
- `-d` or `--details`
  + Print detailed measurements (default level: 0)

## Building MPI-Wattmeter

```
git clone https://github.com/trahay/mpi-wattmeter.git
mkdir mpi-wattmeter/build
cd mpi-wattmeter/build
cmake ..  -DCMAKE_INSTALL_PREFIX=$PWD/../install
make install
```

## Example

In this example, we run NAS Parallel Benchmark BT kernel:
```

$ mpirun -np 4 mpi_wattmeter  ./bin/bt.A.4 
[...]
============================
MPI-Wattmeter summary
============================
#Node:rank      Source          joules          Watt.hour 
3a401-01:0      energy-cores    216.070618      0.060020  
3a401-01:0      energy-pkg      252.904602      0.070251  
3a401-01:0      energy-ram      13.127563       0.003647  
3a401-02:1      energy-cores    194.357422      0.053988  
3a401-02:1      energy-pkg      230.965149      0.064157  
3a401-02:1      energy-ram      12.479248       0.003466  
3a401-03:2      energy-cores    216.447571      0.060124  
3a401-03:2      energy-pkg      253.188782      0.070330  
3a401-03:2      energy-ram      12.913330       0.003587  
3a401-04:3      energy-cores    196.919067      0.054700  
3a401-04:3      energy-pkg      233.507263      0.064863  
3a401-04:3      energy-ram      12.370789       0.003436  

Total:
#Source         total(j)        avg(j)          min(j)          max(j)          total(W.h)      avg(W.h)        min(W.h)        max(W.h)  
energy-cores    823.794678      205.948669      194.357422      216.447571      0.228832        0.057208        0.053988        0.060124  
energy-pkg      970.565796      242.641449      230.965149      253.188782      0.269602        0.067400        0.064157        0.070330  
energy-ram      50.890930       12.722733       12.370789       13.127563       0.014136        0.003534        0.003436        0.003647  

```


Adding the `-d` option prints the collected data for each machine.

Several hardware counters are collected:

- `energy-cores`: energy consumed by the CPU 
- `energy-gpu`: energy consumed by the integrated GPU
- `energy-ram`: energy consumed by the RAM
- `energy-pkg`: energy consumed by the whole socket (usually this corresponds to CPU+RAM)
- `energy-psys`: total energy consumed by the machine
