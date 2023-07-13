/*    This file is part of the Gudhi Library - https://gudhi.inria.fr/ - which is released under MIT.
 *    See file LICENSE or go to https://gudhi.inria.fr/licensing/ for full license details.
 *    Author(s):       Hannah Schreiber
 *
 *    Copyright (C) 2022 Inria
 *
 *    Modification(s):
 *      - YYYY/MM Author: Description of the modification
 */

#ifndef Z2_INTRUSIVE_SET_COLUMN_H
#define Z2_INTRUSIVE_SET_COLUMN_H

#include <iostream>
#include <vector>

#include <boost/intrusive/set.hpp>
#include <gudhi/Simple_object_pool.h>

#include "../utilities/utilities.h"
#include "cell.h"

namespace Gudhi {
namespace persistence_matrix {

template<class Cell_type, class Row_access_option>
class Z2_intrusive_set_column : public Row_access_option
{
public:
//	using Cell = Z2_intrusive_set_cell;
	using Cell = Cell_type;
	using Column_type = boost::intrusive::set <
							Cell
						  , boost::intrusive::constant_time_size<false>
						  , boost::intrusive::base_hook< base_hook_matrix_set_column >  >;
	using iterator = typename Column_type::iterator;
	using const_iterator = typename Column_type::const_iterator;
	using reverse_iterator = typename Column_type::reverse_iterator;
	using const_reverse_iterator = typename Column_type::const_reverse_iterator;

	Z2_intrusive_set_column();
	template<class Container_type>
	Z2_intrusive_set_column(const Container_type& nonZeroRowIndices);
	template<class Container_type>
	Z2_intrusive_set_column(const Container_type& nonZeroRowIndices, dimension_type dimension);
	template<class Row_container_type>
	Z2_intrusive_set_column(index columnIndex, Row_container_type &rowContainer);
	template<class Container_type, class Row_container_type>
	Z2_intrusive_set_column(index columnIndex, const Container_type& nonZeroRowIndices, Row_container_type &rowContainer);
	template<class Container_type, class Row_container_type>
	Z2_intrusive_set_column(index columnIndex, const Container_type& nonZeroRowIndices, dimension_type dimension, Row_container_type &rowContainer);
	Z2_intrusive_set_column(const Z2_intrusive_set_column& column);
	Z2_intrusive_set_column(const Z2_intrusive_set_column& column, index columnIndex);
	template<class Row_container_type>
	Z2_intrusive_set_column(const Z2_intrusive_set_column& column, index columnIndex, Row_container_type &rowContainer);
	Z2_intrusive_set_column(Z2_intrusive_set_column&& column) noexcept;
	~Z2_intrusive_set_column();

	std::vector<bool> get_content(int columnLength = -1) const;
	bool is_non_zero(index rowIndex) const;
	bool is_empty() const;
	dimension_type get_dimension() const;
	template<class Map_type>
	void reorder(Map_type& valueMap);
	void clear();

	iterator begin() noexcept;
	const_iterator begin() const noexcept;
	iterator end() noexcept;
	const_iterator end() const noexcept;
	reverse_iterator rbegin() noexcept;
	const_reverse_iterator rbegin() const noexcept;
	reverse_iterator rend() noexcept;
	const_reverse_iterator rend() const noexcept;

	template<class Cell_range>
	Z2_intrusive_set_column& operator+=(Cell_range const &column);
	friend Z2_intrusive_set_column operator+(Z2_intrusive_set_column column1, Z2_intrusive_set_column const& column2){
		column1 += column2;
		return column1;
	}

	Z2_intrusive_set_column& operator*=(unsigned int v);
	friend Z2_intrusive_set_column operator*(Z2_intrusive_set_column column, unsigned int const& v){
		column *= v;
		return column;
	}
	friend Z2_intrusive_set_column operator*(unsigned int const& v, Z2_intrusive_set_column column){
		column *= v;
		return column;
	}

	friend bool operator==(const Z2_intrusive_set_column& c1, const Z2_intrusive_set_column& c2){
		if (&c1 == &c2) return true;
		return c1.column_ == c2.column_;
	}
	friend bool operator<(const Z2_intrusive_set_column& c1, const Z2_intrusive_set_column& c2){
		if (&c1 == &c2) return false;
		return c1.column_ < c2.column_;
	}

	Z2_intrusive_set_column& operator=(const Z2_intrusive_set_column& other);

	friend void swap(Z2_intrusive_set_column& col1, Z2_intrusive_set_column& col2){
		swap(static_cast<Row_access_option&>(col1),
			 static_cast<Row_access_option&>(col2));
		std::swap(col1.dim_, col2.dim_);
		col1.column_.swap(col2.column_);
	}

protected:
	dimension_type dim_;
	Column_type column_;
	inline static Simple_object_pool<Cell> cellPool_;

	void _delete_cell(iterator& it);
	void _insert_cell(index rowIndex, const iterator& position);

private:
	//Cloner object function
	struct new_cloner
	{
		Cell *operator()(const Cell &clone_this) { 
			// return new Cell(clone_this); 
			return cellPool_.construct(clone_this);
		}
	};

	//The disposer object function
	struct delete_disposer
	{
		delete_disposer(){};
		delete_disposer(Z2_intrusive_set_column* col) : col_(col)
		{};

		void operator()(Cell *delete_this)
		{
			if constexpr (Row_access_option::isActive_)
				col_->unlink(delete_this);
			// delete delete_this;
			cellPool_.destroy(delete_this);
		}

		Z2_intrusive_set_column* col_;
	};
};

template<class Cell_type, class Row_access_option>
inline Z2_intrusive_set_column<Cell_type,Row_access_option>::Z2_intrusive_set_column() : dim_(0)
{
//	static_assert(!Row_access_option::isActive_, "When row access option enabled, a row container has to be provided.");
}

template<class Cell_type, class Row_access_option>
template<class Container_type>
inline Z2_intrusive_set_column<Cell_type,Row_access_option>::Z2_intrusive_set_column(const Container_type &nonZeroRowIndices)
	: dim_(nonZeroRowIndices.size() == 0 ? 0 : nonZeroRowIndices.size() - 1)
{
//	static_assert(!Row_access_option::isActive_, "When row access option enabled, a row container has to be provided.");

	for (index id : nonZeroRowIndices){
		_insert_cell(id, column_.end());
	}
}

template<class Cell_type, class Row_access_option>
template<class Container_type>
inline Z2_intrusive_set_column<Cell_type,Row_access_option>::Z2_intrusive_set_column(const Container_type &nonZeroRowIndices, dimension_type dimension)
	: dim_(dimension)
{
//	static_assert(!Row_access_option::isActive_, "When row access option enabled, a row container has to be provided.");

	for (index id : nonZeroRowIndices){
		_insert_cell(id, column_.end());
	}
}

template<class Cell_type, class Row_access_option>
template<class Row_container_type>
inline Z2_intrusive_set_column<Cell_type,Row_access_option>::Z2_intrusive_set_column(
		index columnIndex, Row_container_type &rowContainer)
	: Row_access_option(columnIndex, rowContainer), dim_(0)
{}

template<class Cell_type, class Row_access_option>
template<class Container_type, class Row_container_type>
inline Z2_intrusive_set_column<Cell_type,Row_access_option>::Z2_intrusive_set_column(
		index columnIndex, const Container_type &nonZeroRowIndices, Row_container_type &rowContainer)
	: Row_access_option(columnIndex, rowContainer), dim_(nonZeroRowIndices.size() == 0 ? 0 : nonZeroRowIndices.size() - 1)
{
	for (index id : nonZeroRowIndices){
		_insert_cell(id, column_.end());
	}
}

template<class Cell_type, class Row_access_option>
template<class Container_type, class Row_container_type>
inline Z2_intrusive_set_column<Cell_type,Row_access_option>::Z2_intrusive_set_column(
		index columnIndex, const Container_type &nonZeroRowIndices, dimension_type dimension, Row_container_type &rowContainer)
	: Row_access_option(columnIndex, rowContainer), dim_(dimension)
{
	for (index id : nonZeroRowIndices){
		_insert_cell(id, column_.end());
	}
}

template<class Cell_type, class Row_access_option>
inline Z2_intrusive_set_column<Cell_type,Row_access_option>::Z2_intrusive_set_column(const Z2_intrusive_set_column &column)
	: dim_(column.dim_)
{
	static_assert(!Row_access_option::isActive_,
			"Copy constructor not available when row access option enabled.");

	column_.clone_from(column.column_, new_cloner(), delete_disposer());
}

template<class Cell_type, class Row_access_option>
inline Z2_intrusive_set_column<Cell_type,Row_access_option>::Z2_intrusive_set_column(
		const Z2_intrusive_set_column &column, index columnIndex)
	: Row_access_option(columnIndex, *column.rows_),
	  dim_(column.dim_)
{
	for (const Cell& cell : column.column_){
		_insert_cell(cell.get_row_index(), column_.end());
	}
}

template<class Cell_type, class Row_access_option>
template<class Row_container_type>
inline Z2_intrusive_set_column<Cell_type,Row_access_option>::Z2_intrusive_set_column(
		const Z2_intrusive_set_column &column, index columnIndex, Row_container_type &rowContainer)
	: Row_access_option(columnIndex, rowContainer),
	  dim_(column.dim_)
{
	for (const Cell& cell : column.column_){
		_insert_cell(cell.get_row_index(), column_.end());
	}
}

template<class Cell_type, class Row_access_option>
inline Z2_intrusive_set_column<Cell_type,Row_access_option>::Z2_intrusive_set_column(Z2_intrusive_set_column &&column) noexcept
	: Row_access_option(std::move(column)),
	  dim_(std::exchange(column.dim_, 0)),
	  column_(std::move(column.column_))
{}

template<class Cell_type, class Row_access_option>
inline Z2_intrusive_set_column<Cell_type,Row_access_option>::~Z2_intrusive_set_column()
{
	for (iterator c_it = column_.begin(); c_it != column_.end(); ){
		_delete_cell(c_it);
	}
}

template<class Cell_type, class Row_access_option>
inline std::vector<bool> Z2_intrusive_set_column<Cell_type,Row_access_option>::get_content(int columnLength) const
{
	if (columnLength < 0) columnLength = column_.rbegin()->get_row_index() + 1;

	std::vector<bool> container(columnLength);
	for (auto it = column_.begin(); it != column_.end() && it->get_row_index() < static_cast<index>(columnLength); ++it){
		container[it->get_row_index()] = 1;
	}
	return container;
}

template<class Cell_type, class Row_access_option>
inline bool Z2_intrusive_set_column<Cell_type,Row_access_option>::is_non_zero(index rowIndex) const
{
	if constexpr (Row_access_option::isActive_){
		return column_.find(Cell(Row_access_option::columnIndex_, rowIndex)) != column_.end();
	} else {
		return column_.find(Cell(rowIndex)) != column_.end();
	}
}

template<class Cell_type, class Row_access_option>
inline bool Z2_intrusive_set_column<Cell_type,Row_access_option>::is_empty() const
{
	return column_.empty();
}

template<class Cell_type, class Row_access_option>
inline dimension_type Z2_intrusive_set_column<Cell_type,Row_access_option>::get_dimension() const
{
	return dim_;
}

template<class Cell_type, class Row_access_option>
template<class Map_type>
inline void Z2_intrusive_set_column<Cell_type,Row_access_option>::reorder(Map_type &valueMap)
{
	Column_type newSet;
	for (auto it = column_.begin(); it != column_.end(); ) {
		if constexpr (Row_access_option::isActive_) {
			// Cell *new_cell = new Cell(Row_access_option::columnIndex_, valueMap[it->get_row_index()]);
			Cell *new_cell = cellPool_.construct(Row_access_option::columnIndex_, valueMap[it->get_row_index()]);
			newSet.insert(newSet.end(), *new_cell);
			auto ittemp = it;
			++it;
			_delete_cell(ittemp);
		} else {
			// Cell *new_cell = new Cell(valueMap[it->get_row_index()]);
			Cell *new_cell = cellPool_.construct(valueMap[it->get_row_index()]);
			newSet.insert(newSet.end(), *new_cell);
			++it;
		}
	}
	//all cells have to be deleted first, to avoid problem with insertion when row is a set
	if constexpr (Row_access_option::isActive_) {
		for (Cell& cell : newSet) {
			Row_access_option::insert_cell(cell.get_row_index(), &cell);
		}
	}
	column_.swap(newSet);
}

template<class Cell_type, class Row_access_option>
inline void Z2_intrusive_set_column<Cell_type,Row_access_option>::clear()
{
	auto it = column_.begin();
	while (it != column_.end()){
		_delete_cell(it);
	}
}

template<class Cell_type, class Row_access_option>
inline typename Z2_intrusive_set_column<Cell_type,Row_access_option>::iterator
Z2_intrusive_set_column<Cell_type,Row_access_option>::begin() noexcept
{
	return column_.begin();
}

template<class Cell_type, class Row_access_option>
inline typename Z2_intrusive_set_column<Cell_type,Row_access_option>::const_iterator
Z2_intrusive_set_column<Cell_type,Row_access_option>::begin() const noexcept
{
	return column_.begin();
}

template<class Cell_type, class Row_access_option>
inline typename Z2_intrusive_set_column<Cell_type,Row_access_option>::iterator
Z2_intrusive_set_column<Cell_type,Row_access_option>::end() noexcept
{
	return column_.end();
}

template<class Cell_type, class Row_access_option>
inline typename Z2_intrusive_set_column<Cell_type,Row_access_option>::const_iterator
Z2_intrusive_set_column<Cell_type,Row_access_option>::end() const noexcept
{
	return column_.end();
}

template<class Cell_type, class Row_access_option>
inline typename Z2_intrusive_set_column<Cell_type,Row_access_option>::reverse_iterator
Z2_intrusive_set_column<Cell_type,Row_access_option>::rbegin() noexcept
{
	return column_.rbegin();
}

template<class Cell_type, class Row_access_option>
inline typename Z2_intrusive_set_column<Cell_type,Row_access_option>::const_reverse_iterator
Z2_intrusive_set_column<Cell_type,Row_access_option>::rbegin() const noexcept
{
	return column_.rbegin();
}

template<class Cell_type, class Row_access_option>
inline typename Z2_intrusive_set_column<Cell_type,Row_access_option>::reverse_iterator
Z2_intrusive_set_column<Cell_type,Row_access_option>::rend() noexcept
{
	return column_.rend();
}

template<class Cell_type, class Row_access_option>
inline typename Z2_intrusive_set_column<Cell_type,Row_access_option>::const_reverse_iterator
Z2_intrusive_set_column<Cell_type,Row_access_option>::rend() const noexcept
{
	return column_.rend();
}

template<class Cell_type, class Row_access_option>
template<class Cell_range>
inline Z2_intrusive_set_column<Cell_type,Row_access_option> &Z2_intrusive_set_column<Cell_type,Row_access_option>::operator+=(Cell_range const &column)
{
	for (const Cell &cell : column) {
		auto it1 = column_.find(cell);
		if (it1 != column_.end()) {
			_delete_cell(it1);
		} else {
			_insert_cell(cell.get_row_index(), column_.end());
		}
	}

	return *this;
}

template<class Cell_type, class Row_access_option>
inline Z2_intrusive_set_column<Cell_type,Row_access_option> &Z2_intrusive_set_column<Cell_type,Row_access_option>::operator*=(unsigned int v)
{
	if (v % 2 == 0){
		clear();
	}

	return *this;
}

template<class Cell_type, class Row_access_option>
inline Z2_intrusive_set_column<Cell_type,Row_access_option> &Z2_intrusive_set_column<Cell_type,Row_access_option>::operator=(const Z2_intrusive_set_column& other)
{
	static_assert (!Row_access_option::isActive_, "= assignement not enabled with row access option.");

	dim_ = other.dim_;
	column_.clone_from(other.column_, new_cloner(), delete_disposer());
	return *this;
}

template<class Cell_type, class Row_access_option>
inline void Z2_intrusive_set_column<Cell_type,Row_access_option>::_delete_cell(iterator &it)
{
	// iterator tmp_it = it;
	// ++it;
	// Cell* tmp_ptr = &(*tmp_it);
	// if constexpr (Row_access_option::isActive_)
	// 	Row_access_option::unlink(tmp_ptr);
	// column_.erase(tmp_it);
	// delete tmp_ptr;
	it = column_.erase_and_dispose(it, delete_disposer(this));
}

template<class Cell_type, class Row_access_option>
inline void Z2_intrusive_set_column<Cell_type,Row_access_option>::_insert_cell(
		index rowIndex, const iterator &position)
{
	if constexpr (Row_access_option::isActive_){
		// Cell *new_cell = new Cell(Row_access_option::columnIndex_, rowIndex);
		Cell *new_cell = cellPool_.construct(Row_access_option::columnIndex_, rowIndex);
		column_.insert(position, *new_cell);
		Row_access_option::insert_cell(rowIndex, new_cell);
	} else {
		// Cell *new_cell = new Cell(rowIndex);
		Cell *new_cell = cellPool_.construct(rowIndex);
		column_.insert(position, *new_cell);
	}
}

} //namespace persistence_matrix
} //namespace Gudhi

template<class Cell_type, class Row_access_option>
struct std::hash<Gudhi::persistence_matrix::Z2_intrusive_set_column<Cell_type,Row_access_option> >
{
	size_t operator()(const Gudhi::persistence_matrix::Z2_intrusive_set_column<Cell_type,Row_access_option>& column) const
	{
		std::size_t seed = 0;
		for (auto& cell : column){
			seed ^= std::hash<unsigned int>()(cell.get_row_index()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
		return seed;
	}
};

#endif // Z2_INTRUSIVE_SET_COLUMN_H
