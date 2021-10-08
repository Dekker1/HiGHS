/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                       */
/*    This file is part of the HiGHS linear optimization suite           */
/*                                                                       */
/*    Written and engineered 2008-2021 at the University of Edinburgh    */
/*                                                                       */
/*    Available as open-source under the MIT License                     */
/*                                                                       */
/*    Authors: Julian Hall, Ivet Galabova, Qi Huangfu, Leona Gottwald    */
/*    and Michael Feldmeier                                              */
/*                                                                       */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/**@file simplex/HEkkDual.h
 * @brief Dual simplex solver for HiGHS
 */
#ifndef SIMPLEX_HEKKDUAL_H_
#define SIMPLEX_HEKKDUAL_H_

#include <set>
#include <string>
#include <vector>

#include "HConfig.h"
#include "simplex/HEkk.h"
#include "simplex/HEkkDualRHS.h"
#include "simplex/HEkkDualRow.h"
#include "simplex/HSimplex.h"
#include "simplex/HVector.h"

// Limit on the number of column slices for parallel calculations. SIP
// uses num_threads-2 slices; PAMI uses num_threads-1 slices
const HighsInt kHighsSlicedLimit = kHighsThreadLimit;
// Was 100, but can't see why this should be higher than kHighsThreadLimit;

/**
 * @brief Dual simplex solver for HiGHS
 */
class HEkkDual {
 public:
  HEkkDual(HEkk& simplex)
      : ekk_instance_(simplex), dualRow(simplex), dualRHS(simplex) {
    initialiseInstance();
    dualRow.setup();
    dualRHS.setup();
    if (!(ekk_instance_.info_.simplex_strategy == kSimplexStrategyDualPlain))
      initialiseInstanceParallel(simplex);
  }

  /**
   * @brief Solve a model instance
   */
  HighsStatus solve();

  const SimplexAlgorithm algorithm = SimplexAlgorithm::kDual;

 public:
  /**
   * @brief Initialise a dual simplex instance
   *
   * Copy dimensions and pointers to matrix, factor and solver-related
   * model data, plus tolerances. Sets up local std::vectors
   * (columnDSE, columnBFRT, column, row_ep and row_ap), and buffers
   * for dualRow and dualRHS.
   */
  void initialiseInstance();

  /**
   * @brief Initialise parallel aspects of a dual simplex instance
   *
   * Sets up data structures for SIP or PAMI
   */
  void initialiseInstanceParallel(HEkk& simplex);

  /**
   * @brief Initialise matrix slices and slices of row_ap or dualRow for SIP or
   * PAMI
   *
   * TODO generalise call slice_matrix[i].setup_lgBs so slice can be
   * used with non-logical initial basis
   */
  void initSlice(
      const HighsInt init_sliced_num  //!< Ideal number of slices - true number
                                      //!< is modified in light of limits
  );

  /**
   * @brief Initialise a dual simplex solve
   */
  void initialiseSolve();

  /**
   * @brief Perform Phase 1 dual simplex iterations
   */
  void solvePhase1();

  /**
   * @brief Perform Phase 2 dual simplex iterations
   */
  void solvePhase2();

  /**
   * @brief Reinvert if INVERT not fresh, then recompute dual and primal values
   *
   * Also collects primal infeasibilities and computes the dual objective value
   */

  void rebuild();

  /**
   * @brief Remove perturbation and recompute the dual solution
   *
   * Also collects primal infeasibilities and computes the dual objective value
   */
  void cleanup();

  /**
   * @brief Perform a single serial dual simplex iteration
   *
   * All the methods it calls have as their first line "if (rebuild_reason)
   * return;", where rebuild_reason is, for example, set to 1 when CHUZR
   * finds no candidate. This causes a break from the inner loop of
   * solvePhase% and, hence, a call to rebuild().
   */
  void iterate();

  /**
   * @brief Perform a single SIP dual simplex iteration
   */
  void iterateTasks();

  /**
   * @brief Perform a single PAMI dual simplex iteration - source code in
   * HEkkDualMulti.cpp
   */
  void iterateMulti();  // in HEkkDualMulti.cpp

  /**
   * @brief Pass the data for the serial iteration analysis, report and rebuild
   * report
   */
  void iterationAnalysisData();

  /**
   * @brief Perform the serial iteration analysis
   */
  void iterationAnalysis();

  /**
   * @brief Pass the data for the PAMI iteration analysis for a minor iteration,
   * report and rebuild report
   */
  void iterationAnalysisMinorData();

  /**
   * @brief Perform the PAMI iteration analysis for a minor iteration
   */
  void iterationAnalysisMinor();

  /**
   * @brief Pass the data for the PAMI iteration analysis for a major iteration
   */
  void iterationAnalysisMajorData();

  /**
   * @brief Perform the PAMI iteration analysis for a major iteration
   */
  void iterationAnalysisMajor();

  /**
   * @brief Single line report after rebuild or cleanup
   */
  void reportRebuild(const HighsInt reason_for_rebuild);

  /**
   * @brief Choose the index of a good row to leave the basis (CHUZR)
   */
  void chooseRow();

  /**
   * @brief Determine whether the updated_edge_weight is accurate enough to
   * be accepted, and update the analysis of weight errors
   */
  bool acceptDualSteepestEdgeWeight(const HighsFloat updated_edge_weight);

  /**
   * @brief Determine whether the updated_edge_weight error should trigger a new
   * Devex framework
   */
  bool newDevexFramework(const HighsFloat updated_edge_weight);

  /**
   * @brief Compute pivot row (PRICE) and choose the index of a good column to
   * enter the basis (CHUZC)
   */
  void chooseColumn(HVector* row_ep);

  /**
   * @brief Choose the index of a good column to enter the basis (CHUZC) by
   * exploiting slices of the pivotal row - for SIP and PAMI
   */
  void chooseColumnSlice(HVector* row_ep);

  /**
   * @brief Compute the pivotal column (FTRAN)
   */
  void updateFtran();

  /**
   * @brief Compute the RHS changes corresponding to the BFRT
   * (FTRAN-BFRT)
   */
  void updateFtranBFRT();

  /**
   * @brief Compute the std::vector required to update DSE weights - being
   * FTRAN applied to the pivotal column (FTRAN-DSE)
   */
  void updateFtranDSE(HVector* DSE_Vector  //!< Pivotal column as RHS for FTRAN
  );
  /**
   * @brief Compare the pivot value computed row-wise and column-wise
   * and determine whether reinversion is advisable
   */
  void updateVerify();

  /**
   * @brief Update the dual values
   */
  void updateDual();

  /**
   * @brief Update the primal values and any edge weights
   */
  void updatePrimal(HVector* DSE_Vector  //!< FTRANned pivotal column
  );

  /**
   * @brief Update the basic and nonbasic variables, iteration count,
   * invertible representation of the basis matrix and row-wise
   * representation of the nonbasic columns, delete the Freelist entry
   * for the entering column, update the primal value for the row
   * where the basis change has occurred, and set the corresponding
   * primal infeasibility value in dualRHS.work_infeasibility, and
   * then determine whether to reinvert according to the synthetic
   * clock
   */
  void updatePivots();

  void shiftCost(const HighsInt iCol, const HighsFloat amount);
  void shiftBack(const HighsInt iCol);

  /**
   * @brief Initialise a Devex framework: reference set is all basic
   * variables
   */
  void initialiseDevexFramework(const bool parallel = false);

  /**
   * @brief Interpret the dual edge weight strategy as setting of a mode and
   * other actions
   */
  void interpretDualEdgeWeightStrategy(
      const HighsInt simplex_dual_edge_weight_strategy);

  bool reachedExactObjectiveBound();
  HighsFloat computeExactDualObjectiveValue();

  /**
   * @brief PAMI: Choose the indices of a good set of rows to leave the
   * basis (CHUZR)
   */
  void majorChooseRow();

  /**
   * @brief PAMI: Perform multiple BTRAN
   */
  void majorChooseRowBtran();

  /**
   * @brief PAMI: Choose the index (from the set of indices) of a good
   * row to leave the basis (CHUZR-MI)
   */
  void minorChooseRow();

  /**
   * @brief PAMI: Update the data during minor iterations
   */
  void minorUpdate();

  /**
   * @brief PAMI: Update the dual values during minor iterations
   */
  void minorUpdateDual();

  /**
   * @brief PAMI: Update the primal values during minor iterations
   */
  void minorUpdatePrimal();

  /**
   * @brief PAMI: Perform a basis change during minor iterations
   */
  void minorUpdatePivots();

  /**
   * @brief PAMI: Update the tableau rows during minor iterations
   */
  void minorUpdateRows();

  /**
   * @brief PAMI: Initialise a new Devex framework during minor iterations
   */
  void minorInitialiseDevexFramework();

  /**
   * @brief PAMI: Perform updates after a set of minor iterations
   */
  void majorUpdate();

  /**
   * @brief PAMI: Prepare for the FTRANs after a set of minor iterations
   */
  void majorUpdateFtranPrepare();

  /**
   * @brief PAMI: Perform the parallel part of multiple FTRANs after a
   * set of minor iterations
   */
  void majorUpdateFtranParallel();

  /**
   * @brief PAMI: Perform the final part of multiple FTRANs after a set
   * of minor iterations
   */
  void majorUpdateFtranFinal();

  /**
   * @brief PAMI: Update the primal values after a set of minor
   * iterations
   */
  void majorUpdatePrimal();

  /**
   * @brief PAMI: Update the invertible representation of the basis
   * matrix after a set of minor iterations
   */
  void majorUpdateFactor();

  /**
   * @brief PAMI: Roll back some iterations if numerical trouble
   * detected when updating the invertible representation of the basis
   * matrix after a set of minor iterations
   */
  void majorRollback();

  // private:
  void saveDualRay();
  void assessPhase1Optimality();
  void assessPhase1OptimalityUnperturbed();
  void exitPhase1ResetDuals();
  void reportOnPossibleLpDualInfeasibility();

  bool checkNonUnitWeightError(std::string message);
  bool dualInfoOk(const HighsLp& lp);
  bool bailoutOnDualObjective();
  HighsDebugStatus debugDualSimplex(const std::string message,
                                    const bool initialise = false);
  HighsFloat* getWorkEdWt() { return &dualRHS.workEdWt[0]; };
  HighsFloat* getWorkEdWtFull() { return &dualRHS.workEdWtFull[0]; };

  // Devex scalars
  HighsInt num_devex_iterations =
      0;  //!< Number of Devex iterations with the current framework
  bool new_devex_framework = false;  //!< Set a new Devex framework
  bool minor_new_devex_framework =
      false;  //!< Set a new Devex framework in PAMI minor iterations

  // References:
  HEkk& ekk_instance_;

  // Model
  HighsInt solver_num_row;
  HighsInt solver_num_col;
  HighsInt solver_num_tot;

  const HighsSparseMatrix* a_matrix;
  const HSimplexNla* simplex_nla;
  HighsSimplexAnalysis* analysis;

  const int8_t* jMove;
  const HighsFloat* workRange;
  const HighsFloat* baseLower;
  const HighsFloat* baseUpper;
  HighsFloat* baseValue;
  HighsFloat* workDual;
  HighsFloat* workValue;
  HighsFloat* colLower;
  HighsFloat* colUpper;
  HighsFloat* rowLower;
  HighsFloat* rowUpper;
  int8_t* nonbasicFlag;

  // Options
  DualEdgeWeightMode dual_edge_weight_mode;
  bool initialise_dual_steepest_edge_weights;
  bool allow_dual_steepest_edge_to_devex_switch;

  const HighsFloat min_dual_steepest_edge_weight = 1e-4;

  HighsFloat Tp;  // Tolerance for primal
  HighsFloat primal_feasibility_tolerance;

  HighsFloat Td;  // Tolerance for dual
  HighsFloat dual_feasibility_tolerance;
  HighsFloat objective_bound;

  HighsInt solve_phase;
  HighsInt rebuild_reason;

  HVector row_ep;
  HVector row_ap;
  HVector col_aq;
  HVector col_BFRT;
  HVector col_DSE;

  HEkkDualRow dualRow;

  // Solving related buffers
  HighsInt dualInfeasCount;

  HEkkDualRHS dualRHS;

  // Simplex pivotal information
  HighsInt row_out;
  HighsInt variable_out;
  HighsInt move_out;  // -1 from small to lower, +1 to upper
  HighsInt variable_in;
  HighsFloat delta_primal;
  HighsFloat theta_dual;
  HighsFloat theta_primal;
  HighsFloat alpha_col;
  HighsFloat alpha_row;
  HighsFloat numericalTrouble;
  // (Local) value of computed weight
  HighsFloat computed_edge_weight;

  bool check_invert_condition = false;

  // Partitioned coefficient matrix
  HighsInt slice_num;
  HighsInt slice_PRICE;
  HighsInt slice_start[kHighsSlicedLimit + 1];
  HighsSparseMatrix slice_a_matrix[kHighsSlicedLimit];
  HighsSparseMatrix slice_ar_matrix[kHighsSlicedLimit];
  HVector slice_row_ap[kHighsSlicedLimit];
  std::vector<HEkkDualRow> slice_dualRow;

  /**
   * @brief Multiple CHUZR data
   */
  struct MChoice {
    HighsInt row_out;
    HighsFloat baseValue;
    HighsFloat baseLower;
    HighsFloat baseUpper;
    HighsFloat infeasValue;
    HighsFloat infeasEdWt;
    HighsFloat infeasLimit;
    HVector row_ep;
    HVector col_aq;
    HVector col_BFRT;
  };

  /**
   * @brief Multiple minor iteration data
   */
  struct MFinish {
    HighsInt move_in;
    HighsFloat shiftOut;
    std::vector<HighsInt> flipList;

    HighsInt row_out;
    HighsInt variable_out;
    HighsInt variable_in;
    HighsFloat alpha_row;
    HighsFloat theta_primal;
    HighsFloat basicBound;
    HighsFloat basicValue;
    HighsFloat EdWt;
    HVector_ptr row_ep;
    HVector_ptr col_aq;
    HVector_ptr col_BFRT;
  };

  HighsInt multi_num;
  HighsInt multi_chosen;
  HighsInt multi_iChoice;
  HighsInt multi_nFinish;
  HighsInt multi_iteration;
  HighsInt multi_chooseAgain;
  MChoice multi_choice[kHighsThreadLimit];
  MFinish multi_finish[kHighsThreadLimit];

  //  HighsFloat build_synthetic_tick;
  //  HighsFloat total_synthetic_tick;
};

#endif /* SIMPLEX_HEKKDUAL_H_ */
