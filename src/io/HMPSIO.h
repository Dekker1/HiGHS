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
/**@file io/HMPSIO.h
 * @brief
 */
#ifndef IO_HMPSIO_H_
#define IO_HMPSIO_H_

#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>

#include "io/Filereader.h"
#include "util/HighsInt.h"

using std::string;
using std::vector;

const HighsInt MPS_ROW_TY_N = 0;
const HighsInt MPS_ROW_TY_E = 1;
const HighsInt MPS_ROW_TY_L = 2;
const HighsInt MPS_ROW_TY_G = 3;
const HighsInt field_1_start = 1;
const HighsInt field_1_width = 2;
const HighsInt field_2_start = 4;
const HighsInt field_2_width = 8;
const HighsInt field_3_start = 14;
const HighsInt field_3_width = 8;
const HighsInt field_4_start = 24;
const HighsInt field_4_width = 12;
const HighsInt field_5_start = 39;
const HighsInt field_5_width = 8;
const HighsInt field_6_start = 49;
const HighsInt field_6_width = 12;

FilereaderRetcode readMps(
    const HighsLogOptions& log_options, const std::string filename,
    HighsInt mxNumRow, HighsInt mxNumCol, HighsInt& numRow, HighsInt& numCol,
    ObjSense& objSense, HighsFloat& objOffset, vector<HighsInt>& Astart,
    vector<HighsInt>& Aindex, vector<HighsFloat>& Avalue, vector<HighsFloat>& colCost,
    vector<HighsFloat>& colLower, vector<HighsFloat>& colUpper,
    vector<HighsFloat>& rowLower, vector<HighsFloat>& rowUpper,
    vector<HighsVarType>& integerColumn, vector<std::string>& col_names,
    vector<std::string>& row_names, const HighsInt keep_n_rows = 0);

HighsStatus writeMps(
    const HighsLogOptions& log_options, const std::string filename,
    const std::string model_name, const HighsInt& num_row,
    const HighsInt& num_col, const HighsInt& q_dim, const ObjSense& sense,
    const HighsFloat& offset, const vector<HighsFloat>& col_cost,
    const vector<HighsFloat>& col_lower, const vector<HighsFloat>& col_upper,
    const vector<HighsFloat>& row_lower, const vector<HighsFloat>& row_upper,
    const vector<HighsInt>& a_start, const vector<HighsInt>& a_index,
    const vector<HighsFloat>& a_value, const vector<HighsInt>& q_start,
    const vector<HighsInt>& q_index, const vector<HighsFloat>& q_value,
    const vector<HighsVarType>& integrality,
    const vector<std::string>& col_names, const vector<std::string>& row_names,
    const bool use_free_format = true);

bool load_mpsLine(FILE* file, HighsVarType& integerVar, HighsInt lmax,
                  char* line, char* flag, HighsFloat* data);

HighsStatus writeModelAsMps(const HighsOptions& options,
                            const std::string filename, const HighsModel& model,
                            const bool free = true);

#endif /* IO_HMPSIO_H_ */
