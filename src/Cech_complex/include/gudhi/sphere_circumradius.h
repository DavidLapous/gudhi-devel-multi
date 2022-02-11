/*    This file is part of the Gudhi Library - https://gudhi.inria.fr/ - which is released under MIT.
 *    See file LICENSE or go to https://gudhi.inria.fr/licensing/ for full license details.
 *    Author(s):       Hind Montassif
 *
 *    Copyright (C) 2021 Inria
 *
 *    Modification(s):
 *      - YYYY/MM Author: Description of the modification
 */

#ifndef SPHERE_CIRCUMRADIUS_H_
#define SPHERE_CIRCUMRADIUS_H_

#include <CGAL/Epeck_d.h> // for #include <CGAL/NewKernel_d/KernelD_converter.h>

#include <cmath>  // for std::sqrt
#include <vector>

namespace Gudhi {

namespace cech_complex {

/** @brief Compute the circumradius of the sphere passing through points given by a range of coordinates.
 * The points are assumed to have the same dimension. */
template<typename Kernel>
class Sphere_circumradius {
 private:
    Kernel kernel_;
 public:
    using Point = typename Kernel::Point_d;
    using Point_cloud = typename std::vector<Point>;

   /** \brief Circumradius of sphere passing through two points using CGAL.
   *
   * @param[in] point_1
   * @param[in] point_2
   * @return Sphere circumradius passing through two points.
   * \tparam Point must be a Kernel::Point_d from CGAL.
   *
   */
  double operator()(const Point& point_1, const Point& point_2) const {
    return std::sqrt(CGAL::to_double(kernel_.squared_distance_d_object()(point_1, point_2))) / 2.;
  }

  /** \brief Circumradius of sphere passing through point cloud using CGAL.
   *
   * @param[in] point_cloud The points.
   * @return Sphere circumradius passing through the points.
   * \tparam Point_cloud must be a range of Kernel::Point_d points from CGAL.
   *
   */
  double operator()(const Point_cloud& point_cloud) const {
    return std::sqrt(CGAL::to_double(kernel_.compute_squared_radius_d_object()(point_cloud.begin(), point_cloud.end())));
  }

};

}  // namespace cech_complex

}  // namespace Gudhi

#endif  // SPHERE_CIRCUMRADIUS_H_
