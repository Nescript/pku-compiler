# Overview

This is a repo for PKU compiler project

# Quick start
just run the following command:

```bash
docker run -it --rm -v /home/nesc/pku-compiler:/root/compiler maxxing/compiler-dev bash
```


# build
to build this project, run
```bash
cmake -DCMAKE_BUILD_TYPE=Debug -B build
cd build && cmake ..
cd .. && cmake --build build
```

each time u add a new file, u have to cd build folder and run
```
cmake ..
```
that will rescan src folder and include folder for new files

# test

to generate IR, run
```bash
build/compiler -koopa test/hello.c -o test/hello.koopa
```

to generate riscv, run
```bash
build/compiler -riscv test/hello.c -o test/hello.riscv
```
