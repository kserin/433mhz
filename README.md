# 433mhz

Project to send 433MHz message with Arduino using [rc-switch library](https://github.com/sui77/rc-switch).

# Protocol

Two layers:
  1. The first one consists of fragments of 32 bits (a `long` for Arduino). This layer is in charge to segment data in several fragments and to ensure intergrity using CRC check.
  2. TODO (addresses, cipher, seq number anti replay attack)

## Layer 1

Fragment datagram:
<table>
  <tr>
    <th width=100>0</th>
    <th width=100>1</th>
    <th width=100>2</th>
    <th width=100>3</th>
    <th width=100>4</th>
    <th width=100>5</th>
    <th width=100>6</th>
    <th width=100>7</th>
  </tr>
  <tr align="center">
    <td colspan="4">Fragment number</td>
    <td colspan="4">CRC4</td>
  </tr>
  <tr align="center">
    <td colspan="8">Data</td>
  </tr>
  <tr align="center">
    <td colspan="8">Data</td>
  </tr>
  <tr align="center">
    <td colspan="8">Data</td>
  </tr>
</table>

The first fragment contains the total size of the data instead of the first data byte:
<table>
  <tr>
    <th width=100>0</th>
    <th width=100>1</th>
    <th width=100>2</th>
    <th width=100>3</th>
    <th width=100>4</th>
    <th width=100>5</th>
    <th width=100>6</th>
    <th width=100>7</th>
  </tr>
  <tr align="center">
    <td colspan="4">Fragment number ( = 0000)</td>
    <td colspan="4">CRC4</td>
  </tr>
  <tr align="center">
    <td colspan="8">Data size</td>
  </tr>
  <tr align="center">
    <td colspan="8">Data</td>
  </tr>
  <tr align="center">
    <td colspan="8">Data</td>
  </tr>
</table>

The protocol allows 16 fragments (4 bits used to code the fragment number). Each fragment allows 3 bytes (24 bits) of data except the first which allows 2 bytes (16 bits).
So the maximum data the layer 1 allows is: __47 bytes__.

:information_source: rc-switch library allows to repeat transmission of each fragment (using `setRepeatTransmit` function). In this case, if the message is only composed of one fragment (only fragment n°0), the message may be transmitted several times. For longer messages (more than one fragment), the duplication of fragments will be transparent (because the protocol will compute the next expected fragment number).

## Layer 2

Datagram:

<table>
  <tr>
    <th colspan="8">bits</th>
  </tr>
  <tr>
    <th width=100>0</th>
    <th width=100>1</th>
    <th width=100>2</th>
    <th width=100>3</th>
    <th width=100>4</th>
    <th width=100>5</th>
    <th width=100>6</th>
    <th width=100>7</th>
  </tr>
  <tr align="center">
    <td colspan="2">Protocol ( = 00)</td>
    <td colspan="6">Data size</td>
  </tr>
  <tr align="center">
    <td colspan="8">Sender address</td>
  </tr>
  <tr align="center">
    <td colspan="8">Receiver address</td>
  </tr>
  <tr align="center">
    <td colspan="8">Sequence number</td>
  </tr>
  <tr align="center">
    <td colspan="8">Sequence number</td>
  </tr>
  <tr align="center">
    <td colspan="8">Sequence number</td>
  </tr>
  <tr align="center">
    <td colspan="8">Sequence number</td>
  </tr>
  <tr align="center">
    <td colspan="8">Secret ID [encrypted part]</td>
  </tr>
  <tr align="center">
    <td colspan="8">Secret ID [encrypted part]</td>
  </tr>
  <tr align="center">
    <td colspan="8" color="red">Data (38 bytes) ... [encrypted part]</td>
  </tr>
</table>

Data size requires onlys 6 bits as the data part is __38 bytes max__ (47 bytes are allowed by layer1 and the header size is 9 bytes).

Addresses are encoded with 8 bits which allows __254 possible addresses__ (0x00 and x0FF are reserved).

The sequence number is a unique number that is incremented at each message. The receiver must ensure to don't trust any message with a sequence number already processed to avoid replay attacks.

The encrypted part (secret ID + data) is encrypted using AES256 CTR algorithm. The CTR nonce is composed of two part: the sequence number for the first 64 bits and the counter for the last 64 bits. As the data is composed of 38 bytes max, only 2 blocs of 128 bits (16 bytes) can be sent for each message.
The secretID is used like a password and it is checked after the message decryption. It also ensures that the payload has been correctly decrypted.
