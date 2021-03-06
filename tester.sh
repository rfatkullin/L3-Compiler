#!/bin/bash
#Run from build directory

binaryFilePath="./test.exe"
resFilePath="res.txt"
args="first second third 4 5"
red='\e[0;31m'
green='\e[0;32m'
NC='\e[0m' # No Color

function Fail
{
	echo -e "${red}[FAIL]$1\t$2${NC}"
	exit 1;
}

if [ "$#" -lt 2 ]
then
	echo -e "HELP: \n\t$0 <path-to-l3-compiler> <test-dir-path>";
	exit 1;
fi

compilerPath=$1
testDirPath=$2
inFiles=`ls ${testDirPath}/*.in`

for inFile in ${inFiles}
do
	outFile="${inFile%.in}.out"
	baseName=`basename ${inFile}`
	ilCodeFilePath="${baseName%.in}.il"

	${compilerPath} "${inFile}" "${ilCodeFilePath}"

	if [ "$?" != 0 ] ; then
	    Fail "[compiler]" "${inFile}"
	fi

	ilasm "${ilCodeFilePath}" /output:"${binaryFilePath}" > /dev/null

	if [ "$?" != 0 ] ; then
	    Fail "[ilasm]" "${inFile}"
	fi

	chmod +x "${binaryFilePath}"

	`${binaryFilePath} ${args} > ${resFilePath}`

	if [ "$?" != 0 ] ; then
	    Fail "[execute]" "${inFile}"
	fi

	`cmp -s ${resFilePath} ${outFile}`

	if [ "$?" != 0 ]
	then
		Fail "[result]" "${inFile}\n${NC}Check by command: vim -d ${resFilePath} ${outFile}"
	fi

	echo -e "${green}[OK]...$inFile${NC}"
done
