# Building Ice-E

This page describes the Ice-E source distribution, including information
about compiler requirements, and instructions for building and testing.

## Build Requirements

### Operating Systems and Compilers

Ice-E is supported on Linux, and was extensively tested using the operating
system and compiler versions listed for our [supported platforms][1].

The Ice-E build system supports both on device and cross compiling. If you
wish to cross compile you need to setup a cross development environment.

For instructions on how to setup a cross development environment, refer to
the [setting up your cross development environment][2] section in the
Ice-E usage page for your target platform.

### Third-Party Libraries

Ice-E depends on the `python`, `bzip2` and `openssl` development libraries.

#### Debian

We provide a script that will download and setup the required libraries from
the Debian 7.8 (Wheezy) repositories:

    curl -fsSL https://github.com/zeroc-ice/icee/raw/v3.6.1/config/install_wheezy_thirdparty.sh | sudo bash

#### Yocto

Ensure the `python`, `bzip2` and `ssl` development packages are installed on your image.
If you are cross compiling they must be included in your SDK.

## Building Ice-E

Edit [`Makefile`](./Makefile) to establish your build configuration. The
comments in the file provide more information. On Debian, pay particular
attention to the variable that defines the location of the third-party
libraries.

If you are cross compiling make sure you have configured your cross
development environment.

In a command window, run `make` to build Ice-E. This will build:

- the Ice for C++ static and dynamic libraries,
- Ice for Python,
- the `slice2cpp` and `slice2py` Slice compilers,
- `glacier2router`, `icebox`, and `iceboxadmin` executables, and
- the C++ and Python test suites.

The C++ static and dynamic libraries are built in a subfolder of `cpp/lib`. The
subfolder's name is dependent on your target platform.

## Installing Ice-E

Run `make install` to install Ice-E in the directory specified by the `prefix`
variable in `Makefile`.

## Test Suite

You can run `allTests.py` from the root directory by running:

    python allTests.py

If you are cross-compiling, you will first need to deploy
the test suite to the target device:

    $ make test_deploy

This command will deploy the Ice-E test suite to the device specified by the
`DEPLOY_TARGET` variable in `Makefile`.

Note that the ssh daemon must be running in the device.

After successfully deploying the test suite, open an ssh session to the
target, and change to the deployment directory.

    ssh user@192.168.7.2
    cd icee


### Yocto Notes

To run all the string converter test it is necessary that your image
contain the `glibc-utils` and `glibc-gconvs` packages.

To run the glacier2 tests you need to manually install `pip` and
subsequently `passlib`. The `zeroc-image-testing` image in the [ZeroC meta layer][3]
contains the necessary build dependencies. You can install pip by following [these instructions][4],
then install passlib by running:

    pip install passlib

[1]: https://doc.zeroc.com/display/Ice36/Supported+Platforms+for+Ice-E+3.6.1
[2]: https://doc.zeroc.com/display/Ice36/Ice-E+Release+Notes
[3]: https://github.com/zeroc-ice/meta-zeroc
[4]: https://pip.pypa.io/en/latest/installing.html
