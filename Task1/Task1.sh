#!/bin/bash

find . -type f -name "*.c" -exec sh -c '
	for f; do
		cp "$f" "$f.orig"
	done ' sh {} +
