/*    This file is part of the Gudhi Library - https://gudhi.inria.fr/ - which is released under MIT.
 *    See file LICENSE or go to https://gudhi.inria.fr/licensing/ for full license details.
 *    Author(s):       Hannah Schreiber
 *
 *    Copyright (C) 2023 Inria
 *
 *    Modification(s):
 *      - YYYY/MM Author: Description of the modification
 */

#include <vector>
#include <limits>

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "zigzag_persistence"
#include <boost/test/unit_test.hpp>

#include <gudhi/Zigzag_persistence.h>

using ZP = Gudhi::zigzag_persistence::Zigzag_persistence<>;
using face_handle = ZP::face_key;
using filtration_value = ZP::filtration_value;
using Interval_index = ZP::Index_interval;
using Interval_filtration = ZP::Filtration_value_interval;

struct cmp_intervals_by_length {
  cmp_intervals_by_length() {}
  bool operator()(Interval_filtration p, Interval_filtration q) {
    if (p.length() != q.length()) {
      return p.length() > q.length();
    }
    if (p.dim() != q.dim()) {
      return p.dim() < q.dim();
    }
    if (p.birth() != q.birth()) {
      return p.birth() < q.birth();
    }
    return p.death() < q.death();
  }
};

BOOST_AUTO_TEST_CASE(constructor) {
  BOOST_CHECK_NO_THROW(ZP zp);
  BOOST_CHECK_NO_THROW(ZP zp(28));
  BOOST_CHECK_NO_THROW(ZP zp(28, 2));
  ZP zp;
  BOOST_CHECK(zp.get_persistence_diagram(0).empty());
}

void test_barcode(ZP& zp, std::vector<Interval_filtration>& barcode) {
  auto bars = zp.get_persistence_diagram(0, true);
  std::stable_sort(bars.begin(), bars.end(), cmp_intervals_by_length());
  std::stable_sort(barcode.begin(), barcode.end(), cmp_intervals_by_length());
  auto it = barcode.begin();
  for (const auto& interval : bars) {
    BOOST_CHECK_EQUAL(interval.dim(), it->dim());
    BOOST_CHECK_EQUAL(interval.birth(), it->birth());
    BOOST_CHECK_EQUAL(interval.death(), it->death());
    ++it;
  }
  BOOST_CHECK(it == barcode.end());
}

void test_indices(ZP& zp, std::vector<Interval_index>& indices, std::vector<filtration_value>& indexToFil) {
  auto it = indices.begin();
  for (const auto& interval : zp.get_index_persistence_diagram()) {
    BOOST_CHECK_EQUAL(interval.dim(), it->dim());
    BOOST_CHECK_EQUAL(interval.birth(), it->birth());
    BOOST_CHECK_EQUAL(interval.death(), it->death());
    auto p = zp.map_index_to_filtration_value(interval.birth(), interval.death());
    BOOST_CHECK_EQUAL(p.first, indexToFil[interval.birth()]);
    BOOST_CHECK_EQUAL(p.second, indexToFil[interval.death()]);
    ++it;
  }
  BOOST_CHECK(it == indices.end());
}

std::vector<std::vector<face_handle> > get_simplices() {
  return {{0},
          {1},
          {2},
          {0, 1},
          {0, 2},
          {3},
          {1, 2},
          {4},
          {3, 4},
          {5},
          {0, 1, 2},
          {4, 5},
          {3, 5},
          {3, 4, 5},
          {0, 1, 2},            // remove
          {3, 4, 5},            // remove
          {1, 4},
          {0, 1, 2},
          {2, 4},
          {3, 4, 5},
          {0, 4},
          {0, 2, 4},
          {1, 2, 4},
          {0, 1, 4},
          {3, 4, 5},            // remove
          {3, 4},                   // remove
          {3, 5},                   // remove
          {0, 1, 2, 4},
          {0, 1, 2, 4}};    // remove
}

std::vector<std::vector<face_handle> > get_boundaries() {
  return {{},
          {},
          {},
          {0, 1},
          {0, 2},
          {},
          {1, 2},
          {},
          {5, 7},
          {},
          {3, 4, 6},
          {7, 9},
          {5, 9},
          {8, 11, 12},
          {10},                         // remove
          {13},                         // remove
          {1, 7},
          {3, 4, 6},
          {2, 7},
          {8, 11, 12},
          {0, 7},
          {4, 18, 20},
          {6, 16, 18},
          {3, 16, 20},
          {19},                         // remove
          {8},                          // remove
          {12},                         // remove
          {17, 21, 22, 23},
          {27}};                        // remove
}

std::vector<filtration_value> get_filtration_values() {
  return {0, 0, 0, 
          1, 1, 1, 
          2, 2, 2, 
          3, 3, 3, 3, 
          4, 
          5, 
          6, 6, 6, 
          7, 7, 7, 7, 7, 7, 
          8, 
          9, 9, 9, 
          10};
}

BOOST_AUTO_TEST_CASE(zigzag_persistence_single) {
  ZP zp(28);
  std::vector<Interval_index> realIndices;
  std::vector<Interval_filtration> realBarcode;
  realIndices.reserve(13);
  realBarcode.reserve(9);

  std::vector<std::vector<face_handle> > simplices = get_boundaries();
  std::vector<filtration_value> filValues = get_filtration_values();

  for (unsigned int i = 0; i < 14; ++i) {
    zp.insert_face(i, simplices[i], simplices[i].size() == 0 ? 0 : simplices[i].size() - 1, filValues[i]);
  }

  realIndices.emplace_back(0, 1, 3);
  realIndices.emplace_back(0, 2, 4);
  realIndices.emplace_back(0, 7, 8);
  realIndices.emplace_back(1, 6, 10);
  realIndices.emplace_back(0, 9, 11);
  realIndices.emplace_back(1, 12, 13);

  realBarcode.emplace_back(0, 0, 1);
  realBarcode.emplace_back(0, 0, 1);
  realBarcode.emplace_back(1, 2, 3);
  realBarcode.emplace_back(1, 3, 4);

  for (unsigned int i = 14; i < 16; ++i) {
    auto id = simplices[i][0];
    zp.remove_face(id, simplices[id].size() == 0 ? 0 : simplices[id].size() - 1, filValues[i]);
  }

  for (unsigned int i = 16; i < 24; ++i) {
    zp.insert_face(i, simplices[i], simplices[i].size() == 0 ? 0 : simplices[i].size() - 1, filValues[i]);
  }

  realIndices.emplace_back(0, 5, 16);
  realIndices.emplace_back(1, 14, 17);
  realIndices.emplace_back(1, 15, 19);
  realIndices.emplace_back(1, 20, 21);
  realIndices.emplace_back(1, 18, 22);

  realBarcode.emplace_back(0, 1, 6);
  realBarcode.emplace_back(1, 5, 6);
  realBarcode.emplace_back(1, 6, 7);

  for (unsigned int i = 24; i < 27; ++i) {
    auto id = simplices[i][0];
    zp.remove_face(id, simplices[id].size() == 0 ? 0 : simplices[id].size() - 1, filValues[i]);
  }

  realIndices.emplace_back(1, 24, 25);
  realBarcode.emplace_back(1, 8, 9);

  zp.insert_face(27, simplices[27], simplices[27].size() == 0 ? 0 : simplices[27].size() - 1, filValues[27]);

  realIndices.emplace_back(2, 23, 27);
  realBarcode.emplace_back(2, 7, 9);

  auto id = simplices[28][0];
  zp.remove_face(id, simplices[id].size() == 0 ? 0 : simplices[id].size() - 1, filValues[28]);

  realBarcode.emplace_back(0, 0, std::numeric_limits<filtration_value>::infinity());
  realBarcode.emplace_back(0, 9, std::numeric_limits<filtration_value>::infinity());
  realBarcode.emplace_back(2, 10, std::numeric_limits<filtration_value>::infinity());

  test_indices(zp, realIndices, filValues);
  test_barcode(zp, realBarcode);
}

BOOST_AUTO_TEST_CASE(zigzag_persistence_single_max1) {
  ZP zp(28, 1);
  std::vector<Interval_index> realIndices;
  std::vector<filtration_value> indexToFil(28);
  std::vector<Interval_filtration> realBarcode;
  realIndices.reserve(5);
  realBarcode.reserve(3);

  std::vector<std::vector<face_handle> > simplices = get_boundaries();
  std::vector<filtration_value> filValues = get_filtration_values();
  unsigned int currIndex = 0;

  for (unsigned int i = 0; i < 14; ++i) {
    zp.insert_face(i, simplices[i], simplices[i].size() == 0 ? 0 : simplices[i].size() - 1, filValues[i]);
    if (simplices[i].size() < 3) {
      indexToFil[currIndex++] = filValues[i];
    }
  }

  realIndices.emplace_back(0, 1, 3);
  realIndices.emplace_back(0, 2, 4);
  realIndices.emplace_back(0, 7, 8);
  realIndices.emplace_back(0, 9, 10);

  realBarcode.emplace_back(0, 0, 1);
  realBarcode.emplace_back(0, 0, 1);

  for (unsigned int i = 14; i < 16; ++i) {
    auto id = simplices[i][0];
    zp.remove_face(id, simplices[id].size() == 0 ? 0 : simplices[id].size() - 1, filValues[i]);
    if (simplices[id].size() < 3) {
      indexToFil[currIndex++] = filValues[i];
    }
  }

  for (unsigned int i = 16; i < 24; ++i) {
    zp.insert_face(i, simplices[i], simplices[i].size() == 0 ? 0 : simplices[i].size() - 1, filValues[i]);
    if (simplices[i].size() < 3) {
      indexToFil[currIndex++] = filValues[i];
    }
  }

  realIndices.emplace_back(0, 5, 12);
  realBarcode.emplace_back(0, 1, 6);

  for (unsigned int i = 24; i < 27; ++i) {
    auto id = simplices[i][0];
    zp.remove_face(id, simplices[id].size() == 0 ? 0 : simplices[id].size() - 1, filValues[i]);
    if (simplices[id].size() < 3) {
      indexToFil[currIndex++] = filValues[i];
    }
  }

  zp.insert_face(27, simplices[27], simplices[27].size() == 0 ? 0 : simplices[27].size() - 1, filValues[27]);
  auto id = simplices[28][0];
  zp.remove_face(id, simplices[id].size() == 0 ? 0 : simplices[id].size() - 1, filValues[28]);

  realBarcode.emplace_back(0, 0, std::numeric_limits<filtration_value>::infinity());
  realBarcode.emplace_back(0, 9, std::numeric_limits<filtration_value>::infinity());

  test_indices(zp, realIndices, indexToFil);
  test_barcode(zp, realBarcode);
}
