cmake_minimum_required(VERSION 3.5)

set(TARGET ncwidgets)
set(VERSION 1.0)

project(
	${TARGET}
	VERSION ${VERSION}
	DESCRIPTION "widgets for curses"
	HOMEPAGE_URL ""
	LANGUAGES C
)

find_package(Curses REQUIRED)
find_library(PANEL_LIBRARY panel)
include_directories(${CURSES_INCLUDE_DIRS})

# SOURCES
file(GLOB TARGET_SOURCES "src/*.c")
add_library(${TARGET} STATIC 
	${TARGET_SOURCES}
)
