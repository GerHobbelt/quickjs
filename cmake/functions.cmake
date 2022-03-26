include(CheckFunctionExists)

function(DUMP)
  foreach(VAR ${ARGN})
    if("${SEPARATOR}" STREQUAL "")
      set(SEPARATOR "\n    ")
    endif("${SEPARATOR}" STREQUAL "")
    set("${VAR}" ${${VAR}})
    string(REGEX REPLACE "[ \t\n]+" "\n" A "${${VAR}}")
    string(REGEX REPLACE "\n" ";" A "${A}")
    string(REGEX REPLACE ";" "${SEPARATOR}" A "${A}")
    message("  ${VAR} = ${A}")
  endforeach(VAR ${ARGN})
endfunction(DUMP)

function(CANONICALIZE OUTPUT_VAR STR)
  string(REGEX REPLACE "^-W" "WARN_" TMP_STR "${STR}")

  string(REGEX REPLACE "-" "_" TMP_STR "${TMP_STR}")
  string(TOUPPER "${TMP_STR}" TMP_STR)

  set("${OUTPUT_VAR}" "${TMP_STR}" PARENT_SCOPE)
endfunction(CANONICALIZE OUTPUT_VAR STR)

function(BASENAME OUTPUT_VAR STR)
  string(REGEX REPLACE ".*/" "" TMP_STR "${STR}")
  if(ARGN)
    string(REGEX REPLACE "\\${ARGN}\$" "" TMP_STR "${TMP_STR}")
  endif(ARGN)

  set("${OUTPUT_VAR}" "${TMP_STR}" PARENT_SCOPE)
endfunction(BASENAME OUTPUT_VAR FILE)

function(ADDPREFIX OUTPUT_VAR PREFIX)
  set(OUTPUT "")
  foreach(ARG ${ARGN})
    list(APPEND OUTPUT "${PREFIX}${ARG}")
  endforeach(ARG ${ARGN})
  set("${OUTPUT_VAR}" "${OUTPUT}" PARENT_SCOPE)
endfunction(ADDPREFIX OUTPUT_VAR PREFIX)

function(ADDSUFFIX OUTPUT_VAR SUFFIX)
  set(OUTPUT "")
  foreach(ARG ${ARGN})
    list(APPEND OUTPUT "${ARG}${SUFFIX}")
  endforeach(ARG ${ARGN})
  set("${OUTPUT_VAR}" "${OUTPUT}" PARENT_SCOPE)
endfunction(ADDSUFFIX OUTPUT_VAR SUFFIX)

function(RELATIVE_PATH OUT_VAR RELATIVE_TO)
  set(LIST "")

  foreach(ARG ${ARGN})
    file(RELATIVE_PATH ARG "${RELATIVE_TO}" "${ARG}")
    list(APPEND LIST "${ARG}")
  endforeach(ARG ${ARGN})

  set("${OUT_VAR}" "${LIST}" PARENT_SCOPE)
endfunction(RELATIVE_PATH RELATIVE_TO OUT_VAR)

macro(APPEND_PARENT VAR)
  set(LIST "${${VAR}}")
  list(APPEND LIST ${ARGN})
  set("${VAR}" "${LIST}" PARENT_SCOPE)
endmacro(APPEND_PARENT VAR)

macro(CHECK_FUNCTION_DEF FUNC)
  string(TOUPPER "${FUNC}" FUNC_U)
  #message("FUNC_U: ${FUNC_U}")
  if(ARGC EQUAL 1)
    string(TOUPPER "HAVE_${FUNC_U}" RESULT_VAR)
  else(ARGC EQUAL 1)
    set(RESULT_VAR "${ARGN}")
  endif(ARGC EQUAL 1)
  #message("RESULT_VAR: ${RESULT_VAR}")
  # message("FUNC: ${FUNC} RESULT_VAR: ${RESULT_VAR}")
  check_function_exists("${FUNC}" "${RESULT_VAR}")
  if(${${RESULT_VAR}})
    set("${RESULT_VAR}" TRUE CACHE BOOL "Have the '${FUNC}' function")
    # add_definitions(-D${RESULT_VAR}) message("${RESULT_VAR} ${${RESULT_VAR}}")
  endif(${${RESULT_VAR}})
endmacro(CHECK_FUNCTION_DEF FUNC RESULT_VAR)

macro(CHECK_FUNCTIONS)
  foreach(FUNC ${ARGN})
    string(TOUPPER "HAVE_${FUNC}" RESULT_VAR)
    check_function_def("${FUNC}" "${RESULT_VAR}")
  endforeach(FUNC ${ARGN})
endmacro(CHECK_FUNCTIONS)

macro(CHECK_INCLUDES)
  foreach(INC ${ARGN})
    string(TOUPPER "HAVE_${INC}" RESULT_VAR)
    string(REGEX REPLACE "[^A-Za-z0-9_]" "_" RESULT_VAR "${RESULT_VAR}")
    check_include_file("${INC}" "${RESULT_VAR}")
    message(STATUS "Checked for ${INC} -- ${${RESULT_VAR}}")
  endforeach(INC ${ARGN})
endmacro(CHECK_INCLUDES)

function(CONTAINS LIST VALUE OUTPUT)
  list(FIND "${LIST}" "${VALUE}" INDEX)
  if(${INDEX} GREATER -1)
    set(RESULT TRUE)
  else(${INDEX} GREATER -1)
    set(RESULT FALSE)
  endif(${INDEX} GREATER -1)
  if(NOT RESULT)
    foreach(ITEM ${${LIST}})
      if("${ITEM}" STREQUAL "${VALUE}")
        set(RESULT TRUE)
      endif("${ITEM}" STREQUAL "${VALUE}")
    endforeach(ITEM ${${LIST}})
  endif(NOT RESULT)
  set("${OUTPUT}" "${RESULT}" PARENT_SCOPE)
endfunction(CONTAINS LIST VALUE OUTPUT)

function(ADD_UNIQUE LIST)
  set(RESULT "${${LIST}}")
  foreach(ITEM ${ARGN})
    contains(RESULT "${ITEM}" FOUND)
    if(NOT FOUND)
      list(APPEND RESULT "${ITEM}")
    endif(NOT FOUND)
  endforeach(ITEM ${ARGN})
  set("${LIST}" "${RESULT}" PARENT_SCOPE)
endfunction(ADD_UNIQUE LIST)


function(CONTAINS LIST VALUE OUTPUT)
  list(FIND "${LIST}" "${VALUE}" INDEX)
  if(${INDEX} GREATER -1)
    set(RESULT TRUE)
  else(${INDEX} GREATER -1)
    set(RESULT FALSE)
  endif(${INDEX} GREATER -1)
  if(NOT RESULT)
    foreach(ITEM ${${LIST}})
      if("${ITEM}" STREQUAL "${VALUE}")
        set(RESULT TRUE)
      endif("${ITEM}" STREQUAL "${VALUE}")
    endforeach(ITEM ${${LIST}})
  endif(NOT RESULT)
  set("${OUTPUT}" "${RESULT}" PARENT_SCOPE)
endfunction(CONTAINS LIST VALUE OUTPUT)
