namespace std {
  namespace experimental {
  inline namespace fundamentals_v1 {
 
    // 5.4, optional for object types
    template <class T> class optional;
 
    // 5.5, In-place construction
    struct in_place_t{};
    constexpr in_place_t in_place{};
 
    // 5.6, Disengaged state indicator
    struct nullopt_t{see below};
    constexpr nullopt_t nullopt(unspecified);
 
    // 5.7, Class bad_optional_access
    class bad_optional_access;
 
    // 5.8, Relational operators
    template <class T>
      constexpr bool operator==(const optional<T>&, const optional<T>&);
    template <class T>
      constexpr bool operator!=(const optional<T>&, const optional<T>&);
    template <class T>
      constexpr bool operator<(const optional<T>&, const optional<T>&);
    template <class T>
      constexpr bool operator>(const optional<T>&, const optional<T>&);
    template <class T>
      constexpr bool operator<=(const optional<T>&, const optional<T>&);
    template <class T>
      constexpr bool operator>=(const optional<T>&, const optional<T>&);
 
    // 5.9, Comparison with nullopt
    template <class T> constexpr bool operator==(const optional<T>&, nullopt_t) noexcept;
    template <class T> constexpr bool operator==(nullopt_t, const optional<T>&) noexcept;
    template <class T> constexpr bool operator!=(const optional<T>&, nullopt_t) noexcept;
    template <class T> constexpr bool operator!=(nullopt_t, const optional<T>&) noexcept;
    template <class T> constexpr bool operator<(const optional<T>&, nullopt_t) noexcept;
    template <class T> constexpr bool operator<(nullopt_t, const optional<T>&) noexcept;
    template <class T> constexpr bool operator<=(const optional<T>&, nullopt_t) noexcept;
    template <class T> constexpr bool operator<=(nullopt_t, const optional<T>&) noexcept;
    template <class T> constexpr bool operator>(const optional<T>&, nullopt_t) noexcept;
    template <class T> constexpr bool operator>(nullopt_t, const optional<T>&) noexcept;
    template <class T> constexpr bool operator>=(const optional<T>&, nullopt_t) noexcept;
    template <class T> constexpr bool operator>=(nullopt_t, const optional<T>&) noexcept;
 
    // 5.10, Comparison with T
    template <class T> constexpr bool operator==(const optional<T>&, const T&);
    template <class T> constexpr bool operator==(const T&, const optional<T>&);
    template <class T> constexpr bool operator!=(const optional<T>&, const T&);
    template <class T> constexpr bool operator!=(const T&, const optional<T>&);
    template <class T> constexpr bool operator<(const optional<T>&, const T&);
    template <class T> constexpr bool operator<(const T&, const optional<T>&);
    template <class T> constexpr bool operator<=(const optional<T>&, const T&);
    template <class T> constexpr bool operator<=(const T&, const optional<T>&);
    template <class T> constexpr bool operator>(const optional<T>&, const T&);
    template <class T> constexpr bool operator>(const T&, const optional<T>&);
    template <class T> constexpr bool operator>=(const optional<T>&, const T&);
    template <class T> constexpr bool operator>=(const T&, const optional<T>&);
 
    // 5.11, Specialized algorithms
    template <class T> void swap(optional<T>&, optional<T>&) noexcept(see below);
    template <class T> constexpr optional<see below> make_optional(T&&);
 
  } // namespace fundamentals_v1
  } // namespace experimental
 
  // 5.12, Hash support
  template <class T> struct hash;
  template <class T> struct hash<experimental::optional<T>>;
 
} // namespace std