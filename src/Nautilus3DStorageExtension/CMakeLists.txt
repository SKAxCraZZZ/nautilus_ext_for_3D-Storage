set(CURRENT_TARGET "nautilus-3DStorage")

file(GLOB SOURCE_FILES "*.c")
file(GLOB HEADER_FILES "*.h")

add_library(${CURRENT_TARGET} SHARED ${SOURCE_FILES} ${HEADER_FILES}
        Commands/Proto/DataContract.pb-c.c
        Commands/Proto/protobuf-c/protobuf-c.c
        Commands/Utils.c
)

find_package(PkgConfig REQUIRED)

pkg_search_module(GLIB REQUIRED glib-2.0)
target_include_directories(${CURRENT_TARGET} PRIVATE ${GLIB_INCLUDE_DIRS})

pkg_search_module(LIBNAUTILUS REQUIRED libnautilus-extension)
target_include_directories(${CURRENT_TARGET} PRIVATE ${LIBNAUTILUS_INCLUDE_DIRS})

target_link_libraries(${CURRENT_TARGET}
    PRIVATE ${GLIB_LDFLAGS}
    PRIVATE ${LIBNAUTILUS_LDFLAGS}
)
