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
Total:
[Total]         energy-cores Energy Consumed (total/avg/min/max): 541.067810/541.067810/541.067810/541.067810 Joules (0.150297/0.150297/0.150297/0.150297 watts.hour)
[Total]         energy-gpu Energy Consumed (total/avg/min/max): 0.232483/0.232483/0.232483/0.232483 Joules (0.000065/0.000065/0.000065/0.000065 watts.hour)
[Total]         energy-pkg Energy Consumed (total/avg/min/max): 607.711060/607.711060/607.711060/607.711060 Joules (0.168809/0.168809/0.168809/0.168809 watts.hour)
[Total]         energy-ram Energy Consumed (total/avg/min/max): 42.524353/42.524353/42.524353/42.524353 Joules (0.011812/0.011812/0.011812/0.011812 watts.hour)
[Total]         energy-psys Energy Consumed (total/avg/min/max): 936.560242/936.560242/936.560242/936.560242 Joules (0.260156/0.260156/0.260156/0.260156 watts.hour)

```


Adding the `-d` option prints the collected data for each machine.

Several hardware counters are collected:

- `energy-cores`: energy consumed by the CPU 
- `energy-gpu`: energy consumed by the integrated GPU
- `energy-ram`: energy consumed by the RAM
- `energy-pkg`: energy consumed by the whole socket (usually this corresponds to CPU+RAM)
- `energy-psys`: total energy consumed by the machine
