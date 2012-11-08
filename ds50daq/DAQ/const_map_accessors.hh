#ifndef CONST_MAP_ACCESSORS_
#define CONST_MAP_ACCESSORS_

namespace const_map_accessors {
 std::string to_upper (const std::string& v) { std::string r(v); std::transform (r.begin(), r.end(), r.begin(), [](int c) { return std::toupper(c); }); return r; }

  template <typename _key, typename _value> _value cmap (const std::map<_key, _value>& _map, const _key& key, const std::string& name) {
    auto it = _map.find (key);
    if (it == _map.end ()) throw cet::exception("Configuration") << "unknow value \"" << key << "\" for " << name;
    return it->second;
  }

  template <typename _value> _value cmap (const std::map<std::string, _value>& _map, const std::string& key, const std::string& name) {
    std::string key_up = to_upper (key);
    auto it = _map.find (key_up);
    if (it == _map.end ()) throw cet::exception("Configuration") << "unknow value \"" << key << "\" for " << name;
    return it->second;
  }

  template <typename A> std::string rfind (const std::map<std::string, A>& _map, const A& rkey) {
    for (const auto x: _map) if (x.second == rkey) return x.first;
    throw cet::exception("Configuration") << "reverse search failed";
  }
}
#endif
