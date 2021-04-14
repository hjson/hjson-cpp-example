cmake_minimum_required(VERSION 3.10)

project(HjsonExample)

# Needed by std::thread on Linux
find_package(Threads REQUIRED)

find_package(hjson REQUIRED
  PATHS "../hjson-cpp/build"
)

# Alternative to find_package, will build Hjson from source code.
#add_subdirectory(../hjson-cpp ${CMAKE_BINARY_DIR}/hjson)

find_package(Qt5 REQUIRED
  COMPONENTS Widgets
  PATHS (
    "../qt/lib/cmake/Qt5"
    "/usr/local/opt/qt@5/lib/cmake/Qt5"
  )
)

# Tell CMake to run moc (a Qt tool) when necessary.
set(CMAKE_AUTOMOC ON)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there.
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(HjsonExample
  src/main.cpp
  src/main_widget.cpp
  src/main_widget.h
  src/config.h
)

target_compile_features(HjsonExample PUBLIC cxx_std_11)

target_link_libraries(HjsonExample
  Qt5::Widgets
  hjson
  Threads::Threads
)

if(WIN32)
  # After every build, copy Qt5 DLLs if needed.
  add_custom_command(
    TARGET HjsonExample POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_FILE:Qt5::Core>
      $<TARGET_FILE:Qt5::Gui>
      $<TARGET_FILE:Qt5::Widgets>
      $<TARGET_FILE_DIR:HjsonExample>
    COMMAND ${CMAKE_COMMAND} -E make_directory
      "$<TARGET_FILE_DIR:HjsonExample>/platforms/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_FILE:Qt5::QWindowsIntegrationPlugin>
      "$<TARGET_FILE_DIR:HjsonExample>/platforms/"
  )
endif()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  PROPERTY VS_STARTUP_PROJECT HjsonExample
)
