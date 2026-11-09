#!/usr/bin/env bash
#
# Copyright (c) 2026, Daily
#
# Copies Unreal Engine's Third Person template content into Content and creates
# the game's materials with Scripts/create_content.py.
#
# Usage:
#   UE_ROOT=/path/to/UnrealEngine ./setup.sh
#

set -euo pipefail

: "${UE_ROOT:?Set UE_ROOT to your Unreal Engine directory}"

HERE="$(cd "$(dirname "$0")" && pwd)"
TEMPLATES="$UE_ROOT/Templates"
EDITOR_CMD="$UE_ROOT/Engine/Binaries/Linux/UnrealEditor-Cmd"

if [[ ! -x "$EDITOR_CMD" ]]; then
    echo "Unable to find executable $EDITOR_CMD" >&2
    exit 1
fi

mkdir -p "$HERE/Content"
cp -R "$TEMPLATES/TP_ThirdPersonBP/Content/." "$HERE/Content/"
for PACK in LevelPrototyping Characters Input; do
    mkdir -p "$HERE/Content/$PACK"
    cp -R "$TEMPLATES/TemplateResources/High/$PACK/Content/." "$HERE/Content/$PACK/"
done
chmod -R u+w "$HERE/Content"

echo "Copied the Third Person template content into $HERE/Content"

"$EDITOR_CMD" "$HERE/PipecatRoom.uproject" \
    -run=pythonscript "-script=$HERE/Scripts/create_content.py" \
    -unattended -nosplash -nullrhi
