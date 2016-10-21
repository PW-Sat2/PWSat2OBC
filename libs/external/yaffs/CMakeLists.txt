set(NAME yaffs)

set(SOURCES
    src/yaffsfs.c
    src/yaffs_guts.c
    src/yaffs_nand.c    
    src/yaffs_checkptrw.c    
    src/yaffs_tagscompat.c
    src/yaffs_ecc.c
    src/yaffs_packedtags2.c
    src/yaffs_yaffs2.c
    src/yaffs_yaffs1.c
    src/yaffs_verify.c
    src/yaffs_hweight.c
    src/yaffs_summary.c
    src/yaffs_allocator.c
    src/yaffs_bitmap.c
    src/yaffs_nameval.c
    src/yaffs_attribs.c
    src/yaffs_tagsmarshall.c
)

add_library(${NAME} STATIC ${SOURCES})

target_include_directories(${NAME} PUBLIC src)

target_compile_definitions(${NAME} PUBLIC
    -DCONFIG_YAFFS_PROVIDE_DEFS
    -DCONFIG_YAFFS_DEFINES_TYPES
    -DCONFIG_YAFFS_DIRECT
    -DCONFIG_YAFFS_YAFFS2 
    -Dloff_t=int64_t
)

set_target_properties(${NAME} PROPERTIES COMPILE_FLAGS "\
    -Wno-type-limits \
    -Wno-sign-compare \
    -Wno-implicit-function-declaration \
    -Wno-unused-but-set-variable \
    -Wno-unused-parameter \
    -Wno-unused-function \
")