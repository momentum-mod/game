import vdf
import json

print 'This program converts form and to the .txt format used by Source Engine to / from .json, ready to be used by POEditor. It is also capable of converting from the exported' \
      ' .json to an usable .txt by Source. It first asks for a language. You must enter the lowercase, english name of the language, so it will search for that language file.' \
      ' If you want to go from .json to .txt, you must name your .json with the language\'s name it contains. Encoding a file will also create a "_ref_exp" file.'

lang = raw_input("Language?\n")
option = raw_input("DECODE (D) (.txt to .json) or ENCODE (E) (.json to .txt)?\n")

if option == "DECODE" or option == "D":
    print "Decoding..."
    d = vdf.load(open("momentum_" + lang + '.txt'), mapper=vdf.VDFDict)
    tokens = []
    for key, value in d['lang']['Tokens'].items():
        tokens.append({'term': key, 'definition': value})
    json.dump(tokens, open("momentum_" + lang + '.json', 'w'), indent=4, sort_keys=True)
    print 'Tokens dumped to .json'

elif option == "ENCODE" or option == "E":
    print "Encoding..."
    with open(lang + '.json') as filez:
        jos = json.load(filez)
        mom = vdf.VDFDict([('lang', vdf.VDFDict([('Language', lang.title()), ('Tokens', vdf.VDFDict())]))])
        for key in jos:
            mom['lang']['Tokens'][key['term']] = key['definition']
        vdf.dump(mom, open('momentum_' + lang + '.txt', 'w', encoding='utf_16_le'), pretty=True)
        print 'momentum_%s exported.' % lang
        if lang == 'english':
            vdf.dump(mom, open('momentum_english_ref_exp.txt', 'w', encoding='utf-8'), pretty=True)
            print 'momentum_english_ref_exp exported.'

else:
    print "Unknown command. DECODE/D or ENCODE/E"