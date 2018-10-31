# Random Benchmark Generator

## Parameter
- Number of variable **N**
- Number of `if-else`  **I**
- Number of baseline **B**


## Approach

First the program generates a template as below. It declares a variable set including **N** random variables and a variable **cur**. Then it chooses an arbitrary variable to output at the end.
```
public void reduce(Text prefix, Iterator<IntWritable> iter,
		OutputCollector<Text, DoubleWritable> output, Reporter reporter) throws IOException {
	// Declare variables v1 - vn and cur
	double cur = 0;
	// double v1 = 0; ....

	while (iter.hasNext()) {
		cur = iter.next().get();
		/* Loop body */
	}

	// Pick one to collect
	output.collect(prefix, new DoubleWritable(vn));
}
```

For the loop body, the program will generate **B**+3***I** lines, which contains:

- **B** lines of  **assignment**
- **I**  lines of   **if ( condition ) {**
- **I**  lines of   **} else {**
- **I**  lines of   **}**

First it determine the order of the lines randomly but in a reasonable way. For example a program like
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

A **condition** is 

```
V cmp V1
```
where **V** is chosen from the variable set. **V1 V2** can be either a variable or a number. **b_op** includes “+ - * / %”, and **cmp** includes “== != >= <= > <”.

For the scalability experiment we remove “/ %” because these operations would be too hard for SMT solver.   
For "*" it has a 1% chance to be used and it is always **V * N** where N is an integer.


