# Try to find the CEGUI library.
# The following variables will be defined:
#
# CEGUI_FOUND - Do we have CEGUI?
# CEGUI_INCLUDE_DIR - Headers for CEGUI
# CEGUI_LIBRARIES - Libraries for CEGUI
# CEGUI_DEFINITIONS - C/CXX flags
#
# Copyright © 2014 The SMC Team

########################################
# pkg-config

# First we need pkg-config.
find_package(PkgConfig)

# Check if we can find it with pkg-config.
# (No, I don’t know why they decided to rename
# themselves to CEGUI-0, really!)
pkg_search_module(PKG_CEGUI CEGUI CEGUI-0)

########################################
# Include dir

# CEGUI’s pkg-config file is broken -- it says /usr/include/cegui-0/cegui,
# but the real include dir is /usr/include/cegui-0/CEGUI. I hate Windows
# devs who don’t care about casefolding. So we just give CMake a hint
# here to look where CEGUI.h is on my personal system.
find_path( CEGUI_INCLUDE_DIR CEGUI.h
           HINTS ${PKG_CEGUI_INCLUDE_DIRS} PATH_SUFFIXES cegui-0/CEGUI )

########################################
# The libraries

# We need to find a lot of libraries, so this macro
# does the main work here.
macro(find_cegui_library LIBNAME)
  message("-- Searching for ${LIBNAME} CEGUI library")

  # CEGUI 0.8 scattered its libraries over /usr/lib and
  # /usr/lib/cegui-0.8, AND it sometimes has a "-0" appended
  # to the library name.
  find_library( CEGUI_${LIBNAME}_LIBRARY
                NAMES CEGUI${LIBNAME} CEGUI${LIBNAME}-0
                HINTS ${PKG_CEGUI_LIBRARY_DIRS}
                PATH_SUFFIXES cegui-0.8 )

  # Error message if not found
  if(CEGUI_${LIBNAME}_LIBRARY)
    message("--   found: ${CEGUI_${LIBNAME}_LIBRARY}")
  else()
    message(SEND_ERROR "CEGUI${LIBNAME} library not found!")
  endif()
endmacro()

# CEGUI consists of a wealth of libraries.
find_cegui_library(Base)
#find_cegui_library(FalagardWRBase) # TODO: Removed in 0.8.0 ?
find_cegui_library(FreeImageImageCodec)
find_cegui_library(TinyXMLParser)

########################################
# The renderers

# User is required to selected a renderer.
# (CEGUI_FIND_COMPONENTS is set by package_handle_standard_args())
if( "${CEGUI_FIND_COMPONENTS}" STREQUAL "" )
  message(SEND_ERROR(" No renderer selected; try ...COMPENENTS OpenGL"))
endif()

set(CEGUI_RENDERER_LIBRARIES "")
foreach(COMPONENT ${CEGUI_FIND_COMPONENTS})
  find_cegui_library("${COMPONENT}Renderer")
  list(APPEND CEGUI_RENDERER_LIBRARIES ${CEGUI_${COMPONENT}Renderer_LIBRARY})
endforeach(COMPONENT)

# Now collect all the libraries in a single variable
set(CEGUI_LIBRARIES
  ${CEGUI_RENDERER_LIBRARIES}
  ${CEGUI_Base_LIBRARY}
  ${CEGUI_FalagardWRBase_LIBRARY}
  ${CEGUI_FreeImageImageCodec_LIBRARY}
  ${CEGUI_TinyXMLParser_LIBRARY}
 )

########################################
# Flags

# This sets -DCEGUI_STATIC! If you forget including
# that into your compilation commands, you will get
# wealths of "undefined references to _imp__*" errors
# on crosscompilation!
set(CEGUI_DEFINITIONS ${PKG_CEGUI_CFLAGS})

########################################
# Package boilerplate

# Register as a CMake module. This sets CEGUI_FIND_COMPONENTS
# to the list supplied by the calling user after COMPONENTS.
find_package_handle_standard_args(CEGUI
  DEFAULT_MSG
  CEGUI_INCLUDE_DIR
  CEGUI_LIBRARIES)

# These variables may be set by the configuring user if
# he knows what he is doing.
mark_as_advanced(CEGUI_INCLUDE_DIR
  CEGUI_BASE_LIBRARY
  CEGUI_FALAGARD_LIBRARY
  CEGUI_FREEIMAGE_CODEC_LIBRARY
  CEGUI_TINYXML_PARSER_LIBRARY
  CEGUI_RENDERER_LIBRARIES
  CEGUI_LIBRARIES)
