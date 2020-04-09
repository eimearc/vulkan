#!/bin/bash

make

DIR=$(PWD)
cd $DIR/vulkan && ./vulkan&
cd $DIR/gl && ./gl&