# A groovy modbus library

[![GitHub release (latest by date including pre-releases)](https://img.shields.io/github/v/release/epsilonrt/libmodbus?include_prereleases)](https://github.com/epsilonrt/libmodbus/releases)  
[![Build Status](https://github.com/epsilonrt/libmodbus/actions/workflows/build.yml/badge.svg)](https://github.com/epsilonrt/libmodbus/actions/workflows/build.yml)

This is a fork of the original [libmodbus](https://github.com/stephane/libmodbus) library with cmake build system for all supported platforms. Some features have been added to the library, such as:  
- Support for Modbus routing to integrate multiple devices on the same medium.  
- ASCII mode for serial communication.

This feature was optional and may be enabled by setting the `MODBUS_ASCII_MODE` option to `ON`  or `MODBUS_ROUTING_ENABLED` to `ON` when configuring the library with cmake.

Thus, the library is now able to be used as underlying communication layer for the [epsilonrt/libmodbuspp](https://github.com/epsilonrt/libmodbuspp) library.

## Overview

libmodbus is a free software library to send/receive data with a device which
respects the Modbus protocol. This library can use a serial port or an Ethernet
connection.

The functions included in the library have been derived from the Modicon Modbus
Protocol Reference Guide which can be obtained from [www.modbus.org](http://www.modbus.org).

The license of libmodbus is *LGPL v2.1 or later*.

The official website is [www.libmodbus.org](http://www.libmodbus.org). The
website contains the latest version of the documentation.

The library is written in C and designed to run on Linux, Mac OS X, FreeBSD, Embox,
QNX and Windows.

You can use the library on MCUs with Embox RTOS.

## Installation

You will only need to install automake, autoconf, libtool and a C compiler (gcc
or clang) to compile the library and asciidoc and xmlto to generate the
documentation (optional).

To install, just run the usual dance, `./configure && make install`. Run
`./autogen.sh` first to generate the `configure` script if required.

You can change installation directory with prefix option, eg. `./configure
--prefix=/usr/local/`. You have to check that the installation library path is
properly set up on your system (*/etc/ld.so.conf.d*) and library cache is up to
date (run `ldconfig` as root if required).

The library provides a *libmodbus.pc* file to use with `pkg-config` to ease your
program compilation and linking.

If you want to compile with Microsoft Visual Studio, you should follow the
instructions in `./src/win32/README.md`.

To compile under Windows, install [MinGW](http://www.mingw.org/) and MSYS then
select the common packages (gcc, automake, libtool, etc). The directory
*./src/win32/* contains a Visual C project.

To compile under OS X with [homebrew](http://mxcl.github.com/homebrew/), you
will need to install the following dependencies first: `brew install autoconf
automake libtool`.

To build under Embox, you have to use its build system.

## Testing

Some tests are provided in *tests* directory, you can freely edit the source
code to fit your needs (it's Free Software :).

See *tests/README* for a description of each program.

For a quick test of libmodbus, you can run the following programs in two shells:

1. ./unit-test-server
2. ./unit-test-client

By default, all TCP unit tests will be executed (see --help for options).

It's also possible to run the unit tests with `make check`.

## To report a bug or to contribute

See [CONTRIBUTING](CONTRIBUTING.md) document.

## Documentation

You can serve the local documentation with:

```shell
pip install mkdocs-material
mkdocs serve
```
