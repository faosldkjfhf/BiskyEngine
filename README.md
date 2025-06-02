# BiskyEngine

A D3D12 Renderer/Engine. 

Currently a work in progress and for me to learn and play with D3D12.

## Features

- [x] Filetypes
  - [x] GLTF 2.0
- [ ] Lighting
  - [x] Blinn-Phong
  - [ ] Shadow mapping
    - [ ] Percentage-closer filtering
  - [ ] PBR
  - [ ] Cubemap/Image-Based Lighting
  - [ ] HDR/Tonemapping
  - [ ] Gamma correction
  - [ ] Bloom
- [ ] Editor
  - [ ] Switch between preloaded models
  - [ ] Load your own models
  - [ ] Scene editor
  - [ ] Profiler

## Implemented Papers

- [ ] [A Reflectance Model for Computer Graphics](https://graphics.pixar.com/library/ReflectanceModel/paper.pdf)
- [ ] [Microfacet Models for Refraction through Rough Surfaces](https://www.graphics.cornell.edu/~bjw/microfacetbsdf.pdf)

## File Structure

`Bisky` contains the code that is wrapped around D3D12.  
`Sandbox` contains an example using the library that I have built.

## Updates

### 6/2/2025 - Bindless Rendering

Did a huge refactor, basiclly rewriting everything that I had originally.  
For a quick summary, here are the biggest points:
- [x] Bindless Rendering
- [x] New Library Structure
- [x] Wrappers over D3D12
Spent a lot of time learning the bindless rendering setup and was able to implement it.
The new file structure is as follows:
- `Bisky`
  - `Core`
  - `Editor`
  - `Graphics`
  - `Renderer`
  - `Scene`
- `Sandbox`
  - `Assets`
  - `Main.cpp`

The main library code lies within `Graphics`, while supporting classes lie in `Core`.
`Editor` contains the ImGui wrapper code. `Renderer` contains resources for drawing.  
`Scene` contains code for setting up a render scene.

## Installation

This project is built with Visual Studio 2022 and requires VCPKG in manifest mode to be activated.
To build/run, clone the repository and open the solution in Visual Studio 2022.
