cmake_minimum_required(VERSION 3.5)
project(wxdialogue)

add_compile_definitions(_WXDIALOGUE_DLL)

if (MSVC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /TC")
	add_compile_options("/MD")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251 /wd4996 /Qpar  /permissive")
	add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/source-charset:utf-8>")

endif()

# 添加文件
aux_source_directory(./c/ WXDL_BASIC_SOURCE)

#aux_source_directory(./ WX_CONFIG_BASIC_SOURCE)

add_library(wxdialogue SHARED ${WXDL_BASIC_SOURCE})

#add_library(wxconfig STATIC ${WX_CONFIG_BASIC_SOURCE})
#target_link_libraries(${PROJECT_NAME} wxc)

add_executable(wxdl_exam ./c/exam/wxdl.c)
target_link_libraries(wxdl_exam wxdialogue)
