add_executable(gem_core_tests gem_core_tests.cpp)

target_link_libraries(gem_core_tests gem)
target_include_directories(gem_core_tests PUBLIC ${GEM_INCLUDES})