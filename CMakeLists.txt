CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(Tulip-plugin-similarity-computer CXX)

FIND_PACKAGE(TULIP REQUIRED)

INCLUDE_DIRECTORIES(${TULIP_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})


SET(PLUGIN_NAME "DoubleVectorSimilarityComputer-${TULIP_VERSION}")

ADD_LIBRARY(${PLUGIN_NAME} SHARED DoubleVectorSimilarityComputer.cpp)
TARGET_LINK_LIBRARIES(${PLUGIN_NAME} ${TULIP_LIBRARIES})
INSTALL(TARGETS ${PLUGIN_NAME} LIBRARY DESTINATION ${TULIP_PLUGINS_DIR})
