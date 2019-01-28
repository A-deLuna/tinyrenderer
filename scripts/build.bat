@echo off
pushd out
cl -O2 -Zi ../main.cpp user32.lib gdi32.lib /link -subsystem:windows,5.2 
popd
