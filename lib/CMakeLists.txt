#Libraries
add_library(WM
	wm/LibWM.cpp
	wm/LibWM.h)

add_library(Util
	util/LibUtil.cpp
	util/LibUtil.h
	)

target_link_libraries(WM X11 Util)

target_include_directories(WM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/wm")
target_include_directories(Util PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/util")
