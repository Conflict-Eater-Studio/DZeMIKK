project(DZeMIKK_Tests LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(dzemikk_tests
    test_hierarchy.cpp
)

target_link_libraries(dzemikk_tests
    PRIVATE
    DZeMIKK
    GTest::gtest_main
)

target_compile_options(dzemikk_tests PRIVATE /utf-8)

include(GoogleTest)
gtest_discover_tests(dzemikk_tests)
