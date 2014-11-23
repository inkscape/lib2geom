@echo Setting environment variables for MinGW build of lib2geom
IF "%DEVLIBS_PATH%"=="" set DEVLIBS_PATH=c:\devlibs
IF "%MINGW_PATH%"=="" set MINGW_PATH=C:\mingw


set RAGEL_BIN=c:\ragel
set GS_BIN=C:\latex\gs\gs9.15\bin
set PYTHON_PATH=C:\Python27
set GRAPHVIZ_BIN="C:\Program Files (x86)\Graphviz2.38\bin"
set GS_BIN=C:\latex\gs\gs8.61\bin

set MINGW_BIN=%MINGW_PATH%\bin
set PKG_CONFIG_PATH=%DEVLIBS_PATH%\lib\pkgconfig
set CMAKE_PREFIX_PATH=%DEVLIBS_PATH%
set GTKMM_BASEPATH=%DEVLIBS_PATH%
set PKG_CONFIG_PATH=%DEVLIBS_PATH%\lib\pkgconfig
set PATH=%MINGW_BIN%;%RAGEL_BIN%;%PATH%;%GS_BIN%;%GRAPHVIZ_BIN%;%PYTHON_PATH%;%DEVLIBS_PATH%\bin
set BOOST_DIR=%DEVLIBS_PATH%\include
