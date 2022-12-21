/*    This file is part of the Gudhi Library - https://gudhi.inria.fr/ - which is released under MIT.
 *    See file LICENSE or go to https://gudhi.inria.fr/licensing/ for full license details.
 *    Author(s):       Hannah Schreiber
 *
 *    Copyright (C) 2022 Inria
 *
 *    Modification(s):
 *      - YYYY/MM Author: Description of the modification
 */

#ifndef B_VECTOR_COLUMN_H
#define B_VECTOR_COLUMN_H

#include <iostream>
#include <vector>
#include <unordered_set>

#include "../../utilities/utilities.h"
#include "../vector_column.h"

namespace Gudhi {
namespace persistence_matrix {

template<class Field_element_type, class Cell_type, class Column_pairing_option, class Row_access_option>
class Vector_boundary_column : public Vector_column<Field_element_type,Cell_type,Column_pairing_option,Row_access_option>
{
private:
	using Base = Vector_column<Field_element_type,Cell_type,Column_pairing_option,Row_access_option>;

public:
	using Cell = typename Base::Cell;
	using Column_type = typename Base::Column_type;
	using iterator = typename Base::iterator;
	using const_iterator = typename Base::const_iterator;

	Vector_boundary_column();
	template<class Boundary_type>
	Vector_boundary_column(const Boundary_type& boundary);
	template<class Boundary_type>
	Vector_boundary_column(const Boundary_type& boundary, dimension_type dimension);
	template<class Row_container_type>
	Vector_boundary_column(index columnIndex, Row_container_type &rowContainer);
	template<class Boundary_type, class Row_container_type>
	Vector_boundary_column(index columnIndex, const Boundary_type& boundary, Row_container_type &rowContainer);
	template<class Boundary_type, class Row_container_type>
	Vector_boundary_column(index columnIndex, const Boundary_type& boundary, dimension_type dimension, Row_container_type &rowContainer);
	Vector_boundary_column(const Vector_boundary_column& column);
	Vector_boundary_column(const Vector_boundary_column& column, index columnIndex);
	Vector_boundary_column(Vector_boundary_column&& column) noexcept;

	std::vector<Field_element_type> get_content(unsigned int columnLength);
	bool is_non_zero(index rowIndex) const;
	bool is_empty();
	int get_pivot();
	Field_element_type get_pivot_value();
	void clear();
	void clear(index rowIndex);
	template<class Map_type>
	void reorder(Map_type& valueMap);

	Vector_boundary_column& operator+=(Vector_boundary_column &column);
	friend Vector_boundary_column operator+(Vector_boundary_column column1, Vector_boundary_column& column2){
		column1 += column2;
		return column1;
	}
	Vector_boundary_column& operator*=(unsigned int v);
	friend Vector_boundary_column operator*(Vector_boundary_column column, unsigned int const& v){
		column *= v;
		return column;
	}
	friend Vector_boundary_column operator*(unsigned int const& v, Vector_boundary_column column){
		column *= v;
		return column;
	}

	Vector_boundary_column& operator=(Vector_boundary_column other);

	friend void swap(Vector_boundary_column& col1, Vector_boundary_column& col2){
		swap(static_cast<Vector_column<Field_element_type,Cell_type,Column_pairing_option,Row_access_option>&>(col1),
			 static_cast<Vector_column<Field_element_type,Cell_type,Column_pairing_option,Row_access_option>&>(col2));
		col1.erasedValues_.swap(col2.erasedValues_);
	}

private:
	std::unordered_set<unsigned int> erasedValues_;

	void _clean_values();
};

template<class Field_element_type, class Cell_type, class Column_pairing_option, class Row_access_option>
inline Vector_boundary_column<Field_element_type,Cell_type,Column_pairing_option,Row_access_option>::Vector_boundary_column() : Base()
{}

template<class Field_element_type, class Cell_type, class Column_pairing_option, class Row_access_option>
template<class Boundary_type>
inline Vector_boundary_column<Field_element_type,Cell_type,Column_pairing_option,Row_access_option>::Vector_boundary_column(const Boundary_type &boundary)
	: Base(boundary)
{}

template<class Field_element_type, class Cell_type, class Column_pairing_option, class Row_access_option>
template<class Boundary_type>
inline Vector_boundary_column<Field_element_type,Cell_type,Column_pairing_option,Row_access_option>::Vector_boundary_column(const Boundary_type &boundary, dimension_type dimension)
	: Base(boundary, dimension)
{}

template<class Field_element_type, class Cell_type, class Column_pairing_option, class Row_access_option>
template<class Row_container_type>
inline Vector_boundary_column<Field_element_type,Cell_type,Column_pairing_option,Row_access_option>::Vector_boundary_column(
		index columnIndex, Row_container_type &rowContainer)
	: Base(columnIndex, rowContainer)
{}

template<class Field_element_type, class Cell_type, class Column_pairing_option, class Row_access_option>
template<class Boundary_type, class Row_container_type>
inline Vector_boundary_column<Field_element_type,Cell_type,Column_pairing_option,Row_access_option>::Vector_boundary_column(
		index columnIndex, const Boundary_type& boundary, Row_container_type &rowContainer)
	: Base(columnIndex, boundary, rowContainer)
{}

template<class Field_element_type, class Cell_type, class Column_pairing_option, class Row_access_option>
template<class Boundary_type, class Row_container_type>
inline Vector_boundary_column<Field_element_type,Cell_type,Column_pairing_option,Row_access_option>::Vector_boundary_column(
		index columnIndex, const Boundary_type& boundary, dimension_type dimension, Row_container_type &rowContainer)
	: Base(columnIndex, boundary, dimension, rowContainer)
{}

template<class Field_element_type, class Cell_type, class Column_pairing_option, class Row_access_option>
inline Vector_boundary_column<Field_element_type,Cell_type,Column_pairing_option,Row_access_option>::Vector_boundary_column(const Vector_boundary_column &column)
	: Base(static_cast<const Base&>(column)),
	  erasedValues_(column.erasedValues_)
{}

template<class Field_element_type, class Cell_type, class Column_pairing_option, class Row_access_option>
inline Vector_boundary_column<Field_element_type,Cell_type,Column_pairing_option,Row_access_option>::Vector_boundary_column(
		const Vector_boundary_column& column, index columnIndex)
	: Base(static_cast<const Base&>(column), columnIndex),
	  erasedValues_(column.erasedValues_)
{}

template<class Field_element_type, class Cell_type, class Column_pairing_option, class Row_access_option>
inline Vector_boundary_column<Field_element_type,Cell_type,Column_pairing_option,Row_access_option>::Vector_boundary_column(Vector_boundary_column &&column) noexcept
	: Base(std::move(static_cast<Base&&>(column))),
	  erasedValues_(std::move(column.erasedValues_))
{}

template<class Field_element_type, class Cell_type, class Column_pairing_option, class Row_access_option>
inline std::vector<Field_element_type> Vector_boundary_column<Field_element_type,Cell_type,Column_pairing_option,Row_access_option>::get_content(unsigned int columnLength)
{
	_clean_values();
	return Base::get_content(columnLength);
}

template<class Field_element_type, class Cell_type, class Column_pairing_option, class Row_access_option>
inline bool Vector_boundary_column<Field_element_type,Cell_type,Column_pairing_option,Row_access_option>::is_non_zero(index rowIndex) const
{
	if (erasedValues_.find(rowIndex) != erasedValues_.end()) return false;

	return Base::is_non_zero(rowIndex);
}

template<class Field_element_type, class Cell_type, class Column_pairing_option, class Row_access_option>
inline bool Vector_boundary_column<Field_element_type,Cell_type,Column_pairing_option,Row_access_option>::is_empty()
{
	_clean_values();
	return Base::column_.empty();
}

template<class Field_element_type, class Cell_type, class Column_pairing_option, class Row_access_option>
inline int Vector_boundary_column<Field_element_type,Cell_type,Column_pairing_option,Row_access_option>::get_pivot()
{
	if (Base::column_.empty()) return -1;

	auto it = erasedValues_.find(Base::column_.back()->get_row_index());
	while (!Base::column_.empty() && it != erasedValues_.end()) {
		erasedValues_.erase(it);
		Base::_delete_cell(Base::column_.back());
		Base::column_.pop_back();
		if (!Base::column_.empty()) it = erasedValues_.find(Base::column_.back()->get_row_index());
	}

	if (Base::column_.empty()) return -1;

	return Base::column_.back()->get_row_index();
}

template<class Field_element_type, class Cell_type, class Column_pairing_option, class Row_access_option>
inline Field_element_type Vector_boundary_column<Field_element_type,Cell_type,Column_pairing_option,Row_access_option>::get_pivot_value()
{
	if (Base::column_.empty()) return 0;

	auto it = erasedValues_.find(Base::column_.back()->get_row_index());
	while (!Base::column_.empty() && it != erasedValues_.end()) {
		erasedValues_.erase(it);
		Base::_delete_cell(Base::column_.back());
		Base::column_.pop_back();
		if (!Base::column_.empty()) it = erasedValues_.find(Base::column_.back()->get_row_index());
	}

	if (Base::column_.empty()) return 0;

	return Base::column_.back()->get_element();
}

template<class Field_element_type, class Cell_type, class Column_pairing_option, class Row_access_option>
inline void Vector_boundary_column<Field_element_type,Cell_type,Column_pairing_option,Row_access_option>::clear()
{
	for (Cell* cell : Base::column_){
		Base::_delete_cell(cell);
	}
	Base::column_.clear();
	erasedValues_.clear();
}

template<class Field_element_type, class Cell_type, class Column_pairing_option, class Row_access_option>
inline void Vector_boundary_column<Field_element_type,Cell_type,Column_pairing_option,Row_access_option>::clear(index rowIndex)
{
	erasedValues_.insert(rowIndex);
}

template<class Field_element_type, class Cell_type, class Column_pairing_option, class Row_access_option>
template<class Map_type>
inline void Vector_boundary_column<Field_element_type,Cell_type,Column_pairing_option,Row_access_option>::reorder(Map_type &valueMap)
{
	Column_type newColumn;
	for (Cell* v : Base::column_) {
		if (erasedValues_.find(v->get_row_index()) == erasedValues_.end()){
			v->set_row_index(valueMap[v->get_row_index()]);
			newColumn.push_back(v);
			if constexpr (Row_access_option::isActive_){
				Row_access_option::unlink(v);
				Row_access_option::insert_cell(v->get_row_index(), v);
			}
		} else {
			Base::_delete_cell(v);
		}
	}
	std::sort(newColumn.begin(), newColumn.end(), [](const Cell* c1, const Cell* c2){return *c1 < *c2;});
	erasedValues_.clear();
	Base::column_.swap(newColumn);
}

template<class Field_element_type, class Cell_type, class Column_pairing_option, class Row_access_option>
inline Vector_boundary_column<Field_element_type,Cell_type,Column_pairing_option,Row_access_option> &
Vector_boundary_column<Field_element_type,Cell_type,Column_pairing_option,Row_access_option>::operator+=(Vector_boundary_column &column)
{
	_clean_values();
	column._clean_values();
	Base::operator+=(column);

	return *this;
}

template<class Field_element_type, class Cell_type, class Column_pairing_option, class Row_access_option>
inline Vector_boundary_column<Field_element_type,Cell_type,Column_pairing_option,Row_access_option> &
Vector_boundary_column<Field_element_type,Cell_type,Column_pairing_option,Row_access_option>::operator*=(unsigned int v)
{
	Base::operator*=(v);

	if (Base::column_.empty())
		erasedValues_.clear();

	return *this;
}

template<class Field_element_type, class Cell_type, class Column_pairing_option, class Row_access_option>
inline Vector_boundary_column<Field_element_type,Cell_type,Column_pairing_option,Row_access_option> &
Vector_boundary_column<Field_element_type,Cell_type,Column_pairing_option,Row_access_option>::operator=(Vector_boundary_column other)
{
	Base::operator=(static_cast<Base&>(other));
	erasedValues_.swap(other.erasedValues_);
	return *this;
}

template<class Field_element_type, class Cell_type, class Column_pairing_option, class Row_access_option>
inline void Vector_boundary_column<Field_element_type,Cell_type,Column_pairing_option,Row_access_option>::_clean_values()
{
	if (erasedValues_.empty()) return;

	Column_type newColumn;
	for (Cell* v : Base::column_){
		if (erasedValues_.find(v->get_row_index()) == erasedValues_.end())
			newColumn.push_back(v);
		else
			Base::_delete_cell(v);
	}
	erasedValues_.clear();
	Base::column_.swap(newColumn);
}

} //namespace persistence_matrix
} //namespace Gudhi

#endif // B_VECTOR_COLUMN_H
