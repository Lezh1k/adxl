#!/bin/bash
#JLinkGDBServer -if SWD -speed 4000 -device LPC824M201
JLinkGDBServer -nosilent -swoport 2332 -select USB -telnetport 2333 -endian little -noir -speed auto -port 2331  -vd -device LPC824M201 -if SWD -nohalt -reportuseraction
