#!/bin/bash

pushd ..
Vendor/Premake/Linux/premake5 --cc=clang --file=premake5.lua gmake2
popd
