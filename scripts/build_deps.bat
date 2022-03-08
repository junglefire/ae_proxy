@echo off

echo build 3rd library ...

set ROOT_DIR=%cd%
set INSTALL_DIR=%ROOT_DIR%\pkg
set PKG_DIR=%ROOT_DIR%\pkg\packages

@echo off
if "%1" == "" (
	echo [usage]build_deps.sh package_name
	echo ::list of packages:
	echo   - libuv
	echo   - zlog
	echo   - spdlog
	echo   - bdb
	echo   - pcap
	echo   - msgpack
	echo   - json
	echo   - cxxopts
	echo   - sqlite3
	exit -1
)

if not exist %INSTALL_DIR% (
	echo create dir %INSTALL_DIR% ...
	mkdir %INSTALL_DIR%
)

if not exist %PKG_DIR% (
	echo create dir %PKG_DIR% ...
	mkdir %PKG_DIR%
)

set UV_VERSION=1.42.0
set MSGPACK_VERSION=4.0.0
set ZLOG_VERSION=1.2.15
set CXXOPTS_VERSION=3.0.0
set SQLITE3_VERSION=3.37.0

if "%1" == "libuv" (
	:: build libuv
	echo ">>>>>--------> build libuv ..."
	if not exist %PKG_DIR%\libuv-v%UV_VERSION%.tar.gz ( 
		echo download %PKG_DIR%\libuv-v%UV_VERSION%.tar.gz...
		wget https://dist.libuv.org/dist/v%UV_VERSION%/libuv-v%UV_VERSION%.tar.gz --output-document=%PKG_DIR%/libuv-v%UV_VERSION%.tar.gz
	)
	cd %PKG_DIR%
	:: uncompress package
	tar -zxvf libuv-v%UV_VERSION%.tar.gz
	cd libuv-v%UV_VERSION%
	:: make build dir
	if not exist build (
		mkdir -p build
	)
	:: generate *.sln file
	cd build
	cmake .. -DBUILD_TESTING=OFF -DCMAKE_INSTALL_PREFIX=%INSTALL_DIR%
	:: compile
	cd ..
	cmake --build build --target INSTALL
) else if "%1" == "msgpack" (
	:: build msgpack
	echo ">>>>>--------> build msgpack ..."
	if not exist %PKG_DIR%\msgpack-c-%MSGPACK_VERSION%.tar.gz (
		wget https://github.com/msgpack/msgpack-c/releases/download/c-%MSGPACK_VERSION%/msgpack-c-%MSGPACK_VERSION%.tar.gz --output-document=%PKG_DIR%/msgpack-c-%MSGPACK_VERSION%.tar.gz
	)
	cd %PKG_DIR%
	tar -zxvf msgpack-c-%MSGPACK_VERSION%.tar.gz
	cd msgpack-c-%MSGPACK_VERSION%
	:: make build dir
	if not exist build (
		mkdir -p build
	)
	cd build
	cmake .. -DCMAKE_INSTALL_PREFIX=%INSTALL_DIR%
	cd ..
	cmake --build build --target INSTALL
) else if "%1" == "cxxopts" (
	:: build cxxopts
	echo ">>>>>--------> build cxxopts ..."
	if not exist %PKG_DIR%\cxxopts-%CXXOPTS_VERSION%.tar.gz (
		wget https://github.com/jarro2783/cxxopts/archive/refs/tags/v%CXXOPTS_VERSION%.tar.gz --output-document=%PKG_DIR%/cxxopts-%CXXOPTS_VERSION%.tar.gz
	)
	cd %PKG_DIR%
	tar -zxvf cxxopts-%CXXOPTS_VERSION%.tar.gz
	:: make
	cd cxxopts-%CXXOPTS_VERSION%
	echo copy header file...
	copy include\cxxopts.hpp %INSTALL_DIR%\include
) else if "%1" == "sqlite_old" (
	:: build sqlite3
	echo ">>>>>--------> build sqlite3 ..."
	if not exist %PKG_DIR%\sqlite-autoconf-%SQLITE3_VERSION%.tar.gz (
		wget https://sqlite.org/2022/sqlite-autoconf-%SQLITE3_VERSION%.tar.gz --output-document=%PKG_DIR%\sqlite-autoconf-%SQLITE3_VERSION%.tar.gz
	)
	cd %PKG_DIR%
	:: tar -zxvf sqlite-autoconf-%SQLITE3_VERSION%.tar.gz
	:: make
	cd sqlite-autoconf-%SQLITE3_VERSION%
	echo nmake...
	nmake -f Makefile.msc
	:: copy header files and lib
	echo copy header file...
	copy sqlite3.h %INSTALL_DIR%\include
	copy sqlite3ext.h %INSTALL_DIR%\include
	echo copy library...
	copy sqlite3.lib %INSTALL_DIR%\lib
) else if "%1" == "sqlite3" (
	:: build sqlite3
	echo ">>>>>--------> build sqlite3 ..."
	cd %PKG_DIR%
	if not exist %PKG_DIR%\sqlite3-cmake-%SQLITE3_VERSION% (
		git clone https://github.com/alex85k/sqlite3-cmake.git sqlite3-cmake-%SQLITE3_VERSION%
	)
	cd %PKG_DIR%\sqlite3-cmake-%SQLITE3_VERSION%
	cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=%INSTALL_DIR%
	nmake install
) else (
	echo unsupported package!
)