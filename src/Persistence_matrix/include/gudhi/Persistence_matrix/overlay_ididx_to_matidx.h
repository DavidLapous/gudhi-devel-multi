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
 * @file overlay_ididx_to_matidx.h
 * @author Hannah Schreiber
 * @brief Contains the @ref Gudhi::persistence_matrix::Id_to_index_overlay class.
 */

#ifndef PM_ID_TO_POS_TRANSLATION_H
#define PM_ID_TO_POS_TRANSLATION_H

#include <cmath>
#include <vector>
#include <cassert>
#include <utility>    //std::swap, std::move & std::exchange
#include <algorithm>  //std::transform
#include <stdexcept>  //std::invalid_argument

namespace Gudhi {
namespace persistence_matrix {

/**
 * @class Id_to_index_overlay overlay_ididx_to_matidx.h gudhi/Persistence_matrix/overlay_ididx_to_matidx.h
 * @ingroup persistence_matrix
 *
 * @brief Overlay for @ref mp_matrices "non-basic matrices" replacing all input and output @ref MatIdx indices of
 * the original methods with @ref IDIdx indices.
 * 
 * @tparam Matrix_type %Matrix type taking the overlay.
 * @tparam Master_matrix_type An instantiation of @ref Matrix from which all types and options are deduced.
 */
template <class Matrix_type, class Master_matrix_type>
class Id_to_index_overlay 
{
 public:
  using index = typename Master_matrix_type::index;                       /**< @ref MatIdx index type. */
  using id_index = typename Master_matrix_type::id_index;                 /**< @ref IDIdx index type. */
  using pos_index = typename Master_matrix_type::pos_index;               /**< @ref PosIdx index type. */
  using dimension_type = typename Master_matrix_type::dimension_type;     /**< Dimension value type. */
  /**
   * @brief Field operators class. Necessary only if @ref PersistenceMatrixOptions::is_z2 is false.
   */
  using Field_operators = typename Master_matrix_type::Field_operators;
  using Field_element_type = typename Master_matrix_type::element_type;   /**< Type of an field element. */
  using boundary_type = typename Master_matrix_type::boundary_type;       /**< Type of an input column. */
  using Column_type = typename Master_matrix_type::Column_type;           /**< Column type. */
  using Row_type = typename Master_matrix_type::Row_type;                 /**< Row type,
                                                                               only necessary with row access option. */
  using bar_type = typename Master_matrix_type::Bar;                      /**< Bar type. */
  using barcode_type = typename Master_matrix_type::barcode_type;         /**< Barcode type. */
  using cycle_type = typename Master_matrix_type::cycle_type;             /**< Cycle type. */
  using Cell_constructor = typename Master_matrix_type::Cell_constructor; /**< Factory of @ref Cell classes. */
  using Column_settings = typename Master_matrix_type::Column_settings;   /**< Structure giving access to the columns to
                                                                               necessary external classes. */

  /**
   * @brief Constructs an empty matrix.
   * 
   * @param colSettings Pointer to an existing setting structure for the columns. The structure should contain all
   * the necessary external classes specifically necessary for the choosen column type, such as custom allocators.
   */
  Id_to_index_overlay(Column_settings* colSettings);
  /**
   * @brief Constructs a new matrix from the given ranges of @ref Matrix::cell_rep_type. Each range corresponds to a
   * column (the order of the ranges are preserved). The content of the ranges is assumed to be sorted by increasing
   * IDs. The IDs of the simplices are also assumed to be consecutive, ordered by filtration value, starting with 0.
   * 
   * @tparam Boundary_type Range type for @ref Matrix::cell_rep_type ranges.
   * Assumed to have a begin(), end() and size() method.
   * @param orderedBoundaries Range of boundaries: @p orderedBoundaries is interpreted as a boundary matrix of a 
   * filtered **simplicial** complex, whose boundaries are ordered by filtration order. 
   * Therefore, `orderedBoundaries[i]` should store the boundary of the \f$ i^{th} \f$ simplex in the filtration,
   * as an ordered list of indices of its facets (again those indices correspond to their respective position
   * in the matrix). That is why the indices of the simplices are assumed to be consecutive and starting with 0 
   * (an empty boundary is interpreted as a vertex boundary and not as a non existing simplex). 
   * All dimensions up to the maximal dimension of interest have to be present. If only a higher dimension is of 
   * interest and not everything should be stored, then use the @ref insert_boundary method instead
   * (after creating the matrix with the @ref Id_to_index_overlay(unsigned int, Column_settings*)
   * constructor preferably).
   * @param colSettings Pointer to an existing setting structure for the columns. The structure should contain all
   * the necessary external classes specifically necessary for the choosen column type, such as custom allocators.
   */
  template <class Boundary_type = boundary_type>
  Id_to_index_overlay(const std::vector<Boundary_type>& orderedBoundaries, 
                      Column_settings* colSettings);
  /**
   * @brief Constructs a new empty matrix and reserves space for the given number of columns.
   * 
   * @param numberOfColumns Number of columns to reserve space for.
   * @param colSettings Pointer to an existing setting structure for the columns. The structure should contain all
   * the necessary external classes specifically necessary for the choosen column type, such as custom allocators.
   */
  Id_to_index_overlay(unsigned int numberOfColumns, Column_settings* colSettings);
  /**
   * @brief Only available for @ref chainmatrix "chain matrices". Constructs an empty matrix and stores the given
   * comparators.
   *
   * @warning If @ref PersistenceMatrixOptions::has_vine_update is false, the comparators are not used.
   * And if @ref PersistenceMatrixOptions::has_vine_update is true, but
   * @ref PersistenceMatrixOptions::has_column_pairings is also true, the comparators are ignored and
   * the current barcode is used to compare birth and deaths. Therefore it is useless to provide them in those cases.
   * 
   * @tparam BirthComparatorFunction Type of the birth comparator: (@ref pos_index, @ref pos_index) -> bool
   * @tparam DeathComparatorFunction Type of the death comparator: (@ref pos_index, @ref pos_index) -> bool
   * @param colSettings Pointer to an existing setting structure for the columns. The structure should contain all
   * the necessary external classes specifically necessary for the choosen column type, such as custom allocators.
   * @param birthComparator Method taking two @ref PosIdx indices as input and returning true if and only if
   * the birth associated to the first position is strictly less than birth associated to
   * the second one with respect to some self defined order. It is used while swapping two unpaired or
   * two negative columns.
   * @param deathComparator Method taking two @ref PosIdx indices as input and returning true if and only if
   * the death associated to the first position is strictly less than death associated to
   * the second one with respect to some self defined order. It is used while swapping two positive but paired
   * columns.
   */
  template <typename BirthComparatorFunction, typename DeathComparatorFunction>
  Id_to_index_overlay(Column_settings* colSettings,
                      const BirthComparatorFunction& birthComparator, 
                      const DeathComparatorFunction& deathComparator);
  /**
   * @brief Only available for @ref chainmatrix "chain matrices". 
   * Constructs a new matrix from the given ranges of @ref Matrix::cell_rep_type. Each range corresponds to a column 
   * (the order of the ranges are preserved). The content of the ranges is assumed to be sorted by increasing IDs.
   * The IDs of the simplices are also assumed to be consecutive, ordered by filtration value, starting with 0. 
   *
   * @warning If @ref PersistenceMatrixOptions::has_vine_update is false, the comparators are not used.
   * And if @ref PersistenceMatrixOptions::has_vine_update is true, but
   * @ref PersistenceMatrixOptions::has_column_pairings is also true, the comparators are ignored and
   * the current barcode is used to compare birth and deaths. Therefore it is useless to provide them in those cases.
   * 
   * @tparam BirthComparatorFunction Type of the birth comparator: (@ref pos_index, @ref pos_index) -> bool
   * @tparam DeathComparatorFunction Type of the death comparator: (@ref pos_index, @ref pos_index) -> bool
   * @tparam Boundary_type  Range type for @ref Matrix::cell_rep_type ranges.
   * Assumed to have a begin(), end() and size() method.
   * @param orderedBoundaries Range of boundaries: @p orderedBoundaries is interpreted as a boundary matrix of a 
   * filtered **simplicial** complex, whose boundaries are ordered by filtration order. 
   * Therefore, `orderedBoundaries[i]` should store the boundary of the \f$ i^{th} \f$ simplex in the filtration,
   * as an ordered list of indices of its facets (again those indices correspond to their respective position
   * in the matrix). That is why the indices of the simplices are assumed to be consecutive and starting with 0 
   * (an empty boundary is interpreted as a vertex boundary and not as a non existing simplex). 
   * All dimensions up to the maximal dimension of interest have to be present. If only a higher dimension is of 
   * interest and not everything should be stored, then use the @ref insert_boundary method instead
   * (after creating the matrix with the @ref Id_to_index_overlay(unsigned int, Column_settings*,
   * const BirthComparatorFunction&, const DeathComparatorFunction&) constructor preferably).
   * @param colSettings Pointer to an existing setting structure for the columns. The structure should contain all
   * the necessary external classes specifically necessary for the choosen column type, such as custom allocators.
   * @param birthComparator Method taking two @ref PosIdx indices as input and returning true if and only if
   * the birth associated to the first position is strictly less than birth associated to
   * the second one with respect to some self defined order. It is used while swapping two unpaired or
   * two negative columns.
   * @param deathComparator Method taking two @ref PosIdx indices as input and returning true if and only if
   * the death associated to the first position is strictly less than death associated to
   * the second one with respect to some self defined order. It is used while swapping two positive but paired
   * columns.
   */
  template <typename BirthComparatorFunction, typename DeathComparatorFunction, class Boundary_type>
  Id_to_index_overlay(const std::vector<Boundary_type>& orderedBoundaries, 
                      Column_settings* colSettings, 
                      const BirthComparatorFunction& birthComparator, 
                      const DeathComparatorFunction& deathComparator);
  /**
   * @brief Only available for @ref chainmatrix "chain matrices".
   * Constructs a new empty matrix and reserves space for the given number of columns.
   *
   * @warning If @ref PersistenceMatrixOptions::has_vine_update is false, the comparators are not used.
   * And if @ref PersistenceMatrixOptions::has_vine_update is true, but
   * @ref PersistenceMatrixOptions::has_column_pairings is also true, the comparators are ignored and
   * the current barcode is used to compare birth and deaths. Therefore it is useless to provide them in those cases.
   * 
   * @tparam BirthComparatorFunction Type of the birth comparator: (@ref pos_index, @ref pos_index) -> bool
   * @tparam DeathComparatorFunction Type of the death comparator: (@ref pos_index, @ref pos_index) -> bool
   * @param numberOfColumns Number of columns to reserve space for.
   * @param colSettings Pointer to an existing setting structure for the columns. The structure should contain all
   * the necessary external classes specifically necessary for the choosen column type, such as custom allocators.
   * @param birthComparator Method taking two @ref PosIdx indices as input and returning true if and only if
   * the birth associated to the first position is strictly less than birth associated to
   * the second one with respect to some self defined order. It is used while swapping two unpaired or
   * two negative columns.
   * @param deathComparator Method taking two @ref PosIdx indices as input and returning true if and only if
   * the death associated to the first position is strictly less than death associated to
   * the second one with respect to some self defined order. It is used while swapping two positive but paired
   * columns.
   */
  template <typename BirthComparatorFunction, typename DeathComparatorFunction>
  Id_to_index_overlay(unsigned int numberOfColumns, 
                      Column_settings* colSettings,
                      const BirthComparatorFunction& birthComparator, 
                      const DeathComparatorFunction& deathComparator);
  /**
   * @brief Copy constructor. If @p operators or @p cellConstructor is not a null pointer, its value is kept
   * instead of the one in the copied matrix.
   * 
   * @param matrixToCopy Matrix to copy.
   * @param colSettings Either a pointer to an existing setting structure for the columns or a null pointer.
   * The structure should contain all the necessary external classes specifically necessary for the choosen column type,
   * such as custom allocators. If null pointer, the pointer stored in @p matrixToCopy is used instead.
   */
  Id_to_index_overlay(const Id_to_index_overlay& matrixToCopy, 
                      Column_settings* colSettings = nullptr);
  /**
   * @brief Move constructor.
   * 
   * @param other Matrix to move.
   */
  Id_to_index_overlay(Id_to_index_overlay&& other) noexcept;
  /**
   * @brief Destructor.
   */
  ~Id_to_index_overlay();

  /**
   * @brief Inserts at the end of the matrix a new ordered column corresponding to the given boundary. 
   * This means that it is assumed that this method is called on boundaries in the order of the filtration. 
   * It also assumes that the faces in the given boundary are identified by their relative position in the filtration, 
   * starting at 0. If it is not the case, use the other
   * @ref insert_boundary(id_index, const Boundary_type&, dimension_type) "insert_boundary" instead by indicating the
   * face ID used in the boundaries when the face is inserted.
   *
   * Different to the constructor, the boundaries do not have to come from a simplicial complex, but also from
   * a more general cell complex. This includes cubical complexes or Morse complexes for example.
   *
   * The content of the new column will vary depending on the underlying @ref mp_matrices "type of the matrix":
   * - If it is a boundary type matrix and only \f$ R \f$ is stored, the boundary is just copied. The column will only 
   *   be reduced later when the barcode is requested in order to apply some optimizations with the additional
   *   knowledge. Hence, the barcode will also not be updated.
   * - If it is a boundary type matrix and both \f$ R \f$ and \f$ U \f$ are stored, the new boundary is stored in its
   *   reduced form and the barcode, if active, is also updated.
   * - If it is a chain type matrix, the new column is of the form 
   *   `IDIdx + linear combination of older column IDIdxs`, where the combination is deduced while reducing the 
   *   given boundary. If the barcode is stored, it will also be updated.
   * 
   * @tparam Boundary_type Range of @ref Matrix::cell_rep_type. Assumed to have a begin(), end() and size() method.
   * @param boundary Boundary generating the new column. The content should be ordered by ID.
   * @param dim Dimension of the face whose boundary is given. If the complex is simplicial, 
   * this parameter can be omitted as it can be deduced from the size of the boundary.
   */
  template <class Boundary_type = boundary_type>
  void insert_boundary(const Boundary_type& boundary, dimension_type dim = -1);
  /**
   * @brief It does the same as the other version, but allows the boundary faces to be identified without restrictions
   * except that all IDs have to be strictly increasing in the order of filtration. Note that you should avoid then
   * to use the other insertion method to avoid overwriting IDs.
   *
   * As a face has to be inserted before one of its cofaces in a valid filtration (recall that it is assumed that
   * the faces are inserted by order of filtration), it is sufficient to indicate the ID of the face being inserted.
   * 
   * @tparam Boundary_type Range of @ref Matrix::cell_rep_type. Assumed to have a begin(), end() and size() method.
   * @param faceIndex @ref IDIdx index to use to identify the new face.
   * @param boundary Boundary generating the new column. The indices of the boundary have to correspond to the 
   * @p faceIndex values of precedent calls of the method for the corresponding faces and should be ordered in 
   * increasing order.
   * @param dim Dimension of the face whose boundary is given. If the complex is simplicial, 
   * this parameter can be omitted as it can be deduced from the size of the boundary.
   */
  template <class Boundary_type = boundary_type>
  void insert_boundary(id_index faceIndex, const Boundary_type& boundary, dimension_type dim = -1);
  /**
   * @brief Returns the column at the given @ref IDIdx index. 
   * For @ref boundarymatrix "RU matrices", the returned column is from \f$ R \f$.
   * The type of the column depends on the choosen options, see @ref PersistenceMatrixOptions::column_type.
   * 
   * @param faceID @ref IDIdx index of the column to return.
   * @return Reference to the column.
   */
  Column_type& get_column(id_index faceID);
  /**
   * @brief Only available if @ref PersistenceMatrixOptions::has_row_access is true.
   * Returns the row at the given @ref rowindex "row index".
   * For @ref boundarymatrix "RU matrices", the returned row is from \f$ R \f$.
   * The type of the row depends on the choosen options, see @ref PersistenceMatrixOptions::has_intrusive_rows.
   *
   * @warning The @ref Cell_column_index::get_column_index "get_column_index" method of the row cells returns the
   * original @ref PosIdx indices (before any swaps) for @ref boundarymatrix "boundary matrices" and
   * @ref MatIdx indices for @ref chainmatrix "chain matrices".
   * 
   * @param rowIndex @ref rowindex "Row index" of the row to return: @ref IDIdx for @ref chainmatrix "chain matrices"
   * or updated @ref IDIdx for @ref boundarymatrix "boundary matrices" if swaps occurred.
   * @return Reference to the row.
   */
  Row_type& get_row(id_index rowIndex);
  /**
   * @brief The effect varies depending on the matrices and the options:
   * - @ref boundarymatrix "boundary matrix" with only \f$ R \f$ stored:
   *    - @ref PersistenceMatrixOptions::has_map_column_container and
   *      @ref PersistenceMatrixOptions::has_column_and_row_swaps are true:
   *      cleans up maps used for the lazy row swaps.
   *    - @ref PersistenceMatrixOptions::has_row_access and @ref PersistenceMatrixOptions::has_removable_rows are true:
   *      assumes that the row is empty and removes it. 
   *    - Otherwise, does nothing.
   * - @ref boundarymatrix "boundary matrix" with \f$ U \f$ stored: only \f$ R \f$ is affected by the above.
   *   If properly used, \f$ U \f$ will never have empty rows.
   * - @ref chainmatrix "chain matrix": only available if @ref PersistenceMatrixOptions::has_row_access and
   *   @ref PersistenceMatrixOptions::has_removable_rows are true.
   *   Assumes that the row is empty and removes it. 
   *
   * @warning The removed rows are always assumed to be empty. If it is not the case, the deleted row cells are not
   * removed from their columns. And in the case of intrusive rows, this will generate a segmentation fault when 
   * the column cells are destroyed later. The row access is just meant as a "read only" access to the rows and the
   * @ref erase_empty_row method just as a way to specify that a row is empty and can therefore be removed from
   * dictionaries. This allows to avoid testing the emptiness of a row at each column cell removal, what can be
   * quite frequent. 
   * 
   * @param rowIndex @ref rowindex "Row index" of the empty row to remove.
   */
  void erase_empty_row(id_index rowIndex);
  /**
   * @brief Only available for RU and @ref chainmatrix "chain matrices" and if
   * @ref PersistenceMatrixOptions::has_removable_columns and @ref PersistenceMatrixOptions::has_vine_update are true.
   * For @ref chainmatrix "chain matrices", @ref PersistenceMatrixOptions::has_map_column_container and
   * @ref PersistenceMatrixOptions::has_column_pairings also need to be true.
   * Assumes that the face is maximal in the current complex and removes it such that the matrix remains consistent
   * (i.e., RU is still an upper triangular decomposition of the @ref boundarymatrix "boundary matrix" and chain is
   * still a compatible bases of the chain complex in the sense of @cite zigzag).
   * The maximality of the face is not verified.
   * Also updates the barcode if it was computed.
   *
   * For @ref chainmatrix "chain matrices", using the other version of the method could perform better depending on
   * how the data is maintained on the side of the user. Then, @ref PersistenceMatrixOptions::has_column_pairings also
   * do not need to be true.
   *
   * See also @ref remove_last.
   * 
   * @param faceID @ref IDIdx index of the face to remove.
   */
  void remove_maximal_face(id_index faceID);
  /**
   * @brief Only available for @ref chainmatrix "chain matrices" and if
   * @ref PersistenceMatrixOptions::has_removable_columns, @ref PersistenceMatrixOptions::has_vine_update
   * and @ref PersistenceMatrixOptions::has_map_column_container are true.
   * Assumes that the face is maximal in the current complex and removes it such that the matrix remains consistent
   * (i.e., it is still a compatible bases of the chain complex in the sense of @cite zigzag).
   * The maximality of the face is not verified.
   * Also updates the barcode if it was computed.
   *
   * To maintain the compatibility, vine swaps are done to move the face up to the end of the filtration. Once at 
   * the end, the removal is trivial. But for @ref chainmatrix "chain matrices", swaps do not actually swap the position
   * of the column every time, so the faces appearing after @p faceIndex in the filtration have to be searched first
   * within the matrix. If the user has an easy access to the @ref IDIdx of the faces in the order of filtration,
   * passing them by argument with @p columnsToSwap allows to skip a linear search process. Typically, if the user knows
   * that the face he wants to remove is already the last face of the filtration, calling
   * @ref remove_maximal_face(id_index, const std::vector<id_index>&) "remove_maximal_face(faceID, {})"
   * will be faster than @ref remove_last().
   *
   * See also @ref remove_last.
   * 
   * @param faceID @ref IDIdx index of the face to remove.
   * @param columnsToSwap Vector of @ref IDIdx indices of the faces coming after @p faceID in the filtration.
   */
  void remove_maximal_face(id_index faceID, const std::vector<id_index>& columnsToSwap);
  /**
   * @brief Only available if @ref PersistenceMatrixOptions::has_removable_columns is true. Additionally, if the
   * matrix is a @ref chainmatrix "chain matrix", either @ref PersistenceMatrixOptions::has_map_column_container has to
   * be true or @ref PersistenceMatrixOptions::has_vine_update has to be false.
   * Removes the last face in the filtration from the matrix and updates the barcode if it is stored.
   * 
   * See also @ref remove_maximal_face.
   *
   * For @ref chainmatrix "chain matrices", if @ref PersistenceMatrixOptions::has_vine_update is true, the last face
   * does not have to be at the end of the matrix and therefore has to be searched first. In this case, if the user
   * already knows the @ref IDIdx of the last face, calling
   * @ref remove_maximal_face(id_index, const std::vector<id_index>&) "remove_maximal_face(faceID, {})"
   * instead allows to skip the search.
   */
  void remove_last();

  /**
   * @brief Returns the maximal dimension of a face stored in the matrix. Only available 
   * if @ref PersistenceMatrixOptions::has_matrix_maximal_dimension_access is true.
   * 
   * @return The maximal dimension.
   */
  dimension_type get_max_dimension() const;
  /**
   * @brief Returns the current number of columns in the matrix.
   * 
   * @return The number of columns.
   */
  index get_number_of_columns() const;
  /**
   * @brief Returns the dimension of the given face. Only available for @ref mp_matrices "non-basic matrices".
   * 
   * @param faceID @ref IDIdx index of the face.
   * @return Dimension of the face.
   */
  dimension_type get_column_dimension(id_index faceID) const;

  /**
   * @brief Adds column corresponding to @p sourceFaceID onto the column corresponding to @p targetFaceID.
   *
   * @warning They will be no verification to ensure that the addition makes sense for the validity of the matrix.
   * For example, a right-to-left addition could corrupt the computation of the barcode if done blindly.
   * So should be used with care.
   * 
   * @param sourceFaceID @ref IDIdx index of the source column.
   * @param targetFaceID @ref IDIdx index of the target column.
   */
  void add_to(id_index sourceFaceID, id_index targetFaceID);
  /**
   * @brief Multiplies the target column with the coefficient and then adds the source column to it.
   * That is: `targetColumn = (targetColumn * coefficient) + sourceColumn`.
   *
   * @warning They will be no verification to ensure that the addition makes sense for the validity of the matrix.
   * For example, a right-to-left addition could corrupt the computation of the barcode if done blindly.
   * So should be used with care.
   * 
   * @param sourceFaceID @ref IDIdx index of the source column.
   * @param coefficient Value to multiply.
   * @param targetFaceID @ref IDIdx index of the target column.
   */
  void multiply_target_and_add_to(id_index sourceFaceID, const Field_element_type& coefficient, id_index targetFaceID);
  /**
   * @brief Multiplies the source column with the coefficient before adding it to the target column.
   * That is: `targetColumn += (coefficient * sourceColumn)`. The source column will **not** be modified.
   *
   * @warning They will be no verification to ensure that the addition makes sense for the validity of the matrix.
   * For example, a right-to-left addition could corrupt the computation of the barcode if done blindly.
   * So should be used with care.
   * 
   * @param coefficient Value to multiply.
   * @param sourceFaceID @ref IDIdx index of the source column.
   * @param targetFaceID @ref IDIdx index of the target column.
   */
  void multiply_source_and_add_to(const Field_element_type& coefficient, id_index sourceFaceID, id_index targetFaceID);

  /**
   * @brief Zeroes the cell at the given coordinates. Not available for @ref chainmatrix "chain matrices".
   * In general, should be used with care to not destroy the validity 
   * of the persistence related properties of the matrix.
   *
   * For @ref boundarymatrix "RU matrices", zeros only the cell in \f$ R \f$.
   * 
   * @param faceID @ref IDIdx index of the face corresponding to the column of the cell.
   * @param rowIndex @ref rowindex "Row index" of the row of the cell.
   */
  void zero_cell(id_index faceID, id_index rowIndex);
  /**
   * @brief Zeroes the column at the given index. Not available for @ref chainmatrix "chain matrices".
   * In general, should be used with care to not destroy the validity 
   * of the persistence related properties of the matrix.
   *
   * For @ref boundarymatrix "RU matrices", zeros only the column in \f$ R \f$.
   * 
   * @param faceID @ref IDIdx index of the face corresponding to the column.
   */
  void zero_column(id_index faceID);
  /**
   * @brief Indicates if the cell at given coordinates has value zero.
   *
   * For @ref boundarymatrix "RU matrices", looks into \f$ R \f$.
   * 
   * @param faceID @ref IDIdx index of the face corresponding to the column of the cell.
   * @param rowIndex @ref rowindex "Row index" of the row of the cell.
   * @return true If the cell has value zero.
   * @return false Otherwise.
   */
  bool is_zero_cell(id_index faceID, id_index rowIndex) const;
  /**
   * @brief Indicates if the column at given index has value zero.
   *
   * For @ref boundarymatrix "RU matrices", looks into \f$ R \f$.
   *
   * Note that for @ref chainmatrix "chain matrices", this method should always return false, as a valid
   * @ref chainmatrix "chain matrix" never has empty columns.
   * 
   * @param faceID @ref IDIdx index of the face corresponding to the column.
   * @return true If the column has value zero.
   * @return false Otherwise.
   */
  bool is_zero_column(id_index faceID);

  /**
   * @brief Returns the @ref IDIdx index of the column which has the given @ref rowindex "row index" as pivot.
   * Assumes that the pivot exists. For @ref boundarymatrix "RU matrices", the column is returned from \f$ R \f$.
   *
   * Recall that the row indices for @ref chainmatrix "chain matrices" correspond to the @ref IDIdx indices and that
   * the row indices for a @ref boundarymatrix "RU matrix" correspond to the updated @ref IDIdx indices which got
   * potentially swapped by a vine swap.
   * 
   * @param faceIndex @ref rowindex "Row index" of the pivot.
   * @return @ref IDIdx index of the column with the given pivot.
   */
  id_index get_column_with_pivot(id_index faceIndex) const;
  /**
   * @brief Returns the @ref rowindex "row index" of the pivot of the given column.
   * 
   * @param faceID @ref IDIdx index of the face corresponding to the column.
   * @return The @ref rowindex "row index" of the pivot.
   */
  id_index get_pivot(id_index faceID);

  /**
   * @brief Resets the matrix to an empty matrix.
   * 
   * @param colSettings Pointer to an existing setting structure for the columns. The structure should contain all
   * the necessary external classes specifically necessary for the choosen column type, such as custom allocators.
   */
  void reset(Column_settings* colSettings) {
    matrix_.reset(colSettings);
    nextIndex_ = 0;
  }

  // void set_operators(Field_operators* operators) { matrix_.set_operators(operators); }

  /**
   * @brief Assign operator.
   */
  Id_to_index_overlay& operator=(const Id_to_index_overlay& other);
  /**
   * @brief Swap operator.
   */
  friend void swap(Id_to_index_overlay& matrix1, Id_to_index_overlay& matrix2) {
    swap(matrix1.matrix_, matrix2.matrix_);
    if (Master_matrix_type::Option_list::is_of_boundary_type) std::swap(matrix1.idToIndex_, matrix2.idToIndex_);
    std::swap(matrix1.nextIndex_, matrix2.nextIndex_);
  }

  void print();  // for debug

  // access to optional methods

  /**
   * @brief Returns the current barcode of the matrix.
   * Available only if @ref PersistenceMatrixOptions::has_column_pairings is true.
   *
   * Recall that we assume that the boundaries were inserted in the order of filtration for the barcode to be valid.
   *
   * @warning For simple @ref boundarymatrix "boundary matrices" (only storing \f$ R \f$), we assume that
   * @ref get_current_barcode is only called once, when the matrix is completed.
   * 
   * @return A reference to the barcode. The barcode is a vector of @ref Matrix::Bar. A bar stores three informations:
   * the @ref PosIdx birth index, the @ref PosIdx death index and the dimension of the bar.
   */
  const barcode_type& get_current_barcode();
  
  /**
   * @brief Only available for simple @ref boundarymatrix "boundary matrices" (only storing \f$ R \f$) and if
   * @ref PersistenceMatrixOptions::has_column_and_row_swaps is true.
   * Swaps the two given columns. Note that it really just swaps two columns and do not updates
   * anything else, nor performs additions to maintain some properties on the matrix.
   * 
   * @param faceID1 First column @ref IDIdx index to swap.
   * @param faceID2 Second column @ref IDIdx index to swap.
   */
  void swap_columns(id_index faceID1, id_index faceID2);
  /**
   * @brief Only available for simple @ref boundarymatrix "boundary matrices" (only storing R)
   * and if @ref PersistenceMatrixOptions::has_column_and_row_swaps is true.
   * Swaps the two given rows. Note that it really just swaps two rows and do not updates
   * anything else, nor performs additions to maintain some properties on the matrix.
   * 
   * @param rowIndex1 First @ref rowindex "row index" to swap.
   * @param rowIndex2 Second @ref rowindex "row index" to swap.
   */
  void swap_rows(index rowIndex1, index rowIndex2);
  /**
   * @brief Only available if @ref PersistenceMatrixOptions::has_vine_update is true.
   * Does the same than @ref vine_swap, but assumes that the swap is non trivial and
   * therefore skips a part of the case study.
   * 
   * @param faceID1 @ref IDIdx index of the first face.
   * @param faceID2 @ref IDIdx index of the second face. It is assumed that the @ref PosIdx of both only differs by one.
   * @return Let \f$ pos1 \f$ be the @ref PosIdx index of @p columnIndex1 and \f$ pos2 \f$ be the @ref PosIdx index of
   * @p columnIndex2. The method returns the @ref MatIdx of the column which has now, after the swap, the @ref PosIdx
   * \f$ max(pos1, pos2) \f$.
   */
  id_index vine_swap_with_z_eq_1_case(id_index faceID1, id_index faceID2);
  /**
   * @brief Only available if @ref PersistenceMatrixOptions::has_vine_update is true.
   * Does a vine swap between two faces which are consecutive in the filtration. Roughly, if \f$ F \f$ is
   * the current filtration represented by the matrix, the method modifies the matrix such that the new state
   * corresponds to a valid state for the filtration \f$ F' \f$ equal to \f$ F \f$ but with the two given faces
   * at swapped positions. Of course, the two faces should not have a face/coface relation which each other ;
   * \f$ F' \f$ has to be a valid filtration.
   * See @cite vineyards for more information about vine and vineyards.
   * 
   * @param faceID1 @ref IDIdx index of the first face.
   * @param faceID2 @ref IDIdx index of the second face. It is assumed that the @ref PosIdx of both only differs by one.
   * @return Let \f$ pos1 \f$ be the @ref PosIdx index of @p columnIndex1 and \f$ pos2 \f$ be the @ref PosIdx index of
   * @p columnIndex2. The method returns the @ref MatIdx of the column which has now, after the swap, the @ref PosIdx
   * \f$ max(pos1, pos2) \f$.
   */
  id_index vine_swap(id_index faceID1, id_index faceID2);
  
  /**
   * @brief Only available if @ref PersistenceMatrixOptions::can_retrieve_representative_cycles is true. Pre-computes
   * the representative cycles of the current state of the filtration represented by the matrix.
   * It does not need to be called before `get_representative_cycles` is called for the first time, but needs to be
   * called before calling `get_representative_cycles` again if the matrix was modified in between. Otherwise the
   * old cycles will be returned.
   */
  void update_representative_cycles();
  /**
   * @brief Only available if @ref PersistenceMatrixOptions::can_retrieve_representative_cycles is true.
   * Returns all representative cycles of the current filtration.
   * 
   * @return A const reference to the vector of representative cycles.
   */
  const std::vector<cycle_type>& get_representative_cycles();
  /**
   * @brief Only available if @ref PersistenceMatrixOptions::can_retrieve_representative_cycles is true.
   * Returns the cycle representing the given bar.
   * 
   * @param bar A bar from the current barcode.
   * @return A const reference to the cycle representing @p bar.
   */
  const cycle_type& get_representative_cycle(const bar_type& bar);

 private:
  using dictionary_type = typename Master_matrix_type::template dictionary_type<index>;

  Matrix_type matrix_;          /**< Interfaced matrix. */
  dictionary_type* idToIndex_; /**< Map from @ref IDIdx index to @ref MatIdx index. */
  index nextIndex_;             /**< Next unused index. */

  void _initialize_map(unsigned int size);
  index _id_to_index(id_index id) const;
};

template <class Matrix_type, class Master_matrix_type>
inline Id_to_index_overlay<Matrix_type, Master_matrix_type>::Id_to_index_overlay(Column_settings* colSettings)
    : matrix_(colSettings), idToIndex_(nullptr), nextIndex_(0) 
{
  _initialize_map(0);
}

template <class Matrix_type, class Master_matrix_type>
template <class Boundary_type>
inline Id_to_index_overlay<Matrix_type, Master_matrix_type>::Id_to_index_overlay(
    const std::vector<Boundary_type>& orderedBoundaries, Column_settings* colSettings)
    : matrix_(orderedBoundaries, colSettings), idToIndex_(nullptr), nextIndex_(orderedBoundaries.size()) 
{
  _initialize_map(orderedBoundaries.size());
  if constexpr (Master_matrix_type::Option_list::is_of_boundary_type) {
    for (unsigned int i = 0; i < orderedBoundaries.size(); i++) {
      idToIndex_->operator[](i) = i;
    }
  }
}

template <class Matrix_type, class Master_matrix_type>
inline Id_to_index_overlay<Matrix_type, Master_matrix_type>::Id_to_index_overlay(unsigned int numberOfColumns,
                                                                                 Column_settings* colSettings)
    : matrix_(numberOfColumns, colSettings), idToIndex_(nullptr), nextIndex_(0) 
{
  _initialize_map(numberOfColumns);
}

template <class Matrix_type, class Master_matrix_type>
template <typename BirthComparatorFunction, typename DeathComparatorFunction>
inline Id_to_index_overlay<Matrix_type, Master_matrix_type>::Id_to_index_overlay(
    Column_settings* colSettings, 
    const BirthComparatorFunction& birthComparator, 
    const DeathComparatorFunction& deathComparator)
    : matrix_(colSettings, birthComparator, deathComparator), idToIndex_(nullptr), nextIndex_(0) 
{
  _initialize_map(0);
}

template <class Matrix_type, class Master_matrix_type>
template <typename BirthComparatorFunction, typename DeathComparatorFunction, class Boundary_type>
inline Id_to_index_overlay<Matrix_type, Master_matrix_type>::Id_to_index_overlay(
    const std::vector<Boundary_type>& orderedBoundaries, 
    Column_settings* colSettings,
    const BirthComparatorFunction& birthComparator, 
    const DeathComparatorFunction& deathComparator)
    : matrix_(orderedBoundaries, colSettings, birthComparator, deathComparator),
      idToIndex_(nullptr),
      nextIndex_(orderedBoundaries.size()) 
{
  _initialize_map(orderedBoundaries.size());
  if constexpr (Master_matrix_type::Option_list::is_of_boundary_type) {
    for (unsigned int i = 0; i < orderedBoundaries.size(); i++) {
      idToIndex_->operator[](i) = i;
    }
  }
}

template <class Matrix_type, class Master_matrix_type>
template <typename BirthComparatorFunction, typename DeathComparatorFunction>
inline Id_to_index_overlay<Matrix_type, Master_matrix_type>::Id_to_index_overlay(
    unsigned int numberOfColumns, 
    Column_settings* colSettings,
    const BirthComparatorFunction& birthComparator, 
    const DeathComparatorFunction& deathComparator)
    : matrix_(numberOfColumns, colSettings, birthComparator, deathComparator),
      idToIndex_(nullptr),
      nextIndex_(0) 
{
  _initialize_map(numberOfColumns);
}

template <class Matrix_type, class Master_matrix_type>
inline Id_to_index_overlay<Matrix_type, Master_matrix_type>::Id_to_index_overlay(
    const Id_to_index_overlay& matrixToCopy, Column_settings* colSettings)
    : matrix_(matrixToCopy.matrix_, colSettings),
      idToIndex_(nullptr),
      nextIndex_(matrixToCopy.nextIndex_) 
{
  if constexpr (Master_matrix_type::Option_list::is_of_boundary_type) {
    idToIndex_ = new dictionary_type(*matrixToCopy.idToIndex_);
  } else {
    idToIndex_ = &matrix_.pivotToColumnIndex_;
  }
}

template <class Matrix_type, class Master_matrix_type>
inline Id_to_index_overlay<Matrix_type, Master_matrix_type>::Id_to_index_overlay(Id_to_index_overlay&& other) noexcept
    : matrix_(std::move(other.matrix_)),
      idToIndex_(std::exchange(other.idToIndex_, nullptr)),
      nextIndex_(std::exchange(other.nextIndex_, 0)) 
{}

template <class Matrix_type, class Master_matrix_type>
inline Id_to_index_overlay<Matrix_type, Master_matrix_type>::~Id_to_index_overlay() 
{
  if constexpr (Master_matrix_type::Option_list::is_of_boundary_type) {
    if (idToIndex_ != nullptr) delete idToIndex_;
  }
}

template <class Matrix_type, class Master_matrix_type>
template <class Boundary_type>
inline void Id_to_index_overlay<Matrix_type, Master_matrix_type>::insert_boundary(const Boundary_type& boundary,
                                                                                  dimension_type dim) 
{
  matrix_.insert_boundary(boundary, dim);
  if constexpr (Master_matrix_type::Option_list::is_of_boundary_type) {
    if constexpr (Master_matrix_type::Option_list::has_map_column_container) {
      idToIndex_->emplace(nextIndex_, nextIndex_);
    } else {
      if (idToIndex_->size() == nextIndex_) {
        idToIndex_->push_back(nextIndex_);
      } else {
        idToIndex_->operator[](nextIndex_) = nextIndex_;
      }
    }
    ++nextIndex_;
  }
}

template <class Matrix_type, class Master_matrix_type>
template <class Boundary_type>
inline void Id_to_index_overlay<Matrix_type, Master_matrix_type>::insert_boundary(id_index faceIndex,
                                                                                  const Boundary_type& boundary,
                                                                                  dimension_type dim) 
{
  if constexpr (Master_matrix_type::Option_list::has_map_column_container) {
    GUDHI_CHECK(idToIndex_->find(faceIndex) == idToIndex_->end(),
                std::invalid_argument("Id_to_index_overlay::insert_boundary - Index for simplex already chosen!"));
  } else {
    GUDHI_CHECK((idToIndex_->size() <= faceIndex || idToIndex_[faceIndex] == static_cast<index>(-1)),
                std::invalid_argument("Id_to_index_overlay::insert_boundary - Index for simplex already chosen!"));
  }
  matrix_.insert_boundary(faceIndex, boundary, dim);
  if constexpr (Master_matrix_type::Option_list::is_of_boundary_type) {
    if constexpr (Master_matrix_type::Option_list::has_map_column_container) {
      idToIndex_->emplace(faceIndex, nextIndex_);
    } else {
      if (idToIndex_->size() <= faceIndex) {
        idToIndex_->resize(faceIndex + 1, -1);
      }
      idToIndex_->operator[](faceIndex) = nextIndex_;
    }
    ++nextIndex_;
  }
}

template <class Matrix_type, class Master_matrix_type>
inline typename Id_to_index_overlay<Matrix_type, Master_matrix_type>::Column_type&
Id_to_index_overlay<Matrix_type, Master_matrix_type>::get_column(id_index faceID) 
{
  return matrix_.get_column(_id_to_index(faceID));
}

template <class Matrix_type, class Master_matrix_type>
inline typename Id_to_index_overlay<Matrix_type, Master_matrix_type>::Row_type&
Id_to_index_overlay<Matrix_type, Master_matrix_type>::get_row(id_index rowIndex) 
{
  return matrix_.get_row(rowIndex);
}

template <class Matrix_type, class Master_matrix_type>
inline void Id_to_index_overlay<Matrix_type, Master_matrix_type>::erase_empty_row(id_index rowIndex) 
{
  return matrix_.erase_empty_row(rowIndex);
}

template <class Matrix_type, class Master_matrix_type>
inline void Id_to_index_overlay<Matrix_type, Master_matrix_type>::remove_maximal_face(id_index faceID) 
{
  if constexpr (Master_matrix_type::Option_list::is_of_boundary_type) {
    std::vector<id_index> indexToID(nextIndex_);
    if constexpr (Master_matrix_type::Option_list::has_map_column_container) {
      for (auto& p : *idToIndex_) {
        indexToID[p.second] = p.first;
      }
    } else {
      for (id_index i = 0; i < idToIndex_->size(); ++i) {
        if (idToIndex_->operator[](i) != static_cast<index>(-1)) indexToID[idToIndex_->operator[](i)] = i;
      }
    }
    --nextIndex_;
    for (index curr = _id_to_index(faceID); curr < nextIndex_; ++curr) {
      matrix_.vine_swap(curr);
      std::swap(idToIndex_->at(indexToID[curr]), idToIndex_->at(indexToID[curr + 1]));
    }
    matrix_.remove_last();
    GUDHI_CHECK(_id_to_index(faceID) == nextIndex_,
                std::logic_error("Id_to_index_overlay::remove_maximal_face - Indexation problem."));

    if constexpr (Master_matrix_type::Option_list::has_map_column_container) {
      idToIndex_->erase(faceID);
    } else {
      idToIndex_->operator[](faceID) = -1;
    }
  } else {
    matrix_.remove_maximal_face(faceID);
  }
}

template <class Matrix_type, class Master_matrix_type>
inline void Id_to_index_overlay<Matrix_type, Master_matrix_type>::remove_maximal_face(
    id_index faceID, const std::vector<id_index>& columnsToSwap) 
{
  static_assert(!Master_matrix_type::Option_list::is_of_boundary_type,
                "'remove_maximal_face(id_index,const std::vector<index>&)' is not available for the chosen options.");
  std::vector<index> translatedIndices;
  std::transform(columnsToSwap.cbegin(), columnsToSwap.cend(), std::back_inserter(translatedIndices),
                 [&](id_index id) { return _id_to_index(id); });
  matrix_.remove_maximal_face(faceID, translatedIndices);
}

template <class Matrix_type, class Master_matrix_type>
inline void Id_to_index_overlay<Matrix_type, Master_matrix_type>::remove_last() 
{
  if (idToIndex_->empty()) return;  //empty matrix

  matrix_.remove_last();

  if constexpr (Master_matrix_type::Option_list::is_of_boundary_type) {
    --nextIndex_;
    if constexpr (Master_matrix_type::Option_list::has_map_column_container) {
      auto it = idToIndex_->begin();
      while (it->second != nextIndex_) ++it;   //should never reach idToIndex_->end()
      idToIndex_->erase(it);
    } else {
      index id = idToIndex_->size() - 1;
      while (idToIndex_->operator[](id) == static_cast<index>(-1)) --id;  // should always stop before reaching -1
      GUDHI_CHECK(idToIndex_->operator[](id) == nextIndex_,
                  std::logic_error("Id_to_index_overlay::remove_last - Indexation problem."));
      idToIndex_->operator[](id) = -1;
    }
  }
}

template <class Matrix_type, class Master_matrix_type>
inline typename Id_to_index_overlay<Matrix_type, Master_matrix_type>::dimension_type
Id_to_index_overlay<Matrix_type, Master_matrix_type>::get_max_dimension() const 
{
  return matrix_.get_max_dimension();
}

template <class Matrix_type, class Master_matrix_type>
inline typename Id_to_index_overlay<Matrix_type, Master_matrix_type>::index
Id_to_index_overlay<Matrix_type, Master_matrix_type>::get_number_of_columns() const 
{
  return matrix_.get_number_of_columns();
}

template <class Matrix_type, class Master_matrix_type>
inline typename Id_to_index_overlay<Matrix_type, Master_matrix_type>::dimension_type
Id_to_index_overlay<Matrix_type, Master_matrix_type>::get_column_dimension(id_index faceID) const 
{
  return matrix_.get_column_dimension(_id_to_index(faceID));
}

template <class Matrix_type, class Master_matrix_type>
inline void Id_to_index_overlay<Matrix_type, Master_matrix_type>::add_to(id_index sourceFaceID, id_index targetFaceID) 
{
  return matrix_.add_to(_id_to_index(sourceFaceID), _id_to_index(targetFaceID));
}

template <class Matrix_type, class Master_matrix_type>
inline void Id_to_index_overlay<Matrix_type, Master_matrix_type>::multiply_target_and_add_to(
    id_index sourceFaceID, const Field_element_type& coefficient, id_index targetFaceID) 
{
  return matrix_.multiply_target_and_add_to(_id_to_index(sourceFaceID), coefficient, _id_to_index(targetFaceID));
}

template <class Matrix_type, class Master_matrix_type>
inline void Id_to_index_overlay<Matrix_type, Master_matrix_type>::multiply_source_and_add_to(
    const Field_element_type& coefficient, id_index sourceFaceID, id_index targetFaceID) 
{
  return matrix_.multiply_source_and_add_to(coefficient, _id_to_index(sourceFaceID), _id_to_index(targetFaceID));
}

template <class Matrix_type, class Master_matrix_type>
inline void Id_to_index_overlay<Matrix_type, Master_matrix_type>::zero_cell(id_index faceID, id_index rowIndex) 
{
  return matrix_.zero_cell(_id_to_index(faceID), rowIndex);
}

template <class Matrix_type, class Master_matrix_type>
inline void Id_to_index_overlay<Matrix_type, Master_matrix_type>::zero_column(id_index faceID) 
{
  return matrix_.zero_column(_id_to_index(faceID));
}

template <class Matrix_type, class Master_matrix_type>
inline bool Id_to_index_overlay<Matrix_type, Master_matrix_type>::is_zero_cell(id_index faceID,
                                                                               id_index rowIndex) const 
{
  return matrix_.is_zero_cell(_id_to_index(faceID), rowIndex);
}

template <class Matrix_type, class Master_matrix_type>
inline bool Id_to_index_overlay<Matrix_type, Master_matrix_type>::is_zero_column(id_index faceID) 
{
  return matrix_.is_zero_column(_id_to_index(faceID));
}

template <class Matrix_type, class Master_matrix_type>
inline typename Id_to_index_overlay<Matrix_type, Master_matrix_type>::id_index
Id_to_index_overlay<Matrix_type, Master_matrix_type>::get_column_with_pivot(id_index simplexIndex) const 
{
  if constexpr (Master_matrix_type::Option_list::is_of_boundary_type) {
    index pos = matrix_.get_column_with_pivot(simplexIndex);
    id_index i = 0;
    while (_id_to_index(i) != pos) ++i;
    return i;
  } else {
    return simplexIndex;
  }
}

template <class Matrix_type, class Master_matrix_type>
inline typename Id_to_index_overlay<Matrix_type, Master_matrix_type>::id_index
Id_to_index_overlay<Matrix_type, Master_matrix_type>::get_pivot(id_index faceID) 
{
  if constexpr (Master_matrix_type::Option_list::is_of_boundary_type) {
    return matrix_.get_pivot(_id_to_index(faceID));
  } else {
    return faceID;
  }
}

template <class Matrix_type, class Master_matrix_type>
inline Id_to_index_overlay<Matrix_type, Master_matrix_type>&
Id_to_index_overlay<Matrix_type, Master_matrix_type>::operator=(const Id_to_index_overlay& other) 
{
  matrix_ = other.matrix_;
  if (Master_matrix_type::Option_list::is_of_boundary_type)
    idToIndex_ = other.idToIndex_;
  else
    idToIndex_ = &matrix_.pivotToColumnIndex_;
  nextIndex_ = other.nextIndex_;

  return *this;
}

template <class Matrix_type, class Master_matrix_type>
inline void Id_to_index_overlay<Matrix_type, Master_matrix_type>::print() 
{
  return matrix_.print();
}

template <class Matrix_type, class Master_matrix_type>
inline const typename Id_to_index_overlay<Matrix_type, Master_matrix_type>::barcode_type&
Id_to_index_overlay<Matrix_type, Master_matrix_type>::get_current_barcode() 
{
  return matrix_.get_current_barcode();
}

template <class Matrix_type, class Master_matrix_type>
inline void Id_to_index_overlay<Matrix_type, Master_matrix_type>::update_representative_cycles() 
{
  matrix_.update_representative_cycles();
}

template <class Matrix_type, class Master_matrix_type>
inline const std::vector<typename Id_to_index_overlay<Matrix_type, Master_matrix_type>::cycle_type>&
Id_to_index_overlay<Matrix_type, Master_matrix_type>::get_representative_cycles() 
{
  return matrix_.get_representative_cycles();
}

template <class Matrix_type, class Master_matrix_type>
inline const typename Id_to_index_overlay<Matrix_type, Master_matrix_type>::cycle_type&
Id_to_index_overlay<Matrix_type, Master_matrix_type>::get_representative_cycle(const bar_type& bar) 
{
  return matrix_.get_representative_cycle(bar);
}

template <class Matrix_type, class Master_matrix_type>
inline void Id_to_index_overlay<Matrix_type, Master_matrix_type>::swap_columns(id_index faceID1, id_index faceID2) 
{
  matrix_.swap_columns(_id_to_index(faceID1), _id_to_index(faceID2));
  std::swap(idToIndex_->at(faceID1), idToIndex_->at(faceID2));
}

template <class Matrix_type, class Master_matrix_type>
inline void Id_to_index_overlay<Matrix_type, Master_matrix_type>::swap_rows(index rowIndex1, index rowIndex2) 
{
  matrix_.swap_rows(rowIndex1, rowIndex2);
}

template <class Matrix_type, class Master_matrix_type>
inline typename Id_to_index_overlay<Matrix_type, Master_matrix_type>::id_index
Id_to_index_overlay<Matrix_type, Master_matrix_type>::vine_swap_with_z_eq_1_case(id_index faceID1, id_index faceID2) 
{
  index first = _id_to_index(faceID1);
  index second = _id_to_index(faceID2);
  if (first > second) std::swap(first, second);

  if constexpr (Master_matrix_type::Option_list::is_of_boundary_type) {
    GUDHI_CHECK(second - first == 1,
                std::invalid_argument(
                    "Id_to_index_overlay::vine_swap_with_z_eq_1_case - The columns to swap are not contiguous."));

    bool change = matrix_.vine_swap_with_z_eq_1_case(first);

    std::swap(idToIndex_->at(faceID1), idToIndex_->at(faceID2));

    if (change) {
      return faceID1;
    }
    return faceID2;
  } else {
    return matrix_.vine_swap_with_z_eq_1_case(first, second);
  }
}

template <class Matrix_type, class Master_matrix_type>
inline typename Id_to_index_overlay<Matrix_type, Master_matrix_type>::id_index
Id_to_index_overlay<Matrix_type, Master_matrix_type>::vine_swap(id_index faceID1, id_index faceID2) 
{
  index first = _id_to_index(faceID1);
  index second = _id_to_index(faceID2);
  if (first > second) std::swap(first, second);

  if constexpr (Master_matrix_type::Option_list::is_of_boundary_type) {
    GUDHI_CHECK(second - first == 1,
                std::invalid_argument("Id_to_index_overlay::vine_swap - The columns to swap are not contiguous."));

    bool change = matrix_.vine_swap(first);

    std::swap(idToIndex_->at(faceID1), idToIndex_->at(faceID2));

    if (change) {
      return faceID1;
    }
    return faceID2;
  } else {
    return matrix_.vine_swap(first, second);
  }
}

template <class Matrix_type, class Master_matrix_type>
inline void Id_to_index_overlay<Matrix_type, Master_matrix_type>::_initialize_map([[maybe_unused]] unsigned int size) 
{
  if constexpr (Master_matrix_type::Option_list::is_of_boundary_type) {
    if constexpr (Master_matrix_type::Option_list::has_map_column_container) {
      idToIndex_ = new dictionary_type(size);
    } else {
      idToIndex_ = new dictionary_type(size, -1);
    }
  } else {
    idToIndex_ = &matrix_.pivotToColumnIndex_;
  }
}

template <class Matrix_type, class Master_matrix_type>
inline typename Id_to_index_overlay<Matrix_type, Master_matrix_type>::index
Id_to_index_overlay<Matrix_type, Master_matrix_type>::_id_to_index(id_index id) const 
{
  if constexpr (Master_matrix_type::Option_list::has_map_column_container) {
    return idToIndex_->at(id);
  } else {
    return idToIndex_->operator[](id);
  }
}

}  // namespace persistence_matrix
}  // namespace Gudhi

#endif  // PM_ID_TO_POS_TRANSLATION_H
