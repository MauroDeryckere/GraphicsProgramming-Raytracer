# Raytracer

A CPU raytracer built with C++20, featuring physically-based rendering with Cook-Torrance materials, soft shadows via area lights, and BVH acceleration.

## Table of Contents
- [Screenshots](#screenshots)
- [Features](#features)
- [Controls](#controls)
- [Building](#building)

## Screenshots

### Reference Scene
![Reference scene](docs/Reference.png)

### Debug Views
![Observed area](docs/ObservedArea.png)
![Radiance](docs/Radiance.png)

## Features

- Whitted-style ray tracing on the CPU
- Parallel rendering with `std::execution::par_unseq`
- Cook-Torrance BRDF (GGX distribution, Smith geometry, Fresnel-Schlick)
- Lambert and Lambert-Phong shading models
- Soft shadows with area lights (triangular)
- Progressive rendering with per-frame accumulation
- Anti-aliasing: random and uniform multi-sampling
- Triangle mesh rendering with OBJ loading
- Bounding Volume Hierarchy (BVH) acceleration
- AABB slab test for ray-box intersection
- Multiple debug visualization modes: observed area, radiance, BRDF, combined
- Tone mapping (Reinhard Jodie, ACES approximation)
- Custom math library (vectors, matrices)

## Controls

**Camera**<br>
WASD: Move<br>
LMB + drag: Rotate<br>

**Settings**<br>
X: Screenshot<br>
F2: Toggle shadows<br>
F3: Cycle light mode (observed area / radiance / BRDF / combined)<br>
F4: Cycle sample mode (random / uniform)<br>
F5: Decrease samples per pixel<br>
F6: Increase samples per pixel<br>
F7: Cycle tone mapping (none / Reinhard Jodie / ACES)<br>

## Building

The project uses CMake.

```bash
cmake -S . -B build
cmake --build build
```
