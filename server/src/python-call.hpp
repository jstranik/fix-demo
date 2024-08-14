#pragma once

#include <boost/python.hpp>
#include <boost/filesystem.hpp>
#include <optional>
#include <thread>
#include <iostream>


/** Acquire python GIL */
struct PyLock final {
  PyLock() : state_(PyGILState_Ensure()) {}
  ~PyLock() { PyGILState_Release(state_); }

private:
  PyGILState_STATE state_;
};

namespace py = boost::python;
namespace fs = boost::filesystem;
class PythonLogic {

public:
  PythonLogic(const std::string &script_path, const std::string logic_name = "Logic")
      : script_path_(script_path), logic_name_(logic_name),
        last_write_time_(fs::last_write_time(script_path)),
        running_(true) {
    reload();
    thread_ = std::thread([this] { watch_file(); });
  }
  ~PythonLogic() {
    running_ = false;
    cv_.notify_one();
    thread_.join();
  }
  std::optional<bool> should_trade(const std::string &symbol, double price) {
      return callpy([&]()->bool{
          return py::extract<bool>(py_instance_.attr("should_trade")(symbol, price));
      });
  }

protected:
  template <class F> std::optional<decltype(std::declval<F>()())> callpy(F&& f) {
      PyLock pylock;
      try {
        return std::optional(f());
      } catch (py::error_already_set const &) {
        PyErr_Print();
        return false;
      }
        }

private:
  const std::string script_path_, logic_name_;
  std::time_t last_write_time_;
  py::object py_class_;
  py::object py_instance_;
  std::thread thread_;
  std::condition_variable cv_;
  std::mutex mutex_;
  bool running_;

  void reload() {
    PyLock pylock;
    try {
      py::object main_module = py::import("__main__");
      py::object main_namespace = main_module.attr("__dict__");

      py::exec_file(script_path_.c_str(), main_namespace);

      py_class_ = main_namespace[logic_name_];
      py_instance_ = py_class_();
    } catch (py::error_already_set const &) {
      PyErr_Print();
    }
  }

  void watch_file() {
    std::mutex mym;
    std::unique_lock lk(mym);
    while (running_) {
      cv_.wait_for(lk, std::chrono::seconds(1), [this] { return !running_; });

      if (!running_)
        break;

      std::time_t current_write_time = fs::last_write_time(script_path_);
      if (current_write_time != last_write_time_) {
        std::cout << "Script changed. Requesting reload..." << std::endl;
        reload();
        last_write_time_ = current_write_time;
      }
    }
  }
};
