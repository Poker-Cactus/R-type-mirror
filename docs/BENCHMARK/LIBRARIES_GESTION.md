# Static vs Dynamic Libraries Comparison

## Static Library (Engine.a / .lib)

| Characteristic | Description |
|----------------|-------------|
| **Linking** | Static: Engine code is copied into the game executable during compilation |
| **Executable Size** | Larger (contains all necessary engine code) |
| **Deployment** | Very Simple: Single executable to distribute |
| **Updates** | Complex: Game must be recompiled for engine changes |
| **Modularity** | Low: Not well-suited for external plugin systems |
| **Performance** | Better: Direct linking, better optimization opportunities by the compiler |

## Dynamic Library (Engine.so / .dll)

| Characteristic | Description |
|----------------|-------------|
| **Linking** | Dynamic: Engine code loaded into memory at runtime |
| **Executable Size** | Smaller (contains only references to the .so/.dll) |
| **Deployment** | More Complex: Requires both the Executable AND the .so/.dll file |
| **Updates** | Easy: Replace .so/.dll without recompiling the game |
| **Modularity** | High: Ideal for plugin systems or Hot Reload |
| **Performance** | Slightly Lower: Indirect jumps (PIC), initial loading time |

## Conclusion

Since both methods have their advantages and disadvantages, the choice depends
on the specific needs of the game project, such as ease of updates, performance requirements, and deployment complexity.

This is why we will use both methods for our Game Engine case:

- **Static library** will be used for performance-critical components,
    such as the core rendering engine and physics
- **Dynamic library** will be used for less critical modules,
    such as extension plugins and development tools
