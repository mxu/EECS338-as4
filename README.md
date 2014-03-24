# EECS338: Operating Systems

## Assignment 4: Bus Service Problem

## Mike Xu

## mxx17

<hr>

## Features

Implements the Bus Service Problem using Linux System V semaphores and shared memory

* Main process initializes shared memory structures and invokes other processes
* Agent process waits for customers and assigns tickets based on remaining seats
* Bus process increments depature times, boards waiting customers, then departs
* Customer process waits in line for a ticket, then boards the bus

<hr>

## How to build and rn

```
$> make clean
$> make
$> ./main [number of busses]
```

Running `./main` with no arguments defaults to 3 busses
