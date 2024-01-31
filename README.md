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

### Shaders

```
// native definitions from the runtime
struct Window;

fun createWindow(title: * byte, width: int, height: int): * Window;
fun setupDemoPipeline(* Window, * Kernel): unit;
fun update(* Window): unit;
fun closed(* Window): bool;
fun destroyWindow(* Window): unit;

// kernel/shader compiled to SPIR-V bytecode
ker fragmentShader(uv: vec2): vec4 {
    return vec4(1.0, 0.0, 0.0, 1.0);
}

fun main(): unit {
    let window: * Window = createWindow("Shader Demo", 800, 600);

    setupDemoPipeline(window, fragmentShader); // pass in compiled kernel

    while (!closed(window)) {
        update(window);
    }

    destroyWindow(window);
}
```


## Future Plans

* Emitting `ker` functions as SPIR-V assembly.
* Supplying kernel as shaders to Vulkan bindings.
* Executing compute shaders directly. Specifying blocks, threads.
* Shared constants in CPU and GPU code.
* Shared memory and shader uniforms. 
