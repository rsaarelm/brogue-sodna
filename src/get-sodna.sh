#!/usr/bin/env bash
cd `dirname $0`
git clone https://github.com/rsaarelm/sodna sodna-0.2.0
cd sodna-0.2.0
git checkout d84f297
make
cp libsodna_sdl2.so ../../bin
