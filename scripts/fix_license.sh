#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
template="$repo_root/.github/license_header.txt"

header_lines="$(awk 'NF { last = NR } END { print last }' "$template")"
expected="$(head -n "$header_lines" "$template" | sed 's/[[:space:]]*$//')"
fixed=0

while IFS= read -r -d '' file; do
    actual="$(head -n "$header_lines" "$file" | sed 's/[[:space:]]*$//')"
    [[ "$actual" == "$expected" ]] && continue

    tmp="$(mktemp)"
    first_line="$(head -n 1 "$file")"

    if [[ "$first_line" == "/*"* ]] && head -n 60 "$file" | grep -q 'Copyright (c)'; then
        end="$(awk 'NR <= 60 && /\*\// { print NR; exit }' "$file")"
        {
            head -n "$header_lines" "$template"
            printf '\n'
            tail -n +"$((end + 1))" "$file" | awk 'NF { seen = 1 } seen'
        } > "$tmp"
    else
        {
            head -n "$header_lines" "$template"
            printf '\n'
            cat "$file"
        } > "$tmp"
    fi

    mv "$tmp" "$file"
    echo "Added/fixed license header: ${file#"$repo_root"/}"
    fixed=$((fixed + 1))
done < <(
    find "$repo_root/src" -type f \( -name '*.c' -o -name '*.h' \) \
        -not -path "$repo_root/src/linclude/*" -print0
)

echo "Files fixed: $fixed"
