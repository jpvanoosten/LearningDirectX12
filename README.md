# LearningDirectX12

This repository is intended to be used as a code repository for learning DirectX 12. The tutorials can be found on https://www.3dgep.com

## Requirements

- Visual Studio 2022 or 2026
- [CMake](https://cmake.org/) 4.0 or newer

## Cloning the Repository

This project uses git submodules. To clone the repository with all submodules, use:

```bash
git clone --recursive https://github.com/jpvanoosten/LearningDirectX12.git
```

If you've already cloned the repository without submodules, you can initialize them with:

```bash
git submodule update --init --recursive
```

## Building the Project

This project uses [CMake](https://cmake.org/) (4.0 or newer) with CMake presets defined in [CMakePresets.json](CMakePresets.json) to generate the project and solution files.

To use this project, run the [GenerateProjectFiles.bat](GenerateProjectFiles.bat) script and open the generated Visual Studio solution file in the build folder that gets created.

Assets for the samples can be downloaded using the [DownloadAssets.bat](DownloadAssets.bat) batch file located in the root folder of this project.

For more instructions see [Getting Started](https://github.com/jpvanoosten/LearningDirectX12/wiki/Getting-Started).
