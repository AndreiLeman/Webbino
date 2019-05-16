#!/usr/bin/env python

# Run this script like this:
# ./html2h.py webroot > html.h
#
# Note that this is untested on Windows/OSX!

import os
import sys

# https://www.safaribooksonline.com/library/view/python-cookbook/0596001673/ch04s16.html
def splitall (path):
    allparts = []
    while 1:
        parts = os.path.split(path)
        if parts[0] == path:  # sentinel for absolute paths
            allparts.insert(0, parts[0])
            break
        elif parts[1] == path: # sentinel for relative paths
            allparts.insert(0, parts[1])
            break
        else:
            path = parts[0]
            allparts.insert(0, parts[1])
    return allparts

def shallStrip (filename):
	name, ext = os.path.splitext (filename)
	if len (ext) > 1:
		ext = ext[1:]
	ext = ext.lower ()
	return ext == "htm" or ext == "html"

def process_file (filename, nostrip = False):
	print >> sys.stderr, "Processing file: %s" % filename

	if not nostrip and not shallStrip (filename):
		print >> sys.stderr, "- File will not be stripped"
		nostrip = True

	try:
		with open (filename, 'rb') as fp:
			# Make up a unique ID for every file to use in C identifiers
			parts = splitall (filename[2:])
			parts = [parts[0]] + [x.capitalize () for x in parts[1:]]
			code = "".join (parts)
			code = code.replace ('.', '_')
			pagename = filename[1:]

			# Convert Windows slashes to Posix slashes
			pagename = pagename.replace ('\\', '/')

			print "const char %s_name[] PROGMEM = \"%s\";" % (code, pagename)
			print
			print "const byte %s[] PROGMEM = {" % code
			i = 0
			b = fp.read (1)
			while len (b) > 0:
				if nostrip or (b != '\n' and b != '\r' and b != '\t'):
					if i % 8 == 0:
						print "\t",
					print "0x%02x, " % ord (b),
					i += 1
					if i % 8 == 0:
						print ""
				b = fp.read (1)

			print "\n};"
			print
			print "const unsigned int %s_len PROGMEM = %u;" % (code, i)
			print
	except IOError as ex:
		print "Cannot open file %s: %s" % (filename, str (ex))
		code = None

	return code

def process_dir (dirpath, nostrip = False):
	print >> sys.stderr, "Processing directory: %s" % dirpath
	idents = []
	for filename in sorted (os.listdir (dirpath)):
		fullfile = os.path.join (dirpath, filename)
		if os.path.isfile (fullfile):
			ident = process_file (fullfile, nostrip)
			if ident is not None:
				idents.append (ident)
		elif os.path.isdir (fullfile):
			idents += process_dir (fullfile, nostrip)
		else:
			print "Skipping %s" % filename
	return idents

def make_include_code (idents):
	ret = ""

	for n, ident in enumerate (idents):
		ret += "const Page page%02d PROGMEM = {%s_name, %s, %s_len, NULL};\n" % (n + 1, ident, ident, ident)

	ret += "\n"

	ret += "const Page* const pages[] PROGMEM = {\n"
	for n in xrange (1, n_pages + 1):
		ret += "\t&page%02d,\n" % n

	ret += "\tNULL\n"
	ret += "};"

	return ret


### MAIN ###

if __name__ == "__main__":
	import argparse

	parser = argparse.ArgumentParser (description = 'Convert a website to be put into flash memory for use with Webbino')
	parser.add_argument ('webroot', metavar = "WEBROOT", help = "Path to website root directory")
	parser.add_argument ('--nostrip', "-n", action = 'store_true', default = False,
						 help = "Do not strip CR/LF/TABs")

	args = parser.parse_args ()

	# The above will raise an error if webroot was not specified, so we can
	# assume it was
	os.chdir (args.webroot)
	idents = process_dir (".", args.nostrip)
	n_pages = len (idents)

	print "/*** CODE TO INCLUDE IN SKETCH ***\n"
	print make_include_code (idents)
	print "\n***/"

	print >> sys.stderr, "Total files processed: %d" % n_pages

	if n_pages > 0:
		# Help with code to be put in sketch
		print >> sys.stderr
		print >> sys.stderr, "Put the following in your sketch:"
		print >> sys.stderr
		print >> sys.stderr, '#include "html.h"'
		print >> sys.stderr
		print >> sys.stderr, make_include_code (idents)
