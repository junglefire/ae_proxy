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
) else (
	echo unsupported package!
)