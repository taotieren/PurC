include(GNUInstallDirs)

calculate_library_versions_from_libtool_triple(PURC 0 0 0)

# These are shared variables, but we special case their definition so that we
# can use the CMAKE_INSTALL_* variables that are populated by the GNUInstallDirs
# macro.
set(LIB_INSTALL_DIR
    "${CMAKE_INSTALL_FULL_LIBDIR}"
    CACHE PATH "Absolute path to library installation directory")
set(EXEC_INSTALL_DIR
    "${CMAKE_INSTALL_FULL_BINDIR}"
    CACHE PATH "Absolute path to executable installation directory")
set(LIBEXEC_INSTALL_DIR
    "${CMAKE_INSTALL_FULL_LIBEXECDIR}/purc"
    CACHE PATH "Absolute path to install executables executed by the library")
set(HEADER_INSTALL_DIR
    "${CMAKE_INSTALL_INCLUDEDIR}"
    CACHE PATH "Absolute path to header installation directory")
set(PURC_HEADER_INSTALL_DIR
    "${CMAKE_INSTALL_INCLUDEDIR}/purc"
    CACHE PATH "Absolute path to PurC header installation directory")

add_definitions(-DBUILDING_LINUX__=1)
# add_definitions(-DPURC_API_VERSION_STRING="${PURC_API_VERSION}")
add_definitions(-DPURC_LIBEXEC_DIR="${LIBEXEC_INSTALL_DIR}")

find_package(ZLIB 1.2.0 REQUIRED)
find_package(GLIB 2.44.0 REQUIRED COMPONENTS gio gio-unix gmodule gobject)
find_package(Threads REQUIRED)
find_package(BISON 3.0 REQUIRED)
find_package(FLEX 2.6.4 REQUIRED)
find_package(
  Python3 3.9.0
  COMPONENTS Interpreter
  REQUIRED)
find_package(Python3 3.9.0 COMPONENTS Development)
find_package(DNSSD)
find_package(Ncurses 5.0)
find_package(LibCheck 0.15.2)
find_package(LibXml2 2.8.0)
find_package(OpenSSL 1.1.1)
find_package(LibGcrypt 1.6.0)
find_package(SQLite3 3.10.0)
find_package(MySQLClient 20.0.0)

purc_option_begin()

if(NOT DNSSD_FOUND)
  set(ENABLE_DNSSD_DEFAULT OFF)
else()
  set(ENABLE_DNSSD_DEFAULT ON)
endif()

set(ENABLE_EXTDVOBJ_FS_DEFAULT ON)
set(ENABLE_EXTDVOBJ_MATH_DEFAULT ON)

if(NOT Python3_Development_FOUND)
  set(ENABLE_EXTDVOBJ_PY_DEFAULT OFF)
else()
  set(ENABLE_EXTDVOBJ_PY_DEFAULT ON)
endif()

if(NOT SQLITE3_FOUND)
  set(ENABLE_SCHEMA_LSQL_DEFAULT OFF)
  set_and_expose_to_build(HAVE_SQLITE3 OFF)
  set(ENABLE_EXTDVOBJ_SQLITE_DEFAULT OFF)
else()
  set(ENABLE_SCHEMA_LSQL_DEFAULT ON)
  set_and_expose_to_build(HAVE_SQLITE3 ON)
  set(ENABLE_EXTDVOBJ_SQLITE_DEFAULT ON)
endif()

if(NOT MYSQLCLIENT_FOUND)
  set(ENABLE_SCHEMA_RSQL_DEFAULT OFF)
  set_and_expose_to_build(HAVE_MYSQLCLIENT OFF)
else()
  set(ENABLE_SCHEMA_RSQL_DEFAULT ON)
  set_and_expose_to_build(HAVE_MYSQLCLIENT ON)
endif()

if(NOT ENABLE_REMOTE_FETCHER_DEFAULT OR NOT ENABLE_REMOTE_FETCHER)
  set(ENABLE_SCHEMA_LSQL_DEFAULT OFF)
  set(ENABLE_SCHEMA_RSQL_DEFAULT OFF)
endif()

if(NOT GLIB_FOUND)
  set(ENABLE_SOCKET_STREAM_DEFAULT OFF)
  set_and_expose_to_build(HAVE_GLIB OFF)
else()
  set(ENABLE_SOCKET_STREAM_DEFAULT ON)
  set_and_expose_to_build(HAVE_GLIB ON)
  if(${GLIB_VERSION} VERSION_LESS 2.70)
    set_and_expose_to_build(HAVE_GLIB_LESS_2_70 ON)
  else()
    set_and_expose_to_build(HAVE_GLIB_LESS_2_70 OFF)
  endif()
endif()

# GUri is available in GLib since version 2.66, but we only want to use it if
# version is >= 2.67.1.↵
if(PC_GLIB_VERSION VERSION_GREATER "2.67.1" OR PC_GLIB_VERSION STREQUAL
                                               "2.67.1")
  set_and_expose_to_build(HAVE_GURI 1)
endif()

if(NOT Ncurses_FOUND)
  set(ENABLE_RENDERER_FOIL_DEFAULT OFF)
  set_and_expose_to_build(HAVE_NCURSES OFF)
else()
  set(ENABLE_RENDERER_FOIL_DEFAULT ON)
  set_and_expose_to_build(HAVE_NCURSES ON)
endif()

if(NOT LIBXML2_FOUND)
  set(ENABLE_DOCTYPE_XML_DEFAULT OFF)
  set_and_expose_to_build(HAVE_LIBXML2 OFF)
else()
  set(ENABLE_DOCTYPE_XML_DEFAULT ON)
  set_and_expose_to_build(HAVE_LIBXML2 ON)
endif()

if(NOT OPENSSL_FOUND)
  set(ENABLE_SSL_DEFAULT OFF)
  set_and_expose_to_build(HAVE_OPENSSL OFF)
else()
  set(ENABLE_SSL_DEFAULT ON)
  set_and_expose_to_build(HAVE_OPENSSL ON)
endif()

if(USE_SOUP2)
  set(SOUP_MINIMUM_VERSION 2.54.0)
  set(SOUP_API_VERSION 2.4)
else()
  set(SOUP_MINIMUM_VERSION 2.99.9)
  set(SOUP_API_VERSION 3.0)
  set(ENABLE_SERVER_PRECONNECT ON)
endif()

find_package(LibSoup ${SOUP_MINIMUM_VERSION})

if(NOT LibSoup_FOUND)
  if(USE_SOUP2)
    message(FATAL_ERROR "libsoup is required.")
  else()
    message(
      FATAL_ERROR
        "libsoup 3 is required. Enable USE_SOUP2 to use libsoup 2 (disables HTTP/2)"
    )
  endif()
endif()

if(NOT LIBSOUP_FOUND OR NOT LIBGCRYPT_FOUND)
  set(ENABLE_REMOTE_FETCHER_DEFAULT OFF)
  if(NOT LIBSOUP_FOUND)
    set_and_expose_to_build(HAVE_LIBSOUP OFF)
  endif()
  if(NOT LIBGCRYPT_FOUND)
    set_and_expose_to_build(HAVE_LIBGCRYPT OFF)
  endif()
else()
  set(ENABLE_REMOTE_FETCHER_DEFAULT ON)
  set_and_expose_to_build(HAVE_LIBSOUP ON)
  set_and_expose_to_build(HAVE_LIBGCRYPT ON)
endif()

# Public options specific to the Linux port. Do not add any options here unless
# there is a strong reason we should support changing the value of the option,
# and the option is not relevant to any other PurC ports.
# PURC_OPTION_DEFINE(USE_SYSTEMD "Whether to enable journald logging" PUBLIC ON)

# Private options specific to the Linux port. Changing these options is
# completely unsupported. They are intended for use only by PurC developers.
# PURC_OPTION_DEFINE(USE_ANGLE_WEBGL "Whether to use ANGLE as WebGL backend."
# PRIVATE OFF) PURC_OPTION_DEPEND(ENABLE_WEBGL ENABLE_GRAPHICS_CONTEXT_GL)

purc_option_default_port_value(ENABLE_DNSSD PUBLIC ${ENABLE_DNSSD_DEFAULT})
purc_option_default_port_value(ENABLE_EXTDVOBJ_MATH PUBLIC
                               ${ENABLE_EXTDVOBJ_MATH_DEFAULT})
purc_option_default_port_value(ENABLE_EXTDVOBJ_FS PUBLIC
                               ${ENABLE_EXTDVOBJ_FS_DEFAULT})
purc_option_default_port_value(ENABLE_EXTDVOBJ_PY PUBLIC
                               ${ENABLE_EXTDVOBJ_PY_DEFAULT})
purc_option_default_port_value(ENABLE_EXTDVOBJ_SQLITE PUBLIC
                               ${ENABLE_EXTDVOBJ_SQLITE_DEFAULT})
purc_option_default_port_value(ENABLE_SOCKET_STREAM PUBLIC
                               ${ENABLE_SOCKET_STREAM_DEFAULT})
purc_option_default_port_value(ENABLE_RENDERER_FOIL PUBLIC
                               ${ENABLE_RENDERER_FOIL_DEFAULT})
purc_option_default_port_value(ENABLE_DOCTYPE_XML PUBLIC
                               ${ENABLE_DOCTYPE_XML_DEFAULT})
purc_option_default_port_value(ENABLE_SSL PUBLIC ${ENABLE_SSL_DEFAULT})
purc_option_default_port_value(ENABLE_REMOTE_FETCHER PUBLIC
                               ${ENABLE_REMOTE_FETCHER_DEFAULT})
purc_option_default_port_value(ENABLE_SCHEMA_LSQL PUBLIC
                               ${ENABLE_SCHEMA_LSQL_DEFAULT})
purc_option_default_port_value(ENABLE_SCHEMA_RSQL PUBLIC
                               ${ENABLE_SCHEMA_RSQL_DEFAULT})

# Finalize the value for all options. Do not attempt to use an option before
# this point, and do not attempt to change any option after this point.
purc_option_end()

if(USE_LIBSECRET)
  find_package(Libsecret)
  if(NOT LIBSECRET_FOUND)
    message(FATAL_ERROR "libsecret is needed for USE_LIBSECRET")
  endif()
endif()

set(PurC_PKGCONFIG_FILE ${CMAKE_BINARY_DIR}/Source/PurC/purc.pc)
set(CSSEng_PKGCONFIG_FILE ${CMAKE_BINARY_DIR}/Source/CSSEng/csseng.pc)
set(DOMRuler_PKGCONFIG_FILE ${CMAKE_BINARY_DIR}/Source/DOMRuler/domruler.pc)
set(RemoteFetcher_PKGCONFIG_FILE
    ${CMAKE_BINARY_DIR}/Source/RemoteFetcher/purc-fetcher.pc)

set(PurC_LIBRARY_TYPE SHARED)
set(CSSEng_LIBRARY_TYPE SHARED)
set(DOMRuler_LIBRARY_TYPE SHARED)
set(RemoteFetcher_LIBRARY_TYPE SHARED)

# CMake does not automatically add --whole-archive when building shared objects
# from a list of convenience libraries. This can lead to missing symbols in the
# final output. We add --whole-archive to all libraries manually to prevent the
# linker from trimming symbols that we actually need later. With ld64 on darwin,
# we use -all_load instead.
macro(ADD_WHOLE_ARCHIVE_TO_LIBRARIES _list_name)
  if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    list(APPEND ${_list_name} -Wl,-all_load)
  else()
    set(_tmp)
    foreach(item IN LISTS ${_list_name})
      if("${item}" STREQUAL "PRIVATE" OR "${item}" STREQUAL "PUBLIC")
        list(APPEND _tmp "${item}")
      else()
        list(APPEND _tmp -Wl,--whole-archive "${item}" -Wl,--no-whole-archive)
      endif()
    endforeach()
    set(${_list_name} ${_tmp})
  endif()
endmacro()

# include(BubblewrapSandboxChecks)
