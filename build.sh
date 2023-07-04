#!/usr/bin/env bash

export EM_CACHE="$(pwd)"/build/.emscriptencache

mkdir build
cd build || exit

mkdir .emscriptencache
emcmake cmake "$cmakeFlags" .. || exit
emmake make -j24 || exit

cd bin || exit
python -m http.server 9000

cd ../..
