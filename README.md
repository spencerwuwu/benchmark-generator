# Random Benchmark Generator

[![Build Status](https://travis-ci.org/spencerwuwu/benchmark-generator.png?branch=master)](https://travis-ci.org/spencerwuwu/benchmark-generator)
[![contributions welcome](https://img.shields.io/badge/contributions-welcome-brightgreen.svg?style=flat)](https://github.com/dwyl/esta/issues)


Developed for the experiements of [J-ReCoVer](http://jrecover.iis.sinica.edu.tw/).

## Usage
```
$ ./generator filename <Variable Baseline If-else>
```
```
$ ./generator filename <Variable Baseline If-else>
  2 files, filename.c and filename.pre will be generated
  filename.pre is the intermediate code for smt-bmc-generator
```
```
./smt-bmc-generator filename
```
The smt-bmc-generator symbolic executes .pre to a set of smt-formulas with
[bounded model checking](http://fmv.jku.at/bmc/).


## Parameter
- Number of variable **N**
- Number of `if-else`  **I**
- Number of baseline **B**


## Approach

First the program generates a template as below. It declares a variable set including 
**N** random variables and a variable **cur**. Then it chooses an arbitrary variable 
to output at the end.
```
int main() {
	// Declare variables v1 - vn and cur
	int cur = 0;
	int v1 = 0; ....

	/* Function body */

	// Pick one to collect
  return 0;
}
```

For the loop body, the program will generate **B**+3***I** lines, which contains:

- **B** lines of  **assignment**
- **I**  lines of   **if ( condition ) {**
- **I**  lines of   **} else {**
- **I**  lines of   **}**

First it determine the order of the lines randomly but in a reasonable way. For example 
a program like
```
if (...) {
	...
}
}
} else
} else
```
would not happen. Then it fills out the content of **assignment** and **condition**.

An **assignment** is  in the form of 

```
V = V1 b_op V2
```
or
```
V = V1 u_op
```

A **condition** is 

```
V cmp V1
```
where **V** is chosen from the variable set. **V1 V2** can be either a variable or a 
number. **b_op** includes “+ - * / %”, **u_op** includes “+= -= *= /= %=”, and **cmp** 
includes “== != >= <= > <”.


