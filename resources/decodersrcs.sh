#! /bin/sh

# Decode the BP2 resource files

# The Bol Processor resource files are encoded in the AppleSingle
# format within the CVS repository to preserve their resource forks.
# This script writes the decoded .rsrc files to ./decoded_resources/

# NOTE: Run this script from the directory that it lives in 
#       (bolprocessor/resources/).
#       Files in the ./decoded_resources/ directory will be overwritten!

# Anthony Kozar
# July 21, 2012

if [ ! -d ./decoded_resources ]
then
	mkdir decoded_resources
fi
for f in *.rsrc
do 
	applesingle -nv -o "decoded_resources/${f}" "$f"
	# touch decoded_resources/${f}
	# copy creation and modification dates to the decoded file
	# NOTE: THERE IS A BUG THAT MAKES THE DATE 1 HR EARLY WHEN DST IS ON
	SetFile -m "`GetFileInfo -m \"${f}\"`" "decoded_resources/${f}"
	SetFile -d "`GetFileInfo -d \"${f}\"`" "decoded_resources/${f}"
done
