#!/bin/bash
#
# createCertsForWIoTP.sh
#
# Script to create creates self-signed CA and Intermediate CA Certificates, 
# and client certificates to work with Watson IoT Platform service.
###########################################################################
#

CURDIR=`pwd`
export CURDIR

parent_dir=$1
cd ${parent_dir}/temp/clientcerts


# Set Device and Gateway type and ID, registered with Watson IoT Platform service.
devType="iotc_test_devType1"
devId="iotc_test_dev1"
gwType="iotc_test_gwType1"
gwId="iotc_test_gw1"

# For root CA, set the following variables as per your requirement.
ROOT_CA_CN="IBM Watson IoTP Sample Root CA"
CA_ENC_TYPE="ECC"             # RSA or ECC
CA_ECC_CURVE="prime256v1"     # For ECC

# For Intermediate CA, set the following variables as per your requirement.
INTERMEDIATE_CA_CN="IBM Watson IoTP Sample Intermediate CA"
INTERMEDIATE_CA_ENC_TYPE="ECC"            # RSA or ECC
INTERMEDIATE_CA_ECC_CURVE="prime256v1"    # For ECC

# Certificate validity
CA_CERT_VALIDITY=7300                     # 20 years
CLIENT_CERT_VALIDITY=7300                 # 20 years

# 
# There is no need to update any other variable unless needed for the environment.
#
echo
echo "Create Root and Intermediate CA certificates to be uploaded on WIoTP service."
echo

# Root CA Files
rootcaKey="CACertificate_${CA_ENC_TYPE}.key"
rootcaCsr="CACertificate_${CA_ENC_TYPE}.csr"
rootcaCert="CACertificate_${CA_ENC_TYPE}.crt"
ca_srl="CACertificate_${CA_ENC_TYPE}.srl"

# Intermediate CA files
intercaKey="interCACertificate_${CA_ENC_TYPE}.key"
intercaCsr="interCACertificate_${CA_ENC_TYPE}.csr"
intercaCert="interCACertificate_${CA_ENC_TYPE}.crt"
inter_srl="interCACertificate_${CA_ENC_TYPE}.srl"

echo
echo "Check whether a .srl file of Root CA is present"
if [ -e ${ca_srl} ]; then
    echo "${ca_srl} already exists, use it"
    x509_serial="-CAserial ${ca_srl}"
else
    echo "${ca_srl} is not found, create it"
    x509_serial="-CAcreateserial"
fi

cat > v3_ext.cnf <<EOF
[ req ]
attributes             = req_attributes
distinguished_name     = req_distinguished_name
req_extensions         = v3_ca
prompt                 = no
[ req_attributes ]
[ v3_ca ]
subjectKeyIdentifier=hash
authorityKeyIdentifier=keyid:always,issuer
basicConstraints       = CA:true
[ req_distinguished_name ]
commonName                      = IBM Watson IoTP Sample Root CA
EOF


# Create root CA key and certificate.
if [ "${CA_ENC_TYPE}" == "RSA" ]; then

    # RSA Root Certificates - RSA 4096
    echo "Create RSA Root CA Key: (${rootcaKey})"
    openssl genrsa -out ${rootcaKey} 4096
    echo "Create RSA Root CA Certificate: (${rootcaCert})"
    openssl req -x509 -new -nodes -key ${rootcaKey} -sha256 -days ${CA_CERT_VALIDITY} -out ${rootcaCert} -subj "/CN=${ROOT_CA_CN}"

else

    # ECC Root Certificates
    echo "Create ECC Root CA Key: (${rootcaKey}) Curve: (${CA_ECC_CURVE})"
    openssl ecparam -genkey -name ${CA_ECC_CURVE} -out ${rootcaKey}
    echo "Create ECC Root CA Certificate: (${rootcaCert})"
    openssl req -x509 -new -days ${CA_CERT_VALIDITY} -key ${rootcaKey} -subj "/CN=${ROOT_CA_CN}" \
        -extensions v3_ca -config v3_ext.cnf -set_serial 1 -out ${rootcaCert}

fi

echo
echo "Check whether a .srl file of Intermediate CA is present"
if [ -e ${inter_srl} ]; then
    echo "${inter_srl} already exists, use it"
    x509_serial="-CAserial ${inter_srl}"
else
    echo "${inter_srl} is not found, create it"
    x509_serial="-CAcreateserial"
fi

echo
echo "Create Intermediate CA"

# Define the v3 extension that will be included in the intermediate certificate
cat > v3_ext.cnf <<EOF
[ req ]
attributes             = req_attributes
distinguished_name     = req_distinguished_name
req_extensions         = v3_req
prompt                 = no
[ req_attributes ]
[ v3_req ]
keyUsage               = critical, digitalSignature, keyCertSign
basicConstraints       = CA:true, pathlen:0
[ req_distinguished_name ]
commonName             = IBM Watson IoTP Sample Intermediate Root CA
EOF

# cat > v3_ext.cnf <<EOF
# [ v3_req ]
# basicConstraints = CA:TRUE, pathlen:0
# keyUsage = critical, digitalSignature, keyCertSign
# EOF

if [ "${CA_ENC_TYPE}" == "RSA" ]; then

    # RSA Intemediate Certificates
    echo "Create RSA Intermediate CA Key: (${intercaKey})"
    openssl genrsa -out ${intercaKey} 4096
    echo "Create RSA Intermediate CA CSR: (${intercaCsr})"
    openssl req -new -key ${intercaKey} -out ${intercaCsr} -subj "/CN=${INTERMEDIATE_CA_CN}"
    echo "Create RSA Intermediate CA Certificate: (${intercaCert})"
    openssl x509 -req -days ${CA_CERT_VALIDITY} -in ${intercaCsr} ${x509_serial} -CA ${rootcaCert} \
        -CAkey ${rootcaKey} -extfile ./v3_ext.cnf -extensions v3_req -out ${intercaCert}

else

    # ECC Intemediate Certificates
    echo "Create ECC intermediate CA Key: (${intercaKey}) Curve: (${INTERMEDIATE_CA_ECC_CURVE})"
    openssl ecparam -genkey -name ${INTERMEDIATE_CA_ECC_CURVE} -out ${intercaKey}
    echo "Create ECC Intermediate CA CSR: (${intercaCsr})"
    openssl req -new -key ${intercaKey} -out ${intercaCsr} -subj "/CN=${INTERMEDIATE_CA_CN}"
    echo "Sign Intermediate CA CSR with (${rootcaCert})"
    openssl x509 -req -days ${CA_CERT_VALIDITY} -in ${intercaCsr} ${x509_serial} -CA ${rootcaCert} \
        -CAkey ${rootcaKey} -extensions v3_req -extfile ./v3_ext.cnf -set_serial 2 \
        -out ${intercaCert}
    
fi


echo
echo "Create key and device/gateway certificates"
echo

# Device certificate file
deviceCert="device_${CA_ENC_TYPE}.crt"
deviceCertAscii="device_${CA_ENC_TYPE}_ascii_dump.txt"

# Gateway certificate file
gatewayCert="gateway_${CA_ENC_TYPE}.crt"
gatewayCertAscii="gateway_${CA_ENC_TYPE}_ascii_dump.txt"

CN_val="Sample Cert"
SAN_D_val="email:d:${devType}:${devId}"
SAN_G_val="email:g:${gwType}:${gwId}"
deviceSerial="0x0111111111"
gatewaySerial="0x0222222222"

echo
echo "CN=${CN_val}"   
echo "SAN_D_VAL=${SAN_D_val}"   
echo "SAN_G_VAL=${SAN_G_val}"   
echo

# Create a key pair & CSR
echo "Create KEY and CSR"
openssl ecparam -genkey -name prime256v1 -out private.key
openssl req -new -key private.key -out private.csr -subj "/CN=${CN_val}"

echo "Create device certificate ${deviceCert}"

# Define the v3 extension that will be included in the device certificate
echo "[ v3_req ]"                                                      > v3_ext.cnf
echo ""                                                                >> v3_ext.cnf
echo "# Extensions to add to a certificate request"                    >> v3_ext.cnf
echo ""                                                                >> v3_ext.cnf
echo "basicConstraints = CA:FALSE"                                     >> v3_ext.cnf
echo "# keyUsage = nonRepudiation, digitalSignature, keyEncipherment"  >> v3_ext.cnf
echo "keyUsage = digitalSignature"                                     >> v3_ext.cnf
echo "subjectAltName = ${SAN_D_val}"                                   >> v3_ext.cnf

# Create device certificate signed by Intermediate CA
openssl x509 -req -days ${CLIENT_CERT_VALIDITY} -in private.csr -set_serial ${deviceSerial} \
    -CA ${intercaCert} -CAkey ${intercaKey} -extfile ./v3_ext.cnf -extensions v3_req -out ${deviceCert}
openssl x509 -in ${deviceCert} -text -noout > ${deviceCertAscii}
echo "Device certificate ${deviceCert} created successfully"
echo

# Define the v3 extension that will be included in the gateway certificate
echo "[ v3_req ]"                                                     > v3_ext.cnf
echo ""                                                               >> v3_ext.cnf
echo "# Extensions to add to a certificate request"                   >> v3_ext.cnf
echo ""                                                               >> v3_ext.cnf
echo "basicConstraints = CA:FALSE"                                    >> v3_ext.cnf
echo "# keyUsage = nonRepudiation, digitalSignature, keyEncipherment" >> v3_ext.cnf
echo "keyUsage = digitalSignature"                                    >> v3_ext.cnf
echo "subjectAltName = ${SAN_G_val}"                                  >> v3_ext.cnf

# Create a Cert signed by Intermediate CA
openssl x509 -req -days ${CLIENT_CERT_VALIDITY} -in private.csr -set_serial ${gatewaySerial} \
    -CA ${intercaCert} -CAkey ${intercaKey} -extfile ./v3_ext.cnf -extensions v3_req -out ${gatewayCert}
openssl x509 -in ${gatewayCert} -text -noout > ${gatewayCertAscii}
echo "Gateway certificate ${gatewayCert} created successfully"
echo

