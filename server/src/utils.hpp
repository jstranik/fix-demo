#include <cstdlib>
#include <exception>
#include <iostream>
#define BOOST_STACKTRACE_GNU_SOURCE_NOT_REQUIRED
#include <boost/stacktrace.hpp>
#include <cxxabi.h>

class app {
    public:
    app() {
      std::set_terminate([] {
        std::exception_ptr eptr = std::current_exception();
        if(eptr) {
          std::string name;
          try {
            std::rethrow_exception(eptr);
          } catch (const std::exception &e) {
            name = typeid(e).name();
            int status;
            char *demangled_name =
                abi::__cxa_demangle(name.c_str(), nullptr, nullptr, &status);
            if (status == 0)
              name = demangled_name;
            std::cerr << name << ": " << e.what() << "\n";
          }
        }
        std::cerr << boost::stacktrace::stacktrace() << std::endl;
        std::exit(EXIT_FAILURE);
      });
    }

    template <class F> int run(const F &f, bool docatch = true) {
      if (docatch)
        try {
          return f();
        } catch (const std::exception &e) {
          print_exception(e);
          return EXIT_FAILURE;
        }
      else
        return f();
    }

  private:
    void print_exception(const std::exception &e, std::size_t depth = 0) {
      std::cerr << "exception: " << std::string(depth, ' ') << e.what() << '\n';
      try {
        std::rethrow_if_nested(e);
    } catch (const std::exception& nested) {
        print_exception(nested, depth + 1);
    }
    }
};
