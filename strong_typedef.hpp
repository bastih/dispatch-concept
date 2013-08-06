#include <boost/operators.hpp>

// Mostly inspired by boosts strong typedef, but without the macros
// Resulting type is ordered just like the wrapped type
template <typename T>
class strong_typedef : boost::totally_ordered1<strong_typedef<T>, boost::totally_ordered2<strong_typedef<T>, T>> {
public:
  strong_typedef() = default;

  strong_typedef(const strong_typedef & t_) :
      value(t_.value) {}

  strong_typedef(const strong_typedef && t_) :
      value(std::move(t_.value)) {}

  explicit strong_typedef(const T val) : value(val) {};

  strong_typedef & operator=(const strong_typedef & rhs) {
    value = rhs.value;
    return *this;
  }

  strong_typedef & operator=(const T & rhs) {
    value = rhs;
    return *this;
  }

  operator const T& () const {
    return value;
  }

  operator T& () {
    return value;
  }

  bool operator==(const strong_typedef& rhs) const {
    return value == rhs.value;
  }

  bool operator<(const strong_typedef& rhs) const {
    return value < rhs.value;
  }
private:
  T value;
};
