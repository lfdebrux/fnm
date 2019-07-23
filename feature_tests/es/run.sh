#!/bin/bash

set -e

DIRECTORY=`dirname $0`

if hash es 2>/dev/null; then
  es $DIRECTORY/run.es
else
  echo "Skipping: \`es\` is not installed"
fi
