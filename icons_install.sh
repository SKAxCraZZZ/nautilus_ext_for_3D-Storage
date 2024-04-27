#!/bin/sh 

ICON_FOLDER="./src/icons"
echo 'Install emblems from '$ICON_FOLDER
xdg-icon-resource install --size 48 --context emblems $ICON_FOLDER/emblem-loadedstore.png emblem-loadedstore
xdg-icon-resource install --size 48 --context emblems $ICON_FOLDER/emblem-outdatedstore.png emblem-outdatedstore
xdg-icon-resource install --size 48 --context emblems $ICON_FOLDER/emblem-notsentstore.png emblem-notsentstore
xdg-icon-resource install --size 48 --context emblems $ICON_FOLDER/emblem-editedstore.png emblem-editedstore
xdg-icon-resource install --size 48 --context emblems $ICON_FOLDER/emblem-abortedstore.png emblem-abortedstore
xdg-icon-resource install --size 48 --context emblems $ICON_FOLDER/emblem-conflictstore.png emblem-conflictstore
gtk-update-icon-cache
echo 'Install finish'
