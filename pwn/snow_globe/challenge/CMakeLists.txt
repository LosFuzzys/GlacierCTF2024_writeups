cmake_minimum_required(VERSION 3.14)

project (snow_globe)

find_package(ImageMagick REQUIRED COMPONENTS MagickCore)
find_package(PkgConfig)

pkg_check_modules(seccomp REQUIRED IMPORTED_TARGET libseccomp)

add_compile_options("$<$<CONFIG:Debug>:-Wall;-Wextra>")
add_compile_options("$<$<CONFIG:Release>:-O2;-g0;-DNDEBUG>")

add_executable(snow_globe snow_globe.c)
target_compile_features(snow_globe PUBLIC c_std_99)
target_compile_options(snow_globe PUBLIC -O2 -ggdb3)
target_link_libraries(snow_globe ImageMagick::MagickCore m)

add_executable(extractor extractor.c)
target_compile_features(extractor PUBLIC c_std_99)
target_link_options(extractor PUBLIC "$<$<CONFIG:Release>:-s>")
target_link_libraries(extractor ImageMagick::MagickCore m)

add_executable(globe_wrapper globe_wrapper.c)
target_compile_features(globe_wrapper PUBLIC c_std_99)
target_link_options(globe_wrapper PUBLIC "$<$<CONFIG:Release>:-s>")
target_link_libraries(globe_wrapper PkgConfig::seccomp)
