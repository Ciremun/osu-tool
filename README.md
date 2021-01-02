# osu!tool

replaces AR, OD, CS lines in [osu!](https://osu.ppy.sh/home) map file  

## Dependencies

C++20 Compiler  

## Run

    g++ main.cpp -o ot -std=c++20
    ./ot [osu-path]

## Usage

    <song-title-word> <song-diff-word> <map-params>

    speed 100 ar=10 cs=5
    (modified file antiplur - speed of link (ktgster) [100 000 000ms])

`Return` reverts previous map file to initial state  
