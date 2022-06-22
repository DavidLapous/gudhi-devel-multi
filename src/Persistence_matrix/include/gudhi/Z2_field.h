/*    This file is part of the Gudhi Library - https://gudhi.inria.fr/ - which is released under MIT.
 *    See file LICENSE or go to https://gudhi.inria.fr/licensing/ for full license details.
 *    Author(s):       Hannah Schreiber
 *
 *    Copyright (C) 2022 Inria
 *
 *    Modification(s):
 *      - YYYY/MM Author: Description of the modification
 */

#ifndef MATRIX_FIELD_Z2_H_
#define MATRIX_FIELD_Z2_H_

#include <utility>

class Z2_field_element {
public:
	Z2_field_element();
	Z2_field_element(int element);
	Z2_field_element(Z2_field_element& toCopy);
	Z2_field_element(Z2_field_element&& toMove) noexcept;

	Z2_field_element& operator+=(Z2_field_element const &f);
	friend Z2_field_element operator+(Z2_field_element f1, Z2_field_element const& f2);

	Z2_field_element& operator-=(Z2_field_element const &f);
	friend Z2_field_element operator-(Z2_field_element f1, Z2_field_element const& f2);

	Z2_field_element& operator*=(Z2_field_element const &f);
	friend Z2_field_element operator*(Z2_field_element f1, Z2_field_element const& f2);

	Z2_field_element& operator=(Z2_field_element other);
	friend void swap(Z2_field_element& f1, Z2_field_element& f2);

	Z2_field_element get_inverse() const;

	static Z2_field_element get_additive_identity();
	static Z2_field_element get_multiplicative_identity();
	static constexpr int get_characteristic();

	unsigned int get_value() const;

private:
	bool element_;
};

inline Z2_field_element::Z2_field_element()
	: element_(false)
{}

inline Z2_field_element::Z2_field_element(int element)
	: element_(element % 2)
{}

inline Z2_field_element::Z2_field_element(Z2_field_element &toCopy)
	: element_(toCopy.element_)
{}

inline Z2_field_element::Z2_field_element(Z2_field_element &&toMove) noexcept
	: element_(std::exchange(toMove.element_, 0))
{}

inline Z2_field_element &Z2_field_element::operator+=(Z2_field_element const &f)
{
	element_ = (element_ != f.element_);
	return *this;
}

inline Z2_field_element &Z2_field_element::operator-=(const Z2_field_element &f)
{
	element_ = (element_ != f.element_);
	return *this;
}

inline Z2_field_element &Z2_field_element::operator*=(const Z2_field_element &f)
{
	element_ = (element_ && f.element_);
	return *this;
}

inline Z2_field_element &Z2_field_element::operator=(Z2_field_element other)
{
	std::swap(element_, other.element_);
	return *this;
}

inline Z2_field_element Z2_field_element::get_inverse() const
{
	return element_ ? Z2_field_element() : Z2_field_element(1);
}

inline Z2_field_element Z2_field_element::get_additive_identity()
{
	return Z2_field_element();
}

inline Z2_field_element Z2_field_element::get_multiplicative_identity()
{
	return Z2_field_element(1);
}

inline constexpr int Z2_field_element::get_characteristic()
{
	return 2;
}

inline unsigned int Z2_field_element::get_value() const
{
	return element_;
}

Z2_field_element operator+(Z2_field_element f1, Z2_field_element const& f2){
	f1 += f2;
	return f1;
}

Z2_field_element operator-(Z2_field_element f1, Z2_field_element const& f2){
	f1 -= f2;
	return f1;
}

Z2_field_element operator*(Z2_field_element f1, Z2_field_element const& f2){
	f1 *= f2;
	return f1;
}

void swap(Z2_field_element& f1, Z2_field_element& f2)
{
	std::swap(f1.element_, f2.element_);
}

#endif  // MATRIX_FIELD_Z2_H_
