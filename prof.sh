#!/bin/sh

CPUPROFILE=/tmp/prof.out ./Debug/nicson test/large-test.json

google-pprof --pdf ./Debug/nicson /tmp/prof.out > prof.pdf

