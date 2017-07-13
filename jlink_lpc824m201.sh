#!/bin/bash
JLinkExe -device LPC824M201 -if SWD -speed 4000 -autoconnect 1 -CommanderScript jlink_program.jlink
