#!/bin/bash
# --address https://api.us-east-1.mbedcloud.com
set -e
set -x
BINARY=null
OLD_BINARY=null
INPUT_MANIFEST_JSON=null
DELTA_BINARY=null
ADDRESS="https://api.us-east-1.mbedcloud.com"
APIKEY=null

function help {
    echo "Available mandatory parameters are:"
    echo "-b|--binary <BINARY>"
    echo "-o|--old-binary <OLD_BINARY>"
    echo "-i|--manifest-input-file <INPUT_MANIFEST_JSON>"
    echo "-k|--apikey <APIKEY>"
    echo "Available optional parameters are:"
    echo "-d|--delta-binary <DELTA_BINARY>. If delta binary is not given, delta image is named BINARY_delta.bin"
    echo "-a|--address <ADDRESS>. If not given, uses production API"
    exit
}

while [[ $# -gt 1 ]]
do
key="$1"
    case $key in

    -b|--binary)
    BINARY="$2"
    BINARY_NAME="$(basename $BINARY .bin)"
    echo "Using binary $BINARY with name $BINARY_NAME."
    shift
    ;;

    -o|--old-binary)
    OLD_BINARY="$2"
    echo "Using old binary $OLD_BINARY."
    shift
    ;;

    -d|--delta-binary)
    DELTA_BINARY="$2"
    echo "Using delta binary $DELTA_BINARY."
    shift
    ;;

    -i|--manifest-input-file)
    INPUT_MANIFEST_JSON="$2"
    echo "Using manifest input file $INPUT_MANIFEST_JSON."
    shift
    ;;

    -a|--address)
    ADDRESS="$2"
    shift
    ;;
    
    -k|--apikey)
    APIKEY="$2"
    shift
    ;;

    *)
    echo "Invalid parameter $2 given."
    help
    ;;
    
esac
shift
done

if [ $BINARY = "null" ] || [ $OLD_BINARY = "null" ] || [ $INPUT_MANIFEST_JSON = "null" ] || [ $APIKEY = "null" ]
then
    help
fi

# if delta file name is not given, use default
if [ $DELTA_BINARY = "null" ]
then
    DELTA_BINARY=$BINARY_NAME"_delta.bin"
fi

# create image and manifest addesses
IMAGEADDRESS=$ADDRESS"/v3/firmware-images"
MANIFESTADDRESS=$ADDRESS"/v3/firmware-manifests"


DELTA_BINARY_NAME="$(basename $DELTA_BINARY .bin)"

MANIFEST=${DELTA_BINARY_NAME}.manifest
echo $MANIFEST

BINARY_FULLPATH=$(readlink -f $BINARY)
OLD_BINARY_FULLPATH=$(readlink -f $OLD_BINARY)
DELTA_BATCH_BINARY_FULLPATH=$(readlink -f ${DELTA_BINARY})
INPUT_MANIFEST_JSON_FULLPATH=$(readlink -f ${INPUT_MANIFEST_JSON})
DELTA_TOOL_FULLPATH=$(readlink -f ./delta-tool.py)
echo "Generating delta $DELTA_BINARY for $BINARY (new image) from $OLD_BINARY (old image)."

python3 $DELTA_TOOL_FULLPATH $OLD_BINARY_FULLPATH $BINARY_FULLPATH -d $DELTA_BATCH_BINARY_FULLPATH -i $INPUT_MANIFEST_JSON_FULLPATH -o delta-tool-generated.json
RANDOM_NAME="$DELTA_BINARY_NAME$$"
UPLOAD_URI="$(curl -X POST -H "Authorization: Bearer $APIKEY" -F "name=$RANDOM_NAME" -F "datafile=@$DELTA_BATCH_BINARY_FULLPATH" "$IMAGEADDRESS" | python2 -c "import sys, json; print json.load(sys.stdin)['datafile']")"
manifest-tool create -i delta-tool-generated.json -o $MANIFEST --payload-format bsdiff-stream -u $UPLOAD_URI
rm -f -r delta-tool-generated.json
