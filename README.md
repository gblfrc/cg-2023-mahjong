# Mahjong
This repository contains the code for the final project for the Computer Graphics course, held at [Politecnico di Milano](https://polimi.it) in Spring 2023. The project aims at implementing the multiple techniques learnt during the course by reproducing a famous solitaire game, [Mahjong](https://en.wikipedia.org/wiki/Mahjong_solitaire). For additional information about the project rules, please see the [related document](Projects%20rules.pdf).

DISCLAIMER: The project is a major task in the course work and the main requirement for its passing. Therefore, any form of plagiarism is highly discouraged and the authors do not hold accountable for any consequence thereof. This repository is published only as a potential source of inspiration for future students taking the course.

## Table of contents
- [Setup](#setup)
  -  [Visual Studio](visual-studio)
- [Troubleshooting](#troubleshooting)
- [Authors](#authors)

## Setup

The structure of this repository aims at reproducing the one of the archives provided by the course professor for the assignments. Therefore, following the same procedure should allow the reader to easily set up the environment and the application. For the sake of convenience, such procedure is reported hereafter for the main IDEs adopted by the authors for development.

### Visual Studio
1. Set up a Vulkan project as shown in the [official documentation](https://vulkan-tutorial.com/Development_environment);
2. Remove the `main.cpp` created during the tutorial;
3. In the project settings, add the `headers` folder under `C++ -> General -> Additional Include Directories`;
4. Download this repository as a *zip* archive;
5. Extract the content of the archive in the VS project folder;
6. Include all the downloaded files and folders in the project;
7. Compile and run the project.



## Troubleshooting
- This repository contains a *models* folder with *.obj* files. It is advisable not to include such files in the VS project for this might cause the throw of a VS compilation error (LNK1136) claiming them to be corrupted.
- It might happen, upon compiling the project with VS, that an error (C4996) is thrown for the presence of an unsafe function *sprintf* in the header file *stb_image_write.h*. To solve this issue, simply replace such function with *sprintf_s*.


## Authors
- [Luca Gerin](https://github.com/LucaGerin)
- [Federico Gibellini](https://github.com/gblfrc)
- [Marta Radaelli](https://github.com/MartaRadaelli)
