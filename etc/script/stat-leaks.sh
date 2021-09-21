#!/bin/bash

grep -A 4 'LEAK SUMMARY' leaks.log | sed 's/==.*== \(.*\)/\1/g'
