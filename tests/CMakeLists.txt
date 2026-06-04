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
    test_gameobject.cpp
    test_componentRegister.cpp
    test_serializers.cpp
    test_findGameObject.cpp
)

target_link_libraries(dzemikk_tests
    PRIVATE
    DZeMIKK
    GTest::gtest_main
)

add_custom_command(TARGET dzemikk_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${FMOD_LIBS_PATH}/fmod.dll"
    $<TARGET_FILE_DIR:dzemikk_tests>

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${FMOD_LIBS_PATH}/fmodL.dll"
    $<TARGET_FILE_DIR:dzemikk_tests>
)

    if (MSVC)
        target_compile_options(dzemikk_tests PRIVATE /utf-8)
    else()
        target_compile_options(dzemikk_tests PRIVATE -finput-charset=UTF-8)

    endif()


include(GoogleTest)
gtest_discover_tests(dzemikk_tests)
