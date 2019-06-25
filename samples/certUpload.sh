#!/bin/bash
#
# Add a CA certificate using Watson IoT Platform REST API
# Method:POST  URL:/authentication/cacertificates
#
# Note: 
# Watson IoT Platform organization ID and API Key is needed to invoke the REST API.
# Use command line options to set required variables or use environment variables.
#

set +x

SCRIPTNAME=$0

CURDIR=`pwd`
export CURDIR

# Logging and error handling functions
function log() { printf '%s\n' "$*"; }
function usage_fatal() { error "$*"; usage >&2; exit 1; }
function logVerbose() {
  if [ ! -z $VERBOSE ]; then
    log $1
  fi
}

function usage() {
    cat <<EOF
Usage: Command to upload CA certificates to WIoTP Service.
       $0 -o <org> -k <key> -t <token> -n <certName> -c <comment>

Options:
  -h,        --help                 Display this usage message and exit.
  -v,        --verbose              Verbose output.
  -o <val>,  --org <val>            WIoTP Organization Id.
  -k <val>,  --APIKey <val>         WIoTP API Key.
  -t <val>,  --APIToken <val>       WIoTP API Token.
  -n <val>,  --certName <val>       CA Certificate name.
  -c <val>,  --comment <val>        Comment for the certificate.

NOTE: You can also set orgid, API key and totekn using Environment variables:
      WIOTP_IDENTITY_ORGID, WIOTP_AUTH_APIKEY, WIOTP_AUTH_TOKEN

EOF
}


# Parse arguments
while [ "$#" -gt 0 ]; do
    arg=$1
    case $1 in
        # convert "--opt=the value" to --opt "the value".
        # the quotes around the equals sign is to work around a
        # bug in emacs' syntax parsing
        --*'='*) shift; set -- "${arg%%=*}" "${arg#*=}" "$@"; continue;;
        -h|--help) usage; exit 0;;
        -o|--org) shift; WIOTP_ORGID=$1;;
        -k|--APIKey) shift; WIOTP_APIKEY=$1;;
        -t|--APIToken) shift; WIOTP_APITOKEN=$1;;
        -n|--certName) shift; WIOTP_CERTNAME=$1;;
        -c|--comment) shift; WIOTP_COMMENT=$1;;
        -v|--verbose) VERBOSE='-v';;
        -*) usage_fatal "unknown option: '$1'";;
        *) break;; # reached the list of file names
    esac
    shift || usage_fatal "option '${arg}' requires a value"
done


log ""
log "IBM Watson IoT Platform CA Certificate Uploader"
log ""

# Check and set required variables from Environment variables if not set
# WIoTP ORGID check
if [[ -z $WIOTP_ORGID ]]; then
    WIOTP_ORGID=$WIOTP_IDENTITY_ORGID
    if [[ -z $WIOTP_ORGID ]]; then
        usage_fatal "WIoTP ORGID is required"
    fi
fi

# APIKEY check
if [[ -z $WIOTP_APIKEY ]]; then
    WIOTP_APIKEY=$WIOTP_AUTH_APIKEY
    if [[ -z $WIOTP_APIKEY ]]; then
        usage_fatal "APIKEY is required"
    fi
fi

# APITOKEN check
if [[ -z $WIOTP_APITOKEN ]]; then
    WIOTP_APITOKEN=$WIOTP_AUTH_TOKEN
    if [[ -z $WIOTP_APITOKEN ]]; then
        usage_fatal "APITOKEN is required"
    fi
fi

# Certificate CERTNAME check
if [[ -z $WIOTP_CERTNAME ]]; then
    usage_fatal "Certificate CERTNAME is required"
fi

# CA File comment
if [[ -z $WIOTP_COMMENT ]]; then
    COMMENT="Certificate ID $WIOTP_CERTNAME"
fi


log "Add CA Certificate file to Watson IoT Platform org: $WIOTP_ORGID"
log "API Key: $WIOTP_APIKEY"
log "API Token: $WIOTP_APITOKEN"
log "CA Certificate Name: $WIOTP_CERTNAME"
log "CA Certificate Comments: $COMMENT"

# Prepare certficiate content for upload
TMPBUF=`cat ./${WIOTP_CERTNAME}`
echo $TMPBUF | sed 's/BEGIN CERTIFICATE/BEGINCERT/' | sed 's/END CERTIFICATE-----/ENDCERT-----/' | sed 's/ /\\n/g' | sed 's/BEGINCERT/BEGIN CERTIFICATE/' | sed 's/ENDCERT-----/END CERTIFICATE-----\\n/'  > ./tmpcertfile.crt

ENCODEDCERTCONTENTS=`cat ./tmpcertfile.crt`

# Prepare payload
echo "{\"comments\":\"$COMMENT\",\"certificate\":\"${ENCODEDCERTCONTENTS}\\n\"}" > ./certpayload.crt

log "Invoke api/v0002/authentication/cacertificates"
logVerbose "DATA: `cat ./certpayload.crt`"
log ""

RESPONSE=$(curl -s -o /dev/null -k -w "%{http_code}" -u "${WIOTP_APIKEY}:${WIOTP_APITOKEN}" --request POST ${VERBOSE} \
  --url https://${WIOTP_ORGID}.internetofthings.ibmcloud.com/api/v0002/authentication/cacertificates \
  --header 'content-type: application/json' \
  --data '@./certpayload.crt')

log "Response: ${RESPONSE}"
log ""


