//-----------------------------------------------------------------------------
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Low frequency Presco tag commands
//-----------------------------------------------------------------------------

#include "cmdlffdx.h"

/*
	FDX-B ISO11784/85 demod  (aka animal tag)  BIPHASE, inverted, rf/32,  with preamble of 00000000001 (128bits)
	8 databits + 1 parity (1)
	CIITT 16 checksum
	NATIONAL CODE, ICAR database
	COUNTRY CODE (ISO3166) or http://cms.abvma.ca/uploads/ManufacturersISOsandCountryCodes.pdf
	FLAG (animal/non-animal)

	38 IDbits   
	10 country code 
	1 extra app bit
	14 reserved bits
	1 animal bit
	16 ccitt CRC chksum over 64bit ID CODE.
	24 appli bits.

	sample: 985121004515220  [ 37FF65B88EF94 ]
*/

static int CmdHelp(const char *Cmd);

int usage_lf_fdx_clone(void){
	PrintAndLog("clone a FDX-B Animal tag to a T55x7 tag.");
	PrintAndLog("Usage: lf fdx clone [h] <country id> <animal id> <Q5>");
	PrintAndLog("Options:");
	PrintAndLog("      h            : This help");
	PrintAndLog("      <country id> : Country id");
	PrintAndLog("      <animal id>  : Animal id");
	// has extended data?
	//reserved/rfu
	//is animal tag
	// extended data
	PrintAndLog("      <Q5>        : specify write to Q5 (t5555 instead of t55x7)");
	PrintAndLog("");
	PrintAndLog("Sample: lf fdx clone 999 112233");
	return 0;
}

int usage_lf_fdx_sim(void) {
	PrintAndLog("Enables simulation of FDX-B Animal tag");
	PrintAndLog("Simulation runs until the button is pressed or another USB command is issued.");
	PrintAndLog("");
	PrintAndLog("Usage:  lf fdx sim [h] <country id> <animal id>");
	PrintAndLog("Options:");
	PrintAndLog("      h            : This help");
	PrintAndLog("      <country id> : Country id");
	PrintAndLog("      <animal id>  : Animal id");
	PrintAndLog("");
	PrintAndLog("Sample: lf fdx sim 999 112233");
	return 0;
}
// clearing the topbit needed for the preambl detection. 
static void verify_values(uint32_t countryid, uint64_t animalid){
	if ((animalid & 0x3FFFFFFFFF) != animalid) {
		animalid &= 0x3FFFFFFFFF;
		PrintAndLog("Animal ID Truncated to 38bits: %"PRIx64, animalid);
	}	
	if ( (countryid & 0x3ff) != countryid ) {
		countryid &= 0x3ff;
		PrintAndLog("Country ID Truncated to 10bits: %03d", countryid);
	}
}

int getFDXBits(uint64_t national_id, uint16_t country, uint8_t isanimal, uint8_t isextended, uint32_t extended, uint8_t *bits) {

    // add preamble ten 0x00 and one 0x01
    memset(bits, 0x00, 10);
	bits[10] = 1;
	
	// 128bits
    // every 9th bit is 0x01, but we can just fill the rest with 0x01 and overwrite
	memset(bits, 0x01, 128);
	
    // add preamble ten 0x00 and one 0x01
    memset(bits, 0x00, 10);

	// add reserved 
	num_to_bytebitsLSBF(0x00, 7, bits + 66);
	num_to_bytebitsLSBF(0x00 >> 7, 7, bits + 74);

	// add animal flag - OK
	bits[65] = isanimal;
	
	// add extended flag - OK
	bits[81] = isextended;
	
	// add national code 40bits - OK
	num_to_bytebitsLSBF(national_id >> 0, 8, bits+11);
	num_to_bytebitsLSBF(national_id >> 8, 8, bits+20);
	num_to_bytebitsLSBF(national_id >> 16, 8, bits+29);
	num_to_bytebitsLSBF(national_id >> 24, 8, bits+38);
	num_to_bytebitsLSBF(national_id >> 32, 6, bits+47);
	
	// add country code - OK
	num_to_bytebitsLSBF(country >> 0, 2, bits+53);
	num_to_bytebitsLSBF(country >> 2, 8, bits+56);

	// add crc-16 - OK
	uint8_t raw[8];
	for (uint8_t i=0; i<8; ++i)
		raw[i] = bytebits_to_byte(bits + 11 + i * 9, 8);
		
	uint16_t crc = crc16_ccitt_kermit(raw, 8);
	num_to_bytebitsLSBF(crc >> 0, 8, bits+83);
	num_to_bytebitsLSBF(crc >> 8, 8, bits+92);
	
	// extended data - OK
	num_to_bytebitsLSBF( extended >> 0 , 8, bits+101);
	num_to_bytebitsLSBF( extended >> 8 , 8, bits+110);
	num_to_bytebitsLSBF( extended >> 16, 8, bits+119);
	return 1;
}

//see ASKDemod for what args are accepted
//almost the same demod as cmddata.c/CmdFDXBdemodBI 
int CmdFdxDemod(const char *Cmd) {

	//Differential Biphase / di-phase (inverted biphase)
	//get binary from ask wave
	if (!ASKbiphaseDemod("0 32 1 0", FALSE)) {
		if (g_debugMode) PrintAndLog("DEBUG: Error - FDX-B ASKbiphaseDemod failed");
		return 0;
	}
	size_t size = DemodBufferLen;
	int ans = FDXBdemodBI(DemodBuffer, &size);
	if (ans < 0){
		if (g_debugMode){
			if (ans == -1)
				PrintAndLog("DEBUG: Error - FDX-B too few bits found");
			else if (ans == -2)
				PrintAndLog("DEBUG: Error - FDX-B preamble not found");
			else if (ans == -3)
				PrintAndLog("DEBUG: Error - FDX-B Size not correct: %d", size);
			else
				PrintAndLog("DEBUG: Error - FDX-B ans: %d", ans);
		}
		return 0;
	}

	setDemodBuf(DemodBuffer, 128, ans);
	// remove marker bits (1's every 9th digit after preamble) (pType = 2)
	size = removeParity(DemodBuffer, 11, 9, 2, 117);
	if ( size != 104 ) {
		if (g_debugMode) PrintAndLog("DEBUG: Error - FDX-B error removeParity: %d", size);
		return 0;
	}

	//got a good demod
	uint64_t NationalCode = ((uint64_t)(bytebits_to_byteLSBF(DemodBuffer+32,6)) << 32) | bytebits_to_byteLSBF(DemodBuffer,32);
	uint16_t countryCode = bytebits_to_byteLSBF(DemodBuffer+38,10);
	uint8_t dataBlockBit = DemodBuffer[48];
	uint32_t reservedCode = bytebits_to_byteLSBF(DemodBuffer+49,14);
	uint8_t animalBit = DemodBuffer[63];
	uint32_t crc16 = bytebits_to_byteLSBF(DemodBuffer+64,16);
	uint32_t extended = bytebits_to_byteLSBF(DemodBuffer+80,24);
	uint64_t rawid = ((uint64_t)
								//bytebits_to_byte(DemodBuffer, 8) << 96) | 
								//bytebits_to_byte(DemodBuffer,32) << 64) |
								bytebits_to_byte(DemodBuffer,32) << 32) |
								bytebits_to_byte(DemodBuffer+32, 32);
	uint8_t raw[8];
	num_to_bytes(rawid, 8, raw);

	uint16_t calcCrc = crc16_ccitt_kermit(raw, 8);
	
	PrintAndLog("\nFDX-B / ISO 11784/5 Animal Tag ID Found:  Raw : %s", sprint_hex(raw, 8));
	PrintAndLog("Animal ID          %04u-%012llu", countryCode, NationalCode);
	PrintAndLog("National Code      %012llu (0x%llX)", NationalCode, NationalCode);
	PrintAndLog("Country Code       %04u", countryCode);
	PrintAndLog("Reserved/RFU       %u (0x04%X)", reservedCode,  reservedCode);
	PrintAndLog("");
	PrintAndLog("Animal Tag         %s", animalBit ? "True" : "False");	
	PrintAndLog("Has extended data  %s [0x%X]", dataBlockBit ? "True" : "False", extended);	
	PrintAndLog("CRC-16             0x%04X - 0x%04X [%s]", crc16, calcCrc, (calcCrc == crc16) ? "Ok" : "Failed");

	if (g_debugMode) {
		PrintAndLog("Start marker %d;   Size %d", ans, size);	
		char *bin = sprint_bin_break(DemodBuffer, size, 16);
		PrintAndLog("DEBUG bin stream:\n%s", bin);
	}
	return 1;
}

int CmdFdxRead(const char *Cmd) {
	CmdLFRead("s");
	getSamples("12000", TRUE);
	return CmdFdxDemod(Cmd);
}

int CmdFdxClone(const char *Cmd) {

	uint32_t countryid = 0;
	uint64_t animalid = 0;
	uint32_t blocks[5] = {T55x7_MODULATION_DIPHASE | T55x7_BITRATE_RF_32 | 4<<T55x7_MAXBLOCK_SHIFT, 0, 0, 0, 0};
	uint8_t bits[128];
	uint8_t *bs = bits;
	memset(bs, 0, sizeof(bits));
	
	char cmdp = param_getchar(Cmd, 0);
	if (strlen(Cmd) == 0 || cmdp == 'h' || cmdp == 'H') return usage_lf_fdx_clone();

	countryid = param_get32ex(Cmd, 0, 0, 10);
	animalid = param_get64ex(Cmd, 1, 0, 10);
	
	//Q5
	if (param_getchar(Cmd, 1) == 'Q' || param_getchar(Cmd, 1) == 'q') {
		//t5555 (Q5) BITRATE = (RF-2)/2 (iceman)
		blocks[0] = T5555_MODULATION_BIPHASE | T5555_INVERT_OUTPUT | 32<<T5555_BITRATE_SHIFT | 4<<T5555_MAXBLOCK_SHIFT;
	}
	
	verify_values(countryid, animalid);
	
	// getFDXBits(uint64_t national_id, uint16_t country, uint8_t isanimal, uint8_t isextended, uint32_t extended, uint8_t *bits) 
	if ( !getFDXBits(animalid, countryid, 1, 0, 0, bs)) {
		PrintAndLog("Error with tag bitstream generation.");
		return 1;
	}	
	
	// convert from bit stream to block data
	blocks[1] = bytebits_to_byte(bs,32);
	blocks[2] = bytebits_to_byte(bs+32,32);
	blocks[3] = bytebits_to_byte(bs+64,32);
	blocks[4] = bytebits_to_byte(bs+96,32);

	PrintAndLog("Preparing to clone FDX-B to T55x7 with animal ID: %04u-%"PRIu64, countryid, animalid);
	PrintAndLog("Blk | Data ");
	PrintAndLog("----+------------");
	PrintAndLog(" 00 | 0x%08x", blocks[0]);
	PrintAndLog(" 01 | 0x%08x", blocks[1]);
	PrintAndLog(" 02 | 0x%08x", blocks[2]);
	PrintAndLog(" 03 | 0x%08x", blocks[3]);
	PrintAndLog(" 04 | 0x%08x", blocks[4]);
	
	UsbCommand resp;
	UsbCommand c = {CMD_T55XX_WRITE_BLOCK, {0,0,0}};

	for (int i = 4; i >= 0; --i) {
		c.arg[0] = blocks[i];
		c.arg[1] = i;
		clearCommandBuffer();
		SendCommand(&c);
		if (!WaitForResponseTimeout(CMD_ACK, &resp, 1000)){
			PrintAndLog("Error occurred, device did not respond during write operation.");
			return -1;
		}
	}
    return 0;
}

int CmdFdxSim(const char *Cmd) {
	uint32_t countryid = 0;
	uint64_t animalid = 0;

	char cmdp = param_getchar(Cmd, 0);
	if (strlen(Cmd) == 0 || cmdp == 'h' || cmdp == 'H') return usage_lf_fdx_sim();

	countryid = param_get32ex(Cmd, 0, 0, 10);
	animalid = param_get64ex(Cmd, 1, 0, 10);
	
	verify_values(countryid, animalid);
	
	uint8_t clk = 32, encoding = 2, separator = 0, invert = 1;
	uint16_t arg1, arg2;
	size_t size = 128;
	arg1 = clk << 8 | encoding;
	arg2 = invert << 8 | separator;

	PrintAndLog("Simulating FDX-B animal ID: %04u-%"PRIu64, countryid, animalid);

	UsbCommand c = {CMD_ASK_SIM_TAG, {arg1, arg2, size}};

	 //getFDXBits(uint64_t national_id, uint16_t country, uint8_t isanimal, uint8_t isextended, uint32_t extended, uint8_t *bits) 
	getFDXBits(animalid, countryid, 1, 0, 0, c.d.asBytes);
	clearCommandBuffer();
	SendCommand(&c);
	return 0;
}

static command_t CommandTable[] = {
    {"help",	CmdHelp,	1, "This help"},
	{"demod",	CmdFdxDemod,1, "Attempt extract tag data from graphbuf"},
	{"read",	CmdFdxRead,	0, "Attempt to read and extract tag data"},
	{"clone",	CmdFdxClone,0, "Clone animal ID tag to T55x7"},
	{"sim",		CmdFdxSim,	0, "Animal ID tag simulator"},
    {NULL, NULL, 0, NULL}
};

int CmdLFFdx(const char *Cmd) {
	clearCommandBuffer();
    CmdsParse(CommandTable, Cmd);
    return 0;
}

int CmdHelp(const char *Cmd) {
    CmdsHelp(CommandTable);
    return 0;
}
