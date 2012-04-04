#!/bin/sh

# This script checks for QCC location and the include/lib files that 
# the build expects to be available  ...

# Modify the paths below to point to the SDL and TouchControlOVerlay libraries.
# These are availale as git repo's here github.com/blackberry/SDL github.com/blackberry/TouchControlOverlay 

PATH_TO_SDL_CODE=$HOME/git/SDL
PATH_TO_TCO_CODE=$HOME/git/TouchControlOverlay

SDL_INC=$PATH_TO_SDL_CODE/include/SDL.h
SDL_LIB=$PATH_TO_SDL_CODE/Device-Release/libSDL12.so
TCO_INC=$PATH_TO_TCO_CODE/inc/png.h
TCO_LIB=$PATH_TO_TCO_CODE/Device-Release/libTouchControlOverlay.so
QCC_BIN=${QNX_HOST}/usr/bin/qcc


VALIDITY=0

chkobject() 
{
 theObject=$1
 if [ -e "$theObject" ]; then
    echo "ok -> $theObject"
    VALIDITY=`expr $VALIDITY + 1`
 else
   echo "error -> $theObject"
 fi
}

chkobject $QCC_BIN
chkobject $SDL_INC
chkobject $SDL_LIB
chkobject $TCO_INC
chkobject $TCO_LIB

if [ $VALIDITY -lt 5 ]; then
  echo "dependancies missing ..."
  exit
else
  export PATH_TO_SDL_CODE=$HOME/git/SDL
  export PATH_TO_TCO_CODE=$HOME/git/TouchControlOverlay
fi


# 
export PATH=${QNX_HOST}/usr/bin:${PATH}

echo "Ok, type the following to build ..."
make PATH_TO_SDL_CODE=$HOME/git/SDL PATH_TO_TCO_CODE=$HOME/git/TouchControlOverlay 
