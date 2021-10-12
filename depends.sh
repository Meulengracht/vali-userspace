#!/bin/bash
SCRIPT=`realpath $0`
SCRIPTPATH=`dirname $SCRIPT`

apt-get update
apt-get -y -qq install python3 python3-pip
pip3 install prettytable Mako pyaml dateutils --upgrade
