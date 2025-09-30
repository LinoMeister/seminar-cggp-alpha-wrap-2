#include <catch2/catch_test_macros.hpp>
#include <alpha_wrap_2/alpha_wrap_2.h>

TEST_CASE("builds and links", "[smoke]") {
    // Smoke test: just construct the oracle and alpha_wrap_2 to ensure linkage
    aw2::point_set_oracle_2 oracle;
    aw2::alpha_wrap_2 aw(oracle);
    SUCCEED("linked");
}
