#pragma once
#include <array>
#include <stdexcept>
#include <vector>
#include <cmath>
#include <algorithm>
#include <QPointF>
#include <ostream>

namespace omm
{

template<typename ValueT> class Vec2
{
public:
  using element_type = ValueT;
  Vec2(const element_type& x, const element_type& y) : x(x), y(y) {}
  Vec2() : Vec2(element_type(0), element_type(0)) {}
  Vec2(const std::array<element_type, 2>& components) : Vec2(components[0], components[1]) {}
  explicit Vec2(const std::vector<ValueT>& components)
  {
    if (components.size() != 2) {
      const auto msg = "Expected vector of size 2 but got " + std::to_string(components.size());
      throw std::length_error(msg);
    } else {
      x = components[0];
      y = components[1];
    }
  }

  Vec2(const QPoint& p) : Vec2(p.x(), p.y()) {}
  Vec2(const QPointF& p) : Vec2(p.x(), p.y()) {}

  QPointF to_pointf() const { return QPointF(static_cast<double>(x), static_cast<double>(y)); }
  QPoint to_point() const { return QPoint(static_cast<int>(x), static_cast<int>(y)); }

  element_type x;
  element_type y;

  element_type& operator[](const std::size_t i)
  {
    switch (i) {
    case 0: return x;
    case 1: return y;
    }
    throw std::out_of_range("Index out of bounds.");
  }

  const element_type& operator[](const std::size_t i) const
  {
    switch (i) {
    case 0: return x;
    case 1: return y;
    }
    throw std::out_of_range("Index out of bounds.");
  }

  std::vector<ValueT> to_stdvec() const { return { x, y }; }
  double euclidean_norm2() const { return x*x + y*y; }
  double euclidean_norm() const { return std::sqrt(euclidean_norm2()); }
  ValueT max_norm() const { return std::max(std::abs(x), std::abs(y)); }
  ValueT max() const { return std::max(x, y); }
  ValueT min() const { return std::min(x, y); }
  void swap(Vec2& other) { std::swap(x, other.x); std::swap(y, other.y); }

  static Vec2<element_type> o() { return Vec2<element_type>(element_type(0), element_type(0)); }

  static Vec2<element_type> max(const Vec2<element_type>& a, const Vec2<element_type> b)
  {
    return Vec2<element_type>(std::max(a.x, b.x), std::max(a.y, b.y));
  }

  static Vec2<element_type> min(const Vec2<element_type>& a, const Vec2<element_type> b)
  {
    return Vec2<element_type>(std::min(a.x, b.x), std::min(a.y, b.y));
  }

  static element_type dot(const Vec2<element_type>& d1, const Vec2<element_type>& d2)
  {
    return d1.x*d2.x + d1.y*d2.y;
  }

  Vec2<element_type> clamped(const Vec2<element_type>& lower, const Vec2<element_type>& upper) const
  {
    return Vec2<element_type>(std::clamp(x, lower.x, upper.x), std::clamp(y, lower.y, upper.y));
  }

  Vec2<element_type>& operator+=(const Vec2<element_type>& rhs)
  {
    x += rhs.x;
    y += rhs.y;
    return *this;
  }

  Vec2<element_type>& operator*=(const element_type& e)
  {
    x *= e;
    y *= e;
    return *this;
  }

  Vec2<element_type>& operator*=(const Vec2<element_type>& rhs)
  {
    x *= rhs.x;
    y *= rhs.y;
    return *this;
  }

  Vec2<element_type>& operator/=(const Vec2<element_type>& rhs)
  {
    x /= rhs.x;
    y /= rhs.y;
    return *this;
  }

  Vec2<element_type>& operator-=(const Vec2<element_type>& rhs) { return (*this) += (-rhs); }
  Vec2<element_type>& operator/=(const element_type& e) { return (*this) *= 1.0/e; }
  double arg() const { return std::atan2(y, x); }
  bool has_nan() const { return std::isnan(x) || std::isnan(y); }
};

template<typename ValueT>
Vec2<ValueT> operator-(const Vec2<ValueT>& d)
{
  return Vec2<ValueT>(-d.x, -d.y);
}

template<typename ValueT> Vec2<ValueT> operator+(const Vec2<ValueT>& d1, const Vec2<ValueT>& d2)
{
  return Vec2<ValueT>(d1.x+d2.x, d1.y+d2.y);
}

template<typename ValueT> Vec2<ValueT> operator-(const Vec2<ValueT>& d1, const Vec2<ValueT>& d2)
{
  return d1 + (-d2);
}

template<typename ValueT> Vec2<ValueT> operator-(const Vec2<ValueT>& d, const ValueT& s)
{
  return d - Vec2<ValueT>(s, s);
}

template<typename ValueT> Vec2<ValueT> operator+(const Vec2<ValueT>& d, const ValueT& s)
{
  return d + Vec2<ValueT>(s, s);
}

template<typename ValueT> Vec2<ValueT> operator*(const Vec2<ValueT>& d, const ValueT s)
{
  return Vec2<ValueT>(d.x*s, d.y*s);
}

template<typename ValueT> Vec2<ValueT> operator*(const ValueT s, const Vec2<ValueT>& d)
{
  return d * s;
}

template<typename ValueT> Vec2<ValueT> operator/(const Vec2<ValueT>& d, const ValueT s)
{
  return d * (1.0/s);
}

template<typename ValueT> Vec2<ValueT> operator*(const Vec2<ValueT>& s, const Vec2<ValueT>& d)
{
  return Vec2<ValueT>(s.x * d.x, s.y * d.y);
}

template<typename ValueT> Vec2<ValueT> operator/(const Vec2<ValueT>& d, const Vec2<ValueT>& s)
{
  return Vec2<ValueT>(d.x / s.x, d.y / s.y);
}

template<typename ValueT> bool operator==(const Vec2<ValueT>& d1, const Vec2<ValueT>& d2)
{
  return d1.x == d2.x && d1.y == d2.y;
}

template<typename ValueT> bool operator!=(const Vec2<ValueT>& d1, const Vec2<ValueT>& d2)
{
  return !(d1 == d2);
}

/**
 * @brief provided only to make std::set and friends work.
 *  It does not do anything mathematically meaningful.
 */
template<typename ValueT> bool operator<(const Vec2<ValueT>& d1, const Vec2<ValueT>& d2)
{
  if (d1.x == d2.x) {
    return d1.y < d2.y;
  } else {
    return d1.x < d2.x;
  }
}

using Vec2f = Vec2<double>;
using Vec2i = Vec2<int>;

template<typename T> std::ostream& operator<<(std::ostream& ostream, const Vec2<T>& vec)
{
  ostream << "[" << vec.x << ", " << vec.y << "]";
  return ostream;
}

template<typename T> bool fuzzy_eq(const Vec2<T>& a, const Vec2<T>& b)
{
  static constexpr double eps = 10e-5;
  return (a-b).euclidean_norm2() < eps;
}

}  // namespace omm
