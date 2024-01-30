# Calcium

Shared programming language for the CPU and GPU.

## Examples

### Hello World
```
fun printf(* byte): int;

fun main(): unit {
    printf("Hello, world!");
}
```

#### Compilation and execution
```bash
./calcium examples/helloworld.ca 
clang native/main.cpp examples/helloworld.o -o helloworld
./helloworld
```

#### Output
```
Hello, world!
```

## Future Plans

* Emitting `ker` functions as SPIR-V assembly.
* Supplying kernel as shaders to Vulkan bindings.
* Executing compute shaders directly. Specifying blocks, threads.
* Shared constants in CPU and GPU code.
* Shared memory and shader uniforms. 
