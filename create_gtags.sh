#!/bin/sh
cd `dirname $0`
#cd ${0%/*} #`dirname $0`
gtags && htags
