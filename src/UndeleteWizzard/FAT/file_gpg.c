/*

    File: file_gpg.c

    Copyright (C) 2008 Christophe GRENIER <grenier@cgsecurity.org>
  
    This software is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
  
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
  
    You should have received a copy of the GNU General Public License along
    with this program; if not, write the Free Software Foundation, Inc., 51
    Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#include <stdio.h>
#include "types.h"
#include "common.h"
#include "filegen.h"
#include "log.h"

static void register_header_check_gpg(file_stat_t *file_stat);
static int header_check_gpg(const unsigned char *buffer, const unsigned int buffer_size, const unsigned int safe_header_only, const file_recovery_t *file_recovery, file_recovery_t *file_recovery_new);
static int data_check_gpg(const unsigned char *buffer, const unsigned int buffer_size, file_recovery_t *file_recovery);
static unsigned int openpgp_packet_tag(const unsigned char *buf);
static unsigned int openpgp_length_type(const unsigned char *buf, unsigned int *length_type);

const file_hint_t file_hint_gpg= {
  .extension="gpg",
  .description="OpenPGP/GPG (Partial support)",
  .min_header_distance=0,
  .max_filesize=PHOTOREC_MAX_FILE_SIZE,
  .recover=1,
  .enable_by_default=1,
  .register_header_check=&register_header_check_gpg
};

/* See rfc4880 OpenPGP Message Format */

/* Public-Key Encrypted Session Key Packets */
#define OPENPGP_TAG_PUBKEY_ENC_SESSION_KEY	1
/* Signature Packet */
#define OPENPGP_TAG_SIGNATURE			2
/* Symmetric-Key Encrypted Session Key Packets */
#define OPENPGP_TAG_SYMKEY_ENC_SESSION_KEY	3
/* One-Pass Signature Packets (Tag 4) */
#define OPENPGP_TAG_ONE_PASS_SIG		4
/* Secret-Key Packet (Tag 5) */
#define OPENPGP_TAG_SEC_KEY			5
/* Public-Key Packet (Tag 6)*/
#define OPENPGP_TAG_PUB_KEY			6
/* Secret-Subkey Packet (Tag 7)	*/
#define OPENPGP_TAG_SEC_SUBKEY			7
/* Compressed Data Packet (Tag 8) */
/* Symmetrically Encrypted Data Packet */
#define OPENPGP_TAG_SYM_ENC_DATA		9
/* Marker Packet (Tag 10) */
#define OPENPGP_TAG_MARKER			10
/* Literal Data Packet (Tag 11)
 * Trust Packet (Tag 12)
 */
 /* User ID Packet */
#define OPENPGP_TAG_USER_ID			13
 /* Public-Subkey Packet (Tag 14) */
#define OPENPGP_TAG_PUB_SUBKEY			14
/* User Attribute Packet (Tag 17)
 */
/* Sym. Encrypted Integrity Protected Data Packet */
#define OPENPGP_TAG_SYM_ENC_INTEGRITY		18
 /* Modification Detection Code Packet (Tag 19) */
static const unsigned char gpg_header_pkey_enc[1]= {0x85};
static const unsigned char gpg_header_symkey_enc[1]= {0x8c};
static const unsigned char gpg_header_seckey[1]= {0x95};
static const unsigned char pgp_header[5]= {0xa8, 0x03, 'P', 'G', 'P'};
#if 0
static const unsigned char gpg_header_pkey[1]= {0x99};
#endif

static void register_header_check_gpg(file_stat_t *file_stat)
{
  register_header_check(0, gpg_header_seckey, sizeof(gpg_header_seckey), &header_check_gpg, file_stat);
  register_header_check(0, gpg_header_symkey_enc, sizeof(gpg_header_symkey_enc), &header_check_gpg, file_stat);
  register_header_check(0, gpg_header_pkey_enc, sizeof(gpg_header_pkey_enc), &header_check_gpg, file_stat);
  register_header_check(0, pgp_header, sizeof(pgp_header), &header_check_gpg, file_stat);
#if 0
  register_header_check(0, gpg_header_pkey, sizeof(gpg_header_pkey), &header_check_gpg, file_stat);
#endif
}


static unsigned int openpgp_packet_tag(const unsigned char *buf)
{
  /* Bit 7 -- Always one */
  if((buf[0]&0x80)==0)
    return 0;	/* Invalid */
  return ((buf[0]&0x40)==0?((buf[0]>>2)&0x0f):(buf[0]&0x3f));
}

static unsigned int openpgp_length_type(const unsigned char *buf, unsigned int *length_type)
{
  /* Bit 7 -- Always one */
  if((buf[0]&0x80)==0)
  {
    /* Invalid */
    *length_type=0;
    return 0;
  }
  if((buf[0]&0x40)==0)
  {
    /* Old format */
    switch(buf[0]&0x3)
    {
      case 0:
	*length_type=2;
	return buf[1];
      case 1:
	*length_type=3;
	return (buf[1] << 8) | buf[2];
      case 2:
	*length_type=5;
	return (buf[1] << 24) |(buf[2] << 16) |  (buf[3] << 8) | buf[4];
      default:
	*length_type=1;
	return 0;
    }
  }
  /* One-Octet Body Length */
  if(buf[1]<=191)
  {
    *length_type=1+1;
    return buf[1];
  }
  /* Two-Octet Body Length */
  if(buf[1]<=223)
  {
    *length_type=1+2;
    return ((buf[1] - 192) << 8) + buf[2] + 192;
  }
  /* Five-Octet Body Length */
  if(buf[1]==255)
  {
    *length_type=1+5;
    return (buf[2] << 24) | (buf[3] << 16) | (buf[4] << 8)  | buf[5];
  }
  /* Partial Body Lengths */
  *length_type=1+1;
  return 1 << (buf[1]& 0x1F);
}

static  int is_valid_pubkey_algo(const int algo)
{
  /*  1          - RSA (Encrypt or Sign)
   *  2          - RSA Encrypt-Only
   *  3          - RSA Sign-Only
   *  16         - Elgamal (Encrypt-Only), see [ELGAMAL]
   *  17         - DSA (Digital Signature Standard)
   *  18         - Reserved for Elliptic Curve
   *  19         - Reserved for ECDSA
   *  20         - Elgamal (Encrypt or Sign)
   *  21         - Reserved for Diffie-Hellman (X9.42, as defined for IETF-S/MIME)
   *  100 to 110 - Private/Experimental algorithm
   */
  if(algo>=100 && algo<=110)
    return 1;
  switch(algo)
  {
    case 1:
    case 2:
    case 3:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
      return 1;
    default:
      return 0;
  }
}

static int is_valid_sym_algo(const int algo)
{
  /*
       0          - Plaintext or unencrypted data
       1          - IDEA [IDEA]
       2          - TripleDES (DES-EDE, [SCHNEIER] [HAC] -
                    168 bit key derived from 192)
       3          - CAST5 (128 bit key, as per [RFC2144])
       4          - Blowfish (128 bit key, 16 rounds) [BLOWFISH]
       5          - Reserved
       6          - Reserved
       7          - AES with 128-bit key [AES]
       8          - AES with 192-bit key
       9          - AES with 256-bit key
       10         - Twofish with 256-bit key [TWOFISH]
       100 to 110 - Private/Experimental algorithm
       */
  switch(algo)
  {
    case 1:
    case 2:
    case 3:
    case 4:
    case 7:
    case 8:
    case 9:
    case 10:
      return 1;
    default:
      return 0;
  }
}

static int is_valid_S2K(const unsigned int algo)
{
  /*  ID          S2K Type
   *  --          --------
   *  0           Simple S2K
   *  1           Salted S2K
   *  2           Reserved value
   *  3           Iterated and Salted S2K
   *  100 to 110  Private/Experimental S2K
   */
  return (algo==0 || algo==1 || algo==3);
}

static int header_check_gpg(const unsigned char *buffer, const unsigned int buffer_size, const unsigned int safe_header_only, const file_recovery_t *file_recovery, file_recovery_t *file_recovery_new)
{
  unsigned int packet_tag[5];
  unsigned int length_type[5];
  unsigned int length[5];
  unsigned int nbr=0;
  unsigned int i=0;
  int start_recovery=0;
  memset(packet_tag, 0, sizeof(packet_tag));
  memset(length_type, 0, sizeof(length_type));
  memset(length, 0, sizeof(length));
  while(nbr<5 && i < buffer_size - 5)
  {
    const unsigned int tag=openpgp_packet_tag(&buffer[i]);
    if((buffer[i]&0x80)==0)
      break;
    packet_tag[nbr]=tag;
    length[nbr]=openpgp_length_type(&buffer[i], &length_type[nbr]);

    log_info("GPG 0x%04x: %02x %02x %02x %02x %02x %02x\n",
	i, buffer[i], buffer[i+1], buffer[i+2], buffer[i+3], buffer[i+4], buffer[i+5]);
    log_info("GPG 0x%04x: %02u tag=%2u, size=%u + %u)\n",
	i, nbr, tag, length_type[nbr], length[nbr]);
#if 0
    if(tag==0 || tag==15 || tag>19)	/* Reserved or unused */
      return 0;
#endif
    if(length_type[nbr]==0)
      break;	/* Don't know how to find the size */
    i+=length_type[nbr];
    if(tag==OPENPGP_TAG_PUBKEY_ENC_SESSION_KEY)
    {
      /* uint8_t  version	must be 3
       * uint64_t pub_key_id
       * uint8_t  pub_key_algo
       *          encrypted_session_key	*/
      if(buffer[i]==3 && is_valid_pubkey_algo(buffer[i+1+8]))
      {
	log_info("GPG :pubkey enc packet: version %u, algo %u, keyid ?\n", buffer[i], buffer[i+1+8]);
      }
      else
	return 0;
    }
    else if(tag==OPENPGP_TAG_SIGNATURE)
    {
      /* v3 - length=5 */
      if(buffer[i]==3 && buffer[i+1]==5 && is_valid_pubkey_algo(buffer[i+1+5+8]))
      {
      }
      /* v4 */
      else if(buffer[i]==4 && is_valid_pubkey_algo(buffer[i+2]))
      {
      }
      else
	return 0;
    }
    else if(tag==OPENPGP_TAG_SYMKEY_ENC_SESSION_KEY)
    {
      /* v4 */
      if(buffer[i]==4 && is_valid_sym_algo(buffer[i+1]) && is_valid_S2K(buffer[i+2]))
      {
      }
      else
	return 0;
    }
    else if(tag==OPENPGP_TAG_ONE_PASS_SIG)
    {
      if(buffer[i]==3 && is_valid_sym_algo(buffer[i+1]))
      {
      }
      else
	return 0;
    }
    else if(tag==OPENPGP_TAG_SYM_ENC_DATA)
    {
      unsigned int j;
      int ok=0;
      /* The symmetric cipher used may be specified in a Public-Key or
       * Symmetric-Key Encrypted Session Key packet that precedes the
       * Symmetrically Encrypted Data packet.
       * PhotoRec assumes it must */
      for(j=0; j<nbr; j++)
      {
	if(packet_tag[j]==OPENPGP_TAG_PUBKEY_ENC_SESSION_KEY ||
	    packet_tag[j]==OPENPGP_TAG_SYMKEY_ENC_SESSION_KEY)
	  ok=1;
      }
      if(ok==0)
	return 0;
    }
    else if(tag==OPENPGP_TAG_MARKER)
    {
      /* Must be at the beginning of the packet */
      if(nbr!=0)
	return 0;
    }
    else if(tag==OPENPGP_TAG_SYM_ENC_INTEGRITY)
    {
      unsigned int j;
      int ok=0;
      /* Version must be 1 */
      if(buffer[i]!=1)
	return 0;
      /* The symmetric cipher used MUST be specified in a Public-Key or
       * Symmetric-Key Encrypted Session Key packet that precedes the
       * Symmetrically Encrypted Data packet. */
      for(j=0; j<nbr; j++)
      {
	if(packet_tag[j]==OPENPGP_TAG_PUBKEY_ENC_SESSION_KEY ||
	    packet_tag[j]==OPENPGP_TAG_SYMKEY_ENC_SESSION_KEY)
	  ok=1;
      }
      if(ok==0)
	return 0;
    }
    else if(tag==OPENPGP_TAG_PUB_KEY ||
	tag==OPENPGP_TAG_PUB_SUBKEY ||
	tag==OPENPGP_TAG_SEC_KEY||
	tag==OPENPGP_TAG_SEC_SUBKEY)
    {
      if(buffer[i]==3 && is_valid_pubkey_algo(buffer[i+1+4+2]))
      { /* version 3 */
      }
      else if(buffer[i]==4 && is_valid_pubkey_algo(buffer[i+1+4]))
      { /* version 4 */
      }
      else
	return 0;
    }
    i+=length[nbr];
    nbr++;
  }
  if(nbr<2)
    return 0;
  if(memcmp(buffer, pgp_header, sizeof(pgp_header))==0)
  {
    reset_file_recovery(file_recovery_new);
    file_recovery_new->extension="pgp";
    return 1;
  }
  /* Public-Key Encrypted Session Key Packet v3 */
  if(buffer[0]==0x85 && buffer[3]==0x03 && is_valid_pubkey_algo(buffer[3+1+8]))
  {
    for(i=1;i<nbr;i++)
    {
      /* Sym. Encrypted and Integrity Protected Data Packet */
      if(packet_tag[i]==OPENPGP_TAG_SYM_ENC_INTEGRITY)
      {
	start_recovery=1;
	break;
      }
      /* Public-Key Encrypted Session Key Packet */
      else if(packet_tag[i]!=OPENPGP_TAG_PUBKEY_ENC_SESSION_KEY)
	return 0;
    }
  }
  if(start_recovery>0)
  {
    reset_file_recovery(file_recovery_new);
    file_recovery_new->extension=file_hint_gpg.extension;
    return 1;
  }
  /* Secret-Key Packet v4 followed by User ID Packet */
  if(buffer[0]==0x95 && buffer[3]==0x04 && packet_tag[1]==OPENPGP_TAG_USER_ID && is_valid_pubkey_algo(buffer[8])>0)
  {
    /* 3: version
     * 4: time
     * 8: public-key algorithm
     */
    reset_file_recovery(file_recovery_new);
    file_recovery_new->extension=file_hint_gpg.extension;
    file_recovery_new->data_check=&data_check_gpg;
    file_recovery_new->file_check=&file_check_size;
    return 1;
  }
  /* symkey enc packet: version 4 followed by Symmetrically Encrypted Data Packet */
  if(buffer[0]==0x8c && buffer[2]==0x04 && is_valid_sym_algo(buffer[3])>0 && packet_tag[1]==OPENPGP_TAG_SYM_ENC_DATA)
  {
    reset_file_recovery(file_recovery_new);
    file_recovery_new->extension=file_hint_gpg.extension;
    return 1;
  }
  /* Public-Key Packet + User ID Packet */
#if 0
  if(buffer[0]==0x99 && packet_tag[1]==OPENPGP_TAG_USER_ID)
    start_recovery=1;
#endif
  return 0;
}

static int data_check_gpg(const unsigned char *buffer, const unsigned int buffer_size, file_recovery_t *file_recovery)
{
  while(file_recovery->calculated_file_size + buffer_size/2  >= file_recovery->file_size &&
      file_recovery->calculated_file_size + 8 < file_recovery->file_size + buffer_size/2)
  {
    unsigned int packet_tag;
    unsigned int length_type;
    unsigned int length;
    unsigned int i=file_recovery->calculated_file_size - file_recovery->file_size + buffer_size/2;
    packet_tag=openpgp_packet_tag(&buffer[i]);
    if(packet_tag==0)	/* Reserved */
      return 2;
    length=openpgp_length_type(&buffer[i], &length_type);
    if(length_type==0)
      return 2;	/* Don't know how to find the size */
#ifdef DEBUG_GPG
    log_info("gpg tag %u, size=%u\n", packet_tag, length);
#endif
    file_recovery->calculated_file_size+=length_type;
    file_recovery->calculated_file_size+=length;
  }
  return 1;
}

