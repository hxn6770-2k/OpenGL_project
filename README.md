## OpenGL Mesh, Texture, and Shader Management Module

## Project Overview

This project implements a core OpenGL rendering pipeline module focusing on efficient mesh generation, texture loading, and GLSL shader program compilation with detailed error handling. It supports procedural geometry creation for complex parametric shapes, sets up interleaved vertex attributes, manages GPU resources, and integrates texture mapping with mipmapping for real-time 3D rendering.

---

## Technical Features & Implementation Details

### Mesh Generation & Management

* **Procedural Mesh Construction:**

  * Supports parametric generation of cups and prisms with configurable parameters such as sector count, top/bottom radius, and height for cups.
  * Generates vertex attributes in a tightly packed interleaved format `[position (vec3), normal (vec3), texCoord (vec2)]` to optimize GPU memory usage and cache locality.
  * Normals are computed analytically to support per-vertex lighting calculations, with outward-pointing radial normals for cylindrical geometry.
  * `std::vector<float>` dynamically stores vertex data for flexible mesh sizes.

* **Buffer & VAO Setup:**

  * Utilizes OpenGL's Vertex Array Objects (VAOs) and Vertex Buffer Objects (VBOs) for state encapsulation and efficient draw calls.
  * Attribute pointers are defined with precise offsets and strides, ensuring correct alignment and enabling the GPU to interpret data without additional overhead.
  * Calculates vertex count by dividing the total float count by the attribute stride to automate draw calls.

### Texture Loading & Configuration

* **Image Loading:**

  * Integrates `stb_image` library for cross-platform image loading supporting multiple image formats (JPEG, PNG, etc.).
  * Handles vertical flipping of texture data to accommodate OpenGL’s bottom-left origin texture coordinate system.

* **Texture Parameters:**

  * Sets wrapping mode to `GL_REPEAT` for tiling textures across UV space.
  * Applies linear filtering (`GL_LINEAR`) for minification and magnification for smooth texture scaling.
  * Supports automatic mipmap generation (`glGenerateMipmap`) for optimized rendering at varied distances.

* **Channel Handling:**

  * Dynamically detects texture format (RGB or RGBA) to allocate GPU texture memory with appropriate internal format (`GL_RGB8`, `GL_RGBA8`).
  * Provides error logging for unsupported channel counts.

### Shader Compilation & Linking

* **Shader Pipeline:**

  * Implements separate vertex and fragment shader creation from raw GLSL source strings.
  * Checks and logs compile errors per shader stage for rapid debugging of GLSL syntax or semantic errors.
  * Links shaders into a program object with error validation and logs linking failures with detailed info.

* **Shader Program Management:**

  * Proper cleanup of shader objects and program on destruction to prevent resource leaks.
  * Binds shader program immediately after creation to ensure correct usage.

### Resource Management & Clean-Up

* Implements robust destruction functions for VAOs, VBOs, textures, and shader programs, ensuring GPU resources are freed correctly.
* Uses RAII-inspired, manual cleanup approach compatible with C-style OpenGL API.

---

## Code Architecture & Design Choices

* **Interleaved Vertex Buffer Format:**
  Combining position, normal, and texture coordinates in one buffer reduces multiple buffer bindings and improves data locality on GPU.
* **Procedural Meshes vs Static Geometry:**
  Procedural generation enables parametric control and can scale to complex shapes without manually authored vertex data.
* **Error Handling & Debugging:**
  Comprehensive logging during shader compilation and linking aids in shader development workflows and integration with external tools.

---

## Usage Summary

* Call `UCreateMesh(type)` with `"cup"` or `"prism"` to generate GPU-ready mesh data.
* Use `UCreateTexture(filename, textureId)` to load an image file as an OpenGL texture object.
* Compile GLSL shaders using `UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, programId)`.
* Bind generated VAO and shader program for rendering.
* Cleanup resources after usage via `UDestroyMesh()`, `UDestroyTexture()`, and `UDestroyShaderProgram()`.

---

## Future Work & Improvements

* Implement indexed element buffers (EBO) to reduce vertex duplication and improve memory bandwidth.
* Expand procedural geometry to support normals for curved surfaces (e.g., smooth normals for cups).
* Add support for normal mapping and advanced material properties in shaders.
* Introduce uniform buffer objects (UBOs) for efficient uniform data management.
* Integrate a scene graph for hierarchical transformations and mesh instancing.
* Wrap OpenGL resource management into C++ classes to enforce RAII and exception safety.

---

## Skills Demonstrated

* Expert-level OpenGL buffer and attribute management with VAOs and VBOs.
* Procedural geometry generation and analytic normal vector computation.
* GLSL shader compilation, linking, and debug logging.
* Texture loading with image format detection, vertical flipping, and mipmap generation.
* GPU resource lifecycle management to prevent memory leaks.
* Strong C++ memory and container management integrated with OpenGL APIs.

---

This project showcases a solid foundation in modern graphics programming, enabling scalable 3D rendering with high-performance GPU data handling and shader-driven pipelines.

---
