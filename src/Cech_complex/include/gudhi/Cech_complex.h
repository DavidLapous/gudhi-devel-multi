/*    This file is part of the Gudhi Library - https://gudhi.inria.fr/ - which is released under MIT.
 *    See file LICENSE or go to https://gudhi.inria.fr/licensing/ for full license details.
 *    Author(s):       Vincent Rouvreau
 *
 *    Copyright (C) 2018 Inria
 *
 *    Modification(s):
 *      - YYYY/MM Author: Description of the modification
 */

#ifndef CECH_COMPLEX_H_
#define CECH_COMPLEX_H_

#include <gudhi/sphere_circumradius.h>       // for Gudhi::cech_complex::Sphere_circumradius
#include <gudhi/graph_simplicial_complex.h>  // for Gudhi::Proximity_graph
#include <gudhi/Debug_utils.h>               // for GUDHI_CHECK
#include <gudhi/Cech_complex_blocker.h>      // for Gudhi::cech_complex::Cech_blocker

#include <iostream>
#include <stdexcept>  // for exception management

namespace Gudhi {

namespace cech_complex {

/**
 * \class Cech_complex
 * \brief Cech complex data structure.
 *
 * \ingroup cech_complex
 *
 * \details
 * The data structure is a proximity graph, containing edges when the edge length is less or equal
 * to a given max_radius. Edge length is computed from `Gudhi::cech_complex::Sphere_circumradius` distance function.
 *
 * \tparam Kernel CGAL kernel.
 *
 * \tparam SimplicialComplexForCechComplex furnishes `Vertex_handle` and `Filtration_value` type definition required
 * by `Gudhi::Proximity_graph` and Cech blocker.
 *
 */
template <typename Kernel, typename SimplicialComplexForCechComplex>
class Cech_complex {
 private:
  // Required by compute_proximity_graph
  using Vertex_handle = typename SimplicialComplexForCechComplex::Vertex_handle;
  using Filtration_value = typename SimplicialComplexForCechComplex::Filtration_value;
  using Proximity_graph = Gudhi::Proximity_graph<SimplicialComplexForCechComplex>;

  using cech_blocker = Cech_blocker<SimplicialComplexForCechComplex, Cech_complex, Kernel>;

  using Point_d = typename cech_blocker::Point_d;
  using Point_cloud = std::vector<Point_d>;

  // Numeric type of coordinates in the kernel
  using FT = typename cech_blocker::FT;
  // Sphere is a pair of point and squared radius.
  using Sphere = typename cech_blocker::Sphere;

  public:
  /** \brief Cech_complex constructor from a list of points.
   *
   * @param[in] points Vector of points where each point is defined as `kernel::Point_d`.
   * @param[in] max_radius Maximal radius value.
   *
   */
  Cech_complex(const Point_cloud & points, Filtration_value max_radius) : max_radius_(max_radius) {

    point_cloud_.assign(points.begin(), points.end());

    cech_skeleton_graph_ = Gudhi::compute_proximity_graph<SimplicialComplexForCechComplex>(
        point_cloud_, max_radius_, Sphere_circumradius<Kernel>());
  }

  /** \brief Initializes the simplicial complex from the proximity graph and expands it until a given maximal
   * dimension, using the Cech blocker oracle.
   *
   * @param[in] complex SimplicialComplexForCech to be created.
   * @param[in] dim_max graph expansion until this given maximal dimension.
   * @exception std::invalid_argument In debug mode, if `complex.num_vertices()` does not return 0.
   *
   */
  void create_complex(SimplicialComplexForCechComplex& complex, int dim_max) {
    GUDHI_CHECK(complex.num_vertices() == 0,
                std::invalid_argument("Cech_complex::create_complex - simplicial complex is not empty"));

    // insert the proximity graph in the simplicial complex
    complex.insert_graph(cech_skeleton_graph_);
    // expand the graph until dimension dim_max
    complex.expansion_with_blockers(dim_max, cech_blocker(&complex, this));
  }

  /** @return max_radius value given at construction. */
  Filtration_value max_radius() const { return max_radius_; }

  /** @param[in] vertex Point position in the range.
   * @return The point.
   */
  const Point_d& get_point(Vertex_handle vertex) const { return point_cloud_[vertex]; }

  /**
   * @return Vector of cached spheres.
   */
  std::vector<Sphere> & get_cache() { return cache_; }

 private:
  Proximity_graph cech_skeleton_graph_;
  Filtration_value max_radius_;
  Point_cloud point_cloud_;
  std::vector<Sphere> cache_;
};

}  // namespace cech_complex

}  // namespace Gudhi

#endif  // CECH_COMPLEX_H_
