/*    This file is part of the Gudhi Library - https://gudhi.inria.fr/ - which is released under MIT.
 *    See file LICENSE or go to https://gudhi.inria.fr/licensing/ for full license details.
 *    Author(s):       Hannah Schreiber
 *
 *    Copyright (C) 2022-24 Inria
 *
 *    Modification(s):
 *      - YYYY/MM Author: Description of the modification
 */

/**
 * @file chain_rep_cycles.h
 * @author Hannah Schreiber
 * @brief Contains the @ref Chain_representative_cycles class and @ref Dummy_chain_representative_cycles structure.
 */

#ifndef PM_CHAIN_REP_CYCLES_H
#define PM_CHAIN_REP_CYCLES_H

#include <utility>    //std::move
#include <algorithm>  //std::sort
#include <vector>

namespace Gudhi {
namespace persistence_matrix {

/**
 * @ingroup persistence_matrix
 *
 * @brief Empty structure.
 * Inheritated instead of @ref Chain_representative_cycles, when the computation of the representative cycles
 * were not enabled.
 */
struct Dummy_chain_representative_cycles {
  friend void swap([[maybe_unused]] Dummy_chain_representative_cycles& d1,
                   [[maybe_unused]] Dummy_chain_representative_cycles& d2) {}

  // Dummy_chain_representative_cycles() {}
  // Dummy_chain_representative_cycles([[maybe_unused]] const Dummy_chain_representative_cycles& matrixToCopy) {}
  // Dummy_chain_representative_cycles([[maybe_unused]] Dummy_chain_representative_cycles&& other) noexcept {}
};

// TODO: add coefficients ? Only Z2 token into account for now.
/**
 * @class Chain_representative_cycles chain_rep_cycles.h gudhi/Persistence_matrix/chain_rep_cycles.h
 * @ingroup persistence_matrix
 *
 * @brief Class managing the representative cycles for @ref Chain_matrix if the option was enabled.
 * 
 * @tparam Master_matrix An instanciation of @ref Matrix from which all types and options are deduced.
 */
template <class Master_matrix>
class Chain_representative_cycles 
{
 public:
  using Bar = typename Master_matrix::Bar;                            /**< Bar type. */
  using cycle_type = typename Master_matrix::cycle_type;              /**< Cycle type. */
  using matrix_type = typename Master_matrix::column_container_type;  /**< Column container type. */

  /**
   * @brief Default constructor.
   */
  Chain_representative_cycles();
  /**
   * @brief Copy constructor.
   * 
   * @param matrixToCopy Matrix to copy.
   */
  Chain_representative_cycles(const Chain_representative_cycles& matrixToCopy);
  /**
   * @brief Move constructor.
   * 
   * @param other Matrix to move.
   */
  Chain_representative_cycles(Chain_representative_cycles&& other) noexcept;

  /**
   * @brief Computes the current representative cycles of the matrix.
   */
  void update_representative_cycles();

  /**
   * @brief Returns the current representative cycles. If the matrix is modified later after the first call,
   * @ref update_representative_cycles has to be called to update the returned cycles.
   * 
   * @return A const reference to a vector of @ref Matrix::cycle_type containing all representative cycles.
   */
  const std::vector<cycle_type>& get_representative_cycles();
  /**
   * @brief Returns the representative cycle corresponding to the given bar.
   * If the matrix is modified later after the first call,
   * @ref update_representative_cycles has to be called to update the returned cycles.
   * 
   * @param bar Bar corresponding to the wanted representative cycle.
   * @return A const reference to the representative cycle.
   */
  const cycle_type& get_representative_cycle(const Bar& bar);

  /**
   * @brief Assign operator.
   */
  Chain_representative_cycles& operator=(Chain_representative_cycles other);
  /**
   * @brief Swap operator.
   */
  friend void swap(Chain_representative_cycles& base1, Chain_representative_cycles& base2) {
    base1.representativeCycles_.swap(base2.representativeCycles_);
    base1.birthToCycle_.swap(base2.birthToCycle_);
  }

 private:
  using chain_matrix = typename Master_matrix::Chain_matrix_type;

  std::vector<cycle_type> representativeCycles_;            /**< Cycle container. */
  std::vector<typename Master_matrix::index> birthToCycle_; /**< Map from birth index to cycle index. */

  //access to inheritating Chain_matrix class
  constexpr chain_matrix* _matrix() { return static_cast<chain_matrix*>(this); }
  constexpr const chain_matrix* _matrix() const { return static_cast<const chain_matrix*>(this); }
};

template <class Master_matrix>
inline Chain_representative_cycles<Master_matrix>::Chain_representative_cycles() 
{}

template <class Master_matrix>
inline Chain_representative_cycles<Master_matrix>::Chain_representative_cycles(
    const Chain_representative_cycles<Master_matrix>& matrixToCopy)
    : representativeCycles_(matrixToCopy.representativeCycles_), birthToCycle_(matrixToCopy.birthToCycle_) 
{}

template <class Master_matrix>
inline Chain_representative_cycles<Master_matrix>::Chain_representative_cycles(
    Chain_representative_cycles<Master_matrix>&& other) noexcept
    : representativeCycles_(std::move(other.representativeCycles_)), birthToCycle_(std::move(other.birthToCycle_)) 
{}

template <class Master_matrix>
inline void Chain_representative_cycles<Master_matrix>::update_representative_cycles() 
{
  birthToCycle_.clear();
  birthToCycle_.resize(_matrix()->get_number_of_columns(), -1);
  representativeCycles_.clear();

  // for birthToCycle_, assumes that @ref PosIdx == @ref IDIdx, ie pivot == birth index... which is not true with vineyards
  // TODO: with vineyard, there is a @ref IDIdx --> @ref PosIdx map stored. somehow get access to it here
  for (typename Master_matrix::id_index i = 0; i < _matrix()->get_number_of_columns(); i++) {
    auto& col = _matrix()->get_column(_matrix()->get_column_with_pivot(i));
    if (!col.is_paired() || i < col.get_paired_chain_index()) {
      cycle_type cycle;
      for (auto& c : col) {
        cycle.push_back(c.get_row_index());
      }
      if constexpr (std::is_same_v<typename Master_matrix::Column_type, typename Master_matrix::Heap_column_type> ||
                    std::is_same_v<typename Master_matrix::Column_type,
                                   typename Master_matrix::Unordered_set_column_type>)
        std::sort(cycle.begin(), cycle.end());
      representativeCycles_.push_back(cycle);
      birthToCycle_[i] = representativeCycles_.size() - 1;
    }
  }
}

template <class Master_matrix>
inline const std::vector<typename Chain_representative_cycles<Master_matrix>::cycle_type>&
Chain_representative_cycles<Master_matrix>::get_representative_cycles() 
{
  if (representativeCycles_.empty()) update_representative_cycles();
  return representativeCycles_;
}

template <class Master_matrix>
inline const typename Chain_representative_cycles<Master_matrix>::cycle_type&
Chain_representative_cycles<Master_matrix>::get_representative_cycle(const Bar& bar) 
{
  if (representativeCycles_.empty()) update_representative_cycles();
  return representativeCycles_[birthToCycle_[bar.birth]];
}

template <class Master_matrix>
inline Chain_representative_cycles<Master_matrix>& Chain_representative_cycles<Master_matrix>::operator=(
    Chain_representative_cycles<Master_matrix> other) 
{
  representativeCycles_.swap(other.representativeCycles_);
  birthToCycle_.swap(other.birthToCycle_);
  return *this;
}

}  // namespace persistence_matrix
}  // namespace Gudhi

#endif  // PM_CHAIN_REP_CYCLES_H
