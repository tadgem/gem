#include "gem/gem.h"
#include "gem_test.h"

BEGIN_TESTS()

TEST("Test Test",
{
   return TEST_RESULT::PASS;
})

TEST("Test Test 2",
{
  glm::vec3 position (1,2,3);
  glm::vec3 scale (0.5f);

  glm::mat4 model = gem::utils::get_model_matrix(position, glm::vec3(0.0), scale);

  spdlog::info("Model Matrix : {}", model);
  return TEST_RESULT::PASS;
})

RUN_TESTS()