# Custom Watercolour Postprocessing Graphic Pipeline

A real-time, GPU-based non-photorealistic rendering (NPR) pipeline built in **C++ / DirectX 11 (HLSL)** that converts a normal 3D scene into a stylised watercolour painting, entirely as a chain of full-screen post-processing passes. Built for the CMP301 Real-Time Graphics module at Abertay University, on top of the course's `DXFramework`.

Every stage of the pipeline is inspectable at runtime through an ImGui debug panel, so intermediate buffers (structure tensor, flow map, DoG edges, quantised colour, etc.) can be viewed and tuned individually.

## What it does

The scene (an animated ocean with ships, a skybox, and simple procedural geometry) is rendered normally, then run through a multi-pass image-space pipeline that mimics traditional watercolour techniques: soft, edge-aware colour blurring, ink-like flow-guided outlines, quantised colour banding, and a paper/canvas texture overlay.

## Pipeline stages

The full per-frame pass order, from `App1::Render()`:

1. **Depth Pass** – renders scene depth, used later to modulate the paper overlay
2. **First Pass** – renders the base 3D scene (ocean, ships, skybox) to a render target
3. **RGB → YCbCr** – converts the frame to luminance/chrominance so filtering can be edge- and colour-aware separately
4. **Structure Tensor** – computes local gradient direction (Sobel-based) per pixel, the basis for flow-aligned filtering
5. **Horizontal / Vertical Smoothing** – blurs the structure tensor to build a stable edge flow map
6. **Bilateral Filter (edge pass)** – edge-preserving smoothing along the flow field to soften colour regions without losing outlines
7. **Difference of Gaussians (DoG)** – edge/outline detection along the flow direction
8. **Flow-Guided DoG (Flow Curve)** – walks along the flow field to trace coherent, ink-like curved lines rather than raw per-pixel edges
9. **Bilateral Filter (abstraction pass)** – a second round of flow-aligned smoothing for stronger painterly abstraction
10. **Colour Quantization** – bands the luminance channel into discrete steps with smoothstep-softened transitions, for a painted/posterised look
11. **Combine Pass** – blends the quantised colour with the traced edge lines
12. **YCbCr → RGB** – converts back to a standard colour space
13. **Paper/Canvas Overlay** – blends in a canvas texture, modulated by scene depth, to simulate paint soaking into paper
14. **Temporal Coherence Pass** – blends with the previous frame's output to reduce flicker between frames
15. **Comparison Pass** – renders a wipe/slider view of the original vs. stylised output
16. **Final Pass** – presents the selected output to the screen

## Features

- **Flow-guided Difference of Gaussians** for coherent, direction-aware line art instead of noisy per-pixel edge detection
- **Edge-aware bilateral filtering** run in two configurable passes (edge softening and abstraction) with adjustable spatial/range sigma
- **Colour quantization** with adjustable smoothing and quantization levels for a hand-painted banding effect
- **Depth-modulated paper/canvas overlay** so the "paint on paper" effect responds to scene depth
- **Temporal blending** to stabilise the effect across frames in the animated scene
- **Before/after comparison slider** to directly compare the original render against the stylised result
- **Full runtime debug UI (ImGui)** exposing every intermediate texture in the pipeline (16 selectable output stages) and every tunable parameter (bilateral filter strength, DoG sensitivity/smoothing/tau, flow curve phi/sigma, quantization level, paper strength, temporal blend strength, and more)
- **Animated demo scene**: a simple wave-simulated ocean, ship models, skybox, and an arcball camera for orbiting the scene

## Project structure

```
E1_Geometry/
├── E1_Geometry/          # Main application project
│   ├── App1.cpp/.h       # Pipeline orchestration, render loop, ImGui panel
│   ├── *Shader.cpp/.h    # C++ wrappers for each shader pass
│   ├── *.hlsl            # HLSL vertex/pixel shaders for each pass
│   │                       (structure tensor, DoG, flow curve, bilateral
│   │                       filter, colour quantization, paper, ocean, etc.)
│   ├── ArcBallCamera.*   # Orbit camera controller
│   └── res/              # Models, textures, canvas/paper texture
├── DXFramework/          # Course-provided DirectX 11 base framework
└── GetLibraries.bat      # Fetches the required DXFramework libraries
```

## Building

This is a Visual Studio / DirectX 11 project.

1. Run `GetLibraries.bat` (inside `E1_Geometry/`) to download the required `lib/` and `x64/` dependencies from the [CMP301_Libraries](https://github.com/Abertay-University-SDI/CMP301_Libraries) repository.
2. Open `E1_Geometry.sln` in Visual Studio.
3. Build and run (x64, DirectX 11 required).

## Background

This project was completed as part of the Honours-year Real-Time Graphics coursework at Abertay University, focused on implementing a research-informed non-photorealistic rendering technique (flow-guided DoG + edge-aware filtering + colour quantization, in the style of the Kyprianidis/Kang family of NPR papers) as a fully real-time, GPU-driven pipeline.

## Media

See the [`Media/`](./Media) folder for skybox/cubemap assets used in the demo scene.
