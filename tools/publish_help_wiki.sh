#!/usr/bin/env bash
# Publishes the pages that `JeffPub --export-help-wiki <folder>` wrote to the
# project's GitHub wiki: the wiki's pages are all replaced by the folder's,
# pages that no longer exist are deleted, and nothing is committed when
# nothing changed. The build workflow's "wiki" job runs it after a push to main.
#
#   tools/publish_help_wiki.sh <pages folder> <wiki git URL> <label>
#
# The commit is "Help from <label>", by JeffOffice LLC. GH_TOKEN, when set,
# signs in over https. A wiki that doesn't exist yet (GitHub makes its
# repository when the first page is saved) is a notice, not a failure.
set -euo pipefail

pages=$1 url=$2 label=$3
for page in Home _Sidebar _Footer; do
  [ -f "$pages/$page.md" ] || { echo "$pages has no $page.md: not publishing" >&2; exit 1; }
done

auth=()
if [ -n "${GH_TOKEN:-}" ]; then
  basic=$(printf 'x-access-token:%s' "$GH_TOKEN" | base64 | tr -d '\n')
  echo "::add-mask::$basic"
  auth=(-c "http.extraheader=AUTHORIZATION: basic $basic")
fi
g() { git ${auth[@]+"${auth[@]}"} -c user.name="JeffOffice LLC" -c user.email="info@jeffoffice.com" -c commit.gpgsign=false "$@"; }

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
wiki=$work/wiki

# A push can lose a race with someone editing the wiki: start over from its
# newest pages.
for try in 1 2 3; do
  rm -rf "$wiki"
  if ! said=$(g clone --quiet "$url" "$wiki" 2>&1); then
    if grep -Eqi "not found|does not exist|does not appear to be a git repository" <<<"$said"; then
      echo "::notice::The wiki at $url doesn't exist yet, so Help was not published. Save its first page on the repository's Wiki tab; the next push to main fills it."
      exit 0
    fi
    echo "$said" >&2
    exit 1
  fi
  branch=$(g -C "$wiki" symbolic-ref --short HEAD)
  find "$wiki" -maxdepth 1 -type f -name '*.md' -delete
  cp "$pages"/*.md "$wiki"/
  g -C "$wiki" add --all
  if g -C "$wiki" diff --cached --quiet; then
    echo "The wiki already has these pages."
    exit 0
  fi
  g -C "$wiki" diff --cached --stat | tail -n 1
  g -C "$wiki" commit --quiet -m "Help from $label" -m "Written by JeffPub --export-help-wiki from resources/help/en."
  if g -C "$wiki" push --quiet origin "HEAD:$branch"; then
    echo "Published Help to the wiki as \"Help from $label\"."
    exit 0
  fi
  echo "The push was refused (try $try of 3)."
done
echo "::error::Help was not published: the wiki kept changing under the push."
exit 1
