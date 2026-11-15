#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
template="$repo_root/.github/license_header.txt"

header_lines="$(awk 'NF { last = NR } END { print last }' "$template")"
expected="$(head -n "$header_lines" "$template" | sed 's/[[:space:]]*$//')"
status=0

while IFS= read -r -d '' file; do
    actual="$(head -n "$header_lines" "$file" | sed 's/[[:space:]]*$//')"
    if [[ "$actual" != "$expected" ]]; then
        echo "::error file=${file#"$repo_root"/}::Missing or invalid license header"
        status=1
    fi
done < <(
    find "$repo_root/src" -type f \( -name '*.c' -o -name '*.h' \) \
        -not -path "$repo_root/src/linclude/*" -print0
)

if [[ $status -eq 0 ]]; then
    echo "License headers OK"
else
    echo "Run scripts/fix_license.sh to add missing headers." >&2
fi

exit $status
