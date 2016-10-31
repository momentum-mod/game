# -*- coding: utf-8 -*-
import codecs
import os
try:
    if not os.path.exists('momentum_english_ref.txt'):
        raise IOError('momentum_english_ref.txt was not found in this directory')
    print 'Trying to convert momentum_english_ref (UTF-8) to momentum_english (UCS-2 LE BOM)'
    with codecs.open('momentum_english_ref.txt', 'r', encoding='utf-8') as ref_file:
        with codecs.open('momentum_english.txt', 'w', 'utf_16_le') as target_file:
            target_file.write(ref_file.read())
except Exception as e:
    print 'An exception (%s) was raised by this script. Let Ruben know (Or try to fix it yourself). Error message:\n%s' % (type(e).__name__, e.message)
__author__ = 'rabsrincon'
# Purpose: Convert reference localization files (UTF-8) to their final state for the source engine, allowing git diff to work
