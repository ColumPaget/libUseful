
/*
Copyright (c) 2015 Colum Paget <colums.projects@googlemail.com>
* SPDX-License-Identifier: LGPL-3.0-or-later
*/


#ifndef LIBUSEFUL_OPENSSL_H
#define LIBUSEFUL_OPENSSL_H
#include "includes.h"

/*
You should be able to activate SSL on any libUseful stream, except pipes.
For client connections this should be as simple as:

S=STREAMOpen("tcp://myhost.com:8080","");
DoSSLClientNegotiation(S, 0);

actually, you can go even simpler:

S=STREAMOpen("tls://myhost.com:8080","");

For server connections you must supply a certificate/key pair, like this:


fd=IPServerAccept(ListenSocket, &PeerAddress);
if (fd > -1)
{
S=STREAMFromFD(fd);
STREAMSetValue(S,"SSL:CertFile","/etc/ssl/myhost.crt");
STREAMSetValue(S,"SSL:KeyFile","/etc/ssl/myhost.key");
DoSSLServerNegotiation(S, 0);
}


Alternatively you can read the entirity of the certificate and key files into memory buffers, and then pass that data using SSL:CertData and SSL:KeyData, like this:

CertInMemory=FileRead(CertInMemory, "/etc/ssl/myhost.crt");
KeyInMemory=FileRead(KeyInMemory, "/etc/ssl/myhost.key");

fd=IPServerAccept(ListenSocket, &PeerAddress);
if (fd > -1)
{
S=STREAMFromFD(fd);
STREAMSetValue(S,"SSL:CertData", CertInMemory);
STREAMSetValue(S,"SSL:KeyData", KeyInMemory);
DoSSLServerNegotiation(S, 0);
}


This can be further simplified using 'STREAMServerNew':


Serv=STREAMServerNew("tls:0.0.0.0:8080", "SSL:CertFile=/etc/ssl/myhost.crt SSL:KeyFile=/etc/ssl/myhost.key");
Client=STREAMServerAccept(Serv);



if you want to do certificate verification on a certificate sent by a peer, then you must
supply a link to either a directory containing certs, or a single certificate file that
is a concatanation of certificates:

S=STREAMOpen("tcp://myhost.com:8080","");
STREAMSetValue(S,"SSL:VerifyCertFile","/etc/ssl/cacert.certs");
DoSSLClientNegotiation(S, 0);


S=STREAMOpen("tcp://myhost.com:8080","");
STREAMSetValue(S,"SSL:VerifyCertDir","/etc/ssl/rootcerts/");
DoSSLClientNegotiation(S, 0);

Or

S=STREAMOpen("tls://myhost.com:8080","SSL:verifyCertFile=/etc/ssl/caert.certs");
 

For Servers things are simliar:

Serv=STREAMServerNew("tls:0.0.0.0:8080", "SSL:CertFile=/etc/ssl/myhost.crt SSL:KeyFile=/etc/ssl/myhost.key SSL:VerifyCertFile=/etc/ssl/cacert.certs");
Client=STREAMServerAccept(Serv);


For both clients and servers you can then look up the value "SSL:CertificateVerify" which will be set to 'OK' if the peer certificate was authenticated:

    ptr=STREAMGetValue(S,"SSL:CertificateVerify");
    if (StrValid(ptr) && (strcmp(ptr, "OK")==0) ) AuthFlags |= SSL_AUTH_CERTIFICATE;

For clients this means you have authenticated the server certificate and thus believe you are speaking to the right server. For servers this usually means you have authenticated a 'user' using tehir personal certificate. 


You can also look up other information about a peer certificate:


ptr=STREAMGetValue(S,"SSL:CertificateCommonName");     - CommonName (hostname for servers or user name for clients)
ptr=STREAMGetValue(S,"SSL:CertificateIssuer");         - Which organization or person issued this certificate
ptr=STREAMGetValue(S,"SSL:CipherDetails");             - Details of cipher being used on he connection
ptr=STREAMGetValue(S,"SSL:Cipher");                    - Name of Cipher being used on he connection
ptr=STREAMGetValue(S,"SSL:CipherBits");                - Bitwidth of Cipher being used on he connection
ptr=STREAMGetValue(S,"SSL:CertificateNotBefore");      - Start Date of Certificate Validity
ptr=STREAMGetValue(S,"SSL:CertificateNotAfter");       - End Date of Certificate Validity
ptr=STREAMSetValue(S,"SSL:CertificateFingerprint");    - Certificate Fingerprint
ptr=STREAMSetValue(S,"SSL:CertificateSubject");        - Certificate Subject, a list of <name>=<value>/<name>=<value> pairs



All the values related to SSL can be set globally to be used with all SSL sockets


LibUsefulSetValue("SSL:CertFile" "/etc/ssl/myhost.crt");
LibUsefulSetValue("SSL:KeyFile" "/etc/ssl/myhost.key");
LibUsefulSetValue("SSL:VerifyCertDir", "/etc/ssl/certs/");

fd=IPServerAccept(ListenSocket, &PeerAddress);
if (fd > -1)
{
S=STREAMFromFD(fd);
DoSSLServerNegotiation(S, 0);
}


SSL Server connections, whether handled with STREAMServerAccept or DoSSLServerNegotiation, will only wait a certain time for the client to complete the handshake. By default this is 50 centiseconds, but that value can be changed by setting '"SSL:AcceptTimeout":

STREAMSetValue(S, "SSL:AcceptTimeout", "100");

*/



//Pass these flags to DoSSLServerNegotiation
#define LU_SSL_PFS             1    //use PerfectForwardSecrecy alogorithims
#define LU_SSL_VERIFY_PEER     2    //verify the certificate offered by the peer
#define LU_SSL_VERIFY_HOSTNAME 4    //Used internally to indicate a client connection should verify remote hostname


#ifdef __cplusplus
extern "C" {
#endif

//Check if SSL Is compiled in/ available for use. returns TRUE or FALSE
int SSLAvailable();

//is peer authenticated. Clients  can use certificate authentication and this function checks if they
//did and if the certificate passed checks
int OpenSSLIsPeerAuth(STREAM *S);

//if you connect a stream to something as a client then you can call this function to activate SSL/TLS on the connection
//currently 'Flags' does nothing, but is included for possible future uses
int DoSSLClientNegotiation(STREAM *S, int Flags);

//if you receive a connection, say with IPServerAccept, then apply this function to it to activate SSL/TLS on the connection
//'Flags' can be any of the LU_SSL_ flags listed above
int DoSSLServerNegotiation(STREAM *S, int Flags);

//functions internally used by STREAM objects.
int OpenSSLSTREAMCheckForBytes(STREAM *S);
int OpenSSLSTREAMReadBytes(STREAM *S, char *Data, int Len);
int OpenSSLSTREAMWriteBytes(STREAM *S, const char *Data, int Len);

//This is called automatically by STREAMClose. You won't generally explicitly call this.
void OpenSSLClose(STREAM *S);

//call this before doing anything else with a STREAM that's been 'accept'-ed from a server socket. If the stream is encrypted
//with SSL/TLS  this will return TRUE, FALSE otherwise
int OpenSSLAutoDetect(STREAM *S);

//call certificate verification code. This is normally called automatically by libUseful,
//and populates the results into SSL:CertificateVerify or SSL:CertificateSubject and SSL:CertificateIssuer.
//If called on a client side connection it will verify the server certificate,
//if called on an ACCEPTED server side connection, it will check any client authentication certificates provided
//'Flags' can be any of the LU_SSL_ flags listed above
int OpenSSLVerifyCertificate(STREAM *S, int Flags);


//Extract 'common name' from certificate subject. SSL:CertificateSubject is a list of values relating to a certificate,
//in the form <name>=<value>/<name>=<value>/... on of these values is CN=<name>/ which this function extracts
char *OpenSSLCertDetailsGetCommonName(char *RetStr, const char *CertDetails);

//you can call this to generate a new, random set of DHParams when a server starts up
//but I wouldn't if I were you. It takes a long time and this function is likely going
//to be replaced by something more versatile soon
void OpenSSLGenerateDHParams();

#ifdef __cplusplus
}
#endif


#endif
