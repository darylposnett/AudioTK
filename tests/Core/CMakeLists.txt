
FILE(GLOB
  ATK_CORE_TEST_SRC
  *.cpp
)

FILE(GLOB
  ATK_CORE_TEST_HEADERS
  *.h*
)

SET(ATK_CORE_TEST_SIMD_SRC
  SIMD/QuaternionConvertFilter.cpp
)

SET(ATK_CORE_TEST_DEFINITIONS)
SET(ATK_CORE_TEST_INCLUDE)
SET(ATK_CORE_TEST_LIBRARIES ATKTools ATKMock ATKCore ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

SET(ATK_CORE_TEST_NAME ATKCore_test)
SET(ATK_CORE_TEST_TESTNAME Core)
SET(ATK_CORE_TEST_FOLDER Core_test)

ATK_add_test(ATK_CORE_TEST)
