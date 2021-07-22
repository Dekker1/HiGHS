#include "catch.hpp"
#include "lp_data/HighsOptions.h"
#include "model/HighsHessian.h"
#include "model/HighsHessianUtils.h"
//#include "<cstdio>"

const bool dev_run = false;

// No commas in test case name.
TEST_CASE("HighsHessian", "[highs_hessian]") {
  HighsOptions options;
  if (!dev_run) options.output_flag = false;

  HighsHessian square_hessian;
  square_hessian.dim_ = 5;
  square_hessian.format_ = HessianFormat::kSquare;
  square_hessian.q_start_ = {0, 4, 7, 9, 12, 15};
  square_hessian.q_index_ = {0, 1, 3, 4, 0, 1, 4, 2, 3, 0, 2, 3, 0, 1, 4};
  square_hessian.q_value_ = {5, 1, -1, 2, 1, 4, 1, 3, -1, -1, -1, 4, 2, 1, 5};

  HighsHessian triangular_hessian;
  triangular_hessian.dim_ = 5;
  triangular_hessian.format_ = HessianFormat::kTriangular;
  triangular_hessian.q_start_ = {0, 4, 6, 8, 9, 10};
  triangular_hessian.q_index_ = {0, 1, 3, 4, 1, 4, 2, 3, 3, 4};
  triangular_hessian.q_value_ = {5, 1, -1, 2, 4, 1, 3, -1, 4, 5};

  const bool square_internal_hessian = false;
  HighsHessian use_hessian;
  if (square_internal_hessian) {
    use_hessian = square_hessian;
  } else {
    use_hessian = triangular_hessian;
  }

  REQUIRE(assessHessian(square_hessian, options) == HighsStatus::kOk);
  if (dev_run) {
    printf("\nReturned square Hessian\n");
    square_hessian.print();
  }

  REQUIRE(assessHessian(triangular_hessian, options) == HighsStatus::kOk);
  if (dev_run) {
    printf("\nReturned triangular Hessian\n");
    triangular_hessian.print();
  }

  if (square_internal_hessian) {
    REQUIRE((square_hessian == use_hessian));
  } else {
    REQUIRE((triangular_hessian == use_hessian));
  }

  // Extract the triangluar Hessian from the square Hessian
  REQUIRE(extractTriangularHessian(options, square_hessian) ==
          HighsStatus::kOk);
  if (dev_run) {
    printf("\nReturned triangularised square Hessian\n");
    square_hessian.print();
  }

  // Extract the triangluar Hessian from the triangular Hessian
  REQUIRE(extractTriangularHessian(options, triangular_hessian) ==
          HighsStatus::kOk);
  if (dev_run) {
    printf("\nReturned triangularised triangular Hessian\n");
    triangular_hessian.print();
  }

  HighsHessian negative_diagonal_hessian = triangular_hessian;
  negative_diagonal_hessian.q_value_[0] =
      -negative_diagonal_hessian.q_value_[0];
  negative_diagonal_hessian.q_value_[4] =
      -negative_diagonal_hessian.q_value_[4];
  negative_diagonal_hessian.q_value_[6] =
      -negative_diagonal_hessian.q_value_[6];
  negative_diagonal_hessian.q_value_[8] =
      -negative_diagonal_hessian.q_value_[8];
  negative_diagonal_hessian.q_value_[9] =
      -negative_diagonal_hessian.q_value_[9];
  REQUIRE(assessHessian(negative_diagonal_hessian, options,
                        ObjSense::kMaximize) == HighsStatus::kOk);

  // Square Hessian with only triangular entries - doubled strictly triangular
  // entries.
  HighsHessian hessian0;
  hessian0.dim_ = 5;
  hessian0.format_ = HessianFormat::kSquare;
  hessian0.q_start_ = {0, 1, 3, 4, 7, 10};
  hessian0.q_index_ = {0, 0, 1, 2, 0, 2, 3, 0, 1, 4};
  hessian0.q_value_ = {5, 2, 4, 3, -2, -2, 4, 4, 2, 5};

  REQUIRE(assessHessian(hessian0, options) == HighsStatus::kOk);
  if (dev_run) {
    printf("\nReturned\n");
    hessian0.print();
  }
  REQUIRE((hessian0 == use_hessian));

  // Nonsymmetric Hessian - with entries resulting in cancellation
  HighsHessian hessian1;
  hessian1.format_ = HessianFormat::kSquare;
  hessian1.dim_ = 5;
  hessian1.q_start_ = {0, 3, 5, 7, 10, 14};
  hessian1.q_index_ = {0, 3, 4, 0, 1, 2, 4, 0, 2, 3, 0, 1, 2, 4};
  hessian1.q_value_ = {5, -5, 1, 2, 4, 3, 1, 3, -2, 4, 3, 2, -1, 5};

  REQUIRE(assessHessian(hessian1, options) == HighsStatus::kOk);
  if (dev_run) {
    printf("\nReturned\n");
    hessian1.print();
  }
  REQUIRE((hessian1 == use_hessian));
}
