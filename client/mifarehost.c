// Merlok, 2011, 2012
// people from mifare@nethemba.com, 2010
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// mifare commands
//-----------------------------------------------------------------------------

#include "mifarehost.h"

// MIFARE
extern int compar_int(const void * a, const void * b) {
	// didn't work: (the result is truncated to 32 bits)
	//return (*(uint64_t*)b - *(uint64_t*)a);

	// better:
	if (*(uint64_t*)b > *(uint64_t*)a) return 1;
	if (*(uint64_t*)b < *(uint64_t*)a) return -1;	
	return 0;

	//return (*(uint64_t*)b > *(uint64_t*)a) - (*(uint64_t*)b < *(uint64_t*)a);
}

// Compare 16 Bits out of cryptostate
int Compare16Bits(const void * a, const void * b) {
	 if ((*(uint64_t*)b & 0x00ff000000ff0000) > (*(uint64_t*)a & 0x00ff000000ff0000)) return 1;	
	 if ((*(uint64_t*)b & 0x00ff000000ff0000) < (*(uint64_t*)a & 0x00ff000000ff0000)) return -1;
	 return 0;

/*	return 
		((*(uint64_t*)b & 0x00ff000000ff0000) > (*(uint64_t*)a & 0x00ff000000ff0000))
		-
		((*(uint64_t*)b & 0x00ff000000ff0000) < (*(uint64_t*)a & 0x00ff000000ff0000))
		;
*/
}

// wrapper function for multi-threaded lfsr_recovery32
void* nested_worker_thread(void *arg)
{
	struct Crypto1State *p1;
	StateList_t *statelist = arg;
	statelist->head.slhead = lfsr_recovery32(statelist->ks1, statelist->nt ^ statelist->uid);	
	
	for (p1 = statelist->head.slhead; *(uint64_t *)p1 != 0; p1++);
	
	statelist->len = p1 - statelist->head.slhead;
	statelist->tail.sltail = --p1;
	qsort(statelist->head.slhead, statelist->len, sizeof(uint64_t), Compare16Bits);
	return statelist->head.slhead;
}

int mfnested(uint8_t blockNo, uint8_t keyType, uint8_t * key, uint8_t trgBlockNo, uint8_t trgKeyType, uint8_t * resultKey, bool calibrate) 
{
	uint16_t i;
	uint32_t uid;
	UsbCommand resp;
	StateList_t statelists[2];
	struct Crypto1State *p1, *p2, *p3, *p4;
	
	UsbCommand c = {CMD_MIFARE_NESTED, {blockNo + keyType * 0x100, trgBlockNo + trgKeyType * 0x100, calibrate}};
	memcpy(c.d.asBytes, key, 6);
	clearCommandBuffer();
	SendCommand(&c);
	if (!WaitForResponseTimeout(CMD_ACK, &resp, 1500)) return -1;

	// error during nested
	if (resp.arg[0]) return resp.arg[0];
	
	memcpy(&uid, resp.d.asBytes, 4);
		
	for (i = 0; i < 2; i++) {
		statelists[i].blockNo = resp.arg[2] & 0xff;
		statelists[i].keyType = (resp.arg[2] >> 8) & 0xff;
		statelists[i].uid = uid;
		memcpy(&statelists[i].nt,  (void *)(resp.d.asBytes + 4 + i * 8 + 0), 4);
		memcpy(&statelists[i].ks1, (void *)(resp.d.asBytes + 4 + i * 8 + 4), 4);
	}
	
	// calc keys	
	pthread_t thread_id[2];
		
	// create and run worker threads
	for (i = 0; i < 2; i++)
		pthread_create(thread_id + i, NULL, nested_worker_thread, &statelists[i]);

	// wait for threads to terminate:
	for (i = 0; i < 2; i++)
		pthread_join(thread_id[i], (void*)&statelists[i].head.slhead);

	// the first 16 Bits of the cryptostate already contain part of our key.
	// Create the intersection of the two lists based on these 16 Bits and
	// roll back the cryptostate
	p1 = p3 = statelists[0].head.slhead; 
	p2 = p4 = statelists[1].head.slhead;

	while (p1 <= statelists[0].tail.sltail && p2 <= statelists[1].tail.sltail) {
		if (Compare16Bits(p1, p2) == 0) {
			
			struct Crypto1State savestate, *savep = &savestate;
			savestate = *p1;
			while(Compare16Bits(p1, savep) == 0 && p1 <= statelists[0].tail.sltail) {
				*p3 = *p1;
				lfsr_rollback_word(p3, statelists[0].nt ^ statelists[0].uid, 0);
				p3++;
				p1++;
			}
			savestate = *p2;
			while(Compare16Bits(p2, savep) == 0 && p2 <= statelists[1].tail.sltail) {
				*p4 = *p2;
				lfsr_rollback_word(p4, statelists[1].nt ^ statelists[1].uid, 0);
				p4++;
				p2++;
			}
		}
		else {
			while (Compare16Bits(p1, p2) == -1) p1++;
			while (Compare16Bits(p1, p2) == 1) p2++;
		}
	}

	p3->even = 0; p3->odd = 0;
	p4->even = 0; p4->odd = 0;
	statelists[0].len = p3 - statelists[0].head.slhead;
	statelists[1].len = p4 - statelists[1].head.slhead;
	statelists[0].tail.sltail = --p3;
	statelists[1].tail.sltail = --p4;

	// the statelists now contain possible keys. The key we are searching for must be in the
	// intersection of both lists. Create the intersection:
	qsort(statelists[0].head.keyhead, statelists[0].len, sizeof(uint64_t), compar_int);
	qsort(statelists[1].head.keyhead, statelists[1].len, sizeof(uint64_t), compar_int);
	
	uint64_t *p5, *p6, *p7;
	p5 = p7 = statelists[0].head.keyhead; 
	p6 = statelists[1].head.keyhead;
	
	while (p5 <= statelists[0].tail.keytail && p6 <= statelists[1].tail.keytail) {
		if (compar_int(p5, p6) == 0) {
			*p7++ = *p5++;
			p6++;
		}
		else {
			while (compar_int(p5, p6) == -1) p5++;
			while (compar_int(p5, p6) == 1) p6++;
		}
	}
	statelists[0].len = p7 - statelists[0].head.keyhead;
	statelists[0].tail.keytail = --p7;

	uint32_t numOfCandidates = statelists[0].len;
	if ( numOfCandidates == 0 ) goto out;
	
	memset(resultKey, 0, 6);
	uint64_t key64 = 0;

	// The list may still contain several key candidates. Test each of them with mfCheckKeys
	// uint32_t max_keys = keycnt > (USB_CMD_DATA_SIZE/6) ? (USB_CMD_DATA_SIZE/6) : keycnt;
	uint8_t keyBlock[USB_CMD_DATA_SIZE] = {0x00};

	for (i = 0; i < numOfCandidates; ++i){
		crypto1_get_lfsr(statelists[0].head.slhead + i, &key64);
		num_to_bytes(key64, 6, keyBlock + i * 6);
	}

	if (!mfCheckKeys(statelists[0].blockNo, statelists[0].keyType, false, numOfCandidates, keyBlock, &key64)) {		
		free(statelists[0].head.slhead);
		free(statelists[1].head.slhead);
		num_to_bytes(key64, 6, resultKey);

		PrintAndLog("UID: %08x target block:%3u key type: %c  -- Found key [%012"llx"]",
			uid,
			(uint16_t)resp.arg[2] & 0xff,
			(resp.arg[2] >> 8) ? 'B' : 'A',
			key64
		);
		return -5;
	}
	
out:
	PrintAndLog("UID: %08x target block:%3u key type: %c",
			uid,
			(uint16_t)resp.arg[2] & 0xff,
			(resp.arg[2] >> 8) ? 'B' : 'A'
	);	

	free(statelists[0].head.slhead);
	free(statelists[1].head.slhead);
	return -4;
}

int mfCheckKeys (uint8_t blockNo, uint8_t keyType, bool clear_trace, uint8_t keycnt, uint8_t * keyBlock, uint64_t * key){
	*key = 0;
	UsbCommand c = {CMD_MIFARE_CHKKEYS, { (blockNo | (keyType<<8)), clear_trace, keycnt}};
	memcpy(c.d.asBytes, keyBlock, 6 * keycnt);
	clearCommandBuffer();
	SendCommand(&c);
	UsbCommand resp;
	if (!WaitForResponseTimeout(CMD_ACK, &resp, 2500)) return 1;
	if ((resp.arg[0] & 0xff) != 0x01) return 2;
	*key = bytes_to_num(resp.d.asBytes, 6);
	return 0;
}
// PM3 imp of J-Run mf_key_brute (part 2)
// ref: https://github.com/J-Run/mf_key_brute
int mfKeyBrute(uint8_t blockNo, uint8_t keyType, uint8_t *key, uint64_t *resultkey){

	#define KEYS_IN_BLOCK 85
	#define KEYBLOCK_SIZE 510
	#define CANDIDATE_SIZE 0xFFFF * 6
	uint8_t found = FALSE;
	uint64_t key64 = 0;
	uint8_t candidates[CANDIDATE_SIZE] = {0x00};
	uint8_t keyBlock[KEYBLOCK_SIZE] = {0x00};

	memset(candidates, 0, sizeof(candidates));
	memset(keyBlock, 0, sizeof(keyBlock));
	
	// Generate all possible keys for the first two unknown bytes.
	for (uint16_t i = 0; i < 0xFFFF; ++i) {		
		uint32_t j = i * 6;		
		candidates[0 + j] = i >> 8;	
		candidates[1 + j] = i;
		candidates[2 + j] = key[2];
		candidates[3 + j] = key[3];
		candidates[4 + j] = key[4];
		candidates[5 + j] = key[5];
	}
	uint32_t counter, i;
	for ( i = 0, counter = 1; i < CANDIDATE_SIZE; i += KEYBLOCK_SIZE, ++counter){

		key64 = 0;
		
		// copy candidatekeys to test key block
		memcpy(keyBlock, candidates + i, KEYBLOCK_SIZE);

		// check a block of generated candidate keys.
		if (!mfCheckKeys(blockNo, keyType, TRUE, KEYS_IN_BLOCK, keyBlock, &key64)) {
			*resultkey = key64;
			found = TRUE;
			break;
		}
		
		// progress 
		if ( counter % 20 == 0 )
			PrintAndLog("tried : %s.. \t %u keys", sprint_hex(candidates + i, 6),  counter * KEYS_IN_BLOCK  );
	}
	return found;
}


// EMULATOR

int mfEmlGetMem(uint8_t *data, int blockNum, int blocksCount) {
	UsbCommand c = {CMD_MIFARE_EML_MEMGET, {blockNum, blocksCount, 0}};
	clearCommandBuffer();
 	SendCommand(&c);
	UsbCommand resp;
	if (!WaitForResponseTimeout(CMD_ACK,&resp,1500)) return 1;
	memcpy(data, resp.d.asBytes, blocksCount * 16);
	return 0;
}

int mfEmlSetMem(uint8_t *data, int blockNum, int blocksCount) {
	return mfEmlSetMem_xt(data, blockNum, blocksCount, 16);
}

int mfEmlSetMem_xt(uint8_t *data, int blockNum, int blocksCount, int blockBtWidth) {
	UsbCommand c = {CMD_MIFARE_EML_MEMSET, {blockNum, blocksCount, blockBtWidth}};
	memcpy(c.d.asBytes, data, blocksCount * blockBtWidth); 
	clearCommandBuffer();
	SendCommand(&c);
	return 0;
}

// "MAGIC" CARD
int mfCSetUID(uint8_t *uid, uint8_t *atqa, uint8_t *sak, uint8_t *oldUID, uint8_t wipecard) {

	uint8_t params = MAGIC_SINGLE;
	uint8_t block0[16];
	memset(block0, 0x00, sizeof(block0));

	int old = mfCGetBlock(0, block0, params);
	if (old == 0)
		PrintAndLog("old block 0:  %s", sprint_hex(block0, sizeof(block0)));
	else 
		PrintAndLog("Couldn't get old data. Will write over the last bytes of Block 0.");	

	// fill in the new values
	// UID
	memcpy(block0, uid, 4); 
	// Mifare UID BCC
	block0[4] = block0[0]^block0[1]^block0[2]^block0[3];
	// mifare classic SAK(byte 5) and ATQA(byte 6 and 7, reversed)
	if ( sak != NULL )
		block0[5]=sak[0];
	
	if ( atqa != NULL ) {
		block0[6]=atqa[1];
		block0[7]=atqa[0];
	}
	PrintAndLog("new block 0:  %s", sprint_hex(block0,16));
	
	if ( wipecard )		 params |= MAGIC_WIPE;	
	if ( oldUID == NULL) params |= MAGIC_UID;
	
	return mfCSetBlock(0, block0, oldUID, params);
}

int mfCSetBlock(uint8_t blockNo, uint8_t *data, uint8_t *uid, uint8_t params) {

	uint8_t isOK = 0;
	UsbCommand c = {CMD_MIFARE_CSETBLOCK, {params, blockNo, 0}};
	memcpy(c.d.asBytes, data, 16); 
	clearCommandBuffer();
	SendCommand(&c);
	UsbCommand resp;
	if (WaitForResponseTimeout(CMD_ACK, &resp, 1500)) {
		isOK  = resp.arg[0] & 0xff;
		if (uid != NULL) 
			memcpy(uid, resp.d.asBytes, 4);
		if (!isOK) 
			return 2;
	} else {
		PrintAndLog("Command execute timeout");
		return 1;
	}
	return 0;
}

int mfCGetBlock(uint8_t blockNo, uint8_t *data, uint8_t params) {
	uint8_t isOK = 0;
	UsbCommand c = {CMD_MIFARE_CGETBLOCK, {params, blockNo, 0}};	
	clearCommandBuffer();
	SendCommand(&c);
	UsbCommand resp;
	if (WaitForResponseTimeout(CMD_ACK,&resp,1500)) {
		isOK  = resp.arg[0] & 0xff;
		memcpy(data, resp.d.asBytes, 16);
		if (!isOK) return 2;
	} else {
		PrintAndLog("Command execute timeout");
		return 1;
	}
	return 0;
}

// SNIFFER
// [iceman] so many global variables....

// constants
static uint8_t trailerAccessBytes[4] = {0x08, 0x77, 0x8F, 0x00};

// variables
char logHexFileName[FILE_PATH_SIZE] = {0x00};
static uint8_t traceCard[4096] = {0x00};
static char traceFileName[FILE_PATH_SIZE] = {0x00};
static int traceState = TRACE_IDLE;
static uint8_t traceCurBlock = 0;
static uint8_t traceCurKey = 0;

struct Crypto1State *traceCrypto1 = NULL;
struct Crypto1State *revstate = NULL;
uint64_t key = 0;
uint32_t ks2 = 0;
uint32_t ks3 = 0;

uint32_t cuid = 0;    // serial number
uint32_t nt =0;      // tag challenge
uint32_t nr_enc =0;  // encrypted reader challenge
uint32_t ar_enc =0;  // encrypted reader response
uint32_t at_enc =0;  // encrypted tag response

int isTraceCardEmpty(void) {
	return ((traceCard[0] == 0) && (traceCard[1] == 0) && (traceCard[2] == 0) && (traceCard[3] == 0));
}

int isBlockEmpty(int blockN) {
	for (int i = 0; i < 16; i++) 
		if (traceCard[blockN * 16 + i] != 0) return 0;

	return 1;
}

int isBlockTrailer(int blockN) {
	return ((blockN & 0x03) == 0x03);
}

int loadTraceCard(uint8_t *tuid, uint8_t uidlen) {
	FILE * f;
	char buf[64] = {0x00};
	uint8_t buf8[64] = {0x00};
	int i, blockNum;
	
	if (!isTraceCardEmpty()) 
		saveTraceCard();
		
	memset(traceCard, 0x00, 4096);
	memcpy(traceCard, tuid, uidlen);

	FillFileNameByUID(traceFileName, tuid, ".eml", uidlen);

	f = fopen(traceFileName, "r");
	if (!f) return 1;
	
	blockNum = 0;
		
	while(!feof(f)){
	
		memset(buf, 0, sizeof(buf));
		if (fgets(buf, sizeof(buf), f) == NULL) {
			PrintAndLog("No trace file found or reading error.");
			if (f) {
				fclose(f);
				f = NULL;
			}
			return 2;
		}

		if (strlen(buf) < 32){
			if (feof(f)) break;
			PrintAndLog("File content error. Block data must include 32 HEX symbols");
			if (f) {
				fclose(f);
				f = NULL;
			}
			return 2;
		}
		for (i = 0; i < 32; i += 2)
			sscanf(&buf[i], "%02X", (unsigned int *)&buf8[i / 2]);

		memcpy(traceCard + blockNum * 16, buf8, 16);

		blockNum++;
	}
	if (f) {
		fclose(f);
		f = NULL;
	}
	return 0;
}

int saveTraceCard(void) {
	
	if ((!strlen(traceFileName)) || (isTraceCardEmpty())) return 0;
	
	FILE * f;
	f = fopen(traceFileName, "w+");
	if ( !f ) return 1;
	
	for (int i = 0; i < 64; i++) {  // blocks
		for (int j = 0; j < 16; j++)  // bytes
			fprintf(f, "%02X", *(traceCard + i * 16 + j)); 
		fprintf(f,"\n");
	}
	fflush(f);
	if (f) {
		fclose(f);
		f = NULL;
	}
	return 0;
}

int mfTraceInit(uint8_t *tuid, uint8_t uidlen, uint8_t *atqa, uint8_t sak, bool wantSaveToEmlFile) {

	if (traceCrypto1) 
		crypto1_destroy(traceCrypto1);

	traceCrypto1 = NULL;

	if (wantSaveToEmlFile) 
		loadTraceCard(tuid, uidlen);
		
	traceCard[4] = traceCard[0] ^ traceCard[1] ^ traceCard[2] ^ traceCard[3];
	traceCard[5] = sak;
	memcpy(&traceCard[6], atqa, 2);
	traceCurBlock = 0;
	cuid = bytes_to_num(tuid+(uidlen-4), 4);
	traceState = TRACE_IDLE;
	return 0;
}

void mf_crypto1_decrypt(struct Crypto1State *pcs, uint8_t *data, int len, bool isEncrypted){
	uint8_t	bt = 0;
	int i;
	
	if (len != 1) {
		for (i = 0; i < len; i++)
			data[i] = crypto1_byte(pcs, 0x00, isEncrypted) ^ data[i];
	} else {
		bt = 0;		
		bt |= (crypto1_bit(pcs, 0, isEncrypted) ^ BIT(data[0], 0)) << 0;
		bt |= (crypto1_bit(pcs, 0, isEncrypted) ^ BIT(data[0], 1)) << 1;
		bt |= (crypto1_bit(pcs, 0, isEncrypted) ^ BIT(data[0], 2)) << 2;
		bt |= (crypto1_bit(pcs, 0, isEncrypted) ^ BIT(data[0], 3)) << 3;			
		data[0] = bt;
	}
	return;
}

int mfTraceDecode(uint8_t *data_src, int len, bool wantSaveToEmlFile) {

	if (traceState == TRACE_ERROR) return 1;

	if (len > 64) {
		traceState = TRACE_ERROR;
		return 1;
	}
	
	uint8_t data[64];
	memset(data, 0x00, sizeof(data));
	
	memcpy(data, data_src, len);
	
	if ((traceCrypto1) && ((traceState == TRACE_IDLE) || (traceState > TRACE_AUTH_OK))) {
		mf_crypto1_decrypt(traceCrypto1, data, len, 0);
		PrintAndLog("DEC| %s", sprint_hex(data, len));
		AddLogHex(logHexFileName, "DEC| ", data, len); 
	}
	
	switch (traceState) {
	case TRACE_IDLE: 
		// check packet crc16!
		if ((len >= 4) && (!CheckCrc14443(CRC_14443_A, data, len))) {
			PrintAndLog("DEC| CRC ERROR!!!");
			AddLogLine(logHexFileName, "DEC| ", "CRC ERROR!!!"); 
			traceState = TRACE_ERROR;  // do not decrypt the next commands
			return 1;
		}
		
		// AUTHENTICATION
		if ((len == 4) && ((data[0] == MIFARE_AUTH_KEYA) || (data[0] == MIFARE_AUTH_KEYB))) {
			traceState = TRACE_AUTH1;
			traceCurBlock = data[1];
			traceCurKey = data[0] == 60 ? 1:0;
			return 0;
		}

		// READ
		if ((len ==4) && ((data[0] == ISO14443A_CMD_READBLOCK))) {
			traceState = TRACE_READ_DATA;
			traceCurBlock = data[1];
			return 0;
		}

		// WRITE
		if ((len ==4) && ((data[0] == ISO14443A_CMD_WRITEBLOCK))) {
			traceState = TRACE_WRITE_OK;
			traceCurBlock = data[1];
			return 0;
		}

		// HALT
		if ((len ==4) && ((data[0] == ISO14443A_CMD_HALT) && (data[1] == 0x00))) {
			traceState = TRACE_ERROR;  // do not decrypt the next commands
			return 0;
		}
		return 0;
	case TRACE_READ_DATA: 
		if (len == 18) {
			traceState = TRACE_IDLE;

			if (isBlockTrailer(traceCurBlock)) {
				memcpy(traceCard + traceCurBlock * 16 + 6, data + 6, 4);
			} else {
				memcpy(traceCard + traceCurBlock * 16, data, 16);
			}
			if (wantSaveToEmlFile) saveTraceCard();
			return 0;
		} else {
			traceState = TRACE_ERROR;
			return 1;
		}
		break;
	case TRACE_WRITE_OK: 
		if ((len == 1) && (data[0] == 0x0a)) {
			traceState = TRACE_WRITE_DATA;
			return 0;
		} else {
			traceState = TRACE_ERROR;
			return 1;
		}
		break;
	case TRACE_WRITE_DATA: 
		if (len == 18) {
			traceState = TRACE_IDLE;
			memcpy(traceCard + traceCurBlock * 16, data, 16);
			if (wantSaveToEmlFile) saveTraceCard();
			return 0;
		} else {
			traceState = TRACE_ERROR;
			return 1;
		}
		break;
	case TRACE_AUTH1: 
		if (len == 4) {
			traceState = TRACE_AUTH2;
			nt = bytes_to_num(data, 4);
			return 0;
		} else {
			traceState = TRACE_ERROR;
			return 1;
		}
		break;
	case TRACE_AUTH2: 
		if (len == 8) {
			traceState = TRACE_AUTH_OK;
			nr_enc = bytes_to_num(data, 4);
			ar_enc = bytes_to_num(data + 4, 4);
			return 0;
		} else {
			traceState = TRACE_ERROR;
			return 1;
		}
		break;
	case TRACE_AUTH_OK: 
		if (len == 4) {
			traceState = TRACE_IDLE;
			at_enc = bytes_to_num(data, 4);
			
			//  decode key here)
			ks2 = ar_enc ^ prng_successor(nt, 64);
			ks3 = at_enc ^ prng_successor(nt, 96);
			revstate = lfsr_recovery64(ks2, ks3);
			lfsr_rollback_word(revstate, 0, 0);
			lfsr_rollback_word(revstate, 0, 0);
			lfsr_rollback_word(revstate, nr_enc, 1);
			lfsr_rollback_word(revstate, cuid ^ nt, 0);
			crypto1_get_lfsr(revstate, &key);
			PrintAndLog("Found Key: [%012"llx"]", key);
			
			//if ( tryMfk64(cuid, nt, nr_enc, ar_enc, at_enc, &key) )
			AddLogUint64(logHexFileName, "Found Key: ", key); 
			
			int blockShift = ((traceCurBlock & 0xFC) + 3) * 16;
			if (isBlockEmpty((traceCurBlock & 0xFC) + 3)) memcpy(traceCard + blockShift + 6, trailerAccessBytes, 4);
			
			if (traceCurKey)
				num_to_bytes(key, 6, traceCard + blockShift + 10);
			else
				num_to_bytes(key, 6, traceCard + blockShift);
			
			if (wantSaveToEmlFile)
				saveTraceCard();

			if (traceCrypto1)
				crypto1_destroy(traceCrypto1);
			
			// set cryptosystem state
			traceCrypto1 = lfsr_recovery64(ks2, ks3);
			
			return 0;
		} else {
			traceState = TRACE_ERROR;
			return 1;
		}
		break;
	default: 
		traceState = TRACE_ERROR;
		return 1;
	}
	return 0;
}

int tryDecryptWord(uint32_t nt, uint32_t ar_enc, uint32_t at_enc, uint8_t *data, int len){
	PrintAndLog("\nEncrypted data: [%s]", sprint_hex(data, len) );
	struct Crypto1State *s;
	ks2 = ar_enc ^ prng_successor(nt, 64);
	ks3 = at_enc ^ prng_successor(nt, 96);
	s = lfsr_recovery64(ks2, ks3);
	mf_crypto1_decrypt(s, data, len, FALSE);
	PrintAndLog("Decrypted data: [%s]", sprint_hex(data, len) );
	crypto1_destroy(s);
	return 0;
}
