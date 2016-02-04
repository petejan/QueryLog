# QueryLog

This module is software to read and write strings to a serial port or GPIB port.

Its intended to provide a means of sending command to instruments in the electronics lab.

Its built with borland C++BuilderX (free version).

        -et                                  - disable EOT mode
        -es                                  - set EOS mode to (0A)
        -c                                   - clear device first
        -r                                   - read device
        -f [+][[hh:]mm:]ss                   - first time (abs or offset)
        -l [+][[hh:]mm:]ss                   - last time (abs or offset)
        -p [[hh:]mm:]ss                      - period
        -n <number of messages>
        -a <GPIB address>
        -d <serial port (eg COM1)>
        -s <serial port setup (19200,N,8,1)>

        -t                                   - print timestamp
        -F <fileName>                        - read setup from a file

        when the -F is specified many polling command can be read from the file
        (lines starting with ; are not processed eg comments).

Peter Jansen
24 August 2005

For the Old Fridge

Start:
-a 6 -es 0x0a R00000000 -c -r -n 1
Set 20C:
-a 6 -es 0x0a U11200 -c -r -n 1
Stop:
-a 6 -es 0x0a R00000001 -c -r -n 1

For the Scanner

-a 9 *RST\n -n 1
-a 9 *IDN?\n -n 1 -r
-a 9 "CONF:TEMP FRTD, (@101)\n" -n 1
-a 9 "CONF:VOLT:DC (@120)\n" -n 1
-a 9 "CONF:CURR:DC (@121)\n" -n 1
-a 9 "ROUT:SCAN (@101,120,121)\n" -n 1
-a 9 READ?\n -r -p 10

To Log Serial Ports

setup to log the comm ports on receiving characters

-d COM1 -s 19200,N,8,1 -r -es 0x0a

For the new fridge

-a 7 -es 0x0a TEMP,S20\n -n 1

-a 7 -es 0x0a MODE:STANDBY\n -n 1
-a 7 -es 0x0a MODE:CONSTANT\n -n 1
-a 7 -es 0x0a TEMP?\n -n 1

