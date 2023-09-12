
#!/bin/bash

REPO_URL="https://github.com/svpcom/rtl8812au.git"
DIR_NAME="rtl8812au"

PATCHES=("0001-rtw_debug_level_adjust.patch"
         "0002-rtw_debug_level_cmd_header.patch"
         "0003-rtw_debug_level_cmd_impl.patch"
         "0004-Makefile_debug_level_adjust.patch")

function init {
    git clone $REPO_URL $DIR_NAME
}

function deinit {
    rm -rf $DIR_NAME
}

function clear {
    cd $DIR_NAME
    make clear
}

function build {
    cd $DIR_NAME

    for patch in "${PATCHES[@]}"; do
        echo "Applying $patch..."
        git apply ../$patch
    done

    make
    echo "Script completed."
}

function install {
    cd $DIR_NAME
    sudo make install
}

function uninstall {
    cd $DIR_NAME
    sudo make uninstall
}

if [ "$1" == "-init" ]; then
    init
elif [ "$1" == "-deinit" ]; then
    deinit
elif [ "$1" == "-clear" ]; then
    clear
elif [ "$1" == "-build" ]; then
    build
elif [ "$1" == "-install" ]; then
    install
elif [ "$1" == "-uninstall" ]; then
    uninstall
else
    echo "Usage: $0 [-init|-deinit|-clear|-build|-install|-uninstall]"
fi

