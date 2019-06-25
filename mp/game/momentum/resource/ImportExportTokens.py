import vdf
import json
import requests
import codecs
# import pycountry

print('This program converts form and to the .txt format used by Source Engine to / from .json, ready to be used by POEditor. It is also capable of converting from the exported' \
      ' .json to an usable .txt by Source. It first asks for a language. You must enter the lowercase, english name of the language, so it will search for that language file.' \
      ' If you want to go from .json to .txt, you must name your .json with the language\'s name it contains. Encoding a file will also create a "_ref_exp" file.')

# countries = [country.alpha2 for country in pycountry.countries]
# print(countries)

lang = input("Language code:\n")
exit()

request = requests.post('https://api.poeditor.com/v2/projects/export', {
    'api_token': '',
    'id': '156379',
    'language': lang,
    'type': 'key_value_json',
    'filters': 'translated',
}).json()

if (request['response']['code'] == '200'):
    fileFetch = requests.get(request['result']['url']).json()
    
    mom = vdf.VDFDict([('lang', vdf.VDFDict([('Language', lang.title()), ('Tokens', vdf.VDFDict())]))])
    for key, value in fileFetch.items():
        mom['lang']['Tokens'][key] = value
    momstr = vdf.dumps(mom, True)
    with codecs.open('momentum_%s.txt' % lang, 'w', 'utf_16_le') as file_out:
        file_out.write(u'\ufeff')   # Write the BOM
        file_out.write(momstr)      # Write the reference
    print('momentum_%s successfully printed!' % lang)


# option = None # input("DECODE (D) (.txt to .json) or ENCODE (E) (.json to .txt)?\n")

# if option == "DECODE" or option == "D":
#     print("Decoding...")
#     d = vdf.load(open("momentum_" + lang + '.txt'), mapper=vdf.VDFDict)
#     tokens = []
#     for key, value in d['lang']['Tokens'].items():
#         tokens.append({'term': key, 'definition': value})
#     json.dump(tokens, open("momentum_" + lang + '.json', 'w'), indent=4, sort_keys=True)
#     print('Tokens dumped to .json')

# elif option == "ENCODE" or option == "E":
#     print("Encoding...")
#     with open(lang + '.json') as filez:
#         jos = json.load(filez)
#         mom = vdf.VDFDict([('lang', vdf.VDFDict([('Language', lang.title()), ('Tokens', vdf.VDFDict())]))])
#         for key in jos:
#             mom['lang']['Tokens'][key['term']] = key['definition']
#         vdf.dump(mom, open('momentum_' + lang + '.txt', 'w', encoding='utf_16_le'), pretty=True)
#         print('momentum_%s exported.' % lang)
#         if lang == 'english':
#             vdf.dump(mom, open('momentum_english_ref_exp.txt', 'w', encoding='utf-8'), pretty=True)
#             print('momentum_english_ref_exp exported.')

# else:
#     print("Unknown command. DECODE/D or ENCODE/E")
