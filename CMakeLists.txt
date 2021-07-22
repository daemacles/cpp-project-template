cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set (PROJECT_NAME MyProject)
project (${PROJECT_NAME})

## USE_MY_LIBRARY in source.
#option (USE_MY_LIBRARY
#        "Use the provided library" ON)

## Support for Clang's CompilationDatabase system
set (CMAKE_EXPORT_COMPILE_COMMANDS 1)

add_subdirectory( third_party/fmt )
add_subdirectory( third_party/pystring )
add_subdirectory( third_party/backward-cpp )

## Compile and create a library.  STATIC is default unless BUILD_SHARED_LIBS
## is on.
#add_library (LibraryName src/sources.cc)

#if (USE_MY_LIBRARY)
  ## Search for include files here as well
  #include_directories ("{PROJECT_SOURCE_DIR}/some_sub_path")

  ## Run Cmake also in this dir
  #add_subdirectory (some_sub_path)

  #set (EXTRA_LIBS ${EXTRA_LIBS} LibraryName)
#endif (USE_MY_LIBRARY)

include_directories( include )
include_directories( third_party )
  
if( WIN32 )
                          ########################
                           ### WINDOWS SPECIFIC ###
                          ########################	
  add_definitions(
    /D_USE_MATH_DEFINES # Have <cmath> define, e.g. M_PI
    /DWIN32_LEAN_AND_MEAN
    /DNOMINMAX          # allow use of std::min/max
    /DNOGDI             # wingdi.h defines ERROR !!!
    /D_HAS_AUTO_PTR_ETC=1 # Boost compatibility
    /DTHIRD_PARTY_PATH=${CMAKE_CURRENT_SOURCE_DIR}
    )

  set( CXX_FLAGS "/W4 /MP /EHsc /openmp /bigobj" )

  # Remove "LINK : warning LNK4098: defaultlib 'LIBCMT' conflicts with use of other libs; use /NODEFAULTLIB:library"
  set( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:libcmt" )
  set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:libcmtd" )
  set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:libcmt" )
  set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:libcmtd" )
  
  set( PLATFORM_LIBS
  # Stack trace functionality
    Dbghelp
  )

else( WIN32 )
                        ############################
                        ### NON-WINDOWS SPECIFIC ###
                        ############################
						## Set optional features.  This will show up as a preprocessor variable
	option (USE_GLIBCXX_DEBUG 
	  "Use the _GLIBCXX_DEBUG macro for nicer debugging and error checking of the c++ standard library.  Warning, it breaks ABI compatibility so don't pass container instantiations between translation units that don't have the same debug mode."
	  OFF)
  
						# Compile flags
	SET (CMAKE_C_FLAGS                "-Wall -std=c11")
	SET (CMAKE_C_FLAGS_DEBUG          "${CMAKE_CFLAGS} -g")
	SET (CMAKE_C_FLAGS_MINSIZEREL     "${CMAKE_CFLAGS} -Os -DNDEBUG")
	SET (CMAKE_C_FLAGS_RELEASE        "${CMAKE_CFLAGS} -O3 -DNDEBUG")
	SET (CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_CFLAGS} -O2 -g")

	SET (CMAKE_CXX_FLAGS                "-Wall")
	SET (CMAKE_CXX_FLAGS_DEBUG          "${CMAKE_CXXFLAGS} -g")
	SET (CMAKE_CXX_FLAGS_MINSIZEREL     "${CMAKE_CXXFLAGS} -Os -DNDEBUG")
	SET (CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXXFLAGS} -O2 -g")
	SET (CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXXFLAGS} -O3 -DNDEBUG")
	
	SET (PLATFORM_LIBS
	  backward-cpp
	)

endif( WIN32 )
                         ##############################
                         ### END PLATFORM DEPENDENT ###
                         ##############################


set (LIBS 
  ${PLATFORM_LIBS}
  pystring
  )

add_executable (${PROJECT_NAME} 
  src/logging.cpp
  src/main.cpp
  src/stacktrace.cpp
  )
  
target_link_libraries (${PROJECT_NAME} 
  ${EXTRA_LIBS} 
  ${LIBS})
  



