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