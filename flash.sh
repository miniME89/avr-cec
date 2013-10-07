#!/bin/sh

avrdude -c stk500 -p m8 -P com8 -U flash:w:build/main.hex
