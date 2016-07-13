import vdf
import json
print 'This program converts form and to the .txt format used by Source Engine to / from .json, ready to be used by POEditor. It differenciates between gameui_ and momentum_ by setting the context of the gameui_ tokens to "gameui". It is also capable of converting from the exported .json to an usable .txt by Source. It first aks for a language. You must enter the lowercase, english name of the language, so it will search for that language file. If you want to go from .json to .txt, you must name your .json with the language\'s name it contains.'
while True:
	gam = raw_input("Look for gameui_*.txt?Y/N")
	files = ['momentum_']
	if gam is "Y" or gam is "y":
    	files.append('gameui_')

    lang = raw_input("Language?\n")
    option = raw_input("DECODE (D) (.txt to .json) or ENCODE (E) (.json to .txt)?\n")

    if option is "DECODE" or option is "D":
        print "Decoding..."
        for loclfile in files:
            d = vdf.load(open(loclfile + lang + '.txt'), mapper=vdf.VDFDict)
            tokens = []
            for key, value in d['lang']['Tokens'].items():
                if loclfile is 'gameui_':
                    tokens.append({'term': key, 'definition': value, 'context':'gameui'})
                else:
                    tokens.append({'term': key, 'definition': value})
            json.dump(tokens, open(loclfile+lang + '.json', 'w'), indent=4, sort_keys=True)
            print 'Tokens dumped to .json'
            
    elif option is "ENCODE" or option is "E":
        print "Encoding..."
        with open(lang + '.json') as filez:
            jos = json.load(filez)
            gui = vdf.VDFDict([('lang', vdf.VDFDict([('Language', lang.title()), ('Tokens', vdf.VDFDict())]))])
            mom = vdf.VDFDict([('lang', vdf.VDFDict([('Language', lang.title()), ('Tokens', vdf.VDFDict())]))])
            for key in jos:
                if key['context'] == 'gameui':
                    gui['lang']['Tokens'][key['term']] = key['definition']
                else:
                    mom['lang']['Tokens'][key['term']] = key['definition']
            if gam is "Y" or gam is "y":
            	vdf.dump(gui, open('gameui_' + lang + '.txt', 'w'), pretty=True)
            	print 'gameui_' + lang + ' exported.'
            vdf.dump(mom, open('momentum_' + lang + '.txt', 'w'), pretty=True)
            print 'momentum_' + lang + ' exported.'

    else:
        print "Unknown command. DECODE/D or ENCODE/E"