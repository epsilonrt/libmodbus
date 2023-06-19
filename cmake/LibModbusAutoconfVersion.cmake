# Copyright © 2023 Pascal JEAN, All rights reserved.
# This file is part of the libmodbus Project.
#
# The libmodbus Project is free software: you can redistribute it and/or modify
# it under the terms of the GNU Library Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The libmodbus Project is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Library Public License for more details.
#
# You should have received a copy of the GNU Library Public License
# along with the libmodbus Project.  If not, see <http://www.gnu.org/licenses/>.
# GetAutoconfVersion.cmake
# output variables:
# ${_prefix}_FOUND
# Si ${_prefix}_FOUND = TRUE:
#   ${_prefix}_VERSION
#   ${_prefix}_MAJOR
#   ${_prefix}_MINOR
#   ${_prefix}_MICRO

function(FindVersionPart _var _part _configure)
  string(REGEX MATCH "m4_define\\(\\[libmodbus_version_${_part}\\]\\,\\ \\[([0-9]*)\\]" _ ${_configure})
  if (NOT CMAKE_MATCH_1)
    message(FATAL_ERROR "Unable to find current project version !")
  endif()
  set(${_var} ${CMAKE_MATCH_1} PARENT_SCOPE)
  # message(STATUS "FindVersionPart: ${_var}: ${CMAKE_MATCH_1}")
endfunction(FindVersionPart)

function(GetAutoconfVersion _prefix _src_dir)
  set (${_prefix}_FOUND FALSE PARENT_SCOPE)

  file(STRINGS "${_src_dir}/configure.ac" configure_ac REGEX "^m4_define")
  # message(STATUS "GetAutoconfVersion: ${_src_dir}/configure.ac= ${configure_ac}")

  FindVersionPart(major "major" "${configure_ac}")
  FindVersionPart(minor "minor" "${configure_ac}")
  FindVersionPart(micro "micro" "${configure_ac}")
  set(version ${major}.${minor}.${micro})

  # message(STATUS "GetAutoconfVersion: version = ${version}")

  set (${_prefix}_FOUND TRUE PARENT_SCOPE)
  set (${_prefix}_VERSION_MAJOR ${major} PARENT_SCOPE)
  set (${_prefix}_VERSION_MINOR ${minor} PARENT_SCOPE)
  set (${_prefix}_VERSION_MICRO ${minor} PARENT_SCOPE)
  set (${_prefix}_VERSION ${version} PARENT_SCOPE)

  unset(configure_ac)
  unset(major)
  unset(minor)
  unset(minor)
  unset(version)
endfunction(GetAutoconfVersion)
