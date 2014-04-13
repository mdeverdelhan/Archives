#!/bin/bash
export CROSSCOMPILE=/opt/Embedix/tools
export QPEDIR=/opt/Qtopia/sharp
export QTDIR=/opt/Qtopia/sharp
export PATH=$QTDIR/bin:$QPEDIR/bin:$CROSSCOMPILE/bin:$PATH
export TMAKEPATH=/opt/Qtopia/tmake/lib/qws/linux-sharp-g++/
export LD_LIBRARY_PATH=$QTDIR/lib:$LD_LIBRARY_PATH
export PS1="\u:\w:arm > "
export QPEDIR QTDIR PATH LD_LIBRARY_PATH TMAKEPATH PS1
#echo "Altered environment for Sharp Zaurus Development ARM"
test -n "$1" && exec $*
