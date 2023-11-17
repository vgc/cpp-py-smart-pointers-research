# C++/Python Smart Pointers Research

This repository is for experiments with C++ smart pointers and Python bindings.

For easier reproducibility, each experiment is self-contained and is located
in a separate folder, e.g., `libs/x01`.

Below is a quick description of each experiment:

| Experiment      | Description |
| --------------- | ----------- |
| [x01](libs/x01) | Basic by-value class, no smart pointers |


# How to build?

```
git clone --recursive https://github.com/vgc/cpp-py-smart-pointers-research
cd https://github.com/vgc/cpp-py-smart-pointers-research
mkdir build && cd build
cmake ..
cmake --build . -j 4
```
