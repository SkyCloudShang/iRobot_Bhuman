@echo off
set SHELLOPTS=igncr
pushd "%~dp0"
bash ../Linux/createModule
popd 
