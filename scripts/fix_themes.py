import os
import xml.etree.ElementTree
from xml.etree.ElementTree import Element

#filepath = 'C:\\Users\Alex\\.emulationstation\\themes\\oldroom 720p\\megadrive\\theme2.xml'


#folderpath = 'C:\\Users\Alex\\.emulationstation\\themes\\oldroom 720p\\'
folderpath = '~/.emulationstation/themes/oldroom 720p/'
fixes = \
{
	"./view/image[@name='scanlines']" : {'zIndex' : '60'},
	"./view/image[@name='borders']"   : {'zIndex' : '70'},
	"./view/video[@name='md_video']"  : {'zIndex' : '50'}
}


for folder, subs, files in os.walk(folderpath):
	for filename in files:
		if not filename.endswith('.xml'): continue
		fullname = os.path.join(folder, filename)
		print('fixing: ' + fullname)
		et = xml.etree.ElementTree.parse(fullname)
		root = et.getroot()
		for xpath, fix in fixes.items():
			#print(xpath)
			for tag in root.findall(xpath):
				for newtag, text in fix.items():
					tgs = tag.findall(newtag)
					if len(tgs) is 0:
						print(newtag, text)
						child = Element(newtag)
						child.text = text
						tag.append(child)
					else:
						for tg in tgs:
							print('Setting <' + newtag + '>' + tg.text + '</' + newtag + '> to ' + text)
							tg.text = text

print("done")
#et.write('output.xml', encoding='utf-8')

from xml.dom import minidom
import xml.etree.ElementTree as ET

#print(ET.tostring(root))
data = ET.tostring(root)
pretty_print = lambda data: '\n'.join([line for line in \
 minidom.parseString(data).toprettyxml(indent='\t').split('\n') if line.strip()])

with open("output.xml", "w") as f:
    f.write(pretty_print(data))
	

	

