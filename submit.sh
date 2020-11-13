#!/bin/bash

SCIPER=328287

TEMPDIR="$(mktemp -d)"
SUBMISSION_DIR="$TEMPDIR/$SCIPER"
mkdir -p "$SUBMISSION_DIR"

./code/cleanup.sh
cp -r code/bin code/CMakeLists.txt code/build.sh code/run.sh code/.gitignore code/cleanup.sh code/src/ "$SUBMISSION_DIR"
(cd $TEMPDIR; zip -r ${SCIPER}.zip $SCIPER)
mv "$TEMPDIR/${SCIPER}.zip" .
rm -r "$TEMPDIR"
