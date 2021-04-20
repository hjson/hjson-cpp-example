# hjson-cpp-example

[![Build Status](https://github.com/hjson/hjson-cpp-example/workflows/test/badge.svg)](https://github.com/hjson/hjson-cpp-example/actions)

This example application showcases best practices for using [Hjson](https://hjson.github.io) configuration files in C++ GUI applications implemented with [Qt 5 Widgets](https://doc.qt.io/qt-5.15/qtwidgets-index.html) and [CMake](https://cmake.org/). The application is tested on Windows, Mac and Ubuntu.

The general idea is that the application code contains default values for all configuration keys, defaults that can be overruled by a config file read from disk at application startup. Some configuration values can be changed in the GUI and are then written to the config file on application exit. Comments, key order and indentation are preserved in the config file when it is rewritten. If no config file exists, a new one is created at application exit, containing default values and values changed in the GUI.

### Configuration tree

The configuration is made available in different parts of the application by passing around a reference to the root node of an *Hjson::Value* tree. In this example project the configuration tree is just a single map (i.e. the root node is of type *Hjson::Type::Map* and contains scalar values). When the user changes a setting in the GUI the configuration tree is updated with the new value. At application exit the (possibly) updated configuration tree is written to the path that the application (possibly) read the configuration from at startup.

### Key strings

In the [Go programming language](https://golang.org/) configuration files can easily be unmarshalled into structs with matching key names. That implementation pattern is unfortunately not possible in C++. The best we can do is to declare all key strings as *constexpr* in the file [config.h](src/config.h) which can then be imported by all files where configuration values are used. This way the compiler helps us avoid misspellings of key names and usage of configurations keys that have been removed.

### CMake and Qt 5

The [CMake file](CMakeLists.txt) for this example project does not automatically download Hjson or Qt 5. The headers and libs for Hjson and Qt 5 are expected to already exist on the local machine. The calls to *find_package* contain hints on where to find Hjson and Qt 5 (in sibling folders to the example project or, for Qt 5, at the default installation path when installed with [Homebrew](https://brew.sh/) on a Mac) in addition to the default paths where CMake will look for them. If all else fails you will need to declare values for `hjson_DIR` and `Qt5_DIR` for CMake. Example:

```bash
cmake \
  -Dhjson_DIR="C:/folderB/hjson-cpp/build64_VS15" \
  -DQt5_DIR="C:/folderA/qt/lib/cmake/Qt5" \
  -DCMAKE_GENERATOR_PLATFORM=x64 \
  ..
```

On Windows you will typically want to copy the necessary Qt 5 DLL files to the same folder as your EXE file, so that you can run your application. In order to do that automatically after every build (but only if needed), the [CMake file](CMakeLists.txt) contains this, which for some reason is not mentioned in the official Qt 5 documentation:

```cmake
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
```

### Handle values of wrong type

When dealing with user input, for example a config file read from disk, you must think through what the application should do if some input is not what you want it to be. There are two approaches to this: either abort the operation and display an error to the user, or silently ignore the error and use the most appropriate value you can think of given the user input.

In this example project the configuration values `mainWindowWidth` and `mainWindowHeight` must be numbers (either integer or floating point), otherwise the application exits with an error message (because of an *Hjson::type_mismatch* exception being thrown). The configuration values `alpha`, `beta` and `gamma` are also expected to be numbers, but thanks to the function *Hjson::Value::to_int64()* any type error is ignored for those values. Instead an attempt is made to parse the value into a number, otherwise 0 is used.

Hjson supports quoteless strings which can cause some confusion for the user. The user might expect all of these values to be parsed into *Hjson::Value* objects of type *Hjson::Type::String*, but the second value will be of type *Hjson::Type::Null* and the third value will be of type *Hjson::Type::Double*:

```
firstname: christopher
lastname: null
version: 2.0
```

If you want all of these values to be used as strings in the application, make sure to call the function *Hjson::Value::to_string()*, as in this example code:

```cpp
  mainLayout->addWidget(new QLabel(
    config[Cfg::exampleString].to_string().c_str()));
```

### Worker threads

A GUI application should never run heavy work on the GUI thread (the main thread). That would cause the GUI to become unresponsive (i.e. the application would freeze) until the heavy work is finished. This example project showcases how to perform heavy work in a separate thread and display a modal window with information for the user until the work has finished. Simply declare a *std::thread* and then immediately detach it, as in the lambda function used as argument in the call to *_AddButton()* near the end of the file [main_widget.cpp](src/main_widget.cpp).

No GUI operation can be performed from any other thread than the GUI thread, therefore the modal window is closed by a call to *QMetaObject::invokeMethod()* at the end of the thread with the *MainWidget*-object and the name of an appropriate [Qt slot](https://doc.qt.io/qt-5/signalsandslots.html) as arguments. The actual operation is then performed at a later time by the GUI thread calling the slot function.

### Automatic testing

Adding automatic testing to a Github project is really easy. Just create a file in your repository with the path [.github/workflows/test.yml](.github/workflows/test.yml) and in it declare (using [this syntax](https://docs.github.com/en/actions/reference/workflow-syntax-for-github-actions)) some action to perform. This example project has a single action called `test` which builds the app on Mac, Windows and two different Ubuntu versions. The action is performed on every push to Github and on every pull request created or updated.
