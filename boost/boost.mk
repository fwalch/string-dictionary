boost_filesystem_sources = codecvt_error_category.cpp operations.cpp path.cpp \
													 path_traits.cpp portability.cpp unique_path.cpp utf8_codecvt_facet.cpp \
													 windows_file_codecvt.cpp
boost_system_sources = error_code.cpp

boost_sources = $(addprefix filesystem/,$(boost_filesystem_sources)) $(addprefix system/,$(boost_system_sources))
boost_cxxflags = -w
