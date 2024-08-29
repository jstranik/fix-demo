#include "utils.hpp"
#include "persistent-data.hpp"
#include <cstdlib>
#include <string>
#include <fmt/format.h>
#include <chrono>
using namespace std::literals;

int main(int argc, char **argv) {
  return app{}.run([&] {
    persistent::persistent_data<
        persistent::map_t<const persistent::persistent_string, double>>
        pm("test.map", 1024);

    if (argv[1] && argv[1] == "store"s) {
      auto now = std::chrono::system_clock::now();
      auto now_ms =
          std::chrono::time_point_cast<std::chrono::milliseconds>(now);
      auto value = now_ms.time_since_epoch().count();
      pm->emplace(persistent::persistent_string(
                      fmt::format("test_{}", value).c_str(), pm.get_alloc()),
                  15);
    } else {
      for (const auto &kv : pm.root()) {
        fmt::print("{} - {}\n", kv.first.c_str(), kv.second);
      }
    }
    return EXIT_SUCCESS;
  });
}
