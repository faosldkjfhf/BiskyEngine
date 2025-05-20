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

## File Structure

"Engine" contains the code that is wrapped around D3D12.
"Game" contains an example using the libraries that I have built.

## Installation

This project is build with Visual Studio 2022 and requires VCPKG in manifest mode to be activated.
To build/run, clone the repository and open the solution in Visual Studio 2022.
