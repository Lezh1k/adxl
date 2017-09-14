#!/bin/bash
export PATH=/home/lezh1k/Distr/jlink/JLink_Linux_V620_x86_64:$PATH
JLinkGDBServer -nosilent -swoport 2332 -select USB -telnetport 2333 -endian little -noir -speed auto -port 2331  -vd -device LPC824M201 -if SWD -reportuseraction
