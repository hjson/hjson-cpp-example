#include "config.h"
#include "main_widget.h"
#include "hjson.h"
#include <QtWidgets>
#include <iostream>


// These default values are used for each config key that was not found in the
// config file read from disk. If no config file was found, this string is
// written to a config.hjson file on application exit.
static const char *_szDefaultConfig = R"(// This file is overwritten on exit by the app HjsonExample.

// If true, enables the slider for alpha in the UI.
enableAlpha: false
// These numbers can be modified in the UI.
alpha: 2030
beta: 64
gamma: 7
// This string will be shown in the UI.
exampleString: This string can be changed in config.hjson
mainWindowWidth: 500
mainWindowHeight: 389
)";


int main(int argc, char* argv[]) {
  int ret = 0;
  QApplication a(argc, argv);
  // Either read a config file specified as an input argument or try to read
  // a file "config.hjson" in the application directory.
  std::string configPath = argc > 1 ? argv[1] :
    QCoreApplication::applicationDirPath().toStdString() + "/config.hjson";
  // The root node for our config tree (will have type Undefined here).
  Hjson::Value config;
  try {
    Hjson::DecoderOptions opt;
    // Throws a helpful error if the user has accidentally given a value for
    // the same config key twice or more.
    opt.duplicateKeyException = true;
    // Makes sure that empty lines and custom indentation are kept when the
    // config file is rewritten.
    opt.whitespaceAsComments  = true;
    config = Hjson::UnmarshalFromFile(configPath, opt);
  } catch(const Hjson::syntax_error& e) {
    std::fprintf(stderr, "Error in config: %s\n\n\n", e.what());
    std::fprintf(stdout, "Default config:\n\n");
    std::fprintf(stdout, "%s", _szDefaultConfig);
    return 1;
  } catch(const Hjson::file_error&) {
  }

  // We have a base of default config values in the string _szDefaultConfig,
  // but all of them (and relevant comments) can be replaced here by what has
  // been read from the config file.
  config = Hjson::Merge(Hjson::Unmarshal(_szDefaultConfig), config);

  try {
    // Add working directory to possible locations for the "platforms" directory.
    QCoreApplication::addLibraryPath(".");
    MainWidget w(config);

    // Throws Hjson::type_mismatch if the config values are not of type Int64
    // or Double. The mismatch can instead be silently ignored by calling
    // to_int64(), for example config[Cfg::mainWindowWidth].to_int64(). Then
    // the value 0 is used if no conversion could be performed, and no error is
    // thrown.
    w.resize(config[Cfg::mainWindowWidth], config[Cfg::mainWindowHeight]);
    w.show();

    // The exec() function call does not return until the UI has been closed by the user.
    ret = a.exec();

    config[Cfg::mainWindowWidth] = w.width();
    config[Cfg::mainWindowHeight] = w.height();
  } catch(const Hjson::type_mismatch& e) {
    std::fprintf(stderr, "Type mismatch in config file: %s\n", e.what());
    return 1;
  }

  try {
    Hjson::EncoderOptions opt;
    // Just because it looks nice.
    opt.omitRootBraces = true;
    // Rewrites the same config file that was previously read as input. Or
    // creates a new config file if none existed.
    Hjson::MarshalToFile(config, configPath, opt);
  } catch(const std::exception& e) {
    std::fprintf(stderr, "Failed to write config file '%s': %s\n", configPath.c_str(), e.what());
    return 1;
  }

  return ret;
}
