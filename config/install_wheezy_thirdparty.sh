# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

PREFIX="/opt/IceE-3.6.0-ThirdParty"

#
# Uninstall a package
#
installPackage ()
{
    PACKAGE=$1
    URL=$2
    
    if [ ! -f $PREFIX/deb/$PACKAGE ]; then
        if ! wget -O $PREFIX/deb/$PACKAGE $URL; then
            echo "Error downloading $PACKAGE"
            exit 1
        fi
    fi
    
    if ! dpkg -x $PREFIX/deb/$PACKAGE $PREFIX/; then
        echo "Error extracting $PACKAGE"
        exit 1
    fi
}

#
# Ask the user for comfirmation.
#

ok=0
confirmed="no"
answer=""

while [[ $ok -eq 0 ]]
do
    echo "Install all Ice-E 3.6.0 Wheezy ARMHF third party packages"
    echo "into $PREFIX? Yes/No"
    read -p "$*" answer
    if [[ ! "$answer" ]]; then
        answer="no"
    else
        answer=$(tr '[:upper:]' '[:lower:]' <<<$answer)
    fi

    if [[ "$answer" == 'y' || "$answer" == 'yes' || "$answer" == 'n' || "$answer" == 'no' ]]; then
        ok=1
    fi

    if [[ $ok -eq 0 ]]; then
        echo "Valid answers are: 'yes', 'y', 'no', 'n'"
    fi
done


if [[ "$answer" == 'y' || "$answer" == 'yes' ]]; then
    confirmed="yes"
else
    confirmed="no"
fi

if [[ "$confirmed" == "no" ]]; then
    echo "Installation cancelled"
    exit 0
fi

if ! mkdir -p $PREFIX/deb; then
    echo "Error creating Installation directory $PREFIX/deb"
    exit 0
fi

installPackage "libssl1.0.0_1.0.1e-2+deb7u17_armhf.deb" \
    "http://security.debian.org/debian-security/pool/updates/main/o/openssl/libssl1.0.0_1.0.1e-2+deb7u17_armhf.deb"

installPackage "libssl-dev_1.0.1e-2+deb7u17_armhf.deb" \
    "http://security.debian.org/debian-security/pool/updates/main/o/openssl/libssl-dev_1.0.1e-2+deb7u17_armhf.deb"

installPackage "libbz2-1.0_1.0.6-4_armhf.deb" \
    "http://ftp.us.debian.org/debian/pool/main/b/bzip2/libbz2-1.0_1.0.6-4_armhf.deb"

installPackage "libbz2-1.0_1.0.6-4_armhf.deb" \
    "http://ftp.us.debian.org/debian/pool/main/b/bzip2/libbz2-1.0_1.0.6-4_armhf.deb"

installPackage "libbz2-dev_1.0.6-4_armhf.deb" \
    "http://ftp.us.debian.org/debian/pool/main/b/bzip2/libbz2-dev_1.0.6-4_armhf.deb"

installPackage "zlib1g_1.2.7.dfsg-13_armhf.deb" \
    "http://ftp.us.debian.org/debian/pool/main/z/zlib/zlib1g_1.2.7.dfsg-13_armhf.deb"


#
# Fix libbz2 symlink
#
rm $PREFIX/usr/lib/arm-linux-gnueabihf/libbz2.so
ln -s $PREFIX/lib/arm-linux-gnueabihf/libbz2.so.1.0 $PREFIX/usr/lib/arm-linux-gnueabihf/libbz2.so