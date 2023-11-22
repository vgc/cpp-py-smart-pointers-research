# C++/Python Smart Pointers Research

This repository is for experiments with C++ smart pointers and Python bindings.

For easier reproducibility, each experiment is self-contained and is located
in a separate folder, e.g., `libs/x01`.

Below is a quick description of each experiment:

Experiment      | Description
--------------- | -----------
[x01](libs/x01) | Basic by-value class, no smart pointers
[x02](libs/x02) | Tree structure using std::unique_ptr
[x03](libs/x03) | Tree structure using std::shared_ptr (minimal)

# How to build?

```
git clone --recursive https://github.com/vgc/cpp-py-smart-pointers-research
cd https://github.com/vgc/cpp-py-smart-pointers-research
mkdir build && cd build
cmake ..
cmake --build . -j 4
```
