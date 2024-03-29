# Calcium

Shared programming language for the CPU and GPU.

## Examples

### Hello World
```kotlin
fun printf(* byte): int;

fun main(): unit {
    printf("Hello, world!");
}
```

#### Compilation and execution
```bash
./build-runtime.sh                # build runtime libraries (stdio, creating a basic window, ...)
./calcium examples/helloworld.ca  # compile program to .o
./build-local.sh                  # link program with runtime
./helloworld
```

#### Output
```
Hello, world!
```

### Shaders

```kotlin
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

* Executing compute shaders and specifying blocks/threads.
* Shared compile-time constants in CPU and GPU code.
* Shared/allocated memory and shader uniforms. 
* Actual semantic analysis.
