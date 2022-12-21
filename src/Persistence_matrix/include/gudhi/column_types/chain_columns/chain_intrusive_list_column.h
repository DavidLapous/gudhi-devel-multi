/*    This file is part of the Gudhi Library - https://gudhi.inria.fr/ - which is released under MIT.
 *    See file LICENSE or go to https://gudhi.inria.fr/licensing/ for full license details.
 *    Author(s):       Hannah Schreiber
 *
 *    Copyright (C) 2022 Inria
 *
 *    Modification(s):
 *      - YYYY/MM Author: Description of the modification
 */

#ifndef C_INTRUSIVE_LIST_COLUMN_H
#define C_INTRUSIVE_LIST_COLUMN_H

#include <unordered_map>
#include <iostream>

#include <boost/intrusive/list.hpp>

#include "../../utilities/utilities.h"
#include "../intrusive_list_column.h"

namespace Gudhi {
namespace persistence_matrix {

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Column_pairing_option, class Row_access_option>
class Intrusive_list_chain_column : public Intrusive_list_column<Field_element_type,Cell_type,Column_pairing_option,Row_access_option>
{
private:
	using Base = Intrusive_list_column<Field_element_type,Cell_type,Column_pairing_option,Row_access_option>;

public:
	using Cell = typename Base::Cell;
	using Column_type = typename Base::Column_type;
	using iterator = typename Base::iterator;
	using const_iterator = typename Base::const_iterator;

	Intrusive_list_chain_column(Dictionnary_type& pivotToColumnIndex);
	template<class Chain_type>
	Intrusive_list_chain_column(const Chain_type& chain, dimension_type dimension, Dictionnary_type& pivotToColumnIndex);
	template<class Row_container_type>
	Intrusive_list_chain_column(index columnIndex, Row_container_type &rowContainer, Dictionnary_type& pivotToColumnIndex);
	template<class Chain_type, class Row_container_type>
	Intrusive_list_chain_column(index columnIndex, const Chain_type& chain, dimension_type dimension, Row_container_type &rowContainer, Dictionnary_type& pivotToColumnIndex);
	Intrusive_list_chain_column(const Intrusive_list_chain_column& column);
	Intrusive_list_chain_column(const Intrusive_list_chain_column& column, index columnIndex);
	Intrusive_list_chain_column(Intrusive_list_chain_column&& column) noexcept;

	int get_pivot() const;
	Field_element_type get_pivot_value();

	Intrusive_list_chain_column& operator+=(Intrusive_list_chain_column &column);
	friend Intrusive_list_chain_column operator+(Intrusive_list_chain_column column1, Intrusive_list_chain_column &column2){
		column1 += column2;
		return column1;
	}
	friend Intrusive_list_chain_column operator*(Intrusive_list_chain_column column, unsigned int const& v){
		column *= v;
		return column;
	}
	friend Intrusive_list_chain_column operator*(unsigned int const& v, Intrusive_list_chain_column column){
		column *= v;
		return column;
	}

	Intrusive_list_chain_column& operator=(const Intrusive_list_chain_column& other);

	friend void swap(Intrusive_list_chain_column& col1, Intrusive_list_chain_column& col2){
		swap(static_cast<Intrusive_list_column<Field_element_type,Cell_type,Column_pairing_option,Row_access_option>&>(col1),
			 static_cast<Intrusive_list_column<Field_element_type,Cell_type,Column_pairing_option,Row_access_option>&>(col2));
	}

private:
	Dictionnary_type* pivotToColumnIndex_;
	int pivot_;		//simplex index associated to the chain
};

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Column_pairing_option, class Row_access_option>
inline Intrusive_list_chain_column<Dictionnary_type,Field_element_type,Cell_type,Column_pairing_option,Row_access_option>::Intrusive_list_chain_column(
		Dictionnary_type& pivotToColumnIndex)
	: Base(), pivotToColumnIndex_(&pivotToColumnIndex), pivot_(-1)
{}

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Column_pairing_option, class Row_access_option>
template<class Chain_type>
inline Intrusive_list_chain_column<Dictionnary_type,Field_element_type,Cell_type,Column_pairing_option,Row_access_option>::Intrusive_list_chain_column(
		const Chain_type& chain, dimension_type dimension, Dictionnary_type& pivotToColumnIndex)
	: Base(chain, dimension), pivotToColumnIndex_(&pivotToColumnIndex), pivot_(chain.empty() ? -1 : chain.rbegin()->first)
{}

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Column_pairing_option, class Row_access_option>
template<class Row_container_type>
inline Intrusive_list_chain_column<Dictionnary_type,Field_element_type,Cell_type,Column_pairing_option,Row_access_option>::Intrusive_list_chain_column(
		index columnIndex, Row_container_type &rowContainer, Dictionnary_type &pivotToColumnIndex)
	: Base(columnIndex, rowContainer), pivotToColumnIndex_(&pivotToColumnIndex), pivot_(-1)
{}

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Column_pairing_option, class Row_access_option>
template<class Chain_type, class Row_container_type>
inline Intrusive_list_chain_column<Dictionnary_type,Field_element_type,Cell_type,Column_pairing_option,Row_access_option>::Intrusive_list_chain_column(
		index columnIndex, const Chain_type& chain, dimension_type dimension, Row_container_type &rowContainer, Dictionnary_type &pivotToColumnIndex)
	: Base(columnIndex, chain, dimension, rowContainer), pivotToColumnIndex_(&pivotToColumnIndex), pivot_(chain.empty() ? -1 : chain.rbegin()->first)
{}

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Column_pairing_option, class Row_access_option>
inline Intrusive_list_chain_column<Dictionnary_type,Field_element_type,Cell_type,Column_pairing_option,Row_access_option>::Intrusive_list_chain_column(
		const Intrusive_list_chain_column& column)
	: Base(static_cast<const Base&>(column)),
	  pivotToColumnIndex_(column.pivotToColumnIndex_),
	  pivot_(column.pivot_)
{}

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Column_pairing_option, class Row_access_option>
inline Intrusive_list_chain_column<Dictionnary_type,Field_element_type,Cell_type,Column_pairing_option,Row_access_option>::Intrusive_list_chain_column(
		const Intrusive_list_chain_column& column, index columnIndex)
	: Base(static_cast<const Base&>(column), columnIndex),
	  pivotToColumnIndex_(column.pivotToColumnIndex_),
	  pivot_(column.pivot_)
{}

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Column_pairing_option, class Row_access_option>
inline Intrusive_list_chain_column<Dictionnary_type,Field_element_type,Cell_type,Column_pairing_option,Row_access_option>::Intrusive_list_chain_column(
		Intrusive_list_chain_column&& column) noexcept
	: Base(std::move(static_cast<Base&&>(column))),
	  pivotToColumnIndex_(std::move(column.pivotToColumnIndex_)),
	  pivot_(std::exchange(column.pivot_, -1))
{}

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Column_pairing_option, class Row_access_option>
inline int Intrusive_list_chain_column<Dictionnary_type,Field_element_type,Cell_type,Column_pairing_option,Row_access_option>::get_pivot() const
{
	return pivot_;
}

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Column_pairing_option, class Row_access_option>
inline Field_element_type Intrusive_list_chain_column<Dictionnary_type,Field_element_type,Cell_type,Column_pairing_option,Row_access_option>::get_pivot_value()
{
	if (pivot_ == -1) return Field_element_type();

	for (const Cell& cell : Base::column_){
		if (cell.get_row_index() == pivot_) return cell.get_element();
	}

	return Field_element_type();	//should never happen if chain column is used properly
}

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Column_pairing_option, class Row_access_option>
inline Intrusive_list_chain_column<Dictionnary_type,Field_element_type,Cell_type,Column_pairing_option,Row_access_option> &
Intrusive_list_chain_column<Dictionnary_type,Field_element_type,Cell_type,Column_pairing_option,Row_access_option>::operator+=(Intrusive_list_chain_column &column)
{
	Base::operator+=(column);

	//assumes that the addition never zeros out this column. If the use of those columns changes at some point, we should think about it.
	if (!Base::is_non_zero(pivot_)){
		std::swap(pivotToColumnIndex_->at(pivot_),
				  pivotToColumnIndex_->at(column.get_pivot()));
		std::swap(pivot_, column.pivot_);
	}

	return *this;
}

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Column_pairing_option, class Row_access_option>
inline Intrusive_list_chain_column<Dictionnary_type,Field_element_type,Cell_type,Column_pairing_option,Row_access_option> &
Intrusive_list_chain_column<Dictionnary_type,Field_element_type,Cell_type,Column_pairing_option,Row_access_option>::operator=(const Intrusive_list_chain_column &other)
{
	Base::operator=(static_cast<const Base&>(other));
	pivotToColumnIndex_ = other.pivotToColumnIndex_;
	pivot_ = other.pivot_;
	return *this;
}

} //namespace persistence_matrix
} //namespace Gudhi

#endif // C_INTRUSIVE_LIST_COLUMN_H
