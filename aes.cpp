#include "Arduino.h"
#include "aes.h"

#define AES_BLK_LEN 16
#define R(v,n)(((v)>>(n))|((v)<<(32-(n))))

AES::AES() {
}

unsigned long AES::M(unsigned long x) {
  unsigned long t = x & 0x80808080;
  return((x ^ t) << 1) ^ ((t >> 7) * 0x1b);
}

void AES::ecb(void *masterKey, void *data) {
  unsigned long *s = (unsigned long*) data;

  // copy 128-bit plain text
  unsigned long text[4];
  for (int i = 0; i < 4; i++) {
    text[i] = s[i];
  }

  // copy 256-bit master key
  unsigned long key[8];
  for (int i = 0; i < 8; i++) {
    key[i] = ((unsigned long*) masterKey)[i];
  }

  unsigned long c=1, r = 0;
  while (true) {
    // 1st part of ExpandKey
    unsigned long w = key[r ? 3 : 7];
    for (int i = 0; i < 4; i++) {
      w = ( w & -256) | sbox[w & 255];
      w = R(w, 8);
    } 
    // AddConstant, update constant
    if (!r) {
      w = R(w,8) ^ c;
      c = this->M(c);
    }
    // AddRoundKey, 2nd part of ExpandKey
    for (int i = 0; i < 4; i++) {
      ((unsigned long*)s)[i] = text[i]^key[r*4+i];
      w = key[r * 4 + i] ^= w;
    }
    // if round 15, stop
    if (c == 27) {
      break;
    }
    r = (r + 1) & 1;
    // SubBytes and ShiftRows
    for (int i = w = 0; i < AES_BLK_LEN; i++) {
      ((byte*) text)[w] = sbox[((byte*) s)[i]];
      w = (w - 3) & 15;
    }
    // if not round 15, MixColumns    
    if ((c != 128) | r) {
      for (int i = 0; i < 4; i++) {
        w = text[i];
        text[i] = R(w, 8) ^ R(w, 16) ^ R(w, 24) ^ this->M(R(w, 8) ^ w);
      }
    }
  }
}

void AES::ctr(void* ctr, void *masterKey, void *data, unsigned long size) {
  byte r, t[AES_BLK_LEN], *p=data, *c=ctr;
  
  while(size) {
    // copy counter+nonce to local buffer
    for(int i = 0; i < AES_BLK_LEN; i++) {
      t[i] = c[i];
    }
    // encrypt t
    this->ecb(masterKey, t);
    // XOR plaintext with ciphertext
    r = size > AES_BLK_LEN ? AES_BLK_LEN : size;
    for(int i = 0; i < r; i++) {
      p[i] ^= t[i];
    }
    // update length + position
    size -= r;
    p += r;
    // update counter
    for (int i = AES_BLK_LEN; i != 0; i--) {
      if (++c[i - 1]) {
        break;
      }
    }
  }
}
