# Animate.hpp

> [!WARNING]
> **UNDER ACTIVE DEVELOPMENT**: This library is currently in an experimental stage. The API is subject to frequent and breaking changes. It is **not recommended for use in production projects** at this time.

A lightweight, header-only C++ animation library designed for game development and interactive applications. It provides an event-driven system to manage animations with support for repetitions, custom update logic, and lifecycle callbacks.

## Features

* **Header-Only:** Easy to integrate into any C++ project.
* **Object Agnostic:** Uses `void*` context to animate any data structure or object.
* **Event-Driven:** Hooks for `onStart`, `onUpdate`, `onEachRepeatStart`, `onEachRepeatEnd`, and `onEnd`.
* **Centralized Management:** Static `AnimationHandler` to update and track all active animations globally.
* **Lightweight:** Minimal dependencies, utilizing standard C++ library components.

## Usage

Simply include `animate.hpp` in your project. In **one** C++ file, define `ANIMATE_HPP_IMPLEMENTATION` before including the header to generate the implementation.

```cpp
#define ANIMATE_HPP_IMPLEMENTATION
#include "animate.hpp"
```

## Quick Start (Raylib Example)

The following example demonstrates how to animate Raylib `Rectangle` structures using the library.

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
    const AnimationId rect_to_screen_size = AnimationHandler::CreateAnimation({
        .onUpdate = [](float progress, void* obj){
            Rectangle* r = static_cast<Rectangle*>(obj);
            r->width = GetScreenWidth() * progress;
            r->height = GetScreenHeight() * progress;
        },
        .onEachRepeatEnd = [](void* obj) {
            Rectangle* r = static_cast<Rectangle*>(obj);
            r->width = 0.0f;
            r->height = 0.0f;
        }
    });

    // 2. Attach the animation to specific objects
    AnimationHandler::AttachAnimation(rect_to_screen_size, &rect2, 3.0f, 2, {
        .onEnd = [rect_to_screen_size, &rect]() {
            // Chains another animation when this one ends
            AnimationHandler::AttachAnimation(rect_to_screen_size, &rect, 2.0f, 3, {});
        }
    });

    while(!WindowShouldClose()) {
        float dt = GetFrameTime();

        // 3. Update all active animations
        AnimationHandler::UpdateAnimations(dt);

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

A configuration struct used to define behavior at different stages of the animation lifecycle.

| Event | Description |
| --- | --- |
| `onStart` | Triggered once when the animation instance begins. |
| `onUpdate` | Triggered every frame. Provides `float progress` (0.0 to 1.0) and the object pointer. |
| `onEachRepeatStart` | Triggered at the beginning of every loop iteration. |
| `onEachRepeatEnd` | Triggered at the end of every loop iteration. |
| `onEnd` | Triggered once the animation has finished all repetitions. |

### AnimationHandler

The static interface for managing the animation lifecycle.

* `CreateAnimation(AnimationEvents events)`: Registers an animation template and returns a unique `AnimationId`.
* `AttachAnimation(AnimationId id, void* obj, float duration, size_t repeat, AnimationEvents events)`: Starts an instance of a registered animation template on a specific object.
* `UpdateAnimations(float dt)`: Advances the timeline for all active instances.
* `RemoveAnimation(AnimationId id)`: Clears a template and stops its associated instances.
* `ClearAnimations()`: Wipes all templates and active instances.

## Roadmap

This project is in its early stages. Below is the plan for reaching a stable `v1.0.0` release.

### Phase 1: Core Functionality (Current Focus)

* [x] Basic animation lifecycle (Start, Update, End).
* [x] Object-agnostic context via `void*`.
* [ ] **Pause/Resume/Restart:** Add methods to control specific animation instances.
* [ ] **Reverse Playback:** Ability to play animations backward.
* [ ] **Time Scaling:** Individual speed control for animations (e.g., slow-motion effects).

### Phase 2: Animation Features & Tweens

* [ ] **Easing Functions:** Built-in support for Linear, Quad, Cubic, Bounce, and Elastic easings.
* [ ] **Tweening Engine:** Dedicated helpers to interpolate between values (Start -> End) without manual math in lambdas.
* [ ] **Chaining System:** A more intuitive way to trigger animations sequentially (e.g., `.Then()`).
* [ ] **Groups:** Manage multiple animations as a single unit (Parallel or Sequential).

### Phase 3: Developer Experience & Safety

* [ ] **Type-Safe API:** Add a template-based wrapper to avoid manual `static_cast` from `void*`.
* [ ] **Performance Profiling:** Optimize the internal `std::vector` management for high-density animation counts.
* [ ] **More Examples:** Integration examples for other frameworks (SDL2, SFML).