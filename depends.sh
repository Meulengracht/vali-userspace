#!/bin/bash
SCRIPT=`realpath $0`
SCRIPTPATH=`dirname $SCRIPT`

apt-get update
apt-get -y -qq install bison flex python3 python3-pip python3-mako
pip3 install prettytable Mako pyaml dateutils --upgrade
pip3 install --upgrade git+https://github.com/meulengracht/glad.git#egg=glad
