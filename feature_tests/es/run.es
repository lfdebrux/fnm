#!/usr/bin/env es

eval `{fnm env --shell=es}

fnm install v8.11.3
fnm use v8.11.3

if {! ~ `{node --version} v8.11.3} {
  echo 'Node version is not v8.11.3!'
  exit 1
}
