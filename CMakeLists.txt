cmake_minimum_required(VERSION 3.20)  # ← важно!
project(QtKursovik)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets)
qt_standard_project_setup()

include(ExternalProject)

set(OCCT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/opencascade")
set(OCCT_INSTALL_DIR "${CMAKE_BINARY_DIR}/occt-install")
set(OCCT_INCLUDE_DIR "${OCCT_INSTALL_DIR}/include/opencascade")
set(OCCT_LIBRARY_DIR "${OCCT_INSTALL_DIR}/lib")

# Список модулей
set(OCCT_MODULES TKernel TKMath TKBRep TKMesh TKPrim TKTopAlgo)

# Формируем список byproducts
set(OCCT_BYPRODUCTS)
foreach(mod ${OCCT_MODULES})
    list(APPEND OCCT_BYPRODUCTS "${OCCT_LIBRARY_DIR}/lib${mod}.so")
endforeach()
# Обязательно включаем Standard_Version.hxx — он генерируется при install!
list(APPEND OCCT_BYPRODUCTS "${OCCT_INCLUDE_DIR}/Standard_Version.hxx")

ExternalProject_Add(
    OpenCASCADE
    SOURCE_DIR "${OCCT_SOURCE_DIR}"
    CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_INSTALL_PREFIX=${OCCT_INSTALL_DIR}
        -DBUILD_MODULE_Draw=OFF
        -DBUILD_MODULE_Visualization=OFF
        -DBUILD_MODULE_DataExchange=OFF
        -DBUILD_LIBRARY_TYPE=Shared
    BUILD_ALWAYS OFF
    BUILD_BYPRODUCTS ${OCCT_BYPRODUCTS}
    INSTALL_DIR "${OCCT_INSTALL_DIR}"
)

# Исходники
file(GLOB_RECURSE sources_cpp CONFIGURE_DEPENDS src/*.cpp)
file(GLOB_RECURSE sources_h CONFIGURE_DEPENDS src/*.h)
add_executable(${PROJECT_NAME} ${sources_cpp} ${sources_h})

# Qt
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::OpenGLWidgets)

# Заголовки OCCT
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${OCCT_INCLUDE_DIR})

# Линковка: используем абсолютные пути (Ninja теперь знает, что они появятся)
foreach(mod ${OCCT_MODULES})
    target_link_libraries(${PROJECT_NAME} PRIVATE "${OCCT_LIBRARY_DIR}/lib${mod}.so")
endforeach()

# Важно: зависимость от цели OpenCASCADE
add_dependencies(${PROJECT_NAME} OpenCASCADE)