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
/**@file mip/HighsRedcostFixing.h
 * @brief reduced cost fixing using the current cutoff bound
 */

#ifndef HIGHS_REDCOST_FIXING_H_
#define HIGHS_REDCOST_FIXING_H_

#include <map>
#include <vector>

#include "mip/HighsDomainChange.h"

class HighsDomain;
class HighsMipSolver;
class HighsLpRelaxation;

class HighsRedcostFixing {
  std::vector<std::multimap<HighsFloat, int>> lurkingColUpper;
  std::vector<std::multimap<HighsFloat, int>> lurkingColLower;

 public:
  std::vector<std::pair<HighsFloat, HighsDomainChange>> getLurkingBounds(
      const HighsMipSolver& mipsolver) const;

  void propagateRootRedcost(const HighsMipSolver& mipsolver);

  static void propagateRedCost(const HighsMipSolver& mipsolver,
                               HighsDomain& localdomain,
                               const HighsLpRelaxation& lp);

  void addRootRedcost(const HighsMipSolver& mipsolver,
                      const std::vector<HighsFloat>& lpredcost, HighsFloat lpobjective);
};

#endif
