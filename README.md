# PPM to PGM Image Converter with Blur Function

This project provides a C implementation that reads a PPM (Portable Pixmap) image file and converts it to a PGM (Portable Graymap) image file. Additionally, the program includes functionality to apply a blur effect to the image.


## Code Versions

The code has three different implementations for converting and blurring the image:

1. **Naive Version**: A straightforward approach that focuses on simplicity.
2. **Integer Version**: An optimized version that leverages integer arithmetic for better performance.
3. **SIMD Version**: A highly optimized version that uses SIMD (Single Instruction, Multiple Data) instructions to enhance performance.

## Directory Structure

- `Implementierung/tests`: Contains test cases for functionality and performance. These tests verify the correctness of the implementation and compare the performance of the three different code versions.
  
- `Implementierung/samples`: Contains sample images that were used for analysis during the development and testing of the project.
  
- `Ausarbeitung`: This directory contains the LaTeX files that provide a detailed explanation of the problem. It also includes a comprehensive report on the work done, with an in-depth discussion of the approach and results.
