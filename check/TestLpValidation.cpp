#include "Avgas.h"
#include "Highs.h"
#include "HighsLpUtils.h"
#include "catch.hpp"

const bool dev_run = false;

// No commas in test case name.
TEST_CASE("LP-validation", "[highs_data]") {
  // Create an empty LP
  HighsLp lp;
  HighsOptions options;
  HighsStatus return_status;
  options.log_dev_level = kHighsLogDevLevelVerbose;
  if (!dev_run) options.output_flag = false;

  Avgas avgas;
  const HighsInt avgas_num_col = 8;
  const HighsInt avgas_num_row = 10;
  HighsInt num_row = 0;
  HighsInt num_row_nz = 0;
  vector<HighsFloat> rowLower;
  vector<HighsFloat> rowUpper;
  vector<HighsInt> ARstart;
  vector<HighsInt> ARindex;
  vector<HighsFloat> ARvalue;

  for (HighsInt row = 0; row < avgas_num_row; row++) {
    avgas.row(row, num_row, num_row_nz, rowLower, rowUpper, ARstart, ARindex,
              ARvalue);
  }

  HighsInt num_col = 0;
  HighsInt num_col_nz = 0;
  vector<HighsFloat> colCost;
  vector<HighsFloat> colLower;
  vector<HighsFloat> colUpper;
  vector<HighsInt> Astart;
  vector<HighsInt> Aindex;
  vector<HighsFloat> Avalue;
  for (HighsInt col = 0; col < avgas_num_col; col++) {
    avgas.col(col, num_col, num_col_nz, colCost, colLower, colUpper, Astart,
              Aindex, Avalue);
  }

  return_status = assessLp(lp, options);
  REQUIRE(return_status == HighsStatus::kOk);
  //  reportLp(lp, HighsLogType::kVerbose);

  const HighsFloat my_infinity = 1e30;
  Highs highs;
  highs.passOptions(options);

  REQUIRE(highs.passModel(lp) == HighsStatus::kOk);
  return_status =
      highs.addRows(num_row, &rowLower[0], &rowUpper[0], 0, NULL, NULL, NULL);
  REQUIRE(return_status == HighsStatus::kOk);

  return_status =
      highs.addCols(num_col, &colCost[0], &colLower[0], &colUpper[0],
                    num_col_nz, &Astart[0], &Aindex[0], &Avalue[0]);
  REQUIRE(return_status == HighsStatus::kOk);

  // Create an empty column
  HighsInt XnumNewCol = 1;
  HighsInt XnumNewNZ = 0;
  vector<HighsFloat> XcolCost;
  XcolCost.resize(XnumNewCol);
  XcolCost[0] = 1;
  vector<HighsFloat> XcolLower;
  XcolLower.resize(XnumNewCol);
  XcolLower[0] = 0;
  vector<HighsFloat> XcolUpper;
  XcolUpper.resize(XnumNewCol);
  XcolUpper[0] = 1e25;
  vector<HighsInt> XAstart;
  XAstart.resize(XnumNewCol);
  vector<HighsInt> XAindex;
  vector<HighsFloat> XAvalue;
  // Add an empty column
  return_status =
      highs.addCols(XnumNewCol, &XcolCost[0], &XcolLower[0], &XcolUpper[0],
                    XnumNewNZ, &XAstart[0], NULL, NULL);
  REQUIRE(return_status == HighsStatus::kOk);
  XcolUpper[0] = my_infinity;
  //  reportLp(lp, HighsLogType::kVerbose);

  // Try to add a column with illegal cost
  HighsStatus require_return_status;
  if (kHighsAllowInfiniteCosts) {
    require_return_status = HighsStatus::kOk;
  } else {
    require_return_status = HighsStatus::kError;
  }
  XcolCost[0] = my_infinity;
  return_status =
      highs.addCols(XnumNewCol, &XcolCost[0], &XcolLower[0], &XcolUpper[0],
                    XnumNewNZ, &XAstart[0], NULL, NULL);
  REQUIRE(return_status == require_return_status);

  XcolCost[0] = -my_infinity;
  return_status =
      highs.addCols(XnumNewCol, &XcolCost[0], &XcolLower[0], &XcolUpper[0],
                    XnumNewNZ, &XAstart[0], NULL, NULL);
  REQUIRE(return_status == require_return_status);

  // Reset to a legitimate cost
  XcolCost[0] = 1;

  // Add a column with bound inconsistency due to upper
  XcolLower[0] = 0;
  XcolUpper[0] = -1;
  return_status =
      highs.addCols(XnumNewCol, &XcolCost[0], &XcolLower[0], &XcolUpper[0],
                    XnumNewNZ, &XAstart[0], NULL, NULL);
  REQUIRE(return_status == HighsStatus::kWarning);

  // Add a column with bound inconsistency due to lower
  XcolLower[0] = 1;
  XcolUpper[0] = 0;
  return_status =
      highs.addCols(XnumNewCol, &XcolCost[0], &XcolLower[0], &XcolUpper[0],
                    XnumNewNZ, &XAstart[0], NULL, NULL);
  REQUIRE(return_status == HighsStatus::kWarning);

  // Add a column with illegal bound due to lower
  XcolLower[0] = my_infinity;
  XcolUpper[0] = 0;
  return_status =
      highs.addCols(XnumNewCol, &XcolCost[0], &XcolLower[0], &XcolUpper[0],
                    XnumNewNZ, &XAstart[0], NULL, NULL);
  REQUIRE(return_status == HighsStatus::kError);

  // Add a column with illegal bound due to upper
  XcolLower[0] = 0;
  XcolUpper[0] = -my_infinity;
  return_status =
      highs.addCols(XnumNewCol, &XcolCost[0], &XcolLower[0], &XcolUpper[0],
                    XnumNewNZ, &XAstart[0], NULL, NULL);
  REQUIRE(return_status == HighsStatus::kError);

  // Add a legitimate column
  XcolLower[0] = 0;
  XcolUpper[0] = 0;
  return_status =
      highs.addCols(XnumNewCol, &XcolCost[0], &XcolLower[0], &XcolUpper[0],
                    XnumNewNZ, &XAstart[0], NULL, NULL);
  REQUIRE(return_status == HighsStatus::kOk);

  //  reportLp(lp, HighsLogType::kVerbose);

  // Add a couple of non-empty columns with some small and large values
  XnumNewCol = 2;
  XnumNewNZ = 7;
  XcolCost.resize(XnumNewCol);
  XcolCost[0] = 1;
  XcolCost[1] = 2;
  XcolLower.resize(XnumNewCol);
  XcolLower[0] = 0;
  XcolLower[1] = 0;
  XcolUpper.resize(XnumNewCol);
  XcolUpper[0] = 1;
  XcolUpper[1] = 1;
  XAstart.resize(XnumNewCol + 1);
  XAindex.resize(XnumNewNZ);
  XAstart[1] = 4;
  XAstart[2] = XnumNewNZ;
  XAindex[0] = 0;
  XAindex[1] = 2;
  XAindex[2] = 3;
  XAindex[3] = 9;
  XAindex[4] = 1;
  XAindex[5] = 3;
  XAindex[6] = 8;
  XAvalue.resize(XnumNewNZ);
  XAvalue[0] = 1;
  XAvalue[1] = 1e-12;
  XAvalue[2] = -1e-20;
  XAvalue[3] = -1;
  XAvalue[4] = -1e60;
  XAvalue[5] = 1e100;
  XAvalue[6] = -1;
  return_status =
      highs.addCols(XnumNewCol, &XcolCost[0], &XcolLower[0], &XcolUpper[0],
                    XnumNewNZ, &XAstart[0], &XAindex[0], &XAvalue[0]);
  REQUIRE(return_status == HighsStatus::kError);

  // Legitimise large matrix entries. Small entries now cause warning
  XAvalue[4] = -1;
  XAvalue[5] = 1;
  return_status =
      highs.addCols(XnumNewCol, &XcolCost[0], &XcolLower[0], &XcolUpper[0],
                    XnumNewNZ, &XAstart[0], &XAindex[0], &XAvalue[0]);
  REQUIRE(return_status == HighsStatus::kWarning);

  if (!dev_run) highs.setOptionValue("output_flag", false);

  const HighsLp& internal_lp = highs.getLp();
  HighsFloat check_value;
  REQUIRE(highs.getCoeff(-1, 0, check_value) == HighsStatus::kError);
  REQUIRE(highs.getCoeff(0, -1, check_value) == HighsStatus::kError);
  REQUIRE(highs.getCoeff(internal_lp.num_row_, 0, check_value) ==
          HighsStatus::kError);
  REQUIRE(highs.getCoeff(0, internal_lp.num_col_, check_value) ==
          HighsStatus::kError);

  const HighsInt check_col = 4;
  const HighsInt check_row = 7;
  REQUIRE(highs.getCoeff(check_col, check_row, check_value) ==
          HighsStatus::kOk);
  REQUIRE(check_value == 0);

  const HighsFloat value = -3;
  REQUIRE(highs.getCoeff(check_row, check_col, check_value) ==
          HighsStatus::kOk);
  REQUIRE(check_value == value);

  // This is a highly anomalous LP. It has two pairs of inconsistent
  // bounds (cols 11 and 12) but also has costs of 1e+30 and -1e+30
  // for columns 9 and 10.

  // LP is found to be unbounded by presolve, but is primal
  // infeasible. With isBoundInfeasible check in solveLp,
  // infeasiblility is identified before reaching a solver, so
  // presolve isn't called
  HighsStatus run_status;
  HighsModelStatus model_status;
  run_status = highs.run();
  REQUIRE(run_status == HighsStatus::kOk);
  model_status = highs.getModelStatus();
  REQUIRE(model_status == HighsModelStatus::kInfeasible);

  REQUIRE(highs.changeCoeff(-1, 0, check_value) == HighsStatus::kError);
  REQUIRE(highs.changeCoeff(0, -1, check_value) == HighsStatus::kError);
  REQUIRE(highs.changeCoeff(internal_lp.num_row_, 0, check_value) ==
          HighsStatus::kError);
  REQUIRE(highs.changeCoeff(0, internal_lp.num_col_, check_value) ==
          HighsStatus::kError);

  const HighsFloat to_value = 99;
  REQUIRE(highs.changeCoeff(check_row, check_col, to_value) ==
          HighsStatus::kOk);
  REQUIRE(highs.getCoeff(check_row, check_col, check_value) ==
          HighsStatus::kOk);
  REQUIRE(check_value == to_value);
}
