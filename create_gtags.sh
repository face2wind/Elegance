#!/bin/sh
cd ${0%/*} #`dirname $0`
gtags && htags
