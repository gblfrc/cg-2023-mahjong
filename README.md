# Mahjong
This repository contains the code for the final project for the Computer Graphics course, held at [Politecnico di Milano](https://polimi.it) in Spring 2023. The project aims at implementing the multiple techniques learnt during the course by reproducing a famous solitaire game, [Mahjong](https://en.wikipedia.org/wiki/Mahjong_solitaire). For additional information about the project rules, please see the [related document](Projects%20rules.pdf).

DISCLAIMER: The project is a major task in the course work and the main requirement for its passing. Therefore, any form of plagiarism is highly discouraged and the authors do not hold accountable for any consequence thereof. This repository is published only as a potential source of inspiration for future students taking the course.

## Table of contents
- [Setup](#setup)
  -  [Visual Studio](#visual-studio)
- [Limitations](#limitations)
- [Troubleshooting](#troubleshooting)
- [Grading](#grading)
- [Authors](#authors)

## Setup

The structure of this repository aims at reproducing the one of the archives provided by the course professor for the assignments. Therefore, following the same procedure should allow the reader to easily set up the environment and the application. For the sake of convenience, such procedure is reported hereafter for the IDEs adopted by all the authors for development, i.e., Visual Studio[^vs].

### Visual Studio
1. Set up a Vulkan project as shown in the [official documentation](https://vulkan-tutorial.com/Development_environment);
2. Remove the `main.cpp` created during the tutorial;
3. In the project settings, add the `headers` folder under `C++ -> General -> Additional Include Directories`;
4. Download this repository as a *zip* archive;
5. Extract the content of the archive in the VS project folder;
6. Include all the downloaded files and folders in the project;
7. Compile and run the project.

## Limitations
- The project is expected to run only on Windows because of a library used in the project: in order to introduce sound effects in the game, indeed, the authors decided to use a Windows-specific library because of its simplicity but at the cost of limiting the application portability. In addition, it is worth mentioning that all the authors owned, at development time, only Windows machines and, therefore, they developed the project under such operating system. 
- The game might not be run on all the GPUs currently available on the market. In order to allow object selection with the mouse cursor, a render-to-texture mechanism was chosen. However, the implementation of this process involved the rendering of a specific image in a host-visible portion of memory with a specific format, i.e., 32-bit signed integer. Some GPUs might not have such memory location available or they might not accept the chosen format, thus preventing the application to run. Nevertheless, the project was tested and launched on multiple devices and it demonstrated to work on Intel, AMD and Nvidia cards, mostly CPU-integrated. Therefore, errors are more likely to occur with dedicated cards.

## Troubleshooting
- This repository contains a *models* folder with *.obj* files. It is advisable not to include such files in the VS project for this might cause the throw of a VS compilation error (LNK1136) claiming them to be corrupted.
- It might happen, upon compiling the project with VS, that an error (C4996) is thrown for the presence of an unsafe function *sprintf* in the header file *stb_image_write.h*. To solve this issue, simply replace such function with *sprintf_s*.

## Grading
The project was presented in the Summer session, on 27th July 2023 and was evaluated with the highest grade.

## Authors
- [Luca Gerin](https://github.com/LucaGerin)
- [Federico Gibellini](https://github.com/gblfrc)
- [Marta Radaelli](https://github.com/MartaRadaelli)

[^vs]: Please notice that the IDE used in this context is Visual Studio Community, which differs from the most common Visual Studio Code.
