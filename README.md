# ifget
Get specific data from a given network interface.

Goal is to be portable: not depend on a given tool
(ifconfig(8), iwconfig(8), iw(8), ip(8)) nor on a
non-standardize outputs.

As a side effect, it should be much more efficient
than spawing a `ifconfig | sed/awk`.

# Build

    % cc ifget.c `uname`.c -o ifget
    % ./ifget help

# Portability
For now, it has only been tested with Linux. Multiple
methods are used to provide basis for other OSs.
For instance, most ioctl(2) should be portable to other
Unix, same goes for getifaddrs(3).

FreeBSD has been partially tested.

Please, report and port :-)

# TODO
Try to reduce ioctl(2) as much as possible in favors
of pseudo fs or syscall. Refactor.
