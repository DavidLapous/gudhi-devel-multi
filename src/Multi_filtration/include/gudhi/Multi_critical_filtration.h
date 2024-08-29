/*    This file is part of the Gudhi Library - https://gudhi.inria.fr/ - which is released under MIT.
 *    See file LICENSE or go to https://gudhi.inria.fr/licensing/ for full license details.
 *    Author(s):       David Loiseaux
 *
 *    Copyright (C) 2023 Inria
 *
 *    Modification(s):
 *      - 2024/08 Hannah Schreiber: Optimization and correction + numeric_limits + doc
 *      - YYYY/MM Author: Description of the modification
 */

#ifndef MULTI_CRITICAL_FILTRATIONS_H_
#define MULTI_CRITICAL_FILTRATIONS_H_

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include <gudhi/Debug_utils.h>
#include <gudhi/One_critical_filtration.h>

namespace Gudhi::multi_filtration {

/**
 * @class Multi_critical_filtration multi_critical_filtration.h gudhi/multi_critical_filtration.h
 * @ingroup multi_filtration
 *
 * @brief Class representing all apparition values of a same simplex for several parameters, that is, the filtration
 * values of a simplex in the setting of a k-critical multi-parameter filtration. The class can be used as a vector
 * whose indices correspond each to one filtration value. Then, the indices of each filtration value correspond to one
 * particular parameter. E.g., \f$ f[g][p] \f$ will be the apparition value at the \f$ p^{th} \f$ parameter of the
 * \f$ g^{th} \f$ generator of the simplex with @ref Multi_critical_filtration \f$ f \f$.
 *
 * @details Overloads `std::numeric_limits` such that:
 * - `std::numeric_limits<Multi_critical_filtration<T,co> >::has_infinity` returns `true`,
 * - `std::numeric_limits<Multi_critical_filtration<T,co> >::infinity()` returns
 * @ref Multi_critical_filtration<T,co>::inf() "",
 * - `std::numeric_limits<Multi_critical_filtration<T,co> >::minus_infinity()` returns
 *   @ref Multi_critical_filtration<T,co>::minus_inf() "",
 * - `std::numeric_limits<Multi_critical_filtration<T,co> >::max()` throws,
 * - `std::numeric_limits<Multi_critical_filtration<T,co> >::max(g,n)` returns a @ref Multi_critical_filtration<T,co>
 * with `g` generators of `n` parameters evaluated at value `std::numeric_limits<T>::max()`,
 * - `std::numeric_limits<Multi_critical_filtration<T,co> >::quiet_NaN()` returns
 * @ref Multi_critical_filtration<T,co>::nan() "".
 *
 * Multi-critical filtrations are filtrations such that the lifetime of each simplex is a union of positive cones, e.g.,
 *  - \f$ \{ x \in \mathbb R^2 : x \ge (1,2)\} \cap \{ x \in \mathbb R^2 : x \ge (2,1)\} \f$ is finitely critical,
 *    and more particularly 2-critical, while
 *  - \f$ \{ x \in \mathbb R^2 : x \ge \mathrm{epigraph}(y \mapsto e^{-y})\} \f$ is not.
 *
 * The particular case of 1-critical filtrations is handled by @ref One_critical_filtration "".
 *
 * @tparam T Arithmetic type of an entry for one parameter of a filtration value. Has to be **signed** and
 * to implement `std::isnan(T)`, `std::numeric_limits<T>::has_quiet_NaN`, `std::numeric_limits<T>::quiet_NaN()`,
 * `std::numeric_limits<T>::has_infinity`, `std::numeric_limits<T>::infinity()` and `std::numeric_limits<T>::max()`.
 * If `std::numeric_limits<T>::has_infinity` returns `false`, a call to `std::numeric_limits<T>::infinity()`
 * can simply throw. Examples are the native types `double`, `float` and `int`.
 * @tparam co If `true`, reverses the poset order, i.e., the order \f$ \le \f$  in \f$ \mathbb R^n \f$ becomes
 * \f$ \ge \f$.
 */
template <typename T, bool co = false>
class Multi_critical_filtration {
 public:
  /**
   * @brief Type of the origin of a "simplex lifetime cone". Common with @ref One_critical_filtration "".
   */
  using Generator = One_critical_filtration<T>;
  // TODO: not the best name...
  using Generators = std::vector<Generator>;                  /**< Container type for the filtration values. */
  using iterator = typename Generators::iterator;             /**< Iterator type for the generator container. */
  using const_iterator = typename Generators::const_iterator; /**< Const iterator type for the generator container. */

  // CONSTRUCTORS

  /**
   * @brief Default constructor. The constructed value will be either at infinity if `co` is true or at minus infinity
   * if `co` is false.
   */
  Multi_critical_filtration() : multi_filtration_(1, co ? Generator::inf() : Generator::minus_inf()) {};
  /**
   * @brief Constructs a filtration value with one generator and @p n parameters. All parameters will be initialized
   * at -inf.
   *
   * @warning The generator `{-inf, -inf, ...}` with \f$ n > 1 \f$ entries is not considered as "minus infinity" (the
   * method @ref is_minus_inf() "", as well as the one of the generator, will not return true). The `-inf` are just
   * meant as placeholders, at least one entry should be modified by the user.
   * Otherwise, either use the static method @ref minus_inf() or set @p n to 1 instead.
   *
   * @param n Number of parameters.
   */
  Multi_critical_filtration(int n) : multi_filtration_({Generator(n)}) {};
  /**
   * @brief Constructs a filtration value with one generator and @p n parameters. All parameters will be initialized
   * with @p value.
   *
   * @warning If @p value is `inf`, `-inf`, or `NaN`, the generator `{value, value, ...}` with \f$ n > 1 \f$ entries
   * is not wrong but will not be considered as respectively "infinity", "minus infinity" or "NaN" (the corresponding
   * methods @ref is_inf(), @ref is_minus_inf() and @ref is_nan() will return false). For this purpose, please use
   * the static methods @ref inf(), @ref minus_inf() and @ref nan() instead.
   *
   * @param n Number of parameters.
   * @param value Value which will be used for each entry.
   */
  Multi_critical_filtration(int n, T value) : multi_filtration_({Generator(n, value)}) {};
  /**
   * @brief Constructs a filtration value with one generator which will be initialzed by the given initializer list.
   *
   * @param init Initializer list with values for each parameter.
   */
  Multi_critical_filtration(std::initializer_list<T> init) : multi_filtration_({Generator(init)}) {};
  /**
   * @brief Constructs a filtration value with one generator which will be initialzed by the given vector.
   *
   * @param v Vector with values for each parameter.
   */
  Multi_critical_filtration(const std::vector<T> &v) : multi_filtration_({v}) {};
  /**
   * @brief Constructs a filtration value with one generator to which the given vector is moved to.
   *
   * @param v Vector with values for each parameter.
   */
  Multi_critical_filtration(std::vector<T> &&v) : multi_filtration_({std::move(v)}) {};
  /**
   * @brief Constructs filtration value with as many generators than elements in the given vector and initialize
   * them with them.
   * @pre All generators in the vector have to have the same number of parameters, i.e., size.
   * Furthermore, the generators have to be a minimal generating set.
   *
   * @warning If the set of generators is not minimal, the behaviour of most methods is undefined. It is possible
   * to call @ref simplify() after construction if there is a doubt to ensure this property.
   *
   * @param v Vector of generators.
   */
  Multi_critical_filtration(const std::vector<Generator> &v) : multi_filtration_(v) {};
  /**
   * @brief Constructs filtration value with as many generators than elements in the given vector and moves those
   * elements to initialize the generators.
   * @pre All generators in the vector have to have the same number of parameters, i.e., size.
   * Furthermore, the generators have to be a minimal generating set.
   *
   * @warning If the set of generators is not minimal, the behaviour of most methods is undefined. It is possible
   * to call @ref simplify() after construction if there is a doubt to ensure this property.
   *
   * @param v Vector of generators.
   */
  Multi_critical_filtration(std::vector<Generator> &&v) : multi_filtration_(std::move(v)) {};
  /**
   * @brief Constructs a filtration value with one generator initialzed by the range given by the begin and end
   * iterators.
   *
   * @param it_begin Start of the range.
   * @param it_end End of the range.
   */
  Multi_critical_filtration(typename std::vector<T>::iterator it_begin, typename std::vector<T>::iterator it_end)
      : multi_filtration_({Generator(it_begin, it_end)}) {};
  /**
   * @brief Constructs a filtration value with one generator initialzed by the range given by the begin and end
   * const iterators.
   *
   * @param it_begin Start of the range.
   * @param it_end End of the range.
   */
  Multi_critical_filtration(typename std::vector<T>::const_iterator it_begin,
                            typename std::vector<T>::const_iterator it_end)
      : multi_filtration_({Generator(it_begin, it_end)}) {};

  // VECTOR-LIKE

  using value_type = T; /**< Entry type. */

  /**
   * @brief Standard operator[].
   */
  Generator &operator[](std::size_t i) { return multi_filtration_[i]; }
  /**
   * @brief Standard operator[] const.
   */
  const Generator &operator[](std::size_t i) const { return multi_filtration_[i]; }

  /**
   * @brief Returns begin iterator of the generator range.
   */
  iterator begin() { return multi_filtration_.begin(); }

  /**
   * @brief Returns end iterator of the generator range.
   */
  iterator end() { return multi_filtration_.end(); }

  /**
   * @brief Returns begin const iterator of the generator range.
   */
  const_iterator begin() const { return multi_filtration_.begin(); }

  /**
   * @brief Returns end const iterator of the generator range.
   */
  const_iterator end() const { return multi_filtration_.end(); }

  /**
   * @brief Returns true if and only if the number of generators is zero.
   */
  bool empty() const { return multi_filtration_.empty(); }

  /**
   * @brief Reserves space for the given number of generators in the underlying container.
   *
   * @param n Number of generators.
   */
  void reserve(std::size_t n) { multi_filtration_.reserve(n); }

  /**
   * @brief Removes all generators.
   */
  void clear() { multi_filtration_.clear(); }

  // CONVERTERS

  /**
   * @brief Casts the object into the type of a generator.
   * @pre The filtration value is 1-critical. If there are more than one generator, only the first will be preserved
   * and if there is no generator, the method will segfault.
   */
  operator Generator() const {
    GUDHI_CHECK(num_generators() == 1, "Casting a " + std::to_string(num_generators()) +
                                           "-critical filtration value into an 1-critical filtration value.");
    return multi_filtration_[0];
  }

  // like numpy
  /**
   * @brief Returns a copy with entries casted into the type given as template parameter.
   *
   * @tparam U New type for the entries.
   * @return Copy with new entry type.
   */
  template <typename U>
  Multi_critical_filtration<U> as_type() const {
    std::vector<One_critical_filtration<U>> out(num_generators());
    for (std::size_t i = 0u; i < num_generators(); ++i) {
      out[i] = multi_filtration_[i].template as_type<U>();
    }
    return Multi_critical_filtration<U>(std::move(out));
  }

  // ACCESS

  /**
   * @brief Returns a reference to the underlying container storing the generators.
   */
  const Generators &get_underlying_container() const { return multi_filtration_; }

  /**
   * @brief Returns the number of parameters.
   */
  std::size_t num_parameters() const { return multi_filtration_.empty() ? 0 : multi_filtration_[0].num_parameters(); }

  /**
   * @brief Returns the number of generators.
   */
  std::size_t num_generators() const { return multi_filtration_.size(); }

  /**
   * @brief Returns a filtration value for which @ref is_inf() returns `true`.
   *
   * @return Infinity.
   */
  constexpr static Multi_critical_filtration inf() { return Multi_critical_filtration(Generator::inf()); }

  /**
   * @brief Returns a filtration value for which @ref is_minus_inf() returns `true`.
   *
   * @return Minus infinity.
   */
  constexpr static Multi_critical_filtration minus_inf() { return Multi_critical_filtration(Generator::minus_inf()); }

  /**
   * @brief Returns a filtration value for which @ref is_nan() returns `true`.
   *
   * @return NaN.
   */
  constexpr static Multi_critical_filtration nan() { return Multi_critical_filtration(Generator::nan()); }

  // DESCRIPTORS

  // TODO: Accept {{-inf, -inf, ...},...} / {{inf, inf, ...},...} / {{NaN, NaN, ...},...} as resp. -inf / inf / NaN.

  /**
   * @brief Returns `true` if and only if the filtration value is considered as infinity.
   */
  bool is_inf() const { return multi_filtration_.size() == 1 && multi_filtration_[0].is_inf(); }

  /**
   * @brief Returns `true` if and only if the filtration value is considered as minus infinity.
   */
  bool is_minus_inf() const { return multi_filtration_.size() == 1 && multi_filtration_[0].is_minus_inf(); }

  /**
   * @brief Returns `true` if and only if the filtration value is considered as NaN.
   */
  bool is_nan() const { return multi_filtration_.size() == 1 && multi_filtration_[0].is_nan(); }

  /**
   * @brief Returns `true` if and only if the filtration value is non-empty and is not considered as infinity,
   * minus infinity or NaN.
   */
  bool is_finite() const {
    if (empty()) return false;
    if (multi_filtration_.size() > 1) return true;
    return multi_filtration_[0].is_finite();
  }

  // COMPARAISON OPERATORS

  // TODO : this costs a lot... optimize / cheat in some way for python ?
  /*
   * Checks if `this`, seen as a birth curve is under the `other` birth curve,
   *
   */

  /**
   * @brief Returns `true` if and only if the positive cones generated by @p b are strictly contained in the
   * positive cones generated by @p a.
   * If @p a and @p b are both not infinite or NaN, they have to have the same number of parameters.
   *
   * Note that not all filtration values are comparable. That is, \f$ a > b \f$ and \f$ b > a \f$ returning both false
   * does **not** imply \f$ a == b \f$.
   */
  friend bool operator<(const Multi_critical_filtration &a, const Multi_critical_filtration &b) {
    for (std::size_t i = 0u; i < b.multi_filtration_.size(); ++i) {
      // for each generator in b, verify if it is strictly in the cone of at least one generator of a
      bool isContained = false;
      for (std::size_t j = 0u; j < a.multi_filtration_.size() && !isContained; ++j) {
        // i<j
        isContained = _strictly_contains(a.multi_filtration_[j], b.multi_filtration_[i]);
      }
      if (!isContained) return false;
    }
    return true;
  }

  /**
   * @brief Returns `true` if and only if the positive cones generated by @p a are strictly contained in the
   * positive cones generated by @p b.
   * If @p a and @p b are both not infinite or NaN, they have to have the same number of parameters.
   *
   * Note that not all filtration values are comparable. That is, \f$ a > b \f$ and \f$ b > a \f$ returning both false
   * does **not** imply \f$ a == b \f$.
   */
  friend bool operator>(const Multi_critical_filtration &a, const Multi_critical_filtration &b) { return b < a; }

  /**
   * @brief Returns `true` if and only if the positive cones generated by @p b are contained in or are (partially)
   * equal to the positive cones generated by @p a.
   * If @p a and @p b are both not infinite or NaN, they have to have the same number of parameters.
   *
   * Note that not all filtration values are comparable. That is, \f$ a \le b \f$ and \f$ b \le a \f$ can both return
   * `false`.
   */
  friend bool operator<=(const Multi_critical_filtration &a, const Multi_critical_filtration &b) {
    // check if this curves is below other's curve
    //  ie for each guy in this, check if there is a guy in other that dominates him
    for (std::size_t i = 0u; i < b.multi_filtration_.size(); ++i) {
      // for each generator in other, verify if it is in the cone of at least one generator of this
      bool isContained = false;
      for (std::size_t j = 0u; j < a.multi_filtration_.size() && !isContained; ++j) {
        // i<j
        isContained = _contains(a.multi_filtration_[j], b.multi_filtration_[i]);
      }
      if (!isContained) return false;
    }
    return true;
  }

  /**
   * @brief Returns `true` if and only if the positive cones generated by @p a are contained in or are (partially)
   * equal to the positive cones generated by @p b.
   * If @p a and @p b are both not infinite or NaN, they have to have the same number of parameters.
   *
   * Note that not all filtration values are comparable. That is, \f$ a \ge b \f$ and \f$ b \ge a \f$ can both return
   * `false`.
   */
  friend bool operator>=(const Multi_critical_filtration &a, const Multi_critical_filtration &b) { return b <= a; }

  /**
   * @brief Returns `true` if and only if for each \f$ i \f$, \f$ a[i] \f$ is equal to \f$ b[i] \f$.
   */
  friend bool operator==(const Multi_critical_filtration &a, const Multi_critical_filtration &b) {
    if (a.num_generators() != b.num_generators()) return false;
    for (auto i = 0u; i < a.num_generators(); i++) {
      if (a[i] != b[i]) return false;
    }
    return true;
  }

  /**
   * @brief Returns `true` if and only if \f$ a == b \f$ returns `false`.
   */
  friend bool operator!=(const Multi_critical_filtration &a, const Multi_critical_filtration &b) { return !(a == b); }

  // MODIFIERS

  /**
   * @brief Sets the number of generators. If there were less generators before, new empty generators are constructed.
   * If there were more generators before, the exceed of generators is destroyed (any generator with index higher or
   * equal than @p n to be more precise).
   *
   * @warning All empty generators have 0 parameters. This can be problematic for some methods if there are also
   * non empty generators in the container. Make sure to fill them with real generators or to remove them before
   * using those methods.
   *
   * @param n New number of generators.
   */
  void set_num_generators(std::size_t n) { multi_filtration_.resize(n); }

  /**
   * @brief Sets all generators to the least common upper bound between the current generator value and the given value.
   *
   * More formally, it pushes the current filtration value to the cone \f$ \{ y \in \mathbb R^n : y \ge x \} \f$
   * originating in \f$ x \f$. The resulting values corresponds to the generators of the intersection of this cone
   * with the union of positive cones generated by the old generators.
   *
   * @param x The target filtration value towards which to push.
   */
  void push_to_least_common_upper_bound(const Generator &x) {
    if (this->is_inf() || this->is_nan() || x.is_nan() || x.is_minus_inf()) return;

    GUDHI_CHECK(x.is_inf() || x.num_parameters() == multi_filtration_[0].num_parameters() || !is_finite(),
                "Pushing to a filtration value with different number of parameters.");

    if (x.is_inf() || this->is_minus_inf()) {
      multi_filtration_ = {x};
      return;
    }
    for (auto &g : *this) {
      g.push_to_least_common_upper_bound(x);
    }

    simplify();
  }

  // TODO: this is not well defined for k-critical <-> k-critical ?

  /**
   * @brief Sets all generators to the greatest common lower bound between the current generator value and the given
   * value.
   *
   * More formally, it pulls the current filtration value to the cone \f$ \{ y \in \mathbb R^n : y \le x \} \f$
   * originating in \f$ x \f$. The resulting values corresponds to the generators of the intersection of this cone
   * with the union of negative cones generated by the old generators.
   *
   * @param x The target filtration value towards which to pull.
   */
  void pull_to_greatest_common_lower_bound(const Generator &x) {
    if (x.is_inf() || this->is_nan() || x.is_nan() || this->is_minus_inf()) return;

    GUDHI_CHECK(x.is_minus_inf() || x.num_parameters() == multi_filtration_[0].num_parameters() || !is_finite(),
                "Pulling to a filtration value with different number of parameters.");

    if (this->is_inf() || x.is_minus_inf()) {
      multi_filtration_ = {x};
      return;
    }
    for (auto &g : *this) {
      g.pull_to_greatest_common_lower_bound(x);
    }

    simplify();
  }

  /**
   * @brief Adds the given generator to the filtration value such that the sets remains minimal.
   * It is therefore possible that the generator is ignored if it does not generated any new lifetime or that
   * old generators disappear if they are overshadowed by the new one.
   * @pre If all are finite, the new generator has to have the same number of parameters than the others.
   *
   * @param x New generator to add.
   * @return true If and only if the generator is actually added to the set of generators.
   * @return false Otherwise.
   */
  bool add_generator(const Generator &x) {
    if (multi_filtration_.empty()) {
      multi_filtration_.push_back(x);
      return true;
    }

    GUDHI_CHECK(x.num_parameters() == multi_filtration_[0].num_parameters() || !is_finite() || !x.is_finite(),
                "Cannot add a generator with different number of parameters.");

    std::size_t end = multi_filtration_.size();

    if (_generator_can_be_added(x, 0, end)) {
      multi_filtration_.resize(end);
      multi_filtration_.push_back(x);
      return true;
    }

    return false;
  }

  /**
   * @brief Adds the given generator to the filtration value without any verifications or simplifications.
   *
   * @warning If the resulting set of generators is not minimal after modification, some methods will have an
   * undefined behaviour. Be sure to call @ref simplify() before using them.
   *
   * @param x
   */
  void add_guaranteed_generator(const Generator &x) { multi_filtration_.push_back(x); }

  /*
   * Same as `compute_coordinates_in_grid`, but does the operation in-place.
   */

  /**
   * @brief Projects the filtration value into the given grid. If @p coordinate is false, the entries are set to
   * the nearest upper bound value with the same parameter in the grid. Otherwise, the entries are set to the indices
   * of those nearest upper bound values.
   * The grid has to be represented as a vector of ordered ranges of values convertible into `T`. An index
   * \f$ i \f$ of the vector corresponds to the same parameter as the index \f$ i \f$ in a generator.
   * The ranges correspond to the possible values of the parameters, ordered by increasing value, forming therefore
   * all together a 2D grid.
   *
   * @tparam oned_array A range of values convertible into `T` ordered by increasing value. Has to implement
   * a begin, end and operator[] method.
   * @param grid Vector of @p oned_array with size at least number of filtration parameters.
   * @param coordinate If true, the values are set to the coordinates of the projection in the grid. If false,
   * the values are set to the values at the coordinates of the projection.
   */
  template <typename oned_array>
  void project_onto_grid(const std::vector<oned_array> &grid, bool coordinate = true) {
    GUDHI_CHECK(grid.size() >= num_parameters(),
                "The grid should not be smaller than the number of parameters in the filtration value.");

    for (auto &x : multi_filtration_) {
      x.project_onto_grid(grid, coordinate);
    }

    if (!coordinate) simplify();
  }

  /**
   * @brief Removes all empty generators from the filtration value. If @p include_infinities is true, it also
   * removes the generators at infinity or minus infinity.
   *
   * @param include_infinities If true, removes also infinity values.
   */
  void remove_empty_generators(bool include_infinities = false) {
    multi_filtration_.erase(std::remove_if(multi_filtration_.begin(), multi_filtration_.end(),
                                           [include_infinities](const Generator &a) {
                                             return a.empty() ||
                                                    ((include_infinities) && (a.is_inf() || a.is_minus_inf()));
                                           }),
                            multi_filtration_.end());
  }

  /**
   * @brief Simplifies the current set of generators such that it becomes minimal. Only necessary if generators were
   * added "by hand" either trough the constructor or with @ref add_guaranteed_generator "".
   */
  void simplify() {
    std::size_t end = 0;

    for (std::size_t curr = 0; curr < multi_filtration_.size(); ++curr) {
      if (_generator_can_be_added(multi_filtration_[curr], 0, end)) {
        std::swap(multi_filtration_[end], multi_filtration_[curr]);
        ++end;
      }
    }

    multi_filtration_.resize(end);
  }

  // FONCTIONNALITIES

  /**
   * @brief Returns a generator with the minimal values of all parameters in any generator of the given filtration
   * value. That is, the greatest lower bound of all generators.
   */
  friend Generator factorize_below(const Multi_critical_filtration &f) {
    if (f.num_generators() == 0) return Generator();
    Generator result(f.num_parameters(), Generator::T_inf);
    for (const auto &g : f) {
      if (g.is_nan() || g.is_minus_inf()) return g;
      if (g.is_inf()) continue;
      for (std::size_t i = 0; i < f.num_parameters(); ++i) {
        result[i] = std::min(result[i], g[i]);
      }
    }
    return result;
  }

  /**
   * @brief Returns a generator with the maximal values of all parameters in any generator of the given filtration
   * value. That is, the least upper bound of all generators.
   */
  friend Generator factorize_above(const Multi_critical_filtration &f) {
    if (f.num_generators() == 0) return Generator();
    Generator result(f.num_parameters(), -Generator::T_inf);
    for (auto &g : f) {
      if (g.is_nan() || g.is_inf()) return g;
      if (g.is_minus_inf()) continue;
      for (std::size_t i = 0; i < g.num_parameters(); ++i) {
        result[i] = std::max(result[i], g[i]);
      }
    }
    return result;
  }

  /**
   * @brief Computes the smallest (resp. the greatest if `co` is true) scalar product of the all generators with the
   * given vector.
   *
   * @tparam U Arithmetic type of the result. Default value: `T`.
   * @param f Filtration value.
   * @param x Vector of coefficients.
   * @return Scalar product of @p f with @p x.
   */
  template <typename U = T>
  friend U compute_linear_projection(const Multi_critical_filtration &f, const std::vector<U> &x) {
    if constexpr (co) {
      U projection = std::numeric_limits<U>::lowest();
      for (const auto &y : f) {
        projection = std::max(projection, compute_linear_projection(y, x));
      }
      return projection;
    } else {
      U projection = std::numeric_limits<U>::max();
      for (const auto &y : f) {
        projection = std::min(projection, compute_linear_projection(y, x));
      }
      return projection;
    }
  }

  /**
   * @brief Computes the coordinates in the given grid, corresponding to the nearest upper bounds of the entries
   * in the given filtration value.
   * The grid has to be represented as a vector of vectors of ordered values convertible into `out_type`. An index
   * \f$ i \f$ of the vector corresponds to the same parameter as the index \f$ i \f$ in a generator.
   * The inner vectors correspond to the possible values of the parameters, ordered by increasing value,
   * forming therefore all together a 2D grid.
   *
   * @tparam out_type Signed arithmetic type. Default value: std::int32_t.
   * @tparam U Type which is convertible into `out_type`.
   * @param f Filtration value to project.
   * @param grid Vector of vectors to project into.
   * @return Filtration value \f$ out \f$ whose entry correspond to the indices of the projected values. That is,
   * the projection of \f$ f[i] \f$ is \f$ grid[i][out[i]] \f$ before simplification (if two generators were
   * projected to the same point, the doubles are removed in the output).
   */
  template <typename out_type = std::int32_t, typename U = T>
  friend Multi_critical_filtration<out_type> compute_coordinates_in_grid(const Multi_critical_filtration &f,
                                                                         const std::vector<std::vector<U>> &grid) {
    Multi_critical_filtration<out_type> coords = f.as_type<out_type>();
    coords.project_onto_grid(grid);
    return coords;
  }

  /**
   * @brief Computes the values in the given grid corresponding to the coordinates given by the given filtration
   * value. That is, if \f$ out \f$ is the result, \f$ out[i] = grid[i][f[i]] \f$. Assumes therefore, that the
   * values stored in the filtration value corresponds to indices existing in the given grid.
   *
   * @tparam U Signed arithmetic type.
   * @param f Filtration value storing coordinates compatible with `grid`.
   * @param grid Vector of vector.
   * @return Filtration value \f$ out \f$ whose entry correspond to \f$ out[i] = grid[i][f[i]] \f$ before
   * simplification (the output is simplified).
   */
  template <typename U>
  friend Multi_critical_filtration<U> evaluate_coordinates_in_grid(const Multi_critical_filtration &f,
                                                                   const std::vector<std::vector<U>> &grid) {
    Multi_critical_filtration<U> out;
    out.set_num_generators(f.num_generators());
    for (std::size_t i = 0; i < f.num_generators(); ++i) {
      out[i] = evaluate_coordinates_in_grid(f[i], grid);
    }
    out.simplify();
    return out;
  }

  // UTILITIES

  /**
   * @brief Outstream operator.
   */
  friend std::ostream &operator<<(std::ostream &stream, const Multi_critical_filtration &f) {
    if (f.is_inf()) {
      stream << "[inf, ..., inf]";
      return stream;
    }
    if (f.is_minus_inf()) {
      stream << "[-inf, ..., -inf]";
      return stream;
    }
    if (f.is_nan()) {
      stream << "[NaN]";
      return stream;
    }
    stream << "(k=" << f.multi_filtration_.size() << ")[";
    for (const auto &val : f) {
      stream << val << "; ";
    }
    if (f.multi_filtration_.size() > 0) {
      stream << "\b"
             << "\b";
    }
    stream << "]";
    return stream;
  }

 public:
  /**
   * @brief Indicates if the class manages multi-critical filtration values.
   */
  constexpr static const bool is_multi_critical = true;

 private:
  Generators multi_filtration_; /**< Container for generators. */

  /**
   * @brief Verifies if @p b is strictly contained in the positive cone originating in `a`.
   */
  static bool _strictly_contains(const Generator &a, const Generator &b) {
    if constexpr (co)
      return a > b;
    else {
      return a < b;
    }
  }
  /**
   * @brief Verifies if @p b is contained in the positive cone originating in `a`.
   */
  static bool _contains(const Generator &a, const Generator &b) {
    if constexpr (co)
      return a >= b;
    else {
      return a <= b;
    }
  }

  // 0 == equal
  // 1 == a dom b
  // 2 == b dom a
  // 3 == none
  static int _get_domination_relation(const Generator &a, const Generator &b) {
    if (a.is_nan() || b.is_nan()) return 3;

    GUDHI_CHECK(a.size() == b.size(),
                "Two generators in the same k-critical value have to have the same numbers of parameters.");

    bool equal = true;
    bool allGreater = true;
    bool allSmaller = true;
    for (unsigned int i = 0; i < a.size(); ++i) {
      if (a[i] < b[i]) {
        if (!allSmaller) return 3;
        equal = false;
        allGreater = false;
      } else if (a[i] > b[i]) {
        if (!allGreater) return 3;
        equal = false;
        allSmaller = false;
      }
    }
    if (equal) return 0;

    if constexpr (co) {
      if (allSmaller) return 1;
      return 2;
    } else {
      if (allGreater) return 1;
      return 2;
    }
  }

  // assumes between 'curr' and 'end' everything is simplified:
  // no nan values and if there is an inf/-inf, then 'end - curr == 1'
  bool _generator_can_be_added(const Generator &x, std::size_t curr, std::size_t &end) {
    if (x.empty() || x.is_nan() || (x.is_inf() && end - curr != 0)) return false;

    if (x.is_minus_inf()) {
      if (end - curr == 1 && multi_filtration_[curr].is_minus_inf()) return false;
      // assumes that everything between curr and end is already simplified
      // so, if end - curr != 1, there can be no minus_inf anymore.
      end = curr;
      return true;
    }

    while (curr != end) {
      int res = _get_domination_relation(multi_filtration_[curr], x);
      if (res == 2 || res == 0) return false;  // x dominates or is equal
      if (res == 1) {                          // x is dominated
        --end;
        std::swap(multi_filtration_[curr], multi_filtration_[end]);
      } else {
        ++curr;
      }
    }
    return true;
  }
};

}  // namespace Gudhi::multi_filtration

namespace std {

template <typename T>
class numeric_limits<Gudhi::multi_filtration::Multi_critical_filtration<T>> {
 public:
  static constexpr bool has_infinity = true;

  static constexpr Gudhi::multi_filtration::Multi_critical_filtration<T> infinity() noexcept {
    return Gudhi::multi_filtration::Multi_critical_filtration<T>::inf();
  };

  // non-standard
  static constexpr Gudhi::multi_filtration::Multi_critical_filtration<T> minus_infinity() noexcept {
    return Gudhi::multi_filtration::Multi_critical_filtration<T>::minus_inf();
  };

  static constexpr Gudhi::multi_filtration::Multi_critical_filtration<T> max() noexcept(false) {
    throw std::logic_error(
        "The maximal value cannot be represented with no finite numbers of generators."
        "Use `max(number_of_generators, number_of_parameters)` instead");
  };

  // non-standard, so I don't want to define default values.
  static constexpr Gudhi::multi_filtration::Multi_critical_filtration<T> max(unsigned int g, unsigned int n) noexcept {
    std::vector<typename Gudhi::multi_filtration::Multi_critical_filtration<T>::Generator> v(
        g, std::vector<T>(n, std::numeric_limits<T>::max()));
    return Gudhi::multi_filtration::Multi_critical_filtration<T>(std::move(v));
  };

  static constexpr Gudhi::multi_filtration::Multi_critical_filtration<T> quiet_NaN() noexcept {
    return Gudhi::multi_filtration::Multi_critical_filtration<T>::nan();
  };
};

}  // namespace std

#endif  // MULTI_CRITICAL_FILTRATIONS_H_
