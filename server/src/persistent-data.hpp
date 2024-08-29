#include <boost/interprocess/interprocess_fwd.hpp>
#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <map>
#include <iostream>

using symbol_t = std::string;
using price_t = double;


namespace persistent {
namespace bip = boost::interprocess;
using segment_t = bip::managed_mapped_file;
using manager_t = segment_t::segment_manager;
template <class T> using alloc_t = bip::allocator<T, manager_t>;

using persistent_string =
    bip::basic_string<char, std::char_traits<char>, alloc_t<char>>;

template <class K, class V>
using map_t = bip::map<K, V, std::less<K>, alloc_t<std::pair<K, V>>>;
template <class Map> class persistent_data {
public:

  using data_t = Map;

  persistent_data(const std::string &file, size_t file_size_kb)
      : mmap_(bip::open_or_create, file.c_str(), file_size_kb * 1024),
        root_(mmap_.find_or_construct<data_t>("root")(mmap_.get_segment_manager())) {
  }
  template <class T> auto *operator->(this T &&self) { return &self.root(); }
  template <class T> auto &root(this T &&self) { return *self.root_; }

  auto get_alloc() const { return mmap_.get_segment_manager(); }

private:
  bip::managed_mapped_file mmap_;
  data_t* root_;
};

} // namespace persistent

