cd build
cmake  -G"Ninja" -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ ..
cmake  --build .
cd ..