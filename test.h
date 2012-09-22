#ifndef MAIN_H
#define MAIN_H
#include <cstdio>
#include <string>

#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/var.h"
#include "jpeg_mem_src.h"
#include "opencv2/imgproc/imgproc.hpp"

/// The Instance class.  One of these exists for each instance of your NaCl
/// module on the web page.  The browser will ask the Module object to create
/// a new Instance for each occurence of the <embed> tag that has these
/// attributes:
///     type="application/x-nacl"
///     src="<PROJECT_NAME>.nmf"
/// To communicate with the browser, you must override HandleMessage() for
/// receiving messages from the borwser, and use PostMessage() to send messages
/// back to the browser.  Note that this interface is entirely asynchronous.
class GetURLHandler;
class testInstance : public pp::Instance {
 public:
  /// The constructor creates the plugin-side instance.
  /// @param[in] instance the handle to the browser-side plugin instance.
  explicit testInstance(PP_Instance instance) : pp::Instance(instance) {}
  virtual ~testInstance() {}

  /// Handler for messages coming in from the browser via postMessage().  The
  /// @a var_message can contain anything: a JSON string; a string that encodes
  /// method names and arguments; etc.  For example, you could use
  /// JSON.stringify in the browser to create a message that contains a method
  /// name and some parameters, something like this:
  ///   var json_message = JSON.stringify({ "myMethod" : "3.14159" });
  ///   nacl_module.postMessage(json_message);
  /// On receipt of this message in @a var_message, you could parse the JSON to
  /// retrieve the method name, match it to a function call, and then call it
  /// with the parameter.
  /// @param[in] var_message The message posted by the browser.
  virtual void HandleMessage(const pp::Var&);
  void HandleImage(const std::string&);
  void HandleXml(const std::string&);
  void CreateMemFile(const std::string&, const std::string&);
  void RecognizeFace();
  IplImage* frame;
 private:
  int width, height;
  std::string url;
  GetURLHandler* handler;
  bool classifierCreated;
};
#endif
