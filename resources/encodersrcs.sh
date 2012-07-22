#! /bin/sh

# Encode the BP2 resource files

# The Bol Processor resource files are encoded in the AppleSingle
# format within the CVS repository to preserve their resource forks.
# If you make changes to the resource files, you must re-encode them
# before committing to CVS.

# This script reads the resource files from ./decoded_resources/
# and writes the encoded .rsrc files to the current directory.

# NOTE: Run this script from the directory that it lives in 
#       (bolprocessor/resources/).
#       Files ending in .rsrc in this directory will be overwritten!

# Anthony Kozar
# July 21, 2012

for f in *.rsrc
do 
	# only re-encode changed files
	if [ "decoded_resources/${f}" -nt "$f" ]
	then
		applesingle encode -nv -o "$f" "decoded_resources/${f}"
		# copy creation and modification dates to the encoded file
		# NOTE: THERE IS A BUG THAT MAKES THE DATE 1 HR EARLY WHEN DST IS ON
		# SetFile -m "`GetFileInfo -m \"decoded_resources/${f}\"`" "${f}"
		# SetFile -d "`GetFileInfo -d \"decoded_resources/${f}\"`" "${f}"
	fi
done
