#!/bin/bash
export PATH=/home/lezh1k/Distr/jlink/JLink_Linux_V620_x86_64:$PATH
JLinkExe -device LPC824M201 -if SWD -speed 4000 -autoconnect 1 -CommanderScript jlink_program.jlink
