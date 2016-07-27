cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_VERBOSE_MAKEFILE on)

set(lib_build_dir /home/victor_ponomarenko/AndroidStudioProjects/Jni3rdPartyLibSample/lib_build_dir)

file(MAKE DIRECTORY ${lib_build_dir})

# configure import libs
set(ffmpeg ${CMAKE_SOURCE_DIR}/../../../../ffmpeg)

add_library( libavcodec SHARED IMPORTED )
set_target_properties( libavcodec PROPERTIES IMPORTED_LOCATION ${ffmpeg}/libavcodec/lib/${ANDROID_ABI}/libavcodec.so )

add_library( libavfilter SHARED IMPORTED )
set_target_properties( libavfilter PROPERTIES IMPORTED_LOCATION ${ffmpeg}/libavfilter/lib/${ANDROID_ABI}/libavfilter.so )

add_library( libavformat SHARED IMPORTED )
set_target_properties( libavformat PROPERTIES IMPORTED_LOCATION ${ffmpeg}/libavformat/lib/${ANDROID_ABI}/libavformat.so )

add_library( libavutil SHARED IMPORTED )
set_target_properties( libavutil PROPERTIES IMPORTED_LOCATION ${ffmpeg}/libavutil/lib/${ANDROID_ABI}/libavutil.so )

add_library( libswresample SHARED IMPORTED )
set_target_properties( libswresample PROPERTIES IMPORTED_LOCATION ${ffmpeg}/libswresample/lib/${ANDROID_ABI}/libswresample.so )

add_library( libswscale SHARED IMPORTED )
set_target_properties( libswscale PROPERTIES IMPORTED_LOCATION ${ffmpeg}/libswscale/lib/${ANDROID_ABI}/libswscale.so )

include_directories(${ffmpeg}/libavcodec/include,
                    ${ffmpeg}/libavfilter/include,
                    ${ffmpeg}/libavformat/include,
                    ${ffmpeg}/libavutil/include,
                    ${ffmpeg}/libswresample/include,
                    ${ffmpeg}/libswscale/include)

set(SOURCES
    src/main/cpp/jni-3rd-party-lib.c,
    src/main/cpp/streaming.cpp)

set(HEADERS
    src/main/cpp/jni-3rd-party-lib.h)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
add_library(jni_lib SHARED ${HEADERS} ${SOURCES})

target_link_libraries(jni_lib log android)