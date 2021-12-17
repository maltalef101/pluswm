#Libraries
add_library(WM
	wm/LibWM.cpp
	wm/LibWM.h)

add_library(Util
	util/LibUtil.cpp
	util/LibUtil.h
	)

add_library(Keybind
	keybind/LibKeybind.cpp
	keybind/LibKeybind.h
	)

add_library(Client
	client/LibClient.cpp
	client/LibClient.h
	)

add_library(Button
	button/LibButton.cpp
	button/LibButton.h
	)

target_link_libraries(WM Client Keybind Button)
target_link_libraries(Client WM Util)
target_link_libraries(Keybind WM)
target_link_libraries(Button X11)

target_include_directories(WM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/wm")
target_include_directories(Util PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/util")
target_include_directories(Keybind PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/keybind")
target_include_directories(Client PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/client")
target_include_directories(Button PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/button")
