#include "layer1.h"
#include "layer2.h"
#include "aes.h"

Layer2::Layer2(Layer1 layer1, byte myAddress) {
  layer1.enableEmit(10, 5);
  
  this->layer1 = layer1;
  this->myAddress = myAddress;
  this->aes = AES();
}

void Layer2::send(byte receiver, byte *data, byte size, unsigned short destSecretId, byte *destAesKey, SequenceNumberGenerator sequenceNumber) {
  Rf433Layer2_Packet packet;

  // Check max data size
  if (size > RF433_LAYER2_MAX_DATA_SIZE) {
    size = RF433_LAYER2_MAX_DATA_SIZE;
  }

  // Build packet
  unsigned long seqNb = sequenceNumber(receiver);
  packet.header.protocol = RF433_LAYER2_PROTOCOL;
  packet.header.size = size;
  packet.header.sender = this->myAddress;
  packet.header.receiver = receiver;
  packet.header.sequenceNumber = seqNb;
  packet.cipheredData.secretId = destSecretId;
  memcpy(packet.cipheredData.data, data, size);

  // Encrypt secretId + data 
  // ctr must be unique for each message. ctr = nonce + counter
  // nonce is the sequence number here
  byte ctr[16] = {0};
  memcpy(ctr, &seqNb, sizeof(unsigned long));
  this->aes.ctr(&ctr, destAesKey, &(packet.cipheredData), sizeof(unsigned short) + size);

  // Send packet
  this->layer1.emit((byte*) &packet, (long) sizeof(Rf433Layer2_Header) + sizeof(unsigned short) + size);
}

int Layer2::receive(byte *data, unsigned short mySecretId, byte *myAesKey, SequenceNumberChecker sequenceNumberCheck) {
  byte buffer[RF433_LAYER1_MAX_DATA_SIZE];

  int received = this->layer1.receive(buffer);
  if (received <= 0) {
    return received;
  }

  // Check received size fits in layer2 packet
  if (received < (sizeof(Rf433Layer2_Header) + sizeof(unsigned short))) {
    return -1;
  }
  if (received > (sizeof(Rf433Layer2_Header) + sizeof(unsigned short)) + RF433_LAYER2_MAX_DATA_SIZE) {
    return -1;
  }
  byte computedDataSize = received - sizeof(Rf433Layer2_Header) - sizeof(unsigned short);

  // Get layer2 header
  Rf433Layer2_Header header;
  memcpy(&header, buffer, sizeof(Rf433Layer2_Header));

  // Check supported protocol
  if (header.protocol != RF433_LAYER2_PROTOCOL) {
    return -1;
  }

  // Check if we are the recipient of this message
  if (header.receiver != this->myAddress) {
    return -1;
  }

  // Check if the packet if fully received
  if (header.size <= 0 || header.size > computedDataSize) {
    return -1;
  }

  // Check the sequence number / sender are accepted
  if (!sequenceNumberCheck(header.sequenceNumber, header.sender)) {
    return -1;
  }

  // Get ciphered part (secretId + data)
  Rf433Layer2_CipheredData cipheredData;
  memcpy(&cipheredData, buffer + sizeof(Rf433Layer2_Header), sizeof(unsigned short) + header.size);

  // Decrypt secretId and data
  byte ctr[16] = {0};
  memcpy(ctr, &(header.sequenceNumber), sizeof(unsigned long));
  this->aes.ctr(&ctr, myAesKey, &cipheredData, sizeof(unsigned short) + header.size);

  // Check secretId is correct
  if (cipheredData.secretId != mySecretId) {
    return -1;
  }

  // Return data
  memcpy(data, &(cipheredData.data), header.size);
  return header.size;
}
