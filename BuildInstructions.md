# Building Ice-E

This page describes the Ice-E source distribution, including information
about compiler requirements, and instructions for building and testing.

## Build Requirements

### Operating Systems and Compilers

Ice-E is supported on Linux, and was extensively tested using the operating
system and compiler versions listed for our [supported platforms][1].

For instructions on how to setup the cross development environment required
to build Ice-E, refer to [Setting up your cross development environment][2] in
the Ice-E release notes.

### Third-Party Libraries

Ice-E depends on the bzip2 and openssl libraries. For your convenience we
provide an script that download and setup the require libraries from
Debian 7.8 (Wheezy) repositories:

    wget https://github.com/zeroc-ice/icee/raw/v3.6.1/config/install_wheezy_thirdparty.sh
    sudo bash

For other build configurations, use the bzip2 and openssl development packages
provided by the OS.

## Building Ice-E

Edit `Makefile` to establish your build configuration. The comments in the
file provide more information. On Debian, pay particular attention to the
variable that defines the location of the third-party libraries.

In a command window, run `make` to build Ice-E. This will build:

- the Ice for C++ static and dynamic libraries,
- Ice for Python,
- the `slice2cpp` and `slice2py` Slice compilers,
- `glacier2router`, `icebox`, and `iceboxadmin` executables, and
- the C++ and Python test suites.

The C++ static and dynamic libraries are built in a subfolder of `cpp/lib`. Its
name is dependent on your target platform.

## Installing Ice-E

Run `make install` to install Ice-E in the directory specified by the `prefix`
variable in `Makefile`.

## Running the Test Suite

If you are cross-compiling, first you need to deploy
the test suite to the target device:

    $ make test_deploy

This command will deploy the Ice-E test suite to the device specified by the
`DEPLOY_TARGET` variable in `Makefile`.

Note that the ssh daemon must be running in the device.

After successfully deploying the test suite, open an ssh session to the
target, change to the deployment directory, and use the `allTests.py` script
to run the test suite:

    ssh user@192.168.7.2
    cd icee
    python allTests.py

If you are building Ice-E on the build device instead of cross-compiling,
you can run `allTests.py` from the root directory:

    python allTests.py

### Yocto Notes

To run all of test suite it is necessary that your Yocto image contain
the `glibc-utils` and `glibc-gconvs` packages. In addition, you will need to
manually install `pip` and subsequently `passlib`.

The `zeroc-image-testing` image in the [ZeroC meta layer][3] contains the
necessary build dependencies. You can install pip by following [these instructions][4].

[1]: https://doc.zeroc.com/display/Ice36/Supported+Platforms+for+Ice-E+3.6.1
[2]: https://doc.zeroc.com/display/Ice36/Ice-E+Release+Notes
[3]: https://github.com/zeroc-ice/layer-zeroc
[4]: https://pip.pypa.io/en/latest/installing.html
