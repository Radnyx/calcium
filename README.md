# Calcium

Shared programming language for the CPU and GPU.

## Current Progress

### Program
```
fun printf(* byte): int;

fun init(): unit {
    printf("Hello, world!");
}
```

### Compilation and execution
```bash
./calcium examples/test.ca 
clang main.c examples/test.o -o test.exe
./test
```

### Output
```
Hello, world!
```

## Future Plans

* Emitting `ker` functions as SPIR-V assembly.
* Executing kernels directly and specifying blocks, threads.
* Supplying kernel as shaders to Vulkan bindings.
