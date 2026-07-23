#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy <https://github.com/algoritnl>
# SPDX-License-Identifier: CC0-1.0

script_dir=$(realpath "$(dirname "${BASH_SOURCE[0]}")")
root_dir=$(realpath "$script_dir/..")

format=${1:-html}
build_dir=${2:-$PWD}

if [[ "$format" == "report" ]]; then

    coverage_dir=$build_dir/coverage_report
    if [[ -d "${coverage_dir}" ]]; then
        rm -rf "${coverage_dir}"
    fi
    mkdir -p "${coverage_dir}"

    gcovr \
        --root "$root_dir" \
        --exclude "$build_dir" \
        --exclude "$root_dir/tests" \
        --merge-lines \
        --exclude-noncode-lines \
        --exclude-throw-branches \
        --exclude-unreachable-branches \
        --html-details "${coverage_dir}/index.html" \
        --html-theme github.blue \
        --print-summary \
        "$build_dir"

    exit 0
fi

if [[ "$format" == "data" ]]; then

    coverage_file=$build_dir/coverage.xml
    if [[ -f "${coverage_file}" ]]; then
        rm "${coverage_file}"
    fi

    gcovr \
        --root "$root_dir" \
        --exclude "$build_dir" \
        --exclude "$root_dir/tests" \
        --merge-lines \
        --exclude-noncode-lines \
        --exclude-throw-branches \
        --exclude-unreachable-branches \
        --xml "$coverage_file" \
        --print-summary \
        "$build_dir"

    exit 0
fi

printf 'Error: unknown format %s.\n' "$format" >&2
exit 1
