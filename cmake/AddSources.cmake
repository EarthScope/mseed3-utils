#utility to create a source collections

macro (add_sources _target)
    if (NOT ${_target}_source_dir)
        init_target_source_dir(${_target})
    endif (NOT ${_target}_source_dir)

    file (RELATIVE_PATH _relPath "${${_target}_source_dir}" "${CMAKE_CURRENT_SOURCE_DIR}")
    foreach (_src ${ARGN})
        if (_relPath)
            list (APPEND ${_target}_SRCS "${_relPath}/${_src}")
        else (_src ${ARGN})
            list (APPEND ${_target}_SRCS "${_src}")
        endif (_relPath)
    endforeach (_src ${ARGN})
    #propigate up to parent
    if (_relPath)
        set (${_target}_SRCS ${${_target}_SRCS} PARENT_SCOPE)
    endif (_relPath)
endmacro (add_sources)

macro (init_target_source_dir _target)
    SET(${_target}_source_dir ${CMAKE_CURRENT_SOURCE_DIR})
endmacro(init_target_source_dir)
