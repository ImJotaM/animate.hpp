# Animate.hpp

> [!WARNING]
> **UNDER ACTIVE DEVELOPMENT**: This library is currently in an experimental stage. The API is subject to frequent and breaking changes. It is **not recommended for use in production projects** at this time.

A lightweight, header-only C++ animation library designed for game development and interactive applications. It provides an event-driven system to manage animations with support for repetitions, custom update logic, and lifecycle callbacks.

## Features

* **Header-Only:** Easy to integrate into any C++ project.
* **Stable Handles:** Uses `InstanceId` to safely control active animations.
* **Object Agnostic:** Uses `void*` context to animate any data structure or object.
* **Event-Driven:** Hooks for `onStart`, `onUpdate`, `onEachRepeatStart`, `onEachRepeatEnd`, and `onEnd`.
* **Centralized Management:** Static `AnimationHandler` to update and track all active animations globally.

## Usage

Simply include `animate.hpp` in your project. In **one** C++ file, define `ANIMATE_HPP_IMPLEMENTATION` before including the header to generate the implementation.

```cpp
#define ANIMATE_HPP_IMPLEMENTATION
#include "animate.hpp"
```

## Quick Start (Raylib Example)

The following example demonstrates how to animate Raylib `Rectangle` structures, chain them, and control an instance in real-time.

```cpp
#include <raylib.h>

#define ANIMATE_HPP_IMPLEMENTATION
#include "animate.hpp"

int main() {
    InitWindow(800, 600, "Animate.hpp Example");
    SetTargetFPS(60);

    Rectangle rect = { 0.0f, 0.0f, 0.0f, 0.0f };
    Rectangle rect2 = { 0.0f, 0.0f, 0.0f, 0.0f };

    // 1. Create a reusable animation template
    AnimationId rect_grow = AnimationHandler::CreateAnimation({
        .onUpdate = [](float progress, void* obj){
            Rectangle* r = static_cast<Rectangle*>(obj);
            r->width = GetScreenWidth() * progress;
            r->height = GetScreenHeight() * progress;
        },
        .onEachRepeatEnd = [](void* obj) {
            Rectangle* r = static_cast<Rectangle*>(obj);
            r->width = r->height = 0.0f;
        }
    });

    InstanceId next_instance;

    // 2. Attach animations and chain them via events
    AnimationHandler::AttachAnimation(rect_grow, &rect2, 3.0f, 2, {
        .onEnd = [rect_grow, &rect, &next_instance]() {
            // Chains another infinite animation when this one ends
            next_instance = AnimationHandler::AttachAnimation(rect_grow, &rect, 2.0f, 0, {});
        }
    });

    float gt = 0;

    while(!WindowShouldClose()) {
        float dt = GetFrameTime();
        gt += dt;

        // 3. Update all active animations
        AnimationHandler::UpdateAnimations(dt);

        // 4. Control specific instances (Stop after 8 seconds)
        if (gt >= 8.0f) {
            AnimationHandler::Stop(next_instance);
        }

        BeginDrawing();
        ClearBackground(BLACK);
        DrawRectangleRec(rect, WHITE);
        DrawRectangleRec(rect2, RED);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

```

## API Overview

### AnimationEvents

| Event | Description |
| --- | --- |
| `onStart` | Triggered once when the animation instance begins. |
| `onUpdate` | Triggered every frame. Provides `float progress` (0.0 to 1.0) and the object pointer. |
| `onEachRepeatStart` | Triggered at the beginning of every loop iteration. |
| `onEachRepeatEnd` | Triggered at the end of every loop iteration. |
| `onEnd` | Triggered once the animation has finished all repetitions or is stopped. |

### AnimationHandler

* `CreateAnimation(events)`: Registers a template and returns an `AnimationId`.
* `AttachAnimation(...)`: Starts an instance and returns an `InstanceId`.
* `UpdateAnimations(dt)`: Advances the timeline for all active instances.
* `Pause(InstanceId)`: Suspends the execution of an instance.
* `Stop(InstanceId)`: Immediately ends an instance and triggers `onEnd`.
* `Continue(InstanceId)`: Resumes a paused instance.
* `Restart(InstanceId)`: Resets time and repeat count of an instance.

## Roadmap

### Phase 1: Core Functionality (Current Focus)

* [x] Basic animation lifecycle (Start, Update, End).
* [x] Object-agnostic context via `void*`.
* [x] **Stable Handles:** Memory-safe IDs that persist across reallocations.
* [X] **Pause/Resume/Restart/Stop:** Add methods to control specific animation instances.
* [ ] **Reverse Playback:** Ability to play animations backward.
* [ ] **Time Scaling:** Individual speed control for animations (e.g., slow-motion effects).`.

### Phase 2: Animation Features & Tweens

* [ ] **Easing Functions:** Built-in support for Linear, Quad, Cubic, Bounce, and Elastic easings.
* [ ] **Tweening Engine:** Dedicated helpers to interpolate between values (Start -> End) without manual math in lambdas.
* [ ] **Chaining System:** A more intuitive way to trigger animations sequentially (e.g., `.Then()`).
* [ ] **Groups:** Manage multiple animations as a single unit (Parallel or Sequential).

### Phase 3: Developer Experience & Safety

* [ ] **Type-Safe API:** Add a template-based wrapper to avoid manual `static_cast` from `void*`.
* [ ] **Performance Profiling:** Optimize the internal management for high-density animation counts.
* [ ] **More Examples:** Integration examples for other frameworks (SDL2, SFML).