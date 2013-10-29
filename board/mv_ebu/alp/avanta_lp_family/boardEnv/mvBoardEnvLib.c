/*******************************************************************************
   Copyright (C) Marvell International Ltd. and its affiliates

   This software file (the "File") is owned and distributed by Marvell
   International Ltd. and/or its affiliates ("Marvell") under the following
   alternative licensing terms.  Once you have made an election to distribute the
   File under one of the following license alternatives, please (i) delete this
   introductory statement regarding license alternatives, (ii) delete the two
   license alternatives that you have not elected to use and (iii) preserve the
   Marvell copyright notice above.

********************************************************************************
   Marvell Commercial License Option

   If you received this File from Marvell and you have entered into a commercial
   license agreement (a "Commercial License") with Marvell, the File is licensed
   to you under the terms of the applicable Commercial License.

********************************************************************************
   Marvell GPL License Option

   If you received this File from Marvell, you may opt to use, redistribute and/or
   modify this File in accordance with the terms and conditions of the General
   Public License Version 2, June 1991 (the "GPL License"), a copy of which is
   available along with the File in the license.txt file or by writing to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or
   on the worldwide web at http://www.gnu.org/licenses/gpl.txt.

   THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED
   WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY
   DISCLAIMED.  The GPL License provides additional details about this warranty
   disclaimer.
********************************************************************************
   Marvell BSD License Option

   If you received this File from Marvell, you may opt to use, redistribute and/or
   modify this File under the following licensing terms.
   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

*   Redistributions of source code must retain the above copyright notice,
            this list of conditions and the following disclaimer.

*   Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

*   Neither the name of Marvell nor the names of its contributors may be
        used to endorse or promote products derived from this software without
        specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/

#include "ctrlEnv/mvCtrlEnvLib.h"
#include "ctrlEnv/mvCtrlEnvSpec.h"
#include "boardEnv/mvBoardEnvLib.h"
#include "ctrlEnv/sys/mvCpuIf.h"
#include "cpu/mvCpu.h"
#include "cntmr/mvCntmr.h"
#include "gpp/mvGpp.h"
#include "twsi/mvTwsi.h"
#include "pex/mvPex.h"
#include "pci/mvPci.h"
#include "device/mvDevice.h"

#if defined(CONFIG_MV_ETH_NETA)
#include "neta/gbe/mvEthRegs.h"
#elif defined(CONFIG_MV_ETH_PP2)
#include "pp2/gbe/mvPp2GbeRegs.h"
#endif

#include "gpp/mvGppRegs.h"

/* defines  */
#undef MV_DEBUG
#ifdef MV_DEBUG
#define DB(x)   x
#else
#define DB(x)
#endif

extern MV_BOARD_INFO *boardInfoTbl[];
extern MV_BOARD_SATR_INFO boardSatrInfo[];
MV_BOARD_CONFIG_TYPE_INFO boardConfigTypesInfo[] = MV_BOARD_CONFIG_INFO;

/* Locals */
static MV_DEV_CS_INFO *boardGetDevEntry(MV_32 devNum, MV_BOARD_DEV_CLASS devClass);
static MV_BOARD_INFO *board = NULL;


/*******************************************************************************
* mvBoardEnvInit
*
* DESCRIPTION:
*	In this function the board environment take care of device bank
*	initialization.
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
MV_VOID mvBoardEnvInit(MV_VOID)
{
	mvBoardIdSet(mvBoardIdGet());

	/*  FPGA board doesn't use MPP neither GPIO */
#if !defined(CONFIG_MACH_AVANTA_LP_FPGA)
	MV_U32 nandDev;
	MV_U32 norDev;

	nandDev = boardGetDevCSNum(0, BOARD_DEV_NAND_FLASH);
	if (nandDev != 0xFFFFFFFF) {
		/* Set NAND interface access parameters */
		nandDev = BOOT_CS;
		MV_REG_WRITE(DEV_BANK_PARAM_REG(nandDev), board->nandFlashReadParams);
		MV_REG_WRITE(DEV_BANK_PARAM_REG_WR(nandDev), board->nandFlashWriteParams);
		MV_REG_WRITE(DEV_NAND_CTRL_REG, board->nandFlashControl);
	}

	norDev = boardGetDevCSNum(0, BOARD_DEV_NOR_FLASH);
	if (norDev != 0xFFFFFFFF) {
		/* Set NOR interface access parameters */
		MV_REG_WRITE(DEV_BANK_PARAM_REG(norDev), board->norFlashReadParams);
		MV_REG_WRITE(DEV_BANK_PARAM_REG_WR(norDev), board->norFlashWriteParams);
		MV_REG_WRITE(DEV_BUS_SYNC_CTRL, 0x11);
	}

	/* Set GPP Out value */
	MV_REG_WRITE(GPP_DATA_OUT_REG(0), board->gppOutValLow);
	MV_REG_WRITE(GPP_DATA_OUT_REG(1), board->gppOutValMid);
	MV_REG_WRITE(GPP_DATA_OUT_REG(2), board->gppOutValHigh);

	/* set GPP polarity */
	mvGppPolaritySet(0, 0xFFFFFFFF, board->gppPolarityValLow);
	mvGppPolaritySet(1, 0xFFFFFFFF, board->gppPolarityValMid);
	mvGppPolaritySet(2, 0xFFFFFFFF, board->gppPolarityValHigh);

	/* Set GPP Out Enable */
	mvGppTypeSet(0, 0xFFFFFFFF, board->gppOutEnValLow);
	mvGppTypeSet(1, 0xFFFFFFFF, board->gppOutEnValMid);
	mvGppTypeSet(2, 0xFFFFFFFF, board->gppOutEnValHigh);
#endif
}

/*******************************************************************************
* mvBoardModelGet - Get Board model
*
* DESCRIPTION:
*       This function returns 16bit describing board model.
*       Board model is constructed of one byte major and minor numbers in the
*       following manner:
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       String describing board model.
*
*******************************************************************************/
MV_U16 mvBoardModelGet(MV_VOID)
{
	return mvBoardIdGet() >> 16;
}

/*******************************************************************************
* mbBoardRevlGet - Get Board revision
*
* DESCRIPTION:
*       This function returns a 32bit describing the board revision.
*       Board revision is constructed of 4bytes. 2bytes describes major number
*       and the other 2bytes describes minor munber.
*       For example for board revision 3.4 the function will return
*       0x00030004.
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       String describing board model.
*
*******************************************************************************/
MV_U16 mvBoardRevGet(MV_VOID)
{
	return mvBoardIdGet() & 0xFFFF;
}

/*******************************************************************************
* mvBoardNameGet - Get Board name
*
* DESCRIPTION:
*       This function returns a string describing the board model and revision.
*       String is extracted from board I2C EEPROM.
*
* INPUT:
*       None.
*
* OUTPUT:
*       pNameBuff - Buffer to contain board name string. Minimum size 32 chars.
*
* RETURN:
*
*       MV_ERROR if informantion can not be read.
*******************************************************************************/
MV_STATUS mvBoardNameGet(char *pNameBuff, MV_U32 size)
{
	mvOsSNPrintf(pNameBuff, size, "%s", board->boardName);
	return MV_OK;
}

/*******************************************************************************
* mvBoardIsPortInSgmii -
*
* DESCRIPTION:
*       This routine returns MV_TRUE for port number works in SGMII or MV_FALSE
*	For all other options.
*
* INPUT:
*       ethPortNum - Ethernet port number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_TRUE - port in SGMII.
*       MV_FALSE - other.
*
*******************************************************************************/
MV_BOOL mvBoardIsPortInSgmii(MV_U32 ethPortNum)
{
#if defined(CONFIG_MACH_AVANTA_LP_FPGA)
	return ethPortNum == 2;
#endif

	return MV_FALSE;
}

/*******************************************************************************
* mvBoardIsPortInGmii
*
* DESCRIPTION:
*       This routine returns MV_TRUE for port number works in SGMII or MV_FALSE
*	For all other options.
*
* INPUT:
*       ethPortNum - Ethernet port number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_TRUE - port in SGMII.
*       MV_FALSE - other.
*
*******************************************************************************/
MV_BOOL mvBoardIsPortInGmii(MV_U32 ethPortNum)
{
	return MV_FALSE;
}

/*******************************************************************************
* mvBoardPhyAddrGet - Get the phy address
*
* DESCRIPTION:
*       This routine returns the Phy address of a given ethernet port.
*
* INPUT:
*       ethPortNum - Ethernet port number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       32bit describing Phy address, -1 if the port number is wrong.
*
*******************************************************************************/
MV_32 mvBoardPhyAddrGet(MV_U32 ethPortNum)
{
#if defined(CONFIG_MACH_AVANTA_LP_FPGA)
	return 8;
#endif

	if (ethPortNum >= board->numBoardMacInfo) {
		DB(mvOsPrintf("%s: Error: invalid ethPortNum (%d)\n", __func__, ethPortNum));
		return MV_ERROR;
	}

	return board->pBoardMacInfo[ethPortNum].boardEthSmiAddr;
}

/*******************************************************************************
* mvBoardPhyAddrSet - Set the phy address
*
* DESCRIPTION:
*       This routine sets the Phy address of a given ethernet port.
*
* INPUT:
*       ethPortNum - Ethernet port number.
*       smiAddr    - requested phy address
*
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
MV_VOID mvBoardPhyAddrSet(MV_U32 ethPortNum, MV_U32 smiAddr)
{
	if (ethPortNum >= board->numBoardMacInfo) {
		DB(mvOsPrintf("%s: Error: invalid ethPortNum (%d)\n", __func__, ethPortNum));
		return;
	}

	board->pBoardMacInfo[ethPortNum].boardEthSmiAddr = smiAddr;
}

/*******************************************************************************
* mvBoardSpecInitGet -
*
* DESCRIPTION:
*       This routine returns the board specific initializtion information.
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       A pointer to the array holding the board specific initializations.
*
*******************************************************************************/
MV_BOARD_SPEC_INIT *mvBoardSpecInitGet(MV_VOID)
{
	return board->pBoardSpecInit;
}

/*******************************************************************************
* mvBoardMacSpeedGet - Get the Mac speed
*
* DESCRIPTION:
*       This routine returns the Mac speed if pre define of a given ethernet port.
*
* INPUT:
*       ethPortNum - Ethernet port number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_BOARD_MAC_SPEED, -1 if the port number is wrong.
*
*******************************************************************************/
MV_BOARD_MAC_SPEED mvBoardMacSpeedGet(MV_U32 ethPortNum)
{
	if (ethPortNum >= board->numBoardMacInfo) {
		mvOsPrintf("%s: Error: wrong eth port (%d)\n", __func__, ethPortNum);
		return BOARD_MAC_SPEED_100M;
	}

#if defined(CONFIG_MACH_AVANTA_LP_FPGA)
	return (ethPortNum == 2) ? BOARD_MAC_SPEED_1000M : BOARD_MAC_SPEED_100M;
#endif

	return board->pBoardMacInfo[ethPortNum].boardMacSpeed;
}

/*******************************************************************************
* mvBoardMacSpeedSet - Set the Mac speed
*
* DESCRIPTION:
*       This routine Sets the Mac speed of a given ethernet port.
*
* INPUT:
*       ethPortNum - Ethernet port number.
*       macSpeed   - Requested MAC speed
*
* OUTPUT:
*       None.
*
* RETURN:
*       None
*
*******************************************************************************/
MV_VOID mvBoardMacSpeedSet(MV_U32 ethPortNum, MV_BOARD_MAC_SPEED macSpeed)
{
	if (ethPortNum >= board->numBoardMacInfo)
		mvOsPrintf("%s: Error: wrong eth port (%d)\n", __func__, ethPortNum);

	board->pBoardMacInfo[ethPortNum].boardMacSpeed = macSpeed;
}

/*******************************************************************************
* mvBoardIsPortLoopback -
*
* DESCRIPTION:
*       This routine returns MV_TRUE for loopback port number or MV_FALSE
*	For all other options.
*
* INPUT:
*       ethPortNum - Ethernet port number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_TRUE - port is loopback.
*       MV_FALSE - other.
*
*******************************************************************************/
MV_BOOL mvBoardIsPortLoopback(MV_U32 ethPortNum)
{
	return (ethPortNum == 2);
}

/*******************************************************************************
* mvBoardTclkGet
*
* DESCRIPTION:
*       This routine extract the controller core clock, aka, TCLK clock.
*
* INPUT:
* 	None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       32bit clock cycles in Hertz.
*
*******************************************************************************/
MV_U32 mvBoardTclkGet(MV_VOID)
{
	MV_U32 tclk;
#ifdef CONFIG_MACH_AVANTA_LP_FPGA
	/* FPGA is limited to 25Mhz */
	return MV_FPGA_CORE_CLK;
#else
	tclk = (MV_REG_READ(MPP_SAMPLE_AT_RESET(1)));
	tclk = ((tclk & 0x400000) >> 22);
	switch (tclk) {
	case 0:
		return MV_BOARD_TCLK_166MHZ;
	case 1:
		return MV_BOARD_TCLK_200MHZ;
	default:
		return MV_BOARD_TCLK_200MHZ;
	}
#endif
}

/*******************************************************************************
* mvBoardL2ClkGet
*
* DESCRIPTION:
*	This routine extract the L2 Cache frequency/clock.
*	Note: this function is called at the very early stage
*	in Linux Kernel, hence, it has to read from SoC register, not
*	from pre-built database.
*
* INPUT:
*	None.
*
* OUTPUT:
*	None.
*
* RETURN:
*	32bit clock cycles in Hertz.
*
*******************************************************************************/
MV_U32 mvBoardL2ClkGet(MV_VOID)
{
#ifdef CONFIG_MACH_AVANTA_LP_FPGA
	return MV_FPGA_L2_CLK;
#else
	MV_U32 clkSelect;
	MV_FREQ_MODE freq[] = MV_SAR_FREQ_MODES;

	clkSelect = MV_REG_READ(MPP_SAMPLE_AT_RESET(1));
	clkSelect = clkSelect & (0x1f << 17);
	clkSelect >>= 17;

	return 1000000 * freq[clkSelect].l2Freq;
#endif
}

/*******************************************************************************
* mvBoardSysClkGet - Get the board SysClk (CPU bus clock , i.e. DDR clock)
*
* DESCRIPTION:
*       This routine extract the CPU bus clock.
*
* INPUT:
*       countNum - Counter number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       32bit clock cycles in Hertz.
*
*******************************************************************************/
MV_U32 mvBoardSysClkGet(MV_VOID)
{
	MV_FREQ_MODE freqMode;
#ifdef CONFIG_MACH_AVANTA_LP_FPGA
	/* FPGA is limited to 25Mhz */
	return MV_FPGA_SYS_CLK;
#else
	if (MV_ERROR != mvCtrlCpuDdrL2FreqGet(&freqMode))
		return (MV_U32)(1000000 * freqMode.ddrFreq);
	else
		return MV_ERROR;
#endif
}

/*******************************************************************************
* mvBoardDebugLedNumGet - Get number of debug Leds
*
* DESCRIPTION:
* INPUT:
*       boardId
*
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
MV_U32 mvBoardDebugLedNumGet(MV_U32 boardId)
{
	return board->activeLedsNumber;
}

/*******************************************************************************
* mvBoardDebugLed - Set the board debug Leds
*
* DESCRIPTION: turn on/off status leds.
*              Note: assume MPP leds are part of group 0 only.
*
* INPUT:
*       hexNum - Number to be displied in hex by Leds.
*
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
MV_VOID mvBoardDebugLed(MV_U32 hexNum)
{
	/* empty */
}

/*******************************************************************************
* mvBoarGpioPinNumGet
*
* DESCRIPTION:
*
* INPUT:
*	gppClass - MV_BOARD_GPP_CLASS enum.
*
* OUTPUT:
*	None.
*
* RETURN:
*       GPIO pin number. The function return -1 for bad parameters.
*
*******************************************************************************/
MV_32 mvBoarGpioPinNumGet(MV_BOARD_GPP_CLASS gppClass, MV_U32 index)
{
	MV_U32 i, indexFound = 0;

	for (i = 0; i < board->numBoardGppInfo; i++) {
		if (board->pBoardGppInfo[i].devClass == gppClass) {
			if (indexFound == index)
				return (MV_U32)board->pBoardGppInfo[i].gppPinNum;
			else
				indexFound++;
		}
	}

	return MV_ERROR;
}

/*******************************************************************************
* mvBoardReset
*
* DESCRIPTION:
*	Reset the board
* INPUT:
*	None.
*
* OUTPUT:
*	None.
*
* RETURN:
*       None
*
*******************************************************************************/
MV_VOID mvBoardReset(MV_VOID)
{
	MV_32 resetPin;

	/* Get gpp reset pin if define */
	resetPin = mvBoardResetGpioPinGet();
	if (resetPin != MV_ERROR)
		MV_REG_BIT_RESET(GPP_DATA_OUT_REG((int)(resetPin/32)), (1 << (resetPin % 32)));
	else
	{
		/* No gpp reset pin was found, try to reset using system reset out */
		MV_REG_BIT_SET( CPU_RSTOUTN_MASK_REG , BIT0);
		MV_REG_BIT_SET( CPU_SYS_SOFT_RST_REG , BIT0);
	}
}

/*******************************************************************************
* mvBoardResetGpioPinGet
*
* DESCRIPTION:
*
* INPUT:
*	None.
*
* OUTPUT:
*	None.
*
* RETURN:
*       GPIO pin number. The function return -1 for bad parameters.
*
*******************************************************************************/
MV_32 mvBoardResetGpioPinGet(MV_VOID)
{
	return mvBoarGpioPinNumGet(BOARD_GPP_RESET, 0);
}

/*******************************************************************************
* mvBoardSDIOGpioPinGet
*
* DESCRIPTION:
*	used for hotswap detection
* INPUT:
*	type - Type of SDIO GPP to get.
*
* OUTPUT:
*	None.
*
* RETURN:
*       GPIO pin number. The function return -1 for bad parameters.
*
*******************************************************************************/
MV_32 mvBoardSDIOGpioPinGet(MV_BOARD_GPP_CLASS type)
{
	if (type != BOARD_GPP_SDIO_POWER &&
	    type != BOARD_GPP_SDIO_DETECT &&
	    type != BOARD_GPP_SDIO_WP)
		return MV_FAIL;

	return mvBoarGpioPinNumGet(type, 0);
}

/*******************************************************************************
* mvBoardUSBVbusGpioPinGet - return Vbus input GPP
*
* DESCRIPTION:
*
* INPUT:
*		int  devNo.
*
* OUTPUT:
*		None.
*
* RETURN:
*       GPIO pin number. The function return -1 for bad parameters.
*
*******************************************************************************/
MV_32 mvBoardUSBVbusGpioPinGet(MV_32 devId)
{
	return mvBoarGpioPinNumGet(BOARD_GPP_USB_VBUS, devId);
}

/*******************************************************************************
* mvBoardUSBVbusEnGpioPinGet - return Vbus Enable output GPP
*
* DESCRIPTION:
*
* INPUT:
*		int  devNo.
*
* OUTPUT:
*		None.
*
* RETURN:
*       GPIO pin number. The function return -1 for bad parameters.
*
*******************************************************************************/
MV_32 mvBoardUSBVbusEnGpioPinGet(MV_32 devId)
{
	return mvBoarGpioPinNumGet(BOARD_GPP_USB_VBUS_EN, devId);
}

/*******************************************************************************
* mvBoardGpioIntMaskGet - Get GPIO mask for interrupt pins
*
* DESCRIPTION:
*		This function returns a 32-bit mask of GPP pins that connected to
*		interrupt generating sources on board.
*		For example if UART channel A is hardwired to GPP pin 8 and
*		UART channel B is hardwired to GPP pin 4 the fuinction will return
*		the value 0x000000110
*
* INPUT:
*		None.
*
* OUTPUT:
*		None.
*
* RETURN:
*		See description. The function return -1 if board is not identified.
*
*******************************************************************************/
MV_U32 mvBoardGpioIntMaskGet(MV_U32 gppGrp)
{
	switch (gppGrp) {
	case 0:
		return board->intsGppMaskLow;
		break;
	case 1:
		return board->intsGppMaskMid;
		break;
	case 2:
		return board->intsGppMaskHigh;
		break;
	default:
		return MV_ERROR;
	}
}

/*******************************************************************************
* mvBoardSlicMppModeGet - Get board MPP Group type for SLIC unit (pre-defined)
*
* DESCRIPTION:
*	if not using auto detection mudules according to board configuration settings,
*	use pre-defined SLIC type from board information
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       32bit value describing MPP control register value.
*
*******************************************************************************/
MV_U32 mvBoardSlicUnitTypeGet(MV_VOID)
{
	return board->pBoardModTypeValue->boardMppSlic;
}

/*******************************************************************************
* mvBoardSlicUnitTypeSet - Get board MPP Group type for SLIC unit (pre-defined)
*
* DESCRIPTION:
*	if not using auto detection mudules according to board configuration settings,
*	use pre-defined SLIC type from board information
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       32bit value describing MPP control register value.
*
*******************************************************************************/
MV_VOID mvBoardSlicUnitTypeSet(MV_U32 slicType)
{
	if (slicType >= MV_BOARD_SLIC_MAX_OPTION) {
		mvOsPrintf("%s: Error: Unsupported SLIC/TDM configuration selected (%x)\n",
				__func__, slicType);
		slicType = MV_BOARD_SLIC_DISABLED;
	}

	board->pBoardModTypeValue->boardMppSlic = slicType;
}
/*******************************************************************************
* mvBoardMppGet - Get board dependent MPP register value
*
* DESCRIPTION:
*	MPP settings are derived from board design.
*	MPP group consist of 8 MPPs. An MPP group represents MPP
*	control register.
*       This function retrieves board dependend MPP register value.
*
* INPUT:
*       mppGroupNum - MPP group number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       32bit value describing MPP control register value.
*
*******************************************************************************/
MV_32 mvBoardMppGet(MV_U32 mppGroupNum)
{
	return board->pBoardMppConfigValue->mppGroup[mppGroupNum];
}

/*******************************************************************************
* mvBoardMppSet - Set board dependent MPP register value
*
* DESCRIPTION:
*	This function updates board dependend MPP register value.
*
* INPUT:
*       mppGroupNum - MPP group number.
*	mppValue - new MPP value to be written
*
* OUTPUT:
*       None.
*
* RETURN:
*       -None
*
*******************************************************************************/
MV_VOID mvBoardMppSet(MV_U32 mppGroupNum, MV_U32 mppValue)
{
	board->pBoardMppConfigValue->mppGroup[mppGroupNum] = mppValue;
}

/*******************************************************************************
* mvBoardMppTypeSet - Set board dependent MPP Group Type value
*
* DESCRIPTION:
*	This function updates board dependend MPP Group Type value.
*
* INPUT:
*       mppGroupNum - MPP group number.
*	groupType - new MPP Group type. derrive MPP Value using groupType
*
* OUTPUT:
*       None.
*
* RETURN:
*       -None
*
*******************************************************************************/
MV_VOID mvBoardMppTypeSet(MV_U32 mppGroupNum, MV_U32 groupType)
{
	MV_U32 mppVal;
	MV_U32 mppGroups[MV_BOARD_MAX_MPP_GROUPS][MV_BOARD_MPP_GROUPS_MAX_TYPES] = MPP_GROUP_TYPES;

	mppVal = mppGroups[mppGroupNum][groupType];
	mvBoardMppSet(mppGroupNum,mppVal);

	/* add Group types update here (if needed for later usage),
	 * and add mvBoardMppTypeGet to detect which type is in use currently */
}

/*******************************************************************************
* mvBoardInfoUpdate - Update Board information structures according to auto-detection.
*
* DESCRIPTION:
*	Update board information according to detection using TWSI bus.
*
* INPUT:
*	None.
*
* OUTPUT:
*       None.
*
* RETURN:
*	None.
*
*******************************************************************************/
MV_VOID mvBoardInfoUpdate(MV_VOID)
{
	MV_U32 smiAddress, slicDev, ethComplex;
	MV_BOARD_MAC_SPEED macSpeed = BOARD_MAC_SPEED_AUTO; /*if Mac is not connected to switch, auto-negotiate speed*/

	/* Update Ethernet complex according to board configuration */
	mvBoardEthComplexInfoUpdate();

	/* Update SMI phy address for MAC0/1 */
	ethComplex = mvBoardEthComplexConfigGet();
	if (ethComplex & MV_ETHCOMP_GE_MAC0_2_GE_PHY_P0)
		smiAddress = 0x0;
	else if (ethComplex & MV_ETHCOMP_GE_MAC0_2_RGMII0)
		smiAddress = 0x4;
	else {				/* else MAC0 is connected to SW port 6 */
		smiAddress = -1;	/* no SMI address if connected to switch */
		if (ethComplex & MV_ETHCOMP_P2P_MAC0_2_SW_SPEED_2G)
			macSpeed = BOARD_MAC_SPEED_2000M;
		else
			macSpeed = BOARD_MAC_SPEED_1000M;
	}
	mvBoardPhyAddrSet(0, smiAddress);
	mvBoardMacSpeedSet(0, macSpeed);

	macSpeed = BOARD_MAC_SPEED_AUTO; /*if Mac is not connected to switch, auto-negotiate speed*/
	if (ethComplex & MV_ETHCOMP_GE_MAC1_2_GE_PHY_P3)
		smiAddress = 0x3;
	else if (ethComplex & MV_ETHCOMP_GE_MAC1_2_RGMII1)
		smiAddress = 0x1;
	else {
		smiAddress = -1; /* no SMI address if connected to switch */
		macSpeed = BOARD_MAC_SPEED_1000M;
	}
	mvBoardPhyAddrSet(1, smiAddress);
	mvBoardMacSpeedSet(1, macSpeed);

	/* Update SLIC device configuration */
	slicDev = mvCtrlSysConfigGet(MV_CONFIG_SLIC_TDM_DEVICE);
	if (slicDev == SLIC_EXTERNAL_ID && (ethComplex & MV_ETHCOMP_GE_MAC1_2_RGMII1))
		mvOsPrintf("%s: Error: board configuration conflict between MAC1 to RGMII-1, " \
				"and External TDM - using RGMII-1 (disabled External TDM)\n\n", __func__);

	mvBoardSlicUnitTypeSet(slicDev);

	/* Update MPP group types and values according to board configuration */
	mvBoardMppIdUpdate();

	/* specific initializations required only for DB-6660 */
	if (mvBoardIdGet() == DB_6660_ID) {
		/* Verify board config vs. SerDes actual setup (Common phy Selector) */
		mvBoardVerifySerdesCofig();

		/* Enable Super-Speed mode on Avanta DB boards (900Mah output) */
		mvBoardUsbSsEnSet(MV_TRUE);

		/* If needed, enable SFP0 TX for SGMII, for DB-6660 */
		if (ethComplex & MV_ETHCOMP_GE_MAC0_2_COMPHY_2)
			mvBoardSgmiiSfp0TxSet(MV_TRUE);
	}
}

/*******************************************************************************
* mvBoardVerifySerdesCofig - Verify board config vs. SerDes actual setup (Common phy Selector)
*
* DESCRIPTION:
	read board configuration for SerDes lanes 1-3,
	and compare it to SerDes common phy Selector (Actual SerDes config in use)
*
* INPUT:
*	None.
*
* OUTPUT:
*       None.
*
* RETURN:
*	None.
*
*******************************************************************************/
MV_VOID mvBoardVerifySerdesCofig(MV_VOID)
{
	MV_U32 laneConfig, laneSelector = MV_REG_READ(MV_COMMON_PHY_REGS_OFFSET);

	/* Lane1:
	 * PCIe0   : board config = 0x0, Selector  = 0x0
	 * SGMII-0 : board config = 0x2,  Selector  = 0x1
	 */
	laneConfig = mvCtrlSysConfigGet(MV_CONFIG_LANE1);
	if (!((laneConfig == 0x0 && ((laneSelector & BIT1) >> 1) == 0x0) ||
		(laneConfig == 0x2 && ((laneSelector & BIT1) >> 1) == 0x1)))
		mvOsPrintf("Error: board configuration conflicts with SerDes configuration\n" \
			"SerDes#1: Board configuration= %x  SerDes Selector = %x\n" \
			, laneConfig, (laneSelector & BIT1) >> 1);

	/* for lane2, board config value is reversed, compared to SerDes selector value:
	 * SATA0   : board config = 0x0, Selector = 0x1
	 * SGMII-0 : board config = 0x1, Selector = 0x0
	 */
	laneConfig = mvCtrlSysConfigGet(MV_CONFIG_LANE2);
	if (!laneConfig != (laneSelector & BIT2) >> 2)
		mvOsPrintf("Error: board configuration conflicts with SerDes configuration\n" \
			"SerDes#2: Board configuration= %x  SerDes Selector = %x\n" \
			, !laneConfig, (laneSelector & BIT2) >> 2);

	laneConfig = mvCtrlSysConfigGet(MV_CONFIG_LANE3);
	if (laneConfig != (laneSelector & BIT3) >> 3)
		mvOsPrintf("Error: board configuration conflicts with SerDes configuration\n" \
			"SerDes#1: Board configuration= %x  SerDes Selector = %x\n" \
			, laneConfig, (laneSelector & BIT3) >> 3);
}

/*******************************************************************************
* mvBoardMppIdUpdate - Update MPP ID's according to modules auto-detection.
*
* DESCRIPTION:
*	Update MPP ID's according to on-board modules as detected using TWSI bus.
*	Update board information for changed mpp values
	Must run AFTER mvBoardEthComplexInfoUpdate
*
* INPUT:
*	None.
*
* OUTPUT:
*       None.
*
* RETURN:
*	None.
*
*******************************************************************************/
MV_VOID mvBoardMppIdUpdate(MV_VOID)
{
	MV_BOARD_BOOT_SRC bootDev;
	MV_SLIC_UNIT_TYPE slicDev;
	MV_U32 ethComplexOptions = mvBoardEthComplexConfigGet();

	/* MPP Groups initialization : */
	/* Set Group 0-1 - Boot device (else if booting from SPI1: Set Groups 3-4) */
	bootDev = mvBoardBootDeviceGroupSet();

	/* Group 2 - SLIC Tdm unit */
	slicDev = mvBoardSlicUnitTypeGet();
	mvBoardMppTypeSet(2, slicDev);

	/* Groups 3-4  - (only if not Booting from SPI1)*/
	if (bootDev != MSAR_0_BOOT_SPI1_FLASH) {
		if (ethComplexOptions & MV_ETHCOMP_GE_MAC1_2_RGMII1) {
			mvBoardMppTypeSet(3, GE1_UNIT);
			if (slicDev == SLIC_LANTIQ_ID)
				mvBoardMppTypeSet(4, GE1_CPU_SMI_CTRL_TDM_LQ_UNIT);
			else /* REF_CLK_OUT */
				mvBoardMppTypeSet(4, GE1_CPU_SMI_CTRL_REF_CLK_OUT);
		} else { /* if RGMII-1 isn't used, set SPI1 MPP's */
			mvBoardMppTypeSet(3, SDIO_SPI1_UNIT);
			if (slicDev == SLIC_LANTIQ_ID)
				mvBoardMppTypeSet(4, SPI1_CPU_SMI_CTRL_TDM_LQ_UNIT);
			else /* REF_CLK_OUT */
				mvBoardMppTypeSet(4, SPI1_CPU_SMI_CTRL_REF_CLK_OUT);
		}
	}

	/* Groups 5-6-7 Set GE0 or Switch port 4 */
	if (ethComplexOptions & MV_ETHCOMP_GE_MAC0_2_RGMII0) {
		mvBoardMppTypeSet(5, GE0_UNIT_PON_TX_FAULT);
		mvBoardMppTypeSet(6, GE0_UNIT);
		if (mvCtrlGetCpuNum() == 0) /* if using Dual CPU ,set UART1 */
			mvBoardMppTypeSet(7, GE0_UNIT_LED_MATRIX);
		else
			mvBoardMppTypeSet(7, GE0_UNIT_UA1_PTP);
	} else if (ethComplexOptions & MV_ETHCOMP_SW_P4_2_RGMII0) {
		mvBoardMppTypeSet(5, SWITCH_P4_PON_TX_FAULT);
		mvBoardMppTypeSet(6, SWITCH_P4);
		if (mvCtrlGetCpuNum() == 0) /* if using Dual CPU ,set UART1 */
			mvBoardMppTypeSet(7, SWITCH_P4_LED_MATRIX);
		else
			mvBoardMppTypeSet(7, SWITCH_P4_UA1_PTP);
	}
}

/*******************************************************************************
* mvBoardEthComplexInfoUpdate
*
* DESCRIPTION:
*	Update etherntComplex configuration,
*	according to modules detection (S@R & board configuration)
*
** INPUT:
*	None.
*
* OUTPUT:
*	None.
*
* RETURN:
*	MV_OK - on success,
*	MV_ERROR - On failure.
*
*******************************************************************************/
MV_STATUS mvBoardEthComplexInfoUpdate(MV_VOID)
{
	MV_U32 ethComplexOptions = 0x0;
	MV_ETH_COMPLEX_TOPOLOGY mac0Config, mac1Config;

	/* Ethernet Complex initialization : */
	/* MAC0/1 */
	mac0Config = mvBoardMac0ConfigGet();
	mac1Config = mvBoardMac1ConfigGet();
	if (mac0Config == MV_ERROR || mac1Config == MV_ERROR) {
		mvOsPrintf("%s: Error: Using default configuration.\n\n", __func__);
		return MV_ERROR;
	}
	ethComplexOptions |= mac0Config | mac1Config;

	/* read if using 2G speed for MAC0 to Switch*/
	if (mvCtrlSysConfigGet(MV_CONFIG_MAC0_SW_SPEED) == 0x0)
		ethComplexOptions |= MV_ETHCOMP_P2P_MAC0_2_SW_SPEED_2G;

	/* if MAC1 is NOT connected to PON SerDes --> connect PON MAC to to PON SerDes */
	if ((ethComplexOptions & MV_ETHCOMP_GE_MAC1_2_PON_ETH_SERDES) == MV_FALSE)
		ethComplexOptions |= MV_ETHCOMP_P2P_MAC_2_PON_ETH_SERDES;

	/* Switch Ports*/
	if ((ethComplexOptions & MV_ETHCOMP_GE_MAC0_2_SW_P6) ||
	    (ethComplexOptions & MV_ETHCOMP_GE_MAC1_2_SW_P4)) {
		/* if MAC0 is NOT connected to GE_PHY_P0 --> connect Switch port 0 to QUAD_PHY_P0 */
		if ((ethComplexOptions & MV_ETHCOMP_GE_MAC0_2_GE_PHY_P0) == MV_FALSE)
			ethComplexOptions |= MV_ETHCOMP_SW_P0_2_GE_PHY_P0;

		/* if MAC1 is BOT connected to GE_PHY_P3 --> connect Switch port 3 to QUAD_PHY_P3 */
		if ((ethComplexOptions & MV_ETHCOMP_GE_MAC1_2_GE_PHY_P3) == MV_FALSE)
			ethComplexOptions |= MV_ETHCOMP_SW_P3_2_GE_PHY_P3;

		/* connect Switch ports 2/3 to QUAD_PHY_P2/3 */
		ethComplexOptions |= (MV_ETHCOMP_SW_P1_2_GE_PHY_P1 | MV_ETHCOMP_SW_P2_2_GE_PHY_P2);
	}

	mvBoardEthComplexConfigSet(ethComplexOptions);
	return MV_OK;
}

/*******************************************************************************
* mvBoardBootDeviceGroupSet - test board Boot configuration and set MPP groups
*
* DESCRIPTION:
*   read board BOOT configuration and set MPP groups accordingly
*	-  Sets groups 0-1 for NAND or SPI0 Boot
*	   Or   groups 3-4 for SPI1 Boot
*	- return Selected boot device
*
* INPUT:
*
* OUTPUT:  None.
*
* RETURN:
*       the selected MV_BOARD_BOOT_SRC
*
*******************************************************************************/
MV_BOARD_BOOT_SRC mvBoardBootDeviceGroupSet()
{
	MV_U32 groupType;
	MV_BOARD_BOOT_SRC bootSrc = mvBoardBootDeviceGet();

	switch (bootSrc) {
	case MSAR_0_BOOT_NAND_NEW:
		mvBoardMppTypeSet(0, NAND_BOOT_V2);
		mvBoardMppTypeSet(1, NAND_BOOT_V2);
		break;
	case MSAR_0_BOOT_SPI_FLASH:
		/* read SPDIF_AUDIO board configuration for DB-6660 board */
		if ((mvBoardIdGet() == DB_6660_ID) &&
			(mvCtrlSysConfigGet(MV_CONFIG_DEVICE_BUS_MODULE) == 0x2))	/* 0x2=SPDIF_AUDIO */
			groupType = SPI0_BOOT_SPDIF_AUDIO;
		else
			groupType = SPI0_BOOT;

		mvBoardMppTypeSet(0, groupType);
		mvBoardMppTypeSet(1, groupType);
		break;
	case MSAR_0_BOOT_SPI1_FLASH:    /* MSAR_0_SPI1 - update Groups 3-4 */
		mvBoardMppTypeSet(3, SDIO_SPI1_UNIT);
		if ( mvBoardSlicUnitTypeGet() == SLIC_LANTIQ_ID)
			mvBoardMppTypeSet(4, SPI1_CPU_SMI_CTRL_TDM_LQ_UNIT);
		else    /*REF_CLK_OUT*/
			mvBoardMppTypeSet(4, SPI1_CPU_SMI_CTRL_REF_CLK_OUT);
		mvBoardMppTypeSet(0, SPI0_BOOT_SPDIF_AUDIO);
		mvBoardMppTypeSet(1, SPI0_BOOT_SPDIF_AUDIO);
		break;
	default:
		return MV_ERROR;
	}
	return bootSrc;
}

/*******************************************************************************
* mvBoardBootDeviceGet -   Get the Selected S@R boot device
*
* DESCRIPTION:
*   read board BOOT configuration from S@R and return Boot device accordingly
*
* INPUT:
*
* OUTPUT:  None.
*
* RETURN:
*       the selected MV_BOARD_BOOT_SRC
*
*******************************************************************************/
MV_BOARD_BOOT_SRC mvBoardBootDeviceGet()
{
	MV_U32 satrBootDeviceValue;
	MV_SATR_BOOT_TABLE satrTable[] = MV_SATR_TABLE_VAL;
	MV_SATR_BOOT_TABLE satrBootEntry;
	MV_BOARD_BOOT_SRC defaultBootSrc;

	/* prepare default boot source, in case of:
	 * 1. S@R read ERROR
	 * 2. Boot from UART is selected as boot source at S@R
	 *    Pre-compiled image type (SPI/NAND) is selected as boot source
	 */
#if defined(MV_SPI_BOOT)
	defaultBootSrc = MSAR_0_BOOT_SPI_FLASH;
	DB(mvOsPrintf("default boot source is SPI-0\n"));
#elif defined(MV_NAND_BOOT)
	defaultBootSrc = MSAR_0_BOOT_NAND_NEW;
	DB(mvOsPrintf("default boot source is NAND\n"));
#endif

	if (mvCtrlSatRRead(MV_SATR_BOOT_DEVICE, &satrBootDeviceValue) != MV_OK) {
		mvOsPrintf("%s: Error: failed to read boot source\n", __func__);
		mvOsPrintf("Using pre-compiled image type as boot source\n");
		return defaultBootSrc;
	}

	/* Get boot source entry from Satr boot table */
	satrBootEntry = satrTable[satrBootDeviceValue];

	/* If booting from UART, return pre-compiled boot source*/
	if (satrBootEntry.bootSrc == MSAR_0_BOOT_UART) {
		mvOsPrintf("\t** Booting from UART (restore DIP-switch to");
		mvOsPrintf(" requested boot source before reset!) **\n");
		return defaultBootSrc;
	}

	/* If not booting from SPI, return boot source*/
	if (satrBootEntry.bootSrc != MSAR_0_BOOT_SPI_FLASH)
		return satrBootEntry.bootSrc;

	/* if booting from SPI ,verify which CS (0/1) */
	if (mvBoardBootAttrGet(satrBootDeviceValue, 1) == MSAR_0_SPI0)
		return MSAR_0_BOOT_SPI_FLASH;
	else
		return MSAR_0_BOOT_SPI1_FLASH;
}

/*******************************************************************************
* mvBoardBootAttrGet -  Get the selected S@R Boot device attributes[1/2/3]
*
* DESCRIPTION:
*   read board BOOT configuration and return attributes accordingly
*
* INPUT:  satrBootDevice - BOOT_DEVICE value from S@R.*
*         attrNum - attribute number [1/2/3]
* OUTPUT:  None.
*
* RETURN:
*       the selected attribute value
*
*******************************************************************************/
MV_U32 mvBoardBootAttrGet(MV_U32 satrBootDeviceValue, MV_U8 attrNum)
{
	MV_SATR_BOOT_TABLE satrTable[] = MV_SATR_TABLE_VAL;
	MV_SATR_BOOT_TABLE satrBootEntry = satrTable[satrBootDeviceValue];

	switch (attrNum) {
	case 1:
		return satrBootEntry.attr1;
		break;
	case 2:
		return satrBootEntry.attr2;
		break;
	case 3:
		return satrBootEntry.attr3;
		break;
	default:
		return MV_ERROR;
	}
}

/*******************************************************************************
* mvBoardMac0ConfigGet - test board configuration and return the correct MAC0 config
*
* DESCRIPTION:
*	test board configuration regarding MAC0
*	if configured to SGMII-0 , will check which lane is configured to SGMII,
*	and return its MV_ETH_COMPLEX_TOPOLOGY define
*	else return error
*
* INPUT:  None.
*
* OUTPUT:  None.
*
* RETURN:
*       if configured correct, the MV_ETH_COMPLEX_TOPOLOGY define, else MV_ERROR
*
*******************************************************************************/
MV_ETH_COMPLEX_TOPOLOGY mvBoardMac0ConfigGet()
{
	MV_ETH_COMPLEX_TOPOLOGY sgmiiLane;
	MV_STATUS isSgmiiLaneEnabled;
	MV_U32 mac0Config = mvCtrlSysConfigGet(MV_CONFIG_MAC0);

	if (mac0Config == MV_ERROR) {
		mvOsPrintf("%s: Error: failed reading MAC0 connection board configuration" \
				"\n", __func__);
		return MV_ERROR;
	}

	/*
	 * if a Serdes lane is configured to SGMII, and conflicts MAC0 settings,
	 * then the selected Serdes lane will be used (overriding MAC0 settings)
	 */
	isSgmiiLaneEnabled = mvBoardLaneSGMIIGet(&sgmiiLane);
	if (isSgmiiLaneEnabled == MV_TRUE && mac0Config != MAC0_2_SGMII) {
		mvOsPrintf("%s: Error: Board configuration conflict for MAC0 connection" \
				" and Serdes Lanes.\n\n", __func__);
		return sgmiiLane;
	}

	switch (mac0Config) {
	case MAC0_2_SW_P6:
		return MV_ETHCOMP_GE_MAC0_2_SW_P6;
	case MAC0_2_GE_PHY_P0:
		return MV_ETHCOMP_GE_MAC0_2_GE_PHY_P0;
	case MAC0_2_RGMII:
		return MV_ETHCOMP_GE_MAC0_2_RGMII0;
	case MAC0_2_SGMII:
		if (isSgmiiLaneEnabled == MV_TRUE)
			return sgmiiLane;
	}

	/* if MAC0 set to SGMII, but no Serdes lane selected, --> ERROR (use defaults) */
	mvOsPrintf("%s: Error: Configuration conflict for MAC0 connection.\n", __func__);
	return MV_ERROR;

}

/*******************************************************************************
* mvBoardMac1ConfigGet - test board configuration and return the correct MAC1 config
*
* DESCRIPTION:
*	test board configuration regarding PON_SERDES
*	if MAC0 is configured to PON SerDes Connection return its MV_ETH_COMPLEX_TOPOLOGY define
*	else test MV_CONFIG_MAC1 configuration
*
* INPUT:  None.
*
* OUTPUT:  None.
*
* RETURN:
*       if configured correct, the MV_ETH_COMPLEX_TOPOLOGY define, else MV_ERROR
*
*******************************************************************************/
MV_ETH_COMPLEX_TOPOLOGY mvBoardMac1ConfigGet()
{
	if (mvCtrlSysConfigGet(MV_CONFIG_PON_SERDES) == 0x1)
		return MV_ETHCOMP_GE_MAC1_2_PON_ETH_SERDES;
	/* else Scan MAC1 config to decide its connection */
	switch (mvCtrlSysConfigGet(MV_CONFIG_MAC1)) {
	case 0x0:
		return MV_ETHCOMP_GE_MAC1_2_RGMII1;
		break;
	case 0x1:
		return MV_ETHCOMP_GE_MAC1_2_SW_P4;
		break;
	case 0x2:
		return MV_ETHCOMP_GE_MAC1_2_GE_PHY_P3;
		break;
	default:
		mvOsPrintf("%s: Error: Configuration conflict for MAC1 connection.\n", __func__);
		return MV_ERROR;
	}
}

/*******************************************************************************
* mvBoardIsLaneSGMII - check if a board lane is configured to SGMII-0
*
* DESCRIPTION:
*	test board configuration regarding lanes-1/2/3
*	if one of them is configured to SGMII-0 , will return its MV_ETH_COMPLEX_TOPOLOGY define
*	else return error
*
* INPUT:  None.
*
* OUTPUT:  None.
*
* RETURN:
*      MV_TRUE if SGMII settings are valid and Enabled only for a single lane
*
*******************************************************************************/
MV_BOOL mvBoardLaneSGMIIGet(MV_ETH_COMPLEX_TOPOLOGY *sgmiiConfig)
{
	MV_U32 laneConfig = 0;
	MV_BOOL isSgmiiLaneEnabled = MV_FALSE;

	if (sgmiiConfig == NULL) {
		mvOsPrintf("%s: Error: NULL pointer parameter\n", __func__);
		return MV_FALSE;
	}

	/* Serdes lane board configuration is only for DB-6660  */
	if (mvBoardIdGet() != DB_6660_ID)
		return MV_FALSE;

	/* Lane 1 */
	if (mvCtrlSysConfigGet(MV_CONFIG_LANE1) == 0x1) {
		laneConfig = MV_ETHCOMP_GE_MAC0_2_COMPHY_1;
		isSgmiiLaneEnabled = MV_TRUE;
	}
	/* Lane 2 */
	if (mvCtrlSysConfigGet(MV_CONFIG_LANE2) == 0x1) {
		if (isSgmiiLaneEnabled == MV_FALSE) {
			laneConfig = MV_ETHCOMP_GE_MAC0_2_COMPHY_2;
			isSgmiiLaneEnabled = MV_TRUE;
		} else /* SGMII was already set in the previous lanes */
			laneConfig = -1;
	}
	/* Lane 3 */
	if (mvCtrlSysConfigGet(MV_CONFIG_LANE3) == 0x1) {
		if (isSgmiiLaneEnabled == MV_FALSE) {
			laneConfig = MV_ETHCOMP_GE_MAC0_2_COMPHY_3;
		} else /* SGMII was already set in the previous lanes */
			laneConfig = -1;
	}

	/* if no Serdes lanes are configured to SGMII */
	if (laneConfig == 0)
		return MV_FALSE;

	/* if more then 1 Serdes lanes are configured to SGMII */
	if (laneConfig == -1) {
		mvOsPrintf("%s: Error: Only one Serdes lanes can be configured to SGMII\n\n", __func__);
		return MV_FALSE;
	}

	*sgmiiConfig = laneConfig;
	return MV_TRUE;
}

/*******************************************************************************
* mvBoardIsInternalSwitchConnected
*
* DESCRIPTION:
*       This routine returns port's connection status
*
* INPUT:
*       ethPortNum - Ethernet port number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       1 - if ethPortNum is connected to switch, 0 otherwise
*
*******************************************************************************/
MV_STATUS mvBoardIsInternalSwitchConnected(void)
{
	MV_U32 ethComplex = mvBoardEthComplexConfigGet();

	if ((ethComplex & MV_ETHCOMP_GE_MAC0_2_SW_P6) ||
	    (ethComplex & MV_ETHCOMP_GE_MAC1_2_SW_P4))
		return MV_TRUE;
	else
		return MV_FALSE;
}

/*******************************************************************************
* mvBoardSwitchConnectedPortGet -
*
* DESCRIPTION:
*       This routine returns the switch port connected to the ethPort
*
* INPUT:
*       ethPortNum - Ethernet port number.
*
* OUTPUT:
*       None.
*
* RETURN:
*	switch port connected to the ethPort
*
*******************************************************************************/
MV_32 mvBoardSwitchConnectedPortGet(MV_U32 ethPort)
{
	MV_U32 ethComplex = mvBoardEthComplexConfigGet();

	if (ethPort >= board->numBoardMacInfo) {
		mvOsPrintf("%s: Error: Illegal port number(%u)\n", __func__, ethPort);
		return MV_FALSE;
	}

	if ((ethPort == 0) && (ethComplex & MV_ETHCOMP_GE_MAC0_2_SW_P6))
		return 6;
	else if ((ethPort == 1) && (ethComplex & MV_ETHCOMP_GE_MAC1_2_SW_P4))
		return 4;
	else
		return -1;

}

/*******************************************************************************
* mvBoardSwitchPortsMaskGet -
*
* DESCRIPTION:
*       This routine returns a mask describing all the connected switch ports
*
* INPUT:
*       switchIdx - index of the switch. Only 0 is supported.
*
* OUTPUT:
*       None.
*
* RETURN:
*
*******************************************************************************/
MV_U32 mvBoardSwitchPortsMaskGet(MV_U32 switchIdx)
{
	MV_U32 mask = 0, c = mvBoardEthComplexConfigGet();

	if (c & MV_ETHCOMP_SW_P0_2_GE_PHY_P0)
		mask |= BIT0;
	if (c & MV_ETHCOMP_SW_P1_2_GE_PHY_P1)
		mask |= BIT1;
	if (c & MV_ETHCOMP_SW_P2_2_GE_PHY_P2)
		mask |= BIT2;
	if (c & MV_ETHCOMP_SW_P3_2_GE_PHY_P3)
		mask |= BIT3;
	if ((c & MV_ETHCOMP_SW_P4_2_RGMII0) || (c & MV_ETHCOMP_GE_MAC1_2_SW_P4))
		mask |= BIT4;
	if (c & MV_ETHCOMP_GE_MAC0_2_SW_P6)
		mask |= BIT6;

	return mask;
}

/*******************************************************************************
* mvBoardSwitchPortForceLinkGet
*
* DESCRIPTION:
*       Return the switch ports force link bitmask.
*
* INPUT:
*       switchIdx - index of the switch. Only 0 is supported.
*
* OUTPUT:
*       None.
*
* RETURN:
*       the ports bitmask, -1 if the switch is not connected.
*
*******************************************************************************/
MV_U32 mvBoardSwitchPortForceLinkGet(MV_U32 switchIdx)
{
	MV_U32 mask = 0, c = mvBoardEthComplexConfigGet();

	if (c & MV_ETHCOMP_GE_MAC1_2_SW_P4)
		mask |= BIT4;
	if (c & MV_ETHCOMP_GE_MAC0_2_SW_P6)
		mask |= BIT6;

	/* If switch port 4 is connected to RGMII-0, the PHY SMI is controlled by CPU MAC,
	 * To avoid address conflict between internal PHY (0x1), SMI is not handled by switch.
	 * Auto-negotiation can not be applied with this configuration - so we use force link */
	if (c & MV_ETHCOMP_SW_P4_2_RGMII0)
		mask |= BIT4;
	return mask;

}

/*******************************************************************************
* mvBoardFreqModesNumGet
*
* DESCRIPTION: Return the number of supported frequency modes for this SoC
*
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      Number of supported frequency modes
*
*******************************************************************************/
MV_U32 mvBoardFreqModesNumGet()
{
	MV_U32 freqNum;

	switch (mvCtrlModelGet()) {
	case MV_6610_DEV_ID:
		freqNum = FREQ_MODES_NUM_6610;
		break;
	case MV_6650_DEV_ID:
		freqNum = FREQ_MODES_NUM_6650;
		break;
	case MV_6660_DEV_ID:
		freqNum = FREQ_MODES_NUM_6660;
		break;
	default:
		mvOsPrintf("%s: Error: failed to read ctrlModel (SoC ID)\n", __func__);
		return MV_ERROR;
	}

	return freqNum;
}


/*******************************************************************************
* mvBoardConfigWrite - write MPP's config and Board general environment configuration
*
* DESCRIPTION:
*       This function writes the environment information that was initialized
*       by mvBoardConfigInit, such as MPP settings, Mux configuration,
*       and Board specific initializations.
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
MV_VOID mvBoardConfigWrite(void)
{
	MV_U32 mppGroup, i, reg;
	MV_BOARD_SPEC_INIT *boardSpec;

	for (mppGroup = 0; mppGroup < MV_MPP_MAX_GROUP; mppGroup++) {
		MV_REG_WRITE(mvCtrlMppRegGet(mppGroup), mvBoardMppGet(mppGroup));
	}

	boardSpec = mvBoardSpecInitGet();
	if (boardSpec != NULL) {
		i = 0;
		while (boardSpec[i].reg != TBL_TERM) {
			reg = MV_REG_READ(boardSpec[i].reg);
			reg &= ~boardSpec[i].mask;
			reg |= (boardSpec[i].val & boardSpec[i].mask);
			MV_REG_WRITE(boardSpec[i].reg, reg);
			i++;
		}
	}
}

/*******************************************************************************
* mvBoardTdmSpiModeGet - return SLIC/DAA connection
*
* DESCRIPTION:
*
* INPUT:
*
* OUTPUT:
*       None.
*
* RETURN:
*
*******************************************************************************/
MV_32 mvBoardTdmSpiModeGet(MV_VOID)
{
	return 0;
}

/*******************************************************************************
* mvBoardTdmDevicesCountGet
*
* DESCRIPTION:
*	Return the number of TDM devices on board.
*
* INPUT:
*	None.
*
* OUTPUT:
*       None.
*
* RETURN:
*	Number of devices.
*
*******************************************************************************/
MV_U8 mvBoardTdmDevicesCountGet(void)
{
	MV_16 index = board->boardTdmInfoIndex;

	if (index == -1)
		return 0;

	return board->numBoardTdmInfo[index];
}

/*******************************************************************************
* mvBoardTdmSpiCsGet
*
* DESCRIPTION:
*	Return the SPI Chip-select number for a given device.
*
* INPUT:
*	devId	- The Slic device ID to get the SPI CS for.
*
* OUTPUT:
*       None.
*
* RETURN:
*	The SPI CS if found, -1 otherwise.
*
*******************************************************************************/
MV_U8 mvBoardTdmSpiCsGet(MV_U8 devId)
{
	MV_16 index;

	index = board->boardTdmInfoIndex;
	if (index == -1)
		return 0;

	if (devId >= board->numBoardTdmInfo[index])
		return -1;

	return board->pBoardTdmInt2CsInfo[index][devId].spiCs;
}

/*******************************************************************************
* mvBoardTdmSpiIdGet
*
* DESCRIPTION:
*	Return SPI port ID per board.
*
* INPUT:
*	None
*
* OUTPUT:
*       None.
*
* RETURN:
*	SPI port ID.
*
*******************************************************************************/
MV_U8 mvBoardTdmSpiIdGet(MV_VOID)
{
	MV_16 index;

	index = board->boardTdmInfoIndex;
	if (index == -1)
		return 0;

	return board->pBoardTdmSpiInfo[0].spiId;
}

/*******************************************************************************
* mvBoardConfigurationPrint
*
* DESCRIPTION:
*	Print on-board detected modules.
*
* INPUT:
*	None.
*
* OUTPUT:
*       None.
*
* RETURN:
*	None.
*
*******************************************************************************/
MV_VOID mvBoardConfigurationPrint(MV_VOID)
{
	char *lane1[] = {"PCIe1", "SGMII-0", "SATA-1", "Invalid Configuration" };
	char *tdmSlic[] = {"None", "SSI", "ISI", "ZSI", "TDM"};
	MV_U32 slicDevice, laneSelector, ethConfig = mvBoardEthComplexConfigGet();

	mvOsOutput("\nBoard configuration:\n");

	/* Mac configuration */
	if (ethConfig & MV_ETHCOMP_GE_MAC0_2_COMPHY_1)
		mvOsOutput("       SGMII0 on MAC0 [Lane1]\n");
	if (ethConfig & MV_ETHCOMP_GE_MAC0_2_COMPHY_2)
		mvOsOutput("       SGMII0 on MAC0 [Lane2]\n");
	if (ethConfig & MV_ETHCOMP_GE_MAC0_2_COMPHY_3)
		mvOsOutput("       SGMII0 on MAC0 [Lane3]\n");


	/* Switch configuration */
	if (ethConfig & MV_ETHCOMP_GE_MAC0_2_SW_P6)
		mvOsOutput("       Ethernet Switch port 6 on MAC0, %s Speed [Default]\n"
				, (ethConfig & MV_ETHCOMP_P2P_MAC0_2_SW_SPEED_2G) ? "2G" : "1G");
	else if ((ethConfig & MV_ETHCOMP_GE_MAC1_2_SW_P4) &&
		!(ethConfig & MV_ETHCOMP_GE_MAC0_2_SW_P6))
		mvOsOutput("       Ethernet Switch port 4 on MAC1, 1G Speed [Default]\n");
	if ((ethConfig & MV_ETHCOMP_GE_MAC0_2_SW_P6) &&
		(ethConfig & MV_ETHCOMP_GE_MAC1_2_SW_P4)) {
		mvOsOutput("       Ethernet Switch port 4 on MAC1, 1G Speed\n");
	}

	/* RGMII */
	if (ethConfig & MV_ETHCOMP_GE_MAC0_2_RGMII0)
		mvOsOutput("       RGMII0 Module on MAC0\n");
	if (ethConfig & MV_ETHCOMP_GE_MAC1_2_RGMII1)
		mvOsOutput("       RGMII1 on MAC1\n");
	if (ethConfig & MV_ETHCOMP_SW_P4_2_RGMII0)
		mvOsOutput("       RGMII0 Module on Switch port #4, 1G speed\n");

	/* Internal GE Quad Phy */
	if (ethConfig & MV_ETHCOMP_GE_MAC0_2_GE_PHY_P0)
			mvOsOutput("       GE-PHY-0 on MAC0\n");
	if (ethConfig & MV_ETHCOMP_GE_MAC1_2_GE_PHY_P3)
			mvOsOutput("       GE-PHY-3 on MAC1\n");
	if ((ethConfig & MV_ETHCOMP_SW_P0_2_GE_PHY_P0) && (ethConfig & MV_ETHCOMP_SW_P1_2_GE_PHY_P1)
		&& (ethConfig & MV_ETHCOMP_SW_P2_2_GE_PHY_P2) && (ethConfig & MV_ETHCOMP_SW_P3_2_GE_PHY_P3))
			mvOsOutput("       4xGE-PHY Module on 4 Switch ports\n");
	else {
		if (ethConfig & MV_ETHCOMP_SW_P0_2_GE_PHY_P0)
			mvOsOutput("       GE-PHY-0 Module on Switch port #0\n");
		if (ethConfig & MV_ETHCOMP_SW_P1_2_GE_PHY_P1)
			mvOsOutput("       GE-PHY-1 Module on Switch port #1\n");
		if (ethConfig & MV_ETHCOMP_SW_P2_2_GE_PHY_P2)
			mvOsOutput("       GE-PHY-2 Module on Switch port #2\n");
		if (ethConfig & MV_ETHCOMP_SW_P3_2_GE_PHY_P3)
			mvOsOutput("       GE-PHY-3 Module on Switch port #3\n");
	}

	/* TDM / Slic configuration */
	slicDevice = mvBoardSlicUnitTypeGet();
	if (slicDevice < MV_BOARD_SLIC_MAX_OPTION) /* 4 supported configurations */
		mvOsOutput("       TDM/SLIC: %s\n", tdmSlic[slicDevice]);
	else
		mvOsOutput("       TDM/SLIC: Unsupported configuration\n");

	/* SERDES Lanes*/
	mvOsOutput("\nSERDES configuration:\n");
	mvOsOutput("       Lane #0: PCIe0\n");	/* Lane 0 is always PCIe0 */

	/* SERDES lanes #1,#2,#3 are relevant only to MV88F6660 SoC*/
	if (mvCtrlModelGet() != MV_6660_DEV_ID)
		return;

	/* Read Common Phy selectors to determine SerDes configuration */
	laneSelector = MV_REG_READ(MV_COMMON_PHY_REGS_OFFSET);
	mvOsOutput("       Lane #1: %s\n", lane1[(laneSelector & BIT1) >> 1]);
	mvOsOutput("       Lane #2: %s\n", (laneSelector & BIT2) >> 2 ? "SATA-0" : "SGMII-0");
	mvOsOutput("       Lane #3: %s\n", (laneSelector & BIT3) >> 3 ? "SGMII-0" : "USB3");
}

/*******************************************************************************
* mvBoardIsGbEPortConnected
*
* DESCRIPTION:
*	Checks if a given GbE port is actually connected to the GE-PHY, internal Switch or any RGMII module.
*
* INPUT:
*	port - GbE port number (0 or 1).
*
* OUTPUT:
*       None.
*
* RETURN:
*	MV_TRUE if port is connected, MV_FALSE otherwise.
*
*******************************************************************************/
MV_BOOL mvBoardIsGbEPortConnected(MV_U32 ethPortNum)
{
	return MV_FALSE;
}

/* Board devices API managments */

/*******************************************************************************
* mvBoardGetDeviceNumber - Get number of device of some type on the board
*
* DESCRIPTION:
*
* INPUT:
*		devType - The device type ( Flash,RTC , etc .. )
*
* OUTPUT:
*       None.
*
* RETURN:
*       If the device is found on the board the then the functions returns the
*		number of those devices else the function returns 0
*
*
*******************************************************************************/
MV_32 mvBoardGetDevicesNumber(MV_BOARD_DEV_CLASS devClass)
{
	MV_U32 foundIndex = 0, devNum;

	for (devNum = START_DEV_CS; devNum < board->numBoardDeviceIf; devNum++)
		if (board->pDevCsInfo[devNum].devClass == devClass)
			foundIndex++;

	return foundIndex;
}

/*******************************************************************************
* mvBoardGetDeviceBaseAddr - Get base address of a device existing on the board
*
* DESCRIPTION:
*
* INPUT:
*       devIndex - The device sequential number on the board
*		devType - The device type ( Flash,RTC , etc .. )
*
* OUTPUT:
*       None.
*
* RETURN:
*       If the device is found on the board the then the functions returns the
*	Base address else the function returns 0xffffffff
*
*
*******************************************************************************/
MV_32 mvBoardGetDeviceBaseAddr(MV_32 devNum, MV_BOARD_DEV_CLASS devClass)
{
	MV_DEV_CS_INFO *devEntry = boardGetDevEntry(devNum, devClass);

	if (devEntry)
		return mvCpuIfTargetWinBaseLowGet(DEV_TO_TARGET(devEntry->deviceCS));

	return 0xFFFFFFFF;
}

/*******************************************************************************
* mvBoardGetDeviceBusWidth - Get Bus width of a device existing on the board
*
* DESCRIPTION:
*
* INPUT:
*       devIndex - The device sequential number on the board
*		devType - The device type ( Flash,RTC , etc .. )
*
* OUTPUT:
*       None.
*
* RETURN:
*       If the device is found on the board the then the functions returns the
*		Bus width else the function returns 0xffffffff
*
*
*******************************************************************************/
MV_32 mvBoardGetDeviceBusWidth(MV_32 devNum, MV_BOARD_DEV_CLASS devClass)
{
	MV_DEV_CS_INFO *devEntry = boardGetDevEntry(devNum, devClass);

	if (devEntry)
		return devEntry->busWidth;

	return 0xFFFFFFFF;
}

/*******************************************************************************
* mvBoardGetDeviceWinSize - Get the window size of a device existing on the board
*
* DESCRIPTION:
*
* INPUT:
*       devIndex - The device sequential number on the board
*		devType - The device type ( Flash,RTC , etc .. )
*
* OUTPUT:
*       None.
*
* RETURN:
*       If the device is found on the board the then the functions returns the
*		window size else the function returns 0xffffffff
*
*
*******************************************************************************/
MV_32 mvBoardGetDeviceWinSize(MV_32 devNum, MV_BOARD_DEV_CLASS devClass)
{
	MV_DEV_CS_INFO *devEntry = boardGetDevEntry(devNum, devClass);

	if (devEntry)
		return mvCpuIfTargetWinSizeGet(DEV_TO_TARGET(devEntry->deviceCS));

	return 0xFFFFFFFF;
}

/*******************************************************************************
* boardGetDevEntry - returns the entry pointer of a device on the board
*
* DESCRIPTION:
*
* INPUT:
*	devIndex - The device sequential number on the board
*	devType - The device type ( Flash,RTC , etc .. )
*
* OUTPUT:
*	None.
*
* RETURN:
*	If the device is found on the board the then the functions returns the
*	dev number else the function returns 0x0
*
*******************************************************************************/
static MV_DEV_CS_INFO *boardGetDevEntry(MV_32 devNum, MV_BOARD_DEV_CLASS devClass)
{
	MV_U32 foundIndex = 0, devIndex;

	for (devIndex = START_DEV_CS; devIndex < board->numBoardDeviceIf; devIndex++) {
		if (board->pDevCsInfo[devIndex].devClass == devClass) {
			if (foundIndex == devNum)
				return &(board->pDevCsInfo[devIndex]);
			foundIndex++;
		}
	}

	/* device not found */
	return NULL;
}

/*******************************************************************************
* boardGetDevCSNum
*
* DESCRIPTION:
*	Return the device's chip-select number.
*
* INPUT:
*	devIndex - The device sequential number on the board
*	devType - The device type ( Flash,RTC , etc .. )
*
* OUTPUT:
*	None.
*
* RETURN:
*	If the device is found on the board the then the functions returns the
*	dev number else the function returns 0x0
*
*******************************************************************************/
MV_U32 boardGetDevCSNum(MV_32 devNum, MV_BOARD_DEV_CLASS devClass)
{
	MV_DEV_CS_INFO *devEntry = boardGetDevEntry(devNum, devClass);

	if (devEntry)
		return devEntry->deviceCS;

	return 0xFFFFFFFF;
}

/*******************************************************************************
* mvBoardIoExpValGet - read a specified value from IO Expanders
*
* DESCRIPTION:
*       This function returns specified value from IO Expanders
*
* INPUT:
*       ioInfo  - relevant IO Expander information
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_U8  :return requested value , if TWSI read was succesfull, else 0xFF.
*
*******************************************************************************/
MV_U8 mvBoardIoExpValGet(MV_BOARD_IO_EXPANDER_TYPE_INFO *ioInfo)
{
	MV_U8 val, mask;

	if (ioInfo==NULL)
		return (MV_U8)MV_ERROR;

	if (mvBoardTwsiGet(BOARD_DEV_TWSI_IO_EXPANDER, ioInfo->expanderNum, ioInfo->regNum, &val) != MV_OK) {
		mvOsPrintf("%s: Error: Read from IO Expander at 0x%x failed\n", __func__
			   , mvBoardTwsiAddrGet(BOARD_DEV_TWSI_IO_EXPANDER, ioInfo->expanderNum));
		return (MV_U8)MV_ERROR;
	}

	mask = (1 << ioInfo->offset);
	return (val & mask) >> ioInfo->offset;
}

/*******************************************************************************
* mvBoardIoExpValSet - write a specified value to IO Expanders
*
* DESCRIPTION:
*       This function writes specified value to IO Expanders
*
* INPUT:
*       ioInfo  - relevant IO Expander information
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_U8  :return requested value , if TWSI read was succesfull, else 0xFF.
*
*******************************************************************************/
MV_STATUS mvBoardIoExpValSet(MV_BOARD_IO_EXPANDER_TYPE_INFO *ioInfo, MV_U8 value)
{
	MV_U8 readVal, configVal;

	if (ioInfo == NULL) {
		mvOsPrintf("%s: Error: Write to IO Expander failed (invalid Expander info)\n", __func__);
		return MV_ERROR;
	}
	/* Read Value */
	if (mvBoardTwsiGet(BOARD_DEV_TWSI_IO_EXPANDER, ioInfo->expanderNum,
					ioInfo->regNum, &readVal) != MV_OK) {
		mvOsPrintf("%s: Error: Read from IO Expander failed\n", __func__);
		return MV_ERROR;
	}

	/* Read Configuration Value */
	if (mvBoardTwsiGet(BOARD_DEV_TWSI_IO_EXPANDER, ioInfo->expanderNum,
					ioInfo->regNum + 6, &configVal) != MV_OK) {
		mvOsPrintf("%s: Error: Read Configuration from IO Expander failed\n", __func__);
		return MV_ERROR;
	}

	/* Modify Configuration value to Enable write for requested bit */
	configVal &= ~(1 << ioInfo->offset);	/* clean bit of old value  */
	if (mvBoardTwsiSet(BOARD_DEV_TWSI_IO_EXPANDER, ioInfo->expanderNum,
					ioInfo->regNum + 6, configVal) != MV_OK) {
		mvOsPrintf("%s: Error: Enable Write to IO Expander at 0x%x failed\n", __func__
			   , mvBoardTwsiAddrGet(BOARD_DEV_TWSI_IO_EXPANDER, ioInfo->expanderNum));
		return MV_ERROR;
	}

	/* Modify */
	readVal &= ~(1 << ioInfo->offset);	/* clean bit of old value  */
	readVal |= (value << ioInfo->offset);

	/* Write */
	if (mvBoardTwsiSet(BOARD_DEV_TWSI_IO_EXPANDER, ioInfo->expanderNum,
					ioInfo->regNum + 2, readVal) != MV_OK) {
		mvOsPrintf("%s: Error: Write to IO Expander at 0x%x failed\n", __func__
			   , mvBoardTwsiAddrGet(BOARD_DEV_TWSI_IO_EXPANDER, ioInfo->expanderNum));
		return MV_ERROR;
	}

	return MV_OK;
}

/*******************************************************************************
* mvBoardTwsiAddrTypeGet
*
* DESCRIPTION:
*	Return the TWSI address type for a given twsi device class.
*
* INPUT:
*	twsiClass - The TWSI device to return the address type for.
*	index	  - The TWSI device index (Pass 0 in case of a single
*		    device)
*
* OUTPUT:
*       None.
*
* RETURN:
*	The TWSI address type.
*
*******************************************************************************/
MV_U8 mvBoardTwsiAddrTypeGet(MV_BOARD_TWSI_CLASS twsiClass, MV_U32 index)
{
	int i;
	MV_U32 indexFound = 0;

	for (i = 0; i < board->numBoardTwsiDev; i++) {
		if (board->pBoardTwsiDev[i].devClass == twsiClass) {
			if (indexFound == index)
				return board->pBoardTwsiDev[i].twsiDevAddrType;
			else
				indexFound++;
		}
	}
	DB(mvOsPrintf("%s: Error: read TWSI address type failed\n", __func__));
	return MV_ERROR;
}

/*******************************************************************************
* mvBoardTwsiAddrGet
*
* DESCRIPTION:
*	Return the TWSI address for a given twsi device class.
*
* INPUT:
*	twsiClass - The TWSI device to return the address type for.
*	index	  - The TWSI device index (Pass 0 in case of a single
*		    device)
*
* OUTPUT:
*       None.
*
* RETURN:
*	The TWSI address.
*
*******************************************************************************/
MV_U8 mvBoardTwsiAddrGet(MV_BOARD_TWSI_CLASS twsiClass, MV_U32 index)
{
	int i;

	for (i = 0; i < board->numBoardTwsiDev; i++) {
		if ((board->pBoardTwsiDev[i].devClass == twsiClass) \
				&& (board->pBoardTwsiDev[i].devClassId == index)){
			return board->pBoardTwsiDev[i].twsiDevAddr;
		}
	}

	return 0xFF;
}

/*******************************************************************************
* mvBoardEthComplexConfigGet - Return ethernet complex board configuration.
*
* DESCRIPTION:
*	Returns the ethernet / Sata complex configuration from the board spec
*	structure.
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       32bit value describing the ethernet complex config.
*
*******************************************************************************/
MV_U32 mvBoardEthComplexConfigGet(MV_VOID)
{
	return board->pBoardModTypeValue->ethSataComplexOpt;
}

/*******************************************************************************
* mvBoardEthComplexConfigSet - Set ethernet complex board configuration.
*
* DESCRIPTION:
*	Sets the ethernet / Sata complex configuration in the board spec
*	structure.
*
* INPUT:
*       ethConfig - 32bit value describing the ethernet complex config.
*
* OUTPUT:
*       None.
*
* RETURN:
*	None.
*
*******************************************************************************/
MV_VOID mvBoardEthComplexConfigSet(MV_U32 ethConfig)
{
	/* Set ethernet complex configuration. */
	board->pBoardModTypeValue->ethSataComplexOpt = ethConfig;
	return;
}

/*******************************************************************************
* mvBoardSatrInfoConfig
*
* DESCRIPTION:
*	Return the SAR fields information for a given SAR class.
*
* INPUT:
*	satrClass - The SATR field to return the information for.
*
* OUTPUT:
*       None.
*
* RETURN:
*	MV_BOARD_SATR_INFO struct with mask, offset and register number.
*
*******************************************************************************/
MV_STATUS mvBoardSatrInfoConfig(MV_SATR_TYPE_ID satrClass, MV_BOARD_SATR_INFO *satrInfo, MV_BOOL read)
{
	int i;
	MV_U32 boardId = mvBoardIdGet();

	/* verify existence of requested SATR type, and pull its data,
	 * if write sequence, check if field is writeable for running board */
	for (i = 0; i < MV_SATR_WRITE_MAX_OPTION ; i++)
		if (boardSatrInfo[i].satrId == satrClass) {

			/* if read sequence, or an authorized write sequence -> return true */
			if (read == MV_TRUE || boardSatrInfo[i].isWriteable[boardId]) {
				*satrInfo = boardSatrInfo[i];
				return MV_OK;
			}
			else
				return MV_ERROR;
		}
	DB(mvOsPrintf("%s: Error: requested MV_SATR_TYPE_ID was not found (%d)\n", __func__,satrClass));
	return MV_ERROR;
}

/*******************************************************************************
* mvBoardConfigTypeGet
*
* DESCRIPTION:
*	Return the Config type fields information for a given Config type class.
*
* INPUT:
*	configClass - The Config type field to return the information for.
*
* OUTPUT:
*       None.
*
* RETURN:
*	MV_BOARD_CONFIG_TYPE_INFO struct with mask, offset and register number.
*
*******************************************************************************/
MV_BOOL mvBoardConfigTypeGet(MV_CONFIG_TYPE_ID configClass, MV_BOARD_CONFIG_TYPE_INFO *configInfo)
{
	int i;
	MV_U32 boardId = mvBoardIdGet();

	/* verify existence of requested config type, pull its data,
	 * and check if field is relevant to current running board */
	for (i = 0; i < MV_CONFIG_TYPE_MAX_OPTION ; i++)
		if (boardConfigTypesInfo[i].configId == configClass) {
			*configInfo = boardConfigTypesInfo[i];
			if (boardConfigTypesInfo[i].isActiveForBoard[boardId])
				return MV_TRUE;
			else
				return MV_FALSE;
		}
	mvOsPrintf("%s: Error: requested MV_CONFIG_TYPE_ID was not found (%d)\n", __func__, configClass);
	return MV_FALSE;
}

/*******************************************************************************
* mvBoardIoExpanderTypeGet
*
* DESCRIPTION:
*	Return the Config type fields information for a given Config type class.
*
* INPUT:
*	configClass - The Config type field to return the information for.
*
* OUTPUT:
*       None.
*
* RETURN:
*	MV_BOARD_CONFIG_TYPE_INFO struct with mask, offset and register number.
*
*******************************************************************************/
MV_STATUS mvBoardIoExpanderTypeGet(MV_IO_EXPANDER_TYPE_ID ioClass,
		MV_BOARD_IO_EXPANDER_TYPE_INFO *ioInfo)
{
	MV_U32 i;

	MV_BOARD_IO_EXPANDER_TYPE_INFO ioe6660[] = MV_BOARD_IO_EXP_DB6660_INFO;
	MV_BOARD_IO_EXPANDER_TYPE_INFO ioe6650[] = MV_BOARD_IO_EXP_DB6650_INFO;
	MV_BOARD_IO_EXPANDER_TYPE_INFO *ioe;
	MV_U8 n = 0;

	switch (mvBoardIdGet()) {
	case DB_6650_ID:
		ioe = ioe6650;
		n = ARRSZ(ioe6650);
		break;
	case DB_6660_ID:
		ioe = ioe6660;
		n = ARRSZ(ioe6660);
		break;
	default:
		mvOsPrintf("%s: Error: IO Expander doesn't exists on board\n", __func__);
		return MV_ERROR;
	}

	/* verify existance of requested config type, pull its data */
	for (i = 0; i < n ; i++)
		if (ioe[i].ioFieldid == ioClass) {

			*ioInfo = ioe[i];
			return MV_OK;
		}

	mvOsPrintf("%s: Error: requested IO expander id was not found (%d)\n",
			__func__, ioClass);
	return MV_ERROR;
}

/*******************************************************************************
* mvBoardExtPhyBufferSelect - enable/disable buffer status
*
* DESCRIPTION:
*	This function enables/disables the buffer status.
*
* INPUT:
*	enable - Boolean to indicate requested status
*
* OUTPUT:
*	None.
*
* RETURN:
*	None.
*
*******************************************************************************/
MV_STATUS mvBoardExtPhyBufferSelect(MV_BOOL enable)
{
	MV_BOARD_IO_EXPANDER_TYPE_INFO ioInfo;
	if (mvBoardIoExpanderTypeGet(MV_IO_EXPANDER_EXT_PHY_SMI_EN, &ioInfo) != MV_OK) {
		mvOsPrintf("%s: Error: Write to IO expander failed (External Phy SMI Buffer select)\n", __func__);
		return MV_ERROR;
	}

	return mvBoardIoExpValSet(&ioInfo, (enable ? 0x0 : 0x1));
}

/*******************************************************************************
* mvBoardSgmiiSfp0TxSet - enable/disable SGMII_SFP0_TX_DISABLE status
*
* DESCRIPTION:
*	This function enables/disables the field status.
*
* INPUT:
*	enable - Boolean to indicate requested status
*
* OUTPUT:
*	None.
*
* RETURN:
*	None.
*
*******************************************************************************/
MV_STATUS mvBoardSgmiiSfp0TxSet(MV_BOOL enable)
{
	MV_BOARD_IO_EXPANDER_TYPE_INFO ioInfo;

	if (mvBoardIoExpanderTypeGet(MV_IO_EXPANDER_SFP0_TX_DIS, &ioInfo) != MV_OK) {
		mvOsPrintf("%s: Error: Write to IO expander failed (SFP0_TX_DIS)\n", __func__);
		return MV_ERROR;
	}

	return mvBoardIoExpValSet(&ioInfo, (enable ? 0x0 : 0x1));
}

/*******************************************************************************
* mvBoardUsbSsEnSet - enable/disable USB_SS_EN status
*
* DESCRIPTION:
*	This function enables/disables USB_SS_EN status.
*	USB_SS_EN = 1 --> Enable USB 3.0 900mA current limit
*
* INPUT:
*	enable - Boolean to indicate requested status
*
* OUTPUT:
*	None.
*
* RETURN:
*	None.
*
*******************************************************************************/
MV_STATUS mvBoardUsbSsEnSet(MV_BOOL enable)
{
	MV_BOARD_IO_EXPANDER_TYPE_INFO ioInfo;

	if (mvBoardIoExpanderTypeGet(MV_IO_EXPANDER_USB_SUPER_SPEED, &ioInfo) != MV_OK) {
		mvOsPrintf("%s: Error: Write to IO expander failed (USB_SS_EN)\n", __func__);
		return MV_ERROR;
	}

	return mvBoardIoExpValSet(&ioInfo, (enable ? 0x1 : 0x0));
}

/*******************************************************************************
* mvBoardIdSet - Set Board model
*
* DESCRIPTION:
*       This function sets the board ID.
*       Board ID is 32bit word constructed of board model (16bit) and
*       board revision (16bit) in the following way: 0xMMMMRRRR.
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       void
*
*******************************************************************************/
MV_VOID mvBoardIdSet(MV_U32 boardId)
{
	if (boardId >= MV_MAX_BOARD_ID)
		mvOsPrintf("%s: Error: wrong boardId (%d)\n", __func__, boardId);

	board = boardInfoTbl[boardId];
}

/*******************************************************************************
* mvBoardIdGet - Get Board model
*
* DESCRIPTION:
*       This function returns board ID.
*       Board ID is 32bit word constructed of board model (16bit) and
*       board revision (16bit) in the following way: 0xMMMMRRRR.
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       32bit board ID number, '-1' if board is undefined.
*
*******************************************************************************/
MV_U32 mvBoardIdGet(MV_VOID)
{
	MV_U32 boardId, value;

#ifdef CONFIG_MACH_AVANTA_LP_FPGA
	boardId = MV_BOARD_ID_AVANTA_LP_FPGA;
#else
	value = MV_REG_READ(MPP_SAMPLE_AT_RESET(1));
	boardId = ((value & (0xF0)) >> 4);
#endif
	if (boardId >= MV_MAX_BOARD_ID) {
		mvOsPrintf("%s: Error: read wrong board (%d)\n", __func__, boardId);
		return MV_INVALID_BOARD_ID;
	}
	return boardId;
}

/*******************************************************************************
* mvBoardTwsiGet -
*
* DESCRIPTION:
*
* INPUT:
*	device num - one of three devices
*	reg num - 0 or 1
*
* OUTPUT:
*		None.
*
* RETURN:
*		reg value
*
*******************************************************************************/
MV_STATUS mvBoardTwsiGet(MV_BOARD_TWSI_CLASS twsiClass, MV_U8 devNum, MV_U8 regNum, MV_U8 *pData)
{
	MV_TWSI_SLAVE twsiSlave;
	MV_TWSI_ADDR slave;
	MV_U8 data;

	/* TWSI init */
	slave.type = ADDR7_BIT;
	slave.address = 0;
	mvTwsiInit(0, TWSI_SPEED, mvBoardTclkGet(), &slave, 0);

	DB(mvOsPrintf("Board: TWSI Read device\n"));
	twsiSlave.slaveAddr.address = mvBoardTwsiAddrGet(twsiClass, devNum);
	twsiSlave.slaveAddr.type = mvBoardTwsiAddrTypeGet(twsiClass, devNum);

	twsiSlave.validOffset = MV_TRUE;
	/* Use offset as command */
	twsiSlave.offset = regNum;

	if (twsiClass == BOARD_DEV_TWSI_EEPROM)
		twsiSlave.moreThen256 = MV_TRUE;
	else
		twsiSlave.moreThen256 = MV_FALSE;

	if (MV_OK != mvTwsiRead(0, &twsiSlave, &data, 1)) {
		mvOsPrintf("%s: Twsi Read fail\n", __func__);
		return MV_ERROR;
	}
	DB(mvOsPrintf("Board: Read S@R succeded\n"));

	*pData = data;
	return MV_OK;
}

/*******************************************************************************
* mvBoardTwsiSatRSet
*
* DESCRIPTION:
*
* INPUT:
*	devNum - one of three devices
*	regNum - 0 or 1
*	regVal - value
*
*
* OUTPUT:
*	None.
*
* RETURN:
*	reg value
*
*******************************************************************************/
MV_STATUS mvBoardTwsiSet(MV_BOARD_TWSI_CLASS twsiClass, MV_U8 devNum, MV_U8 regNum, MV_U8 regVal)
{
	MV_TWSI_SLAVE twsiSlave;
	MV_TWSI_ADDR slave;

	/* TWSI init */
	slave.type = ADDR7_BIT;
	slave.address = 0;
	mvTwsiInit(0, TWSI_SPEED, mvBoardTclkGet(), &slave, 0);

	/* Read MPP module ID */
	twsiSlave.slaveAddr.address = mvBoardTwsiAddrGet(twsiClass, devNum);
	twsiSlave.slaveAddr.type = mvBoardTwsiAddrTypeGet(twsiClass, devNum);
	twsiSlave.validOffset = MV_TRUE;
	DB(mvOsPrintf("%s: TWSI Write addr %x, type %x, data %x\n", __func__,
		      twsiSlave.slaveAddr.address, twsiSlave.slaveAddr.type, regVal));
	/* Use offset as command */
	twsiSlave.offset = regNum;

	/* need to use 2 address bytes when accessing EEPROM */
	if (twsiClass == BOARD_DEV_TWSI_EEPROM)
		twsiSlave.moreThen256 = MV_TRUE; /* use  2 address bytes */
	else
		twsiSlave.moreThen256 = MV_FALSE; /* use  1 address byte */


	if (MV_OK != mvTwsiWrite(0, &twsiSlave, &regVal, 1)) {
		DB(mvOsPrintf("%s: Write S@R fail\n", __func__));
		return MV_ERROR;
	}
	DB(mvOsPrintf("%s: Write S@R succeded\n", __func__));

	return MV_OK;
}

/*******************************************************************************
* mvBoardSmiScanModeGet - Get Switch SMI scan mode
*
* DESCRIPTION:
*       This routine returns Switch SMI scan mode.
*
* INPUT:
*       switchIdx - index of the switch. Only 0 is supported.
*
* OUTPUT:
*       None.
*
* RETURN:
*       1 for SMI_MANUAL_MODE, -1 if the port number is wrong or if not relevant.
*
*******************************************************************************/
MV_32 mvBoardSmiScanModeGet(MV_U32 switchIdx)
{
	return BOARD_ETH_SWITCH_SMI_SCAN_MODE;
}

/*******************************************************************************
* mvBoardSwitchCpuPortGet - Get the the Ethernet Switch CPU port
*
* DESCRIPTION:
*	This routine returns the Switch CPU port if connected, -1 else.
*
* INPUT:
*	switchIdx - index of the switch. Only 0 is supported.
*
* OUTPUT:
*	None.
*
* RETURN:
*	the Switch CPU port, -1 if the switch is not connected.
*
*******************************************************************************/
MV_U32 mvBoardSwitchCpuPortGet(MV_U32 switchIdx)
{
	MV_U32 c = mvBoardEthComplexConfigGet();
	MV_U32 cpuPort = -1;

	if (c & MV_ETHCOMP_GE_MAC0_2_SW_P6)
		cpuPort = 6;
	else if (c & MV_ETHCOMP_GE_MAC1_2_SW_P4)
		cpuPort = 4;
	else
		mvOsPrintf("%s: Error: No CPU port.\n", __func__);

	return cpuPort;
}

/*******************************************************************************
* mvBoardIsEthConnected - detect if a certain Ethernet port is active
*
* DESCRIPTION:
*	This routine returns true if a certain Ethernet port is active and usable
*
* INPUT:
*	ethNum - index of the ethernet port requested
*
* OUTPUT:
*	None.
*
* RETURN:
*	MV_TRUE if the requested ethernet port is connected and usable.
*
*******************************************************************************/
MV_BOOL mvBoardIsEthConnected(MV_U32 ethNum)
{
	MV_U32 c = mvBoardEthComplexConfigGet();
	MV_BOOL isConnected = MV_FALSE;

	if (ethNum >= board->numBoardMacInfo) {
		mvOsPrintf("%s: Error: Illegal port number(%u)\n", __func__, ethNum);
		return MV_FALSE;
	}

	/*
	 * Determine if port is both active and usable:
	 * 1. active : if MAC is set as active in Ethernet complex board configuration
	 * 2. usable : - MAC always usable when connected to RGMII, COMPHY, or GE-PHY
	 *             - if connected to switch ,a MAC is usable only as the CPU Port
	 *               (if a 2nd MAC is connected to switch, it used for Loopback)
	 */

	if ((ethNum == 0) && ((c & MV_ETHCOMP_GE_MAC0_2_GE_PHY_P0) ||
			(c & MV_ETHCOMP_GE_MAC0_2_RGMII0) ||
			(c & MV_ETHCOMP_GE_MAC0_2_COMPHY_2) ||
			((c & MV_ETHCOMP_GE_MAC0_2_SW_P6) && mvBoardMacCpuPortGet() == 0)))
			isConnected = MV_TRUE;

	if ((ethNum == 1) && ((c & MV_ETHCOMP_GE_MAC1_2_GE_PHY_P3) ||
			(c & MV_ETHCOMP_GE_MAC1_2_RGMII1) ||
			((c & MV_ETHCOMP_GE_MAC1_2_SW_P4) && mvBoardMacCpuPortGet() == 1)))
			isConnected = MV_TRUE;

	if ((ethNum == 2) && mvBoardIsPortLoopback(ethNum))
			isConnected = MV_TRUE;

	return isConnected;
}

/*******************************************************************************
* mvBoardMacCpuPortGet - returns the MAC CPU port connected to switch
*
* DESCRIPTION:
*	This routine returns true returns the MAC CPU port connected to switch
*
* INPUT:
*	None
*
* OUTPUT:
*	None
*
* RETURN:
*	the MAC CPU port number connected to switch
*
*******************************************************************************/
MV_U32 mvBoardMacCpuPortGet(MV_VOID)
{
	MV_U32 c = mvBoardEthComplexConfigGet();
	MV_U32 macCpuPort = -1;

	if (c & MV_ETHCOMP_GE_MAC0_2_SW_P6)		/* MAC0 is the default CPU port */
		macCpuPort = 0;
	/* only If MAC0 isn't connected to switch, then MAC1 is the CPU port
	 * If both MAC0 and MAC1 connected to switch, MAC1 is used for Loopback */
	else if (c & MV_ETHCOMP_GE_MAC1_2_SW_P4)
		macCpuPort = 1;
	else
		DB(mvOsPrintf("%s: Error: No MAC CPU port.\n", __func__));

	return macCpuPort;
}

/*******************************************************************************
* mvBoardConfigAutoDetectEnabled
*
* DESCRIPTION:
*	Indicate if the board supports auto configuration and detection of
*	modules. This is usually enabled for DB boards only.
*
* INPUT:
*	None.
*
* OUTPUT:
*	None.
*
* RETURN:
*       MV_TRUE if auto-config/detection is enabled.
*	MV_FALSE otherwise.
*
*******************************************************************************/
MV_BOOL mvBoardConfigAutoDetectEnabled()
{
	return board->configAutoDetect;
}
