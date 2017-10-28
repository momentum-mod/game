# -*- coding: utf-8 -*-
from __future__ import print_function
import codecs
import os

infile = 'momentum_english_ref.res'
outfile = 'momentum_english.txt'

try:
    if not os.path.exists(infile):
        raise IOError(infile + ' was not found in this directory')
    print('Trying to convert momentum_english_ref (UTF-8) to momentum_english (UCS-2 LE BOM)')
    with codecs.open(infile, 'r', encoding='utf-8') as ref_file:
        with codecs.open(outfile, 'w', 'utf_16_le') as target_file:
            target_file.write(u'\ufeff')        # Write the BOM
            target_file.write(ref_file.read())  # Write the reference
    print('Successfully converted file!')
except Exception as e:
    print('An exception (%s) was raised by this script. Let Ruben know (Or try to fix it yourself). Error message:\n%s' % (type(e).__name__, e.message))
__author__ = 'rabsrincon, gocnak'
# Purpose: Convert reference localization files (UTF-8) to their final state for the source engine (UCS-2 LE BOM), allowing git diff to work
