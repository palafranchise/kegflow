
#ifndef config_usr_h
#define config_usr_h

// Copy and paste your configuration into this file
//===============================================================
#define AWS_IOT_MQTT_HOST "<HOST>" 	// your endpoint
#define AWS_IOT_MQTT_PORT 8883									// your port
#define AWS_IOT_CLIENT_ID	"kegflow"						// your client ID
#define AWS_IOT_MY_THING_NAME "BeerFlow"						// your thing name
#define AWS_IOT_ROOT_CA_FILENAME "VeriSign-Class 3-Public-Primary-Certification-Authority-G5.pem"           // your root-CA filename
#define AWS_IOT_CERTIFICATE_FILENAME "3802b4d6cb-certificate.pem.crt"                 // your certificate filename
#define AWS_IOT_PRIVATE_KEY_FILENAME "3802b4d6cb-private.pem.key"              // your private key filename
//===============================================================
// SDK config, DO NOT modify it
#define AWS_IOT_PATH_PREFIX "../certs/"
#define AWS_IOT_ROOT_CA_PATH AWS_IOT_PATH_PREFIX AWS_IOT_ROOT_CA_FILENAME			// use this in config call
#define AWS_IOT_CERTIFICATE_PATH AWS_IOT_PATH_PREFIX AWS_IOT_CERTIFICATE_FILENAME	// use this in config call
#define AWS_IOT_PRIVATE_KEY_PATH AWS_IOT_PATH_PREFIX AWS_IOT_PRIVATE_KEY_FILENAME	// use this in config call

#endif
