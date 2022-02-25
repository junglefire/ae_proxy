@echo off

set ROOT_DIR=%cd%

@echo off

if "%1" == "" (
	if not exist %ROOT_DIR%\build ( 
		echo create `build` dir...
		mkdir build
	)
	cd build
	echo generate makefile...
	cmake ..
	cd ..
	echo build the project...
	cmake --build build
	exit -1
) else if "%1" == "clean" (
	rd/s/q build
) else (
	echo unsupported command `"%1"`!
)
