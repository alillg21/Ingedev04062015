//Subversion reference
//$HeadURL: https://svn.ingenico.com/UTAPROJECTS/sea-partner/5W/trunk/ISO5WPH/log/hdl.c $
//$Id: hdl.c 1920 2009-04-23 01:58:29Z ajloreto $

#include <string.h>
#include "log.h"
#include <stdio.h>
#include "bri.h"
#include "prepaid.h"
#include "logon.h" 
#include "LineEncryption.h" 

#define ISO_BUFFER_LEN 2048 
static void hdlSubmenu(word msg1, word msg2);

static void hdlSelect(word MnuItm)
{
	int ret;
	char buf[lenMnu + 1];
	byte Exps;

	traReset();
	MAPPUTWORD(traMnuItm, MnuItm, lblKO);
	ret = valOpr();
	CHECK(ret > 0, lblKO);
	MAPGET(MnuItm, buf, lblKO);
	MAPPUTSTR(traCtx, buf + 2, lblKO);
	MAPGETBYTE(appIsDisable, Exps, lblKO);

	//if(Exps != 1){
		switch (MnuItm)
		{
			case mnuPre:
				hdlSubmenu(MnuItm, mnuPReprint);
				break;
				
			case mnuPembelian:
				hdlSubmenu(MnuItm, mnuBrizzi);
				break;

			case mnuBrizzi: 
				hdlSubmenu(MnuItm, mnuAbsen);
				break;      

			case mnuAbsen:
				hdlSubmenu(MnuItm, mnuCardService);
				break;

			case mnuCardService:
				hdlSubmenu(MnuItm, mnuMiniATM);
				break;

#ifdef T_BANK
			case mnuMiniATM: 
				hdlSubmenu(MnuItm, mnuTCash);
				break;

			case mnuTCash: 
				hdlSubmenu(MnuItm, mnuTunai);
				break;
#else
			case mnuMiniATM: 
				hdlSubmenu(MnuItm, mnuTunai);
				break;
#endif
			case mnuTunai:
				hdlSubmenu(MnuItm, mnuSSB);
				break; 
				
			case mnuSSB:
				hdlSubmenu(MnuItm, mnuCustomer);
				break;
					
			case mnuPReprint:
				hdlSubmenu(MnuItm, mnuMrcLog);
				break;

			case mnuMrcLog:
				hdlSubmenu(MnuItm, mnuSimasEnd);
				break;

			case mnuPrepaidPembayaran:
				hdlSubmenu(MnuItm, mnuPrepaidReprint);
				break;

			case mnuPrepaidReprint:
				hdlSubmenu(MnuItm, mnuPrepaidReprint);
				break;

			case mnuPrepaidReport:            
				hdlSubmenu(MnuItm, mnuBrizziEnd);
				break;

			case mnuShifts:
				hdlSubmenu(MnuItm, mnuAbsenEnd);
				break;

			case mnuShiftsI:
				hdlSubmenu(MnuItm, mnuShiftsII);
				break;
				
			case mnuShiftsII:
				hdlSubmenu(MnuItm, mnuShiftsIII);
				break;
				
			case mnuShiftsIII:
				hdlSubmenu(MnuItm, mnuAbsenEnd);
				break;

			case mnuCReprint:
				hdlSubmenu(MnuItm,mnuCReport);
				break;

			case mnuCReport:
				hdlSubmenu(MnuItm,mnuCardServiceEnd);
				break;

			case mnuInformasi:
				hdlSubmenu(MnuItm, mnuTransfer);
				break;

			case mnuTransfer:
				hdlSubmenu(MnuItm, mnuPembayaran);
				break;

			case mnuPembayaran:
				hdlSubmenu(MnuItm, mnuIsiUlang);
				break;

			case mnuIsiUlang:
				hdlSubmenu(MnuItm, mnuRegistrasi); 
				break;   

			case mnuRegistrasi:
				hdlSubmenu(MnuItm, mnuMReprintReview);
				break; 

			case mnuRegistrasiTrxBanking:
				hdlSubmenu(MnuItm, mnuMReprintReview);
				break;

			case mnuMReprintReview:
				hdlSubmenu(MnuItm, mnuMReport);
				break;

			case mnuMReport:
				hdlSubmenu(MnuItm, mnuMiniATMEnd); 
				break;

			case mnuTReprint:
				hdlSubmenu(MnuItm,mnuTReport);
				break;
				
			case mnuTReport:
				hdlSubmenu(MnuItm,mnuTunaiEnd);
				break;
#ifdef T_BANK
			case mnuTCashReprint:
				hdlSubmenu(MnuItm,mnuTCashReport);
				break;
				
			case mnuTCashReport:
				hdlSubmenu(MnuItm,mnuTCashEnd);
				break;
#endif
			case mnuSIM:
				hdlSubmenu(MnuItm, mnuBPKB);
				break;
				
			case mnuBPKB:
				hdlSubmenu(MnuItm, mnuMutasiRan);
				break;
				
			case mnuMutasiRan:
				hdlSubmenu(MnuItm, mnuSSBEnd);
				break;

			case mnuPDateDetail:
				hdlSubmenu(MnuItm, mnuPAllDetail);
				break;
				
			case mnuPAllDetail:
				hdlSubmenu(MnuItm, mnuBrizziEnd);
				break;

			case mnuPembayaranPostPaid:
				hdlSubmenu(MnuItm, mnuPembayaranTvBerlangganan);
				break;  

			case mnuPembayaranTvBerlangganan:
				hdlSubmenu(MnuItm, mnuPembayaranTiketPesawat);
				break;

			case mnuPembayaranTiketPesawat:
				hdlSubmenu(MnuItm, mnuPembayaranPdam);
				break;
			
			case mnuPembayaranPdam:
				hdlSubmenu(MnuItm, mnuPembayaranPLN);
				break;

			case mnuPembayaranPLN:
				hdlSubmenu(MnuItm, mnuPembayaranKK);
				break;

			case mnuPembayaranKK:
				hdlSubmenu(MnuItm, mnuPembayaranPendidikan);
				break;
				
			case mnuPembayaranKKCITI:
				hdlSubmenu(MnuItm, mnuPembayaranKKStanChar);
				break;
				
			case mnuPembayaranKKStanChar:
				hdlSubmenu(MnuItm, mnuPembayaranKKHSBC);
				break;
				
			case mnuPembayaranKKHSBC:
				hdlSubmenu(MnuItm, mnuPembayaranKKRBS);
				break;
				
			case mnuPembayaranKKRBS:
				hdlSubmenu(MnuItm, mnuPembayaranPendidikan);
				break;	

			case mnuPembayaranPendidikan:
				hdlSubmenu(MnuItm, mnuPembayaranCicilan);
				break;

			case mnuPembayaranCicilan:
				hdlSubmenu(MnuItm, mnuPembayaranZakat);
				break;

			case mnuPembayaranZakat:
				hdlSubmenu(MnuItm, mnuPembayaranInfaq);
				break;

			case mnuPembayaranInfaq:        
				hdlSubmenu(MnuItm, mnuIsiUlang);
				break;        

			case mnuPembayaranSimpati:
				hdlSubmenu(MnuItm, mnuPembayaranMentari);
				break;
				
			case mnuPembayaranMentari:
				hdlSubmenu(MnuItm, mnuPembayaranIm3);
				break;
				
			case mnuPembayaranIm3:
				hdlSubmenu(MnuItm, mnuPembayaranEsia);
				break;
				
			case mnuPembayaranEsia:
				hdlSubmenu(MnuItm, mnuPembayaranSmart); 
				break;
				
			case mnuPembayaranSmart:
				hdlSubmenu(MnuItm, mnuPembayaranFren); 
				break;
				
			case mnuPembayaranFren:
				hdlSubmenu(MnuItm, mnuPembayaranThree); 
				break;
				
			case mnuPembayaranThree:
				hdlSubmenu(MnuItm, mnuPembayaranAxis);
				break;
				
			case mnuPembayaranAxis:
				hdlSubmenu(MnuItm, mnuPembayaranXl); 
				break;
       
			case mnuPembayaranXl:
				hdlSubmenu(MnuItm, mnuRegistrasi);
				break;    
				
			case mnuPembayaranDPLK:        
				hdlSubmenu(MnuItm, mnuIsiUlang);
				break;
				
			case mnuMReprint:
				hdlSubmenu(MnuItm, mnuMReview);
				break;

			case mnuMReview:
				hdlSubmenu(MnuItm, mnuMReport);
				break;
				
			case mnuMrcInfo:
				hdlSubmenu(MnuItm, mnuMrcBatStan);
				break;
				
			case mnuMrcBatStan:
				hdlSubmenu(MnuItm, mnuMrcChgPwd);
				break;
				
			case mnuViewLog:
				hdlSubmenu(MnuItm, mnuTest);
				break;
				
			case mnuSIMBaru:
				hdlSubmenu(MnuItm, mnuSIMPanjang);
				break;
				
			case mnuSIMPanjang:
				hdlSubmenu(MnuItm, mnuSIMPeningkatan);
				break;
				
			case mnuSIMPeningkatan:
				hdlSubmenu(MnuItm, mnuSIMPenurunan);
				break;
				
			case mnuSIMPenurunan:
				hdlSubmenu(MnuItm, mnuSIMMutasi);
				break;
				
			case mnuSIMMutasi:
				hdlSubmenu(MnuItm, mnuSIMHilang);
				break;
				
			case mnuSIMHilang:
				hdlSubmenu(MnuItm, mnuBPKB);
				break;

			case mnuSIMMutasiGolSam:
				hdlSubmenu(MnuItm, mnuSIMMutasiPeningkatan);
				break;
				
			case mnuSIMMutasiPeningkatan:
				hdlSubmenu(MnuItm, mnuSIMMutasiPenurunan);
				break;
				
			case mnuSIMMutasiPenurunan:
				hdlSubmenu(MnuItm, mnuSIMHilang);
				break;

			case mnuSIMHilangGolSam:
				hdlSubmenu(MnuItm, mnuSIMHilangPeningkatan);
				break;
				
			case mnuSIMHilangPeningkatan:
				hdlSubmenu(MnuItm, mnuSIMHilangPenurunan);
				break;
				
			case mnuSIMHilangPenurunan:
				hdlSubmenu(MnuItm, mnuCustomer);
				break;

			case mnuBPKBBaru:
				hdlSubmenu(MnuItm, mnuBPKBGanti);
				break;
				
			case mnuBPKBGanti:
				hdlSubmenu(MnuItm, mnuCustomer);
				break;

			case mnuTMS:
				hdlSubmenu(MnuItm, mnuTerm);
				break;
				
			case mnuTerm:
				hdlSubmenu(MnuItm, mnuCmm);
				break;
			case mnuTerm2:
				hdlSubmenu(MnuItm, mnuTermAppli);
				break;
			case mnuCmm:
				hdlSubmenu(MnuItm, mnuFn99);
				break;

			case mnuCmm2:
				hdlSubmenu(MnuItm, mnuCmmCS);
				break;

#ifdef __TEST__

			case mnuTest:
				hdlSubmenu(MnuItm, mnuEnd);
				break;

			case mnuTestData:
				hdlSubmenu(MnuItm, mnuTestIso);
				break;
			case mnuTestIso:
				hdlSubmenu(MnuItm, mnuTestCmm);
				break;
			case mnuTestCmm:
				hdlSubmenu(MnuItm, mnuEnd);
				break;
			case mnuTestIsoRqs:
				hdlSubmenu(MnuItm, mnuTestIsoRsp);
				break;
			case mnuTestIsoRsp:
				hdlSubmenu(MnuItm, mnuEnd);
				break;
#endif

			case mnuCustomer:
				hdlSubmenu(MnuItm, mnuMerchant);
				break;

			case mnuMerchant:
				hdlSubmenu(MnuItm, mnuAdmin);
				break;

			case mnuAdmin:
				hdlSubmenu(MnuItm, mnuFn99); 
				break;
				
			case mnuFn99:
				hdlSubmenu(MnuItm, mnuFn99End);
				break;

			case mnuAktivasiKartu:
			case mnuRenewalKartu:
			case mnuReissuePIN:
			case mnuGantiPIN:
			case mnuPIN:
			case mnuGantiPasswordSPV:
				pmtBRI();
				break; 

			case mnuCLastTrx:
				briLogDuplicata(CARD_SERVICE_LOG);
				break;

			case mnuCAnyTrx:
				briLogPrintTxn(CARD_SERVICE_LOG);
				break;

			case mnuCTodaySummary:
				briPrintTotalToday(CARD_SERVICE_LOG);
				break;

			case mnuCTodayDetail:
				briPrintDetailToday(CARD_SERVICE_LOG);
				break;

			case mnuCPastSummary:
				briPrintTotal(CARD_SERVICE_LOG);
				break;

			case mnuCPastDetail:
				briPrintDetail(CARD_SERVICE_LOG);
				break;   

#ifdef T_BANK
			case mnuTCashLastTrx:
				briLogDuplicata(TCASH_LOG);
				break;

			case mnuTCashAnyTrx:
				briLogPrintTxn(TCASH_LOG);
				break;

			case mnuTCashTodaySummary:
				briPrintTotalToday(TCASH_LOG);
				break;

			case mnuTCashTodayDetail:
				briPrintDetailToday(TCASH_LOG);
				break;

			case mnuTCashPastSummary:
				briPrintTotal(TCASH_LOG);
				break;

			case mnuTCashPastDetail:
				briPrintDetail(TCASH_LOG);
				break;
#endif

			case mnuSetorSimpanan:
			case mnuSetorPinjaman:
			case mnuTarikTunai:
				pmtBRI();
				break;  

			case mnuVoidTarikTunai:        
				pmtVoidTarikTunai();
				break;  

			case mnuTLastTrx: 
				briLogDuplicata(TUNAI_LOG);
				break;

			case mnuTAnyTrx:
				briLogPrintTxn(TUNAI_LOG);
				break;

			case mnuTTodaySummary:
				briPrintTotalToday(TUNAI_LOG);
				break;
				
			case mnuTTodayDetail:
				briPrintDetailToday(TUNAI_LOG);
				break;
				
			case mnuTPastSummary:
				briPrintTotal(TUNAI_LOG);
				break;
				
			case mnuTPastDetail:
				briPrintDetail(TUNAI_LOG);
				break;
			
			case mnuInfoSaldo:
			case mnuInfoSaldoBankLain:
			case mnuMiniStatement:
			case mnuMutRek:
			case mnuPrevilege:
				pmtBRI();
				break;
		
			case mnuTransferSesamaBRI:
			case mnuTransferAntarBank:
				pmtBRI();
				break;        
		
			case mnuInfoKodeBank1:
			case mnuInfoKodeBank:
				infoKodeBank();
				break;

			case mnuPembayaranBriva:
			case mnuPembayaranTelkom:
			case mnuPembayaranDPLKR:
			case mnuPembayaranHalo:        
			case mnuPembayaranMatrix:
			case mnuPembayaranPLNPasca:
			case mnuPembayaranPLNPra:
			case mnuPembayaranPLNToken:
				pmtBRI();
				break;

			case mnuPembayaranCicilanFIF:
			case mnuPembayaranCicilanBAF:
			case mnuPembayaranCicilanOTO:
			case mnuPembayaranCicilanFinansia:
			case mnuPembayaranCicilanVerena:
			case mnuPembayaranCicilanWOM:
				pmtBRI();
				break;
			
			case mnuPembayaranTvInd:
			case mnuPembayaranTvOkTv:
			case mnuPembayaranTvTopTv:
				pmtBRI();
				break;

			case mnuPembayaranTiketGaruda:
			case mnuPembayaranTiketLionAir:
			case mnuPembayaranTiketSriwijaya:
			case mnuPembayaranTiketMandala:
				pmtBRI();
				break;

			case mnuPembayaranPdamSby:
				pmtBRI();
				break;

			case mnuRegistrasiTrxTransfer:
			case mnuRegistrasiTrxPulsa:
				pmtBRI();
				break;
				
			case mnuPembayaranSPP:
				pmtBRI();
				break;
				
			case mnuPembayaranKodeUniv:
				infoKodeUniv();
				break;
				
			case mnuPembayaranSimpati50:
			case mnuPembayaranSimpati100:
//			case mnuPembayaranSimpati150:
			case mnuPembayaranSimpati200:
			case mnuPembayaranSimpati300:
			case mnuPembayaranSimpati500:
			case mnuPembayaranSimpati1000:
				pmtBRI();
				break;
			
			case mnuPembayaranMentari25:
			case mnuPembayaranMentari50:
			case mnuPembayaranMentari100:
			case mnuPembayaranMentari250:
			case mnuPembayaranMentari500:
			case mnuPembayaranMentari1000:
				pmtBRI();  
				break;         
			
			case mnuPembayaranIm325:    
			case mnuPembayaranIm350:    
			case mnuPembayaranIm375:    
			case mnuPembayaranIm3100:   
			case mnuPembayaranIm3150:   
			case mnuPembayaranIm3200:   
			case mnuPembayaranIm3500:   
			case mnuPembayaranIm31000:  
				pmtBRI();  
				break;
			
			case mnuPembayaranXl25: 
			case mnuPembayaranXl50: 
			case mnuPembayaranXl75: 
			case mnuPembayaranXl100: 
			case mnuPembayaranXl150: 
			case mnuPembayaranXl200: 
			case mnuPembayaranXl300: 
			case mnuPembayaranXl500: 
				pmtBRI();  
				break;               
			
			case mnuPembayaranEsia25: 
			case mnuPembayaranEsia50: 
			case mnuPembayaranEsia75: 
			case mnuPembayaranEsia100: 
			case mnuPembayaranEsia150: 
			case mnuPembayaranEsia200: 
				pmtBRI();  
				break;               
		
			case mnuPembayaranSmart20: 
			case mnuPembayaranSmart25:
			case mnuPembayaranSmart50: 
			case mnuPembayaranSmart100:
			case mnuPembayaranSmart150: 
			case mnuPembayaranSmart200:
			case mnuPembayaranSmart300:
			case mnuPembayaranSmart500:

				pmtBRI();  
				break;               
			
			case mnuPembayaranFren20:
			case mnuPembayaranFren25:
			case mnuPembayaranFren50:
			case mnuPembayaranFren100:
			case mnuPembayaranFren150:
			case mnuPembayaranFren200:
			case mnuPembayaranFren300:
			case mnuPembayaranFren500: 
				pmtBRI();  
				break;   
			
			case mnuPembayaranThree20:
			case mnuPembayaranThree50:
			case mnuPembayaranThree75:
			case mnuPembayaranThree100:
			case mnuPembayaranThree150:
			case mnuPembayaranThree300:
			case mnuPembayaranThree500: 
				pmtBRI();  
				break;   
			
			case mnuPembayaranAxis25:
			case mnuPembayaranAxis50:
			case mnuPembayaranAxis75:
			case mnuPembayaranAxis100:
			case mnuPembayaranAxis150:
			case mnuPembayaranAxis200:
			case mnuPembayaranAxis300:
				pmtBRI();  
				break;   
			
			case mnuPembayaranKKBRI:
				pmtBRI();
				break;

			case mnuPembayaranKKCITIKTA:
			case mnuPembayaranKKCITIKK:
				pmtBRI();
				break;

			case mnuPembayaranKKStanCharKK:
			case mnuPembayaranKKStanCharKTA:
				pmtBRI();  
				break;

			case mnuPembayaranKKANZ:
				pmtBRI();  
				break;

			case mnuPembayaranKKHSBCKTA:
			case mnuPembayaranKKHSBCKK:
				pmtBRI();
				break;

			case mnuPembayaranKKRBSKK:
			case mnuPembayaranKKRBSKTA:
				pmtBRI();
				break;

			case mnuPembayaranZakatDhuafa:
			case mnuPembayaranZakatYBM: 
			case mnuPembayaranInfaqDhuafa:
			case mnuPembayaranInfaqYBM: 
			case mnuPembayaranDPLKDhuafa:
			case mnuPembayaranDPLKYBM:                 
				pmtBRI();  
				break;               
#ifdef T_BANK
			case mnuCashIn:
			case mnuCashOut:
#endif
			case mnuRegistrasiInternetBanking:
			case mnuRegistrasiPhoneBanking:
			case mnuRegistrasiSmsBanking:
				pmtBRI();  
				break;  

			
			case mnuMLastTrx:
				briLogDuplicata(MINI_ATM_LOG);
				break;
				
			case mnuMAnyTrx:
				briLogPrintTxn(MINI_ATM_LOG);
				break;
				
			
			case mnuMVLastTrx:
				briViewBat(MINI_ATM_LOG,1);
				break;
				
			case mnuMVAnyTrx:
				briViewBat(MINI_ATM_LOG,2);
				break;
				
			
			case mnuMTodaySummary:
				briPrintTotalToday(MINI_ATM_LOG);
				break;
				
			case mnuMTodayDetail:
				briPrintDetailToday(MINI_ATM_LOG);
				break;
				
			case mnuMPastSummary:
				briPrintTotal(MINI_ATM_LOG);
				break;
				
			case mnuMPastDetail:
				briPrintDetail(MINI_ATM_LOG);
				break;

			case mnuSale:
				pmtSale();
				break;
				
			case mnuSaleRedeem:
				pmtSaleRedeem();
				break; 
				
			case mnuInstallment:         
				pmtBRI();
				break;      
				
			case mnuRefund:
				pmtRefund();
				break;
				
			case mnuPreaut:
				pmtPreaut();
				break;
					
			case mnuAut:
				pmtAut();
				break;

			case mnuPreAuth:
				pmtPreAuth();
				break;

			case mnuPreCom:
				pmtPreCom();
				break;

			case mnuPreCancel:
				pmtPreCancel();
				break;
										
			case mnuVoid:
				pmtVoid();
				break;
				
			case mnuOffline:
				pmtOffline();
				break;

			case mnuSettlement:
				logSettlement();
				break;
				
			case mnuMrcChgPwd:
				admChangePwd();
				break;
				
			case mnuMrcReset:
			case mnuFn99MrcReset:		
				admReset();
				break;
				
			case mnuMrcResetRev:
			case mnuFn99MrcResetRev:
				admRevReset();
				admTCReset();
				break;
				
			case mnuDeleteCLog:
				cLogReset();
				break;
				
			case mnuDeleteMLog:
				mLogReset();
				break;
				
			case mnuDeleteTLog:
				tLogReset();	        	        
				break;

			case mnuDeleteTCashLog:
				tCashLogReset();
				break;
				
			case mnuViewCardService:
				viewCardServiceLog();
				break;
				
			case mnuViewMiniAtm:
				viewMiniAtmLog();
				break;
				
			case mnuViewTunai:
				viewTunaiLog();
				break;

			case mnuViewTCash:
				viewTCashLog();
				break;

#ifdef PREPAID
			case mnuInitialize:
			case mnuSaldo:
			case mnuPrepaidInfoSaldo:
			case mnuInfoDeposit:
//			case mnuPrepaidPembayaran:
			case mnuPrepaidPayment:
			case mnuTopUpOnline:
			case mnuTopUpDeposit:
			case mnuPrepaidRedeem:
			case mnuPrepaidPaymentDisc:
			case mnuPrepaidReaktivasi:
			case mnuPrepaidVoid:
			case mnuAktivasiDeposit:
			case mnuPrepaidInfoKartu:
			case mnuPrepaidPrintLog:
				pmtBrizzi();
				break;
				
			case mnuPrepaidSettlement:
				pmtPrepaidSettlement();
				break;
				
			case mnuPRLastTrx:
				prepaidReprint(1);
				break;
				
			case mnuPRAnyTrx:
				prepaidReprint(0);
				break;
				
			case mnuPDateSummary:
				prepaidDateSummary();
				break;
				
			case mnuPAllSummary:
				prepaidAllSummary();
				break;	  
				
			case mnuPDateDetailAll:
				prepaidDateDetail(0);
				break;
				
			case mnuPAllDetailAll:
				prepaidAllDetail(0);
				break;	  
				
			case mnuPDateDetailPayment:
				prepaidDateDetail(1);
				break;
				
			case mnuPAllDetailPayment:
				prepaidAllDetail(1);
				break;	   

			case mnuSetorPasti:
				pmtBRI();
				break;

				
#endif

#ifdef _USE_ECR
			case mnuPOS:
				POSPrintReferenceNum();
				break;
#endif

#ifdef ABSENSI
			case mnuDatang:
			case mnuPulangs:
			case mnuIstirahatA:
			case mnuIstirahatB:
			case mnuLemburA:
			case mnuLemburB:
			case mnuShiftsIA:
			case mnuShiftsIB:
			case mnuShiftsIIA:
			case mnuShiftsIIB:
			case mnuShiftsIIIA:
			case mnuShiftsIIIB:
				pmtAbsen();
				break;
#endif

#ifdef SSB
			case mnuSIMBaruA :
				MAPPUTSTR(traTotalOutstanding, "11", lblKO);
				pmtBRI();
			break;
			case mnuSIMBaruAU :
				MAPPUTSTR(traTotalOutstanding, "11", lblKO);
				pmtBRI();
				break;
			case mnuSIMBaruB1 :
				MAPPUTSTR(traTotalOutstanding, "11", lblKO);
				pmtBRI();
				break;
			case mnuSIMBaruB1U :
				MAPPUTSTR(traTotalOutstanding, "11", lblKO);
				pmtBRI();
				break;
			case mnuSIMBaruB2 :
				MAPPUTSTR(traTotalOutstanding, "11", lblKO);
				pmtBRI();
				break;
			case mnuSIMBaruB2U :
				MAPPUTSTR(traTotalOutstanding, "11", lblKO);
				pmtBRI();
				break;
			case mnuSIMBaruC :
				MAPPUTSTR(traTotalOutstanding, "17", lblKO);
				pmtBRI();
				break;
			case mnuSIMBaruD :
				MAPPUTSTR(traTotalOutstanding, "18", lblKO);
				pmtBRI();
				break;
			case mnuSIMPanjangA :
				MAPPUTSTR(traTotalOutstanding, "21", lblKO);
				pmtBRI();
				break;
			case mnuSIMPanjangAU :
				MAPPUTSTR(traTotalOutstanding, "22", lblKO);
				pmtBRI();
				break;
			case mnuSIMPanjangB1 :
				MAPPUTSTR(traTotalOutstanding, "23", lblKO);
				pmtBRI();
				break;
			case mnuSIMPanjangB1U :
				MAPPUTSTR(traTotalOutstanding, "24", lblKO);
				pmtBRI();
				break;
			case mnuSIMPanjangB2 :
				MAPPUTSTR(traTotalOutstanding, "25", lblKO);
				pmtBRI();
				break;
			case mnuSIMPanjangB2U :
				MAPPUTSTR(traTotalOutstanding, "26", lblKO);
				pmtBRI();
				break;
			case mnuSIMPanjangC :
				MAPPUTSTR(traTotalOutstanding, "27", lblKO);
				pmtBRI();
				break;
			case mnuSIMPanjangD :
				MAPPUTSTR(traTotalOutstanding, "28", lblKO);
				pmtBRI();
				break;
			case mnuSIMPeningkatanAU :
				MAPPUTSTR(traTotalOutstanding, "32", lblKO);
				pmtBRI();
				break;
			case mnuSIMPeningkatanB1 :
				MAPPUTSTR(traTotalOutstanding, "33", lblKO);
				pmtBRI();
				break;
			case mnuSIMPeningkatanB1U :
				MAPPUTSTR(traTotalOutstanding, "34", lblKO);
				pmtBRI();
				break;
				
			case mnuSIMPeningkatanB2 :
				MAPPUTSTR(traTotalOutstanding, "35", lblKO);
				pmtBRI();
				break;
				
			case mnuSIMPeningkatanB2U :
				MAPPUTSTR(traTotalOutstanding, "36", lblKO);
				pmtBRI();
				break;

			case mnuSIMPenurunanA :
				MAPPUTSTR(traTotalOutstanding, "41", lblKO);
				pmtBRI();
				break;
				
			case mnuSIMPenurunanAU :
				MAPPUTSTR(traTotalOutstanding, "42", lblKO);
				pmtBRI();
				break;
				
			case mnuSIMPenurunanB1 :
				MAPPUTSTR(traTotalOutstanding, "43", lblKO);
				pmtBRI();
				break;
				
			case mnuSIMPenurunanB1U :
				MAPPUTSTR(traTotalOutstanding, "44", lblKO);
				pmtBRI();
				break;
				
			case mnuSIMPenurunanB2 :
				MAPPUTSTR(traTotalOutstanding, "45", lblKO);
				pmtBRI();
				break;
				
			case mnuSIMPenurunanB2U :
				MAPPUTSTR(traTotalOutstanding, "11", lblKO);
				pmtBRI();
				break;
				
			case mnuSIMGolSamA :
				MAPPUTSTR(traTotalOutstanding, "51", lblKO);
				pmtBRI();
				break;
				
			case mnuSIMGolSamAU :
				MAPPUTSTR(traTotalOutstanding, "52", lblKO);
				pmtBRI();
				break;
				
			case mnuSIMGolSamB1 :
				MAPPUTSTR(traTotalOutstanding, "53", lblKO);
				pmtBRI();
				break;
				
			case mnuSIMGolSamB1U :
				MAPPUTSTR(traTotalOutstanding, "54", lblKO);
				pmtBRI();
				break;
				
			case mnuSIMGolSamB2 :
				MAPPUTSTR(traTotalOutstanding, "55", lblKO);
				pmtBRI();
				break;
				
			case mnuSIMGolSamB2U :
				MAPPUTSTR(traTotalOutstanding, "56", lblKO);
				pmtBRI();
				break;
				
			case mnuSIMGolSamC :
				MAPPUTSTR(traTotalOutstanding, "57", lblKO);
				pmtBRI();
				break;
				
			case mnuSIMGolSamD :
				MAPPUTSTR(traTotalOutstanding, "58", lblKO);
				pmtBRI();
				break;
				
			case mnuSIMMutasiPeningkatanAU :
				MAPPUTSTR(traTotalOutstanding, "5B", lblKO);
				pmtBRI();
				break;
				
			case mnuSIMMutasiPeningkatanB1 :
				MAPPUTSTR(traTotalOutstanding, "5C", lblKO);
				pmtBRI();
				break;
				
			case mnuSIMMutasiPeningkatanB1U :
				MAPPUTSTR(traTotalOutstanding, "5D", lblKO);
				pmtBRI();
				break;
				
			case mnuSIMMutasiPeningkatanB2 :
				MAPPUTSTR(traTotalOutstanding, "5E", lblKO);
				pmtBRI();
				break;
				
			case mnuSIMMutasiPeningkatanB2U :
				MAPPUTSTR(traTotalOutstanding, "5F", lblKO);
				pmtBRI();
				break;

			case mnuSIMMutasiPenurunanA :
				MAPPUTSTR(traTotalOutstanding, "5K", lblKO);
				pmtBRI();
				break;
				
			case mnuSIMMutasiPenurunanAU :
				MAPPUTSTR(traTotalOutstanding, "5L", lblKO);
				pmtBRI();
				break;
				
			case mnuSIMMutasiPenurunanB1 :
				MAPPUTSTR(traTotalOutstanding, "5M", lblKO);
				pmtBRI();
				break;
				
			case mnuSIMMutasiPenurunanB1U :
				MAPPUTSTR(traTotalOutstanding, "5N", lblKO);
				pmtBRI();
				break;
				
			case mnuSIMMutasiPenurunanB2 :
				MAPPUTSTR(traTotalOutstanding, "5O", lblKO);
				pmtBRI();
				break;
				
			case mnuSIMMutasiPenurunanB2U :
				MAPPUTSTR(traTotalOutstanding, "11", lblKO);
				pmtBRI();
				break;
			
			case mnuSIMHilangA :
				MAPPUTSTR(traTotalOutstanding, "61", lblKO);
				pmtBRI();
				break;
			
			case mnuSIMHilangAU :
				MAPPUTSTR(traTotalOutstanding, "62", lblKO);
				pmtBRI();
				break;
			
			case mnuSIMHilangB1 :
				MAPPUTSTR(traTotalOutstanding, "63", lblKO);
				pmtBRI();
				break;
				
			case mnuSIMHilangB1U :
				MAPPUTSTR(traTotalOutstanding, "64", lblKO);
				pmtBRI();
				break;
			
			case mnuSIMHilangB2 :
				MAPPUTSTR(traTotalOutstanding, "65", lblKO);
				pmtBRI();
				break;
			
			case mnuSIMHilangB2U :
				MAPPUTSTR(traTotalOutstanding, "66", lblKO);
				pmtBRI();
				break;
			
			case mnuSIMHilangC :
				MAPPUTSTR(traTotalOutstanding, "67", lblKO);
				pmtBRI();
				break;
			
			case mnuSIMHilangD :
				MAPPUTSTR(traTotalOutstanding, "68", lblKO);
				pmtBRI();
				break;
			
			case mnuSIMHilangPeningkatanAU :
				MAPPUTSTR(traTotalOutstanding, "6B", lblKO);
				pmtBRI();
				break;
			
			case mnuSIMHilangPeningkatanB1 :
				MAPPUTSTR(traTotalOutstanding, "6C", lblKO);
				pmtBRI();
				break;
			
			case mnuSIMHilangPeningkatanB1U :
				MAPPUTSTR(traTotalOutstanding, "6D", lblKO);
				pmtBRI();
				break;
			
			case mnuSIMHilangPeningkatanB2 :
				MAPPUTSTR(traTotalOutstanding, "6E", lblKO);
				pmtBRI();
				break;
			
			case mnuSIMHilangPeningkatanB2U :
				MAPPUTSTR(traTotalOutstanding, "6F", lblKO);
				pmtBRI();
				break;

			case mnuSIMHilangPenurunanA :
				MAPPUTSTR(traTotalOutstanding, "6K", lblKO);
				pmtBRI();
				break;
			
			case mnuSIMHilangPenurunanAU :
				MAPPUTSTR(traTotalOutstanding, "6L", lblKO);
				pmtBRI();
				break;
			
			case mnuSIMHilangPenurunanB1 :
				MAPPUTSTR(traTotalOutstanding, "6M", lblKO);
				pmtBRI();
				break;
			
			case mnuSIMHilangPenurunanB1U :
				MAPPUTSTR(traTotalOutstanding, "6N", lblKO);
				pmtBRI();
				break;
			
			case mnuSIMHilangPenurunanB2 :
				MAPPUTSTR(traTotalOutstanding, "6O", lblKO);
				pmtBRI();
				break;
			
			case mnuUjiSim :
				MAPPUTSTR(traTotalOutstanding, "70", lblKO);
				pmtBRI();
				break;
			
			case mnuBPKBBaruRoda2 :
				MAPPUTSTR(traTotalOutstanding, "81", lblKO);
				pmtBRI();
				break;
			
			case mnuBPKBBaruRoda4 :
				MAPPUTSTR(traTotalOutstanding, "82", lblKO);
				pmtBRI();
				break;
			
			case mnuBPKBGantiRoda2 :
				MAPPUTSTR(traTotalOutstanding, "91", lblKO);
				pmtBRI();
				break;
			
			case mnuBPKBGantiRoda4 :
				MAPPUTSTR(traTotalOutstanding, "92", lblKO);
				pmtBRI();
				break;
			
			case mnuSTCK :
				MAPPUTSTR(traTotalOutstanding, "A0", lblKO);
				pmtBRI();
				break;
			
			case mnuMutasiRanRoda2 :
				MAPPUTSTR(traTotalOutstanding, "B1", lblKO);
				pmtBRI();
				break;
			
			case mnuMutasiRanRoda4 :
				MAPPUTSTR(traTotalOutstanding, "B2", lblKO);
				pmtBRI();
				break;
#endif
			case mnuMrcInitRpt:
				admInitRpt();
				break;
			
			case mnuMrcViewTot:
				admViewTotals();
				break;
			
			case mnuMrcSWver:
				admSWver();
				break;

			case mnuMrcViewRoc:
				admViewRoc();
				break;
			
			case mnuMrcSetRoc:
				admSetRoc();
				break;
			
			case mnuMrcViewBatNum:
				admViewBatchNum();
				break;
			
			case mnuMrcSetBatNum:
				admSetBatchNum();
				break;			
		
			case mnuMrcViewSTAN:
				admViewStan();
				break;
			
			case mnuMrcSetSTAN:
				admSetStan();
				break;
			
			case mnuMrcViewBat:
				admViewBat();
				break;
			
			case mnuMrcViewTxn:
				admViewTxn();
				break;
			
			case mnuMrcPrnTra:
				logPrintTxn(NORMAL_LOG);
				break;
				
			case mnuMrcReprint:
				logDuplicata(NORMAL_LOG);
				break;
			
			case mnuMrcSetDup:
				logSetDup();
				break;
			
			case mnuMrcHostTot:
				logHostTotals();
				break;
			
			case mnuMrcDetails:
				printAllLog();
				break;
			
			case mnuMrcBatTot:
				logBatchTotals();
				break;

			//case mnuTMS:
			case mnuTMSLP:
				admTMSLoadParam();
				break;
			
			case mnuTMSVC:
				admTMSViewConfig();
				break;
			
			case mnuTMSPC:
				admTMSPrintConfig();
				break;
			
			case mnuTMSConfig:
				admTMSConfig();
				break;

			case mnuTDT:
				admTDT();
				break;
				
			case mnuTermAppli:
				admTermAppli();
				break;
			
			case mnuTMKey:
			case mnuFn99TMKey:
				admMasterKey();
				break;
			
			case mnuEDCFunction:
				admEDCFunction();
				break;	

			case mnuCmmVS:
				admCmmViewSetting();
				break;
				
			case mnuCmmPS:
				admCmmPrintSetting();
				break;
				
			case mnuCmmLS:
				admCmmLineSetup();
				break;
				
			case mnuCmmGS:
				admCmmGConfigure();
				break;
				
			case mnuCmmCS:
				admConnSettings();
				break;

			case mnuBalance:
				pmtBalance();
				break;

			case mnuLogon:
				debitLogon(0,1);
				break;

			default:
				break;
		}
	/*}
	else
	{
		switch (MnuItm)
		{
			case mnuPembelian:
				hdlSubmenu(MnuItm, mnuBrizzi);
				break;

			case mnuBrizzi: 
				hdlSubmenu(MnuItm, mnuAbsen);
				break;      

			case mnuAbsen:
				hdlSubmenu(MnuItm, mnuCardService);
				break;

			case mnuCardService:
				hdlSubmenu(MnuItm, mnuMiniATM);
				break;

			case mnuMiniATM: 
				hdlSubmenu(MnuItm, mnuTCash);
				break;

			case mnuTCash: 
				hdlSubmenu(MnuItm, mnuTunai);
				break;

			case mnuTunai:
				hdlSubmenu(MnuItm, mnuSSB);
				break; 
				
			case mnuSSB:
				hdlSubmenu(MnuItm, mnuCustomer);
				break;

			case mnuPReprint:
				hdlSubmenu(MnuItm, mnuMrcLog);
				break;

			case mnuMrcLog:
				hdlSubmenu(MnuItm, mnuPembelianEnd);
				break;

			case mnuPrepaidPembayaran:
				hdlSubmenu(MnuItm, mnuPrepaidReprint);
				break;

			case mnuPrepaidReprint:
				hdlSubmenu(MnuItm, mnuPrepaidReport);
				break;

			case mnuPrepaidReport:            
				hdlSubmenu(MnuItm, mnuBrizziEnd);
				break;

			case mnuShifts:
				hdlSubmenu(MnuItm, mnuAbsenEnd);
				break;

			case mnuShiftsI:
				hdlSubmenu(MnuItm, mnuShiftsII);
				break;
				
			case mnuShiftsII:
				hdlSubmenu(MnuItm, mnuShiftsIII);
				break;
				
			case mnuShiftsIII:
				hdlSubmenu(MnuItm, mnuAbsenEnd);
				break;

			case mnuCReprint:
				hdlSubmenu(MnuItm,mnuCReport);
				break;

			case mnuCReport:
				hdlSubmenu(MnuItm,mnuCardServiceEnd);
				break;

			case mnuInformasi:
				hdlSubmenu(MnuItm, mnuTransfer);
				break;

			case mnuTransfer:
				hdlSubmenu(MnuItm, mnuPembayaran);
				break;

			case mnuPembayaran:
				hdlSubmenu(MnuItm, mnuIsiUlang);
				break;

			case mnuIsiUlang:
				hdlSubmenu(MnuItm, mnuRegistrasi); 
				break;   

			case mnuRegistrasi:
				hdlSubmenu(MnuItm, mnuMReprintReview);
				break; 

			case mnuRegistrasiTrxBanking:
				hdlSubmenu(MnuItm, mnuMiniATMEnd);
				break;

			case mnuMReprintReview:
				hdlSubmenu(MnuItm, mnuMReport);
				break;

			case mnuMReport:
				hdlSubmenu(MnuItm, mnuMiniATMEnd);
				break;

			case mnuTReprint:
				hdlSubmenu(MnuItm,mnuTReport);
				break;
				
			case mnuTReport:
				hdlSubmenu(MnuItm,mnuTunaiEnd);
				break;

			case mnuSIM:
				hdlSubmenu(MnuItm, mnuBPKB);
				break;
				
			case mnuBPKB:
				hdlSubmenu(MnuItm, mnuMutasiRan);
				break;
				
			case mnuMutasiRan:
				hdlSubmenu(MnuItm, mnuSSBEnd);
				break;

			case mnuPDateDetail:
				hdlSubmenu(MnuItm, mnuPAllDetail);
				break;
				
			case mnuPAllDetail:
				hdlSubmenu(MnuItm, mnuBrizziEnd);
				break;

			case mnuPembayaranPostPaid:
				hdlSubmenu(MnuItm, mnuPembayaranTvBerlangganan);
				break;  

			case mnuPembayaranTvBerlangganan:
				hdlSubmenu(MnuItm, mnuPembayaranTiketPesawat);
				break;

			case mnuPembayaranTiketPesawat:
				hdlSubmenu(MnuItm, mnuPembayaranPdam);
				break;
			
			case mnuPembayaranPdam:
				hdlSubmenu(MnuItm, mnuPembayaranPLN);
				break;

			case mnuPembayaranPLN:
				hdlSubmenu(MnuItm, mnuPembayaranKK);
				break;

			case mnuPembayaranKK:
				hdlSubmenu(MnuItm, mnuPembayaranPendidikan);
				break;
				
			case mnuPembayaranKKCITI:
				hdlSubmenu(MnuItm, mnuPembayaranKKStanChar);
				break;
				
			case mnuPembayaranKKStanChar:
				hdlSubmenu(MnuItm, mnuPembayaranKKHSBC);
				break;
				
			case mnuPembayaranKKHSBC:
				hdlSubmenu(MnuItm, mnuPembayaranKKRBS);
				break;
				
			case mnuPembayaranKKRBS:
				hdlSubmenu(MnuItm, mnuPembayaranPendidikan);
				break;	

			case mnuPembayaranPendidikan:
				hdlSubmenu(MnuItm, mnuPembayaranCicilan);
				break;

			case mnuPembayaranCicilan:
				hdlSubmenu(MnuItm, mnuPembayaranZakat);
				break;

			case mnuPembayaranZakat:
				hdlSubmenu(MnuItm, mnuPembayaranInfaq);
				break;

			case mnuPembayaranInfaq:        
				hdlSubmenu(MnuItm, mnuIsiUlang);
				break;        

			case mnuPembayaranSimpati:
				hdlSubmenu(MnuItm, mnuPembayaranMentari);
				break;
				
			case mnuPembayaranMentari:
				hdlSubmenu(MnuItm, mnuPembayaranIm3);
				break;
				
			case mnuPembayaranIm3:
				hdlSubmenu(MnuItm, mnuPembayaranEsia);
				break;
				
			case mnuPembayaranEsia:
				hdlSubmenu(MnuItm, mnuPembayaranSmart); 
				break;
				
			case mnuPembayaranSmart:
				hdlSubmenu(MnuItm, mnuPembayaranFren); 
				break;
				
			case mnuPembayaranFren:
				hdlSubmenu(MnuItm, mnuPembayaranThree); 
				break;
				
			case mnuPembayaranThree:
				hdlSubmenu(MnuItm, mnuPembayaranAxis);
				break;
				
			case mnuPembayaranAxis:
				hdlSubmenu(MnuItm, mnuPembayaranXl); 
				break;
       
			case mnuPembayaranXl:
				hdlSubmenu(MnuItm, mnuRegistrasi);
				break;    
				
			case mnuPembayaranDPLK:        
				hdlSubmenu(MnuItm, mnuIsiUlang);
				break;
				
			case mnuMReprint:
				hdlSubmenu(MnuItm, mnuMiniATMEnd);
				break;

			case mnuMReview:
				hdlSubmenu(MnuItm, mnuMReport);
				break;
				
			case mnuMrcInfo:
				hdlSubmenu(MnuItm, mnuMrcBatStan);
				break;
				
			case mnuMrcBatStan:
				hdlSubmenu(MnuItm, mnuMrcChgPwd);
				break;
				
			case mnuViewLog:
				hdlSubmenu(MnuItm, mnuTest);
				break;
				
			case mnuSIMBaru:
				hdlSubmenu(MnuItm, mnuSIMPanjang);
				break;
				
			case mnuSIMPanjang:
				hdlSubmenu(MnuItm, mnuSIMPeningkatan);
				break;
				
			case mnuSIMPeningkatan:
				hdlSubmenu(MnuItm, mnuSIMPenurunan);
				break;
				
			case mnuSIMPenurunan:
				hdlSubmenu(MnuItm, mnuSIMMutasi);
				break;
				
			case mnuSIMMutasi:
				hdlSubmenu(MnuItm, mnuSIMHilang);
				break;
				
			case mnuSIMHilang:
				hdlSubmenu(MnuItm, mnuBPKB);
				break;

			case mnuSIMMutasiGolSam:
				hdlSubmenu(MnuItm, mnuSIMMutasiPeningkatan);
				break;
				
			case mnuSIMMutasiPeningkatan:
				hdlSubmenu(MnuItm, mnuSIMMutasiPenurunan);
				break;
				
			case mnuSIMMutasiPenurunan:
				hdlSubmenu(MnuItm, mnuSIMHilang);
				break;

			case mnuSIMHilangGolSam:
				hdlSubmenu(MnuItm, mnuSIMHilangPeningkatan);
				break;
				
			case mnuSIMHilangPeningkatan:
				hdlSubmenu(MnuItm, mnuSIMHilangPenurunan);
				break;
				
			case mnuSIMHilangPenurunan:
				hdlSubmenu(MnuItm, mnuCustomer);
				break;

			case mnuBPKBBaru:
				hdlSubmenu(MnuItm, mnuBPKBGanti);
				break;
				
			case mnuBPKBGanti:
				hdlSubmenu(MnuItm, mnuCustomer);
				break;

			case mnuTMS:
				hdlSubmenu(MnuItm, mnuTerm);
				break;
				
			case mnuTerm:
				hdlSubmenu(MnuItm, mnuCmm);
				break;
			case mnuTerm2:
				hdlSubmenu(MnuItm, mnuTermAppli);
				break;
			case mnuCmm:
				hdlSubmenu(MnuItm, mnuFn99);
				break;

			case mnuCmm2:
				hdlSubmenu(MnuItm, mnuCmmCS);
				break;

#ifdef __TEST__

			case mnuTest:
				hdlSubmenu(MnuItm, mnuEnd);
				break;

			case mnuTestData:
				hdlSubmenu(MnuItm, mnuTestIso);
				break;
			case mnuTestIso:
				hdlSubmenu(MnuItm, mnuTestCmm);
				break;
			case mnuTestCmm:
				hdlSubmenu(MnuItm, mnuEnd);
				break;
			case mnuTestIsoRqs:
				hdlSubmenu(MnuItm, mnuTestIsoRsp);
				break;
			case mnuTestIsoRsp:
				hdlSubmenu(MnuItm, mnuEnd);
				break;
#endif

			case mnuCustomer:
				hdlSubmenu(MnuItm, mnuMerchant);
				break;

			case mnuMerchant:
				hdlSubmenu(MnuItm, mnuAdmin);
				break;

			case mnuAdmin:
				hdlSubmenu(MnuItm, mnuFn99); 
				break;
				
			case mnuFn99:
				hdlSubmenu(MnuItm, mnuFn99End);
				break;
						
			case mnuAktivasiKartu:
			case mnuRenewalKartu:
			case mnuReissuePIN:
			case mnuPIN:
			case mnuGantiPIN:
			case mnuGantiPasswordSPV:
			case mnuCLastTrx:
			case mnuCAnyTrx:
			case mnuCTodaySummary:
			case mnuCTodayDetail:
			case mnuCPastSummary:
			case mnuCPastDetail:
			case mnuSetorSimpanan:
			case mnuSetorPinjaman:
			case mnuTarikTunai:
			case mnuVoidTarikTunai:
			case mnuTLastTrx: 
			case mnuTAnyTrx:
			case mnuTTodaySummary:
			case mnuTTodayDetail:
			case mnuTPastSummary:
			case mnuTPastDetail:
			case mnuInfoSaldo:
			case mnuInfoSaldoBankLain:
			case mnuMiniStatement:
			case mnuMutRek:
			case mnuPrevilege:
			case mnuTransferSesamaBRI:
			case mnuTransferAntarBank:
			case mnuInfoKodeBank1:
			case mnuInfoKodeBank:
			case mnuPembayaranBriva:
			case mnuPembayaranTelkom:
			case mnuPembayaranDPLKR:
			case mnuPembayaranHalo:        
			case mnuPembayaranMatrix:
			case mnuPembayaranPLNPasca:
			case mnuPembayaranPLNPra:
			case mnuPembayaranPLNToken:
			case mnuPembayaranCicilanFIF:
			case mnuPembayaranCicilanBAF:
			case mnuPembayaranCicilanOTO:
			case mnuPembayaranCicilanFinansia:
			case mnuPembayaranCicilanVerena:
			case mnuPembayaranCicilanWOM:
			case mnuPembayaranTvInd:
			case mnuPembayaranTvOkTv:
			case mnuPembayaranTvTopTv:
			case mnuPembayaranTiketGaruda:
			case mnuPembayaranTiketLionAir:
			case mnuPembayaranTiketSriwijaya:
			case mnuPembayaranTiketMandala:
			case mnuPembayaranPdamSby:
			case mnuRegistrasiTrxTransfer:
			case mnuRegistrasiTrxPulsa:
			case mnuPembayaranSPP:
			case mnuPembayaranKodeUniv:
			case mnuPembayaranSimpati50:
			case mnuPembayaranSimpati100:
//			case mnuPembayaranSimpati150:
			case mnuPembayaranSimpati200:
			case mnuPembayaranSimpati300:
			case mnuPembayaranSimpati500:
			case mnuPembayaranSimpati1000:
			case mnuPembayaranMentari25:
			case mnuPembayaranMentari50:
			case mnuPembayaranMentari100:
			case mnuPembayaranMentari250:
			case mnuPembayaranMentari500:
			case mnuPembayaranMentari1000:
			case mnuPembayaranIm325:    
			case mnuPembayaranIm350:    
			case mnuPembayaranIm375:    
			case mnuPembayaranIm3100:   
			case mnuPembayaranIm3150:   
			case mnuPembayaranIm3200:   
			case mnuPembayaranIm3500:   
			case mnuPembayaranIm31000:
			case mnuPembayaranXl25: 
			case mnuPembayaranXl50: 
			case mnuPembayaranXl75: 
			case mnuPembayaranXl100: 
			case mnuPembayaranXl150: 
			case mnuPembayaranXl200: 
			case mnuPembayaranXl300: 
			case mnuPembayaranXl500: 
			case mnuPembayaranEsia25: 
			case mnuPembayaranEsia50: 
			case mnuPembayaranEsia75: 
			case mnuPembayaranEsia100: 
			case mnuPembayaranEsia150: 
			case mnuPembayaranEsia200:
			case mnuPembayaranSmart20: 
			case mnuPembayaranSmart25:
			case mnuPembayaranSmart50: 
			case mnuPembayaranSmart100:
			case mnuPembayaranSmart150: 
			case mnuPembayaranSmart200:
			case mnuPembayaranSmart300:
			case mnuPembayaranSmart500:

			case mnuPembayaranFren20:
			case mnuPembayaranFren25:
			case mnuPembayaranFren50:
			case mnuPembayaranFren100:
			case mnuPembayaranFren150:
			case mnuPembayaranFren200:
			case mnuPembayaranFren300:
			case mnuPembayaranFren500:
			case mnuPembayaranThree20:
			case mnuPembayaranThree50:
			case mnuPembayaranThree75:
			case mnuPembayaranThree100:
			case mnuPembayaranThree150:
			case mnuPembayaranThree300:
			case mnuPembayaranThree500:
			case mnuPembayaranAxis25:
			case mnuPembayaranAxis50:
			case mnuPembayaranAxis75:
			case mnuPembayaranAxis100:
			case mnuPembayaranAxis150:
			case mnuPembayaranAxis200:
			case mnuPembayaranAxis300:
			case mnuPembayaranKKBRI:
			case mnuPembayaranKKCITIKTA:
			case mnuPembayaranKKCITIKK:
			case mnuPembayaranKKStanCharKK:
			case mnuPembayaranKKStanCharKTA:
			case mnuPembayaranKKANZ:
			case mnuPembayaranKKHSBCKTA:
			case mnuPembayaranKKHSBCKK:
			case mnuPembayaranKKRBSKK:
			case mnuPembayaranKKRBSKTA:
			case mnuPembayaranZakatDhuafa:
			case mnuPembayaranZakatYBM: 
			case mnuPembayaranInfaqDhuafa:
			case mnuPembayaranInfaqYBM: 
			case mnuPembayaranDPLKDhuafa:
			case mnuPembayaranDPLKYBM: 
			case mnuCashIn:
			case mnuCashOut:
			case mnuRegistrasiInternetBanking:
			case mnuRegistrasiPhoneBanking:
			case mnuRegistrasiSmsBanking:
			case mnuMLastTrx:
			case mnuMAnyTrx:
			case mnuMVLastTrx:
			case mnuMVAnyTrx:
			case mnuMTodaySummary:
			case mnuMTodayDetail:
			case mnuMPastSummary:
			case mnuMPastDetail:
			case mnuSale:
			case mnuSaleRedeem:
			case mnuInstallment:
			case mnuRefund:
			case mnuPreaut:
			case mnuAut:
			case mnuVoid:
			case mnuOffline:
			case mnuSettlement:
			case mnuMrcChgPwd:
			case mnuMrcReset:
			case mnuFn99MrcReset:
			case mnuMrcResetRev:
			case mnuFn99MrcResetRev:
			case mnuDeleteCLog:
			case mnuDeleteMLog:
			case mnuDeleteTLog:
			case mnuViewCardService:
			case mnuViewMiniAtm:
			case mnuViewTunai:
			case mnuInitialize:
			case mnuSaldo:
			case mnuPrepaidInfoSaldo:
			case mnuInfoDeposit:
			case mnuPrepaidPayment:
			case mnuTopUpOnline:
			case mnuTopUpDeposit:
	//		case mnuPrepaidPembayaran:
			case mnuPrepaidRedeem:
			case mnuPrepaidPaymentDisc:
			case mnuPrepaidReaktivasi:
			case mnuPrepaidVoid:	
			case mnuAktivasiDeposit:
			case mnuPrepaidInfoKartu:
			case mnuPrepaidPrintLog:
			case mnuPrepaidSettlement:
			case mnuPRLastTrx:
			case mnuPRAnyTrx:
			case mnuPDateSummary:
			case mnuPAllSummary:
			case mnuPDateDetailAll:
			case mnuPAllDetailAll:
			case mnuPDateDetailPayment:
			case mnuPAllDetailPayment:
			case mnuDatang:
			case mnuPulangs:
			case mnuIstirahatA:
			case mnuIstirahatB:
			case mnuLemburA:
			case mnuLemburB:
			case mnuShiftsIA:
			case mnuShiftsIB:
			case mnuShiftsIIA:
			case mnuShiftsIIB:
			case mnuShiftsIIIA:
			case mnuShiftsIIIB:
			case mnuSIMBaruA :
			case mnuSIMBaruAU :
			case mnuSIMBaruB1 :
			case mnuSIMBaruB1U :
			case mnuSIMBaruB2 :
			case mnuSIMBaruB2U :
			case mnuSIMBaruC :
			case mnuSIMBaruD :
			case mnuSIMPanjangA :
			case mnuSIMPanjangAU :
			case mnuSIMPanjangB1 :
			case mnuSIMPanjangB1U :
			case mnuSIMPanjangB2 :
			case mnuSIMPanjangB2U :
			case mnuSIMPanjangC :
			case mnuSIMPanjangD :
			case mnuSIMPeningkatanAU :
			case mnuSIMPeningkatanB1 :
			case mnuSIMPeningkatanB1U :
			case mnuSIMPeningkatanB2 :
			case mnuSIMPeningkatanB2U :
			case mnuSIMPenurunanA :
			case mnuSIMPenurunanAU :
			case mnuSIMPenurunanB1 :
			case mnuSIMPenurunanB1U :
			case mnuSIMPenurunanB2 :
			case mnuSIMPenurunanB2U :
			case mnuSIMGolSamA :
			case mnuSIMGolSamAU :
			case mnuSIMGolSamB1 :
			case mnuSIMGolSamB1U :
			case mnuSIMGolSamB2 :
			case mnuSIMGolSamB2U :
			case mnuSIMGolSamC :
			case mnuSIMGolSamD :
			case mnuSIMMutasiPeningkatanAU :
			case mnuSIMMutasiPeningkatanB1 :
			case mnuSIMMutasiPeningkatanB1U :
			case mnuSIMMutasiPeningkatanB2 :
			case mnuSIMMutasiPeningkatanB2U :
			case mnuSIMMutasiPenurunanA :
			case mnuSIMMutasiPenurunanAU :
			case mnuSIMMutasiPenurunanB1 :
			case mnuSIMMutasiPenurunanB1U :
			case mnuSIMMutasiPenurunanB2 :
			case mnuSIMMutasiPenurunanB2U :
			case mnuSIMHilangA :
			case mnuSIMHilangAU :
			case mnuSIMHilangB1 :
			case mnuSIMHilangB1U :
			case mnuSIMHilangB2 :
			case mnuSIMHilangB2U :
			case mnuSIMHilangC :
			case mnuSIMHilangD :
			case mnuSIMHilangPeningkatanAU :
			case mnuSIMHilangPeningkatanB1 :
			case mnuSIMHilangPeningkatanB1U :
			case mnuSIMHilangPeningkatanB2 :
			case mnuSIMHilangPeningkatanB2U :
			case mnuSIMHilangPenurunanA :
			case mnuSIMHilangPenurunanAU :
			case mnuSIMHilangPenurunanB1 :
			case mnuSIMHilangPenurunanB1U :
			case mnuSIMHilangPenurunanB2 :
			case mnuUjiSim :
			case mnuBPKBBaruRoda2 :
			case mnuBPKBBaruRoda4 :
			case mnuBPKBGantiRoda2 :
			case mnuBPKBGantiRoda4 :
			case mnuSTCK :
			case mnuMutasiRanRoda2 :
			case mnuMutasiRanRoda4 :
			case mnuMrcInitRpt:
			case mnuMrcViewTot:
			case mnuMrcSWver:
			case mnuMrcViewRoc:
			case mnuMrcSetRoc:
			case mnuMrcViewBatNum:
			case mnuMrcSetBatNum:
			case mnuMrcViewSTAN:
			case mnuMrcSetSTAN:
			case mnuMrcViewBat:
			case mnuMrcViewTxn:
			case mnuMrcPrnTra:
			case mnuMrcReprint:
			case mnuMrcSetDup:
			case mnuMrcHostTot:
			case mnuMrcDetails:
			case mnuMrcBatTot:
			case mnuTMSLP:
			case mnuTMSVC:
			case mnuTMSPC:
			case mnuTMSConfig:
			case mnuTDT:
			case mnuTermAppli:
			case mnuTMKey:
			case mnuFn99TMKey: 
			case mnuEDCFunction:
			case mnuCmmVS:
			case mnuCmmPS:
			case mnuCmmLS:
			case mnuCmmGS:
			case mnuCmmCS:
			case mnuBalance:
			case mnuLogon: 
				//disableSubmenu();
				break;
				
			default:
				break;
		}
	}*/
	goto lblEnd;
lblKO:
lblEnd:
	;
}

static void hdlSubmenu(word msg1, word msg2)
{
	int ret;
	//byte sta;                   //menu state, it is (upper item)*10 + (current item) //@agmr - bugfix
	int sta;
	char lvl;                   //menu level
	char mnu[MNUMAX][dspW + 1]; //the final menu array prepared to mnuSelect
	char *ptr[MNUMAX];          //array of pointers to mnu items
	word idx[MNUMAX];           //the correspondent indices
	byte idxP;                  //index within ptr array
	word idxM;                  //take values from msg1..msg2
	char itm[lenMnu + 1];       //(menu level)+(access type)+(item name)+(ending zero)

	//    byte MnuPop;

	memset(ptr, 0, sizeof(ptr));
	MAPGET(msg1, itm, lblKO);   //it is the menu name
	VERIFY(strlen(itm) <= lenMnu + 1);
	strcpy(mnu[0], itm + 2);    //omit menu level and access type
	lvl = itm[0] + 1;

	idxP = 0;
	ptr[idxP++] = mnu[0];       //it is the menu name
	ret = nvmHold(0);
	for (idxM = msg1 + 1; idxM < msg2; idxM++)
	{    //build mnu and ptr arrays
		MAPGET(idxM, itm, lblKO);   //retrieve menu item
		if(itm[0] != lvl)
			continue;           //only items of level lvl are included
		VERIFY(strlen(itm) <= lenMnu + 1);
		strcpy(mnu[idxP], itm + 2); //the first and second characters are menu level and access type, it is not showed
		ptr[idxP] = mnu[idxP];  //fill the pointers array
		idx[idxP] = idxM;       //save the index to be returned if this item will be selected
		if(++idxP >= MNUMAX)    //no more than MNUMAX items in menu
			break;
	}
	nvmRelease(0);
	if(idxP == 1)
		goto lblEnd;            //empty menu - nothing to do

	sta = 0;                    //it is started with the current item 1 at the top of the screen
	do
	{
		ret = mnuSelect((Pchar *) ptr, sta, 60);    //perform user dialog
		if(ret <= 0)            //timeout or aborted - nothing to do
			break;
		VERIFY(ret > 0);
		VERIFY(ret < MNUMAX * MNUMAX);
		sta = ret;
		idxM = idx[sta % MNUMAX];   //sta%MNUMAX is the current item selected
		hdlSelect((word) (idxM));
		//        MAPGETBYTE(appMnuPop,MnuPop,lblKO);
		//        if(MnuPop) break;
	}
	while(sta);
	goto lblEnd;

lblKO:
lblEnd:
	;
}

/** Event received when an application has been selected from the Menu.
 * \header log\\log.h
 * \source log\\hdl.c 
 */
void hdlMainMenu(void)
{
	int ret;

	trcS("hdlMainMenu: Beg\n");
	MAPPUTWORD(traMnuItm, mnuMainMenu, lblKO);
	hdlSubmenu(mnuMainMenu, mnuEnd);
	trcS("hdlMainMenu: End\n");
lblKO:
	;
}

static int isNewSoftware(void)
{
	int ret;
	char dat[lenCmpDat + 1];
	char tim[lenCmpTim + 1];

	MAPGET(appCmpDat, dat, lblKO);
	VERIFY(ret <= lenCmpDat + 1);

	MAPGET(appCmpTim, tim, lblKO);
	VERIFY(ret <= lenCmpTim + 1);

	if((strcmp(dat, getCmpDat()) == 0) && (strcmp(tim, getCmpTim()) == 0))
		return 0;
	return 1;
lblKO:
	return -1;
}

static byte mapOK = 0;          //is mapInit called?
void hdlPowerOn(void)
{
    int ret;
    char key;
    char serialNum[20 + 1];     // length of NO_SERIE
    char serialNumTemp[20 + 1];     // length of NO_SERIE
    char sap[16 + 1];
    char dvc;
    byte binSerialNum[20 + 1];
    char tempBuf[25];
    byte res[5];

	trcS("hdlPowerOn: Beg\n");

	ret = mapInit();            //setup map tables
	CHECK(ret > 0, lblKO);
	mapOK = 1;

	traReset();

#ifdef FOR_SIMAS_NOT_USED
	mapGet(rptHdrSiteAddr3, tempBuf, 0);
	MAPPUTBYTE(appMQPSVEPSFlag, tempBuf[2],lblKO);
	//MAPPUTBYTE(appPINDebitFlag, tempBuf[3],lblKO);
	//MAPPUTBYTE(appPINCreditFlag, tempBuf[4],lblKO);
	MAPPUTSTR(appMQPSVEPSValue, tempBuf+5, lblKO);

	MAPGETBYTE(appMQPSVEPSFlag,res[0],lblKO);
	//MAPGETBYTE(appPINDebitFlag,res[1],lblKO);
	//MAPGETBYTE(appPINCreditFlag,res[2],lblKO);
#endif //FOR_SIMAS_NOT_USED

#ifndef REVERSAL_TERPISAH
    setBriReversalIdx(); //@agmr - BRI3
#endif

//    menuInit();// @agmr
    modifyMnuLayout(); //@agmr

	ret = isNewSoftware();
//ret = 0; // tests NANTI DITUTUP !!!
	if (ret == 0) {
		//@@AS0.0.26 - start
		resetLogonDateTime();
		setFirstPowerOnFlag(1);
		setFirstPowerOnTempFlag(1); //@@SIMAS-INIT_DB
		#ifdef _USE_ECR
		setECRFlag(0);
		#endif //_USE_ECR

//		if (IsICT220()) {
//			ttestall(0,500); //delay for 3 seconds
//		}
//		else {
//			ttestall(0,3000); //delay for 3 seconds
//		}
		debitLogon(0,1); //@@AS0.0.26
		//@@AS0.0.26 - end
		goto lblEnd; //it is negative if the DFS is not created; it is positive if compile time is not the same
	}
	dspClear();
	dspLS(1, "  NEW SOFTWARE  ");
	dspLS(2, "  MEMORY RESET  ");

	tmrStart(0, 3 * 100);
//    tmrStart(0, 10 * 100);
	kbdStart(1);
	key = 0;
	while(tmrGet(0))
	{          //3 seconds to cancel
		key = kbdKey();
		if(key != 0)
			break;
	}
	kbdStop();
	tmrStop(0);

	dspClear();
	if(key == kbdANN)
		return;                 //operation cancelled by the user

	dspLS(1, "  MEMORY RESET  ");
	dftReset(keyBeg);           //fill database by default values

	memset(serialNum, 0, sizeof(serialNum));
	memset(serialNumTemp, 0, sizeof(serialNum));
	memset(binSerialNum, 0, sizeof(binSerialNum));
	memset(sap, 0, sizeof(sap));
	memset(&dvc, 0, sizeof(dvc));
	
	getSapSer(sap, serialNum, dvc);
	
	mapGet(emvIFDSerNum, serialNumTemp, lenIFDSerNum+1);  
	memcpy(serialNumTemp+1,serialNum,lenIFDSerNum+1);
	ret = mapPut(emvIFDSerNum, serialNumTemp, lenIFDSerNum+1);
       CHECK(ret > 0, lblKO);
//NANTI BUKA
	mapInit();                  //re-initialise map tables
	traReset();

    modifyMnuLayout(); //@agmr

	resetLogonDateTime(); //@@AS0.0.26
	setFirstPowerOnFlag(1);//@@AS0.0.26
	#ifdef _USE_ECR
		setECRFlag(0);
#endif //_USE_ECR
		
#ifndef REVERSAL_TERPISAH
	setBriReversalIdx(); //@agmr - BRI3
#endif	

#ifdef LINE_ENCRYPTION
//    tleProcess(1); //@agmr - jika memory reset 
   //tleGetKey(); //@@AS-SIMAS , this is not required for Simas
#endif
    
	dspLS(2, "      DONE     ");
	//@@AS0.0.26 - start
//	if (IsICT220()) {
//		ttestall(0,300); //delay for 3 seconds
//	}
//	else {
//		ttestall(0,3000); //delay for 3 seconds
//	}
	debitLogon(0,1);
	//@@AS0.0.26 - end
//	usrInfo(infSearchingDisplay);

	goto lblEnd;
lblKO:
	//fatal error, the terminal is not in a working state
	//to be treated
lblEnd:
#ifdef PREPAID
	//autoSamInit(); //@agmr
#endif 	
#ifdef __EMV__

	emvInitAid();
#endif

	//activate cache buffers
	mapCache(mnuBeg);
	mapLoad(mnuBeg);

	mapCache(msgBeg);
	mapLoad(msgBeg);

	mapCache(scrBeg);

    batchSync(); //@agmr - multi settle
    
	trcS("hdlPowerOn: End\n");
}

/** Function called before treatment event received.
 *    -  0 : Event PowerOn received.
 *    - !0 : Other Event received.                    
 * \header log\\log.h
 * \source log\\hdl.c 
 */
void hdlBefore(void)
{
	nvmStart();
	//dspStart();
	prtStart();
	if(mapOK)
		traReset();
}

/** Function called after treatment event received.
 * \header log\\log.h
 * \source log\\hdl.c 
 */
void hdlAfter(void)
{
	dspStop();
	prtStop();
}

/** Function called to confirm if card can be treated.
 * \param    buf (I-) Mag card info.
 * \header log\\log.h
 * \source log\\hdl.c 
 */
int hdlAskMagStripe(const char *buf)
{
	int ret;
	char trk2[lenTrk2 + 1];

	VERIFY(buf);

	memset(trk2, 0, lenTrk2);
	memcpy(trk2, buf + 128, lenTrk2);

	MAPPUTSTR(traTrk2, trk2, lblKO);

	//validate the track2 acceptance
	//..

	MAPPUTBYTE(traEntMod, 'M', lblKO);  //means that the card can be treated by the application

	return 1;
lblKO:
	return 0;
}

static word getTraType(byte defTra)
{
	switch (defTra)
	{
		case 0:
			return mnuSale;
		case 1:
//			return mnuPreaut;
            return mnuAut; //@agmr - BRI5
		case 2:
			return mnuRefund;
		case 3:
			return mnuOffline;
		case 4:
			return mnuVoid;
		case 5:
			return mnuAdjust;
		case 6:
			return mnuSettlement;
		case 7:
			return mnuSaleRedeem;
		default:
			return mnuSale;
			break;
	}
	return 0;
}

static void hdlTraType(word traType)
{
	switch (traType)
	{
		case mnuSale:
			pmtSale();
			break;
		case mnuSaleRedeem:
			pmtSaleRedeem();
			break;
		case mnuRefund:
			pmtRefund();
			break;
		case mnuVoid:
			pmtVoid();
			break;
		case mnuSettlement:
			logSettlement();
			break;
		default:
			pmtSale();
			break;
	}
}

/** Event received when the Mag Card has been swiped.
 *  The function should return the acceptance level.
 * \param    buf (I-) Mag card info.
 * \return   
 *  The acceptance level of the card.  
 * \header log\\log.h
 * \source log\\hdl.c                                   
*/

void hdlMagStripe(const char *buf) {
    int ret;
    char trk2[lenTrk2 + 1];
    word MnuItm;
    byte defTra;

    VERIFY(buf);

    memset(trk2, 0, lenTrk2);
    memcpy(trk2, buf + 128, lenTrk2);
    MAPPUTSTR(traTrk2, trk2, lblKO);
    MAPPUTBYTE(traEntMod, 'M', lblKO);
    MAPGETBYTE(appDefTra, defTra, lblKO);

    MnuItm = getTraType(defTra);
    MAPPUTWORD(traMnuItm, MnuItm, lblKO);

    //CHECK(valCard() == 1, lblKO);
    hdlTraType(MnuItm);
    return;
  lblKO:
    ;
}
#if 0
void hdlMagStripe2(const char *buf) {
    int ret;
    char trk2[lenTrk2 + 1];
    word MnuItm;
    byte defTra;
    int count;
    byte ServiceCode;
    word inf = 0; //info screen to be displayed at the end
  

    VERIFY(buf);

    
    memset(trk2, 0, lenTrk2);
    memcpy(trk2, buf + 128, lenTrk2);
    MAPPUTSTR(traTrk2, trk2, lblKO);
    MAPPUTBYTE(traEntMod, 'M', lblKO);
    MAPGETBYTE(appDefTra, defTra, lblKO);

    MAPPUTBYTE(appFallBackMod, 0, lblKO);
    MAPGETBYTE(appServiceCode, ServiceCode, lblKO);

    if(ServiceCode == 1){	
		for(count = 0; count < 20; count++){
	      		if(buf[128 + count] == 0x3D)
		  		break;
	       }

		if( (buf[128+count+5] == 0x32 || buf[128+count+5] == 0x36)){
		       if ((EMVFallbackFlagSet != 1) || (EMV_ENTRYMODE == 1)){
			   	EMV_ENTRYMODE = 0;
		  	       dspLS(2, "CHIP CARD PLS");
				inf = infEMVChipDetect;
		  	       goto lblKO;
		       }
			else{
				  MAPPUTBYTE(appFallBackMod, 1, lblExit);
				  EMV_ENTRYMODE = 1;
				  EMVFallbackFlagSet = 0;
				  MnuItm = getTraType(defTra);
    				  MAPPUTWORD(traMnuItm, MnuItm, lblKO);
				  hdlTraType(MnuItm);
			}
	       }
		else{
			     MnuItm = getTraType(defTra);
    	                   MAPPUTWORD(traMnuItm, MnuItm, lblKO);
	                   hdlTraType(MnuItm);			
		}
    	}
		
   else{
    	MnuItm = getTraType(defTra);
    	MAPPUTWORD(traMnuItm, MnuItm, lblKO);
	hdlTraType(MnuItm);
   }
    return;

  lblExit:
  	;
  lblKO:
  	 if(inf)
        usrInfo(inf);
    ;
}
#endif

// for EMV applications
/** Event received when the Smart Card has been inserted. 
 *  The function should return the acceptance level.
 * \param    buf (I-) Smart card info.
 * \return   
 *  The acceptance level of the card.  
 * \header log\\log.h
 * \source log\\hdl.c                                   
 */
void hdlSmartCard(const byte * buf) {
#ifdef __EMV__
    int ret;
    byte aid[1 + 16];
    byte fsBuf[__FSBUFLEN__];
    word MnuItm = 0;
    byte defTra;

    VERIFY(buf);


    //ret= isInitialised();
    //if(ret!=1) return;

    memcpy(fsBuf, buf, __FSBUFLEN__);
    ret = emvSetSelectResponse(aid, fsBuf);
    CHECK(ret >= 0, lblKO);

    emvSetFSbuf(fsBuf);         //set the buffer to hold the last APPLICATION select command

#ifdef __TELIUM__
    ret = emvFinalSelect((byte *) aid);  // Initiate the FinalSelect from the candidate application chosen
    switch (ret) {
      case (-eEmvErrReselect):
                  case (-eEmvErrFallback):
                              goto lblFallBack;
      default:
          break;
    }
//                mapPut(emvAid, (byte *)aid,lenAID);
//                CHECK(ret >= 0, lblKO);
#endif


    // Tambahan, untuk mendapatkan AID dalam BIT 55
    //-------------------------------------------
	ret = mapPut(emvDFNam, aid, 1+lenDFNam);
        CHECK(ret >= 0, lblKO);
    //-------------------------------------------

    ret = emvMapAidMove(aid);
    CHECK(ret >= 0, lblKO);

    MAPPUTBYTE(traEntMod, 'C', lblKO);
    MAPGETBYTE(appDefTra, defTra, lblKO);

    MnuItm = getTraType(defTra);
    MAPPUTWORD(traMnuItm, MnuItm, lblKO);

   // emvCallMultiCurr();
   //dspClear();

    hdlTraType(MnuItm);
    return;
	
lblFallBack:
	;
  lblKO:
    ;
#endif
}

static int checkTransLog(void)
{
	word count, total = 0;
	int ret;

	MAPGETCARD(totAppSaleCnt, count, lblKO);
	total += count;

	MAPGETCARD(totAppRefundCnt, count, lblKO);
	total += count;

	MAPGETCARD(totAppOfflineCnt, count, lblKO);
	total += count;

	MAPGETCARD(totAppManualCnt, count, lblKO);
	total += count;

	MAPGETCARD(totAppVoidCnt, count, lblKO);
	total += count;

	CHECK(total > 0, lblKO);
	return 1;
lblKO:
	return -1;
}

/** Event received when a scheduled event is triggered.
 * \return      
 * \header log\\log.h
 * \source log\\hdl.c 
 */
void hdlWakeUp(void)
{
	//wake up event processing
	//This function is called once per minute
	int ret;
	char cur[lenDatTim + 1];    //current date time
	char cls[lenEndTime + 1];   //close day time
	card h1, h2, m1, m2, s1;
	byte doit;
	word LogIdx;
	//char  tid[9];
	//int     tempTime;

	//ret = MAPGET(acqTID,tid,lblKO);
	ret = mapGet(traDatTim, cur, lenDatTim + 1);
	CHECK(ret > 0, lblKO);
	ret = mapGet(appEndTime, cls, lenEndTime + 1);
	CHECK(ret > 0, lblKO);
	
	dec2num(&h1, cur + 8, 2);   //h1 = current hour
	dec2num(&m1, cur + 10, 2);  //m1 = current minute
	dec2num(&s1, cur + 12, 2);  //s1 = second minute
	dec2num(&h2, cls + 0, 2);   //h2 = close hour
	dec2num(&m2, cls + 2, 2);   //m2 = close minute

	if(h2 > 23)
		return;                 //no automatic close day

	doit = 0;
	if(h1 == h2)
	{
		if(m1 >= m2)
			doit = 1;
	}
	if(h1 == 0)
		h1 = 24;
	if(h1 == h2 + 1)
	{
		if(m1 < m2)
			doit = 1;
	}
	if(!doit)
		return;                 //it is not time to do close day

	//is there something in transaction log?
	LogIdx = checkTransLog();
	CHECK(LogIdx == 1, lblKO);

	ret = mapPutWord(traMnuItm, mnuSettlement);
	CHECK(ret >= 0, lblKO);
	logSettlement();

	return;
lblKO:
	;
}

int hdlTimer(card id, card * date, card * time)
{

#ifndef __TELIUM__
	char hhmmss[6 + 1];
	card hh, mm, ss;

	VERIFY(date);
	VERIFY(time);

	hdlWakeUp();                //perform treatment

	tim2asc(hhmmss, *time);
	dec2num(&ss, hhmmss + 4, 2);
	VERIFY(ss < 60);
	dec2num(&mm, hhmmss + 2, 2);
	VERIFY(mm < 60);
	dec2num(&hh, hhmmss + 0, 2);
	VERIFY(hh < 24);

	mm++;                       //now re-schedule it to one minute later
	if(mm >= 60)
	{
		mm = 0;
		hh++;
	}
	if(hh >= 24)
	{
		hh = 0;
	}
	*date = 0xFFFFFFFFL;

	num2dec(hhmmss + 0, hh, 2);
	num2dec(hhmmss + 2, mm, 2);
	num2dec(hhmmss + 4, ss, 2);

	asc2tim(time, hhmmss);
#endif

return 0;                   //0 means without re-scheduling, 1 means with re-schrduling
}

void hex_dump_char(const char *pzTitle, const byte *buff, long len)
{
	card x, y;
	char buffer[ISO_BUFFER_LEN];
	char buffer2[ISO_BUFFER_LEN];
	char szBuff[32+1];
	char szBuffTemp[ISO_BUFFER_LEN];
	char ascii[16+1];
	byte start;
	int slen;
	int pos;
	byte *buffdata;
	int counter;		// used to count line number

	start = 0;
	counter = 0;
	pos = 0;

	MEM_ZERO(buffer);
	MEM_ZERO(ascii);
	MEM_ZERO(szBuff);
	MEM_ZERO(szBuffTemp);
	
	// copy the data to another memory
	buffdata = (byte *)buff;

	// print title, len and memory address first
 	prtES(__COMPRESSED__, pzTitle);
	sprintf(szBuff, "Len:%d\t\t\t\tAddr:%p", (int)len, buff);
	prtES(__COMPRESSED__, szBuff);

	// if there is no data in the buffer
	// memory is empty
	if (len == 0 || !buffdata) return;

	for (x=0; x<len; x++)
	{
		if (start == 0) // first column w/ line number
		{
			sprintf(&buffer[0], "%02d | %02X ", counter, *buffdata);
			counter++;
		}
		else // next column
		{
			slen = strlen(buffer);
			sprintf(&buffer[slen], "%02X ", *buffdata);
		}

		// get ascii character
		sprintf(&ascii[pos], "%c", ((*buffdata <= 0x1f || *buffdata > 0x7f) ? '.' : *buffdata));

		// next byte
		*buffdata++;
		pos++;

		if (start == 9)
		{
			start = 0;
			pos = 0;

			sprintf(szBuffTemp, "%s %s", buffer, ascii);
			prtES(__COMPRESSED__, szBuffTemp);

			MEM_ZERO(buffer);
			MEM_ZERO(ascii);			
			MEM_ZERO(szBuffTemp);
		}
		else
			start++;
	}

	if (start <= 9)
	{
		MEM_ZERO(buffer2);
		MEM_ZERO(szBuffTemp);

		memcpy(buffer2, buffer, strlen(buffer));
		for (y = 0; y < (35 - strlen(buffer2)); y++)
		{
			slen = strlen(buffer);
			sprintf(&buffer[slen], "%s", " "); // fill the empty column with space
		}
		
		sprintf(szBuffTemp, "%s %s", buffer, ascii);
		prtES(__COMPRESSED__, szBuffTemp); // print last bytes
	}

	tmrPause(1);
}

void FnCommunication()
{// Setting Communication
//	int ret;

	trcS("hdlMainMenu: Beg\n");
	//hdlSubmenu(mnuCmm, mnuCmmCS);
	hdlSelect(mnuCmm2);
	trcS("hdlMainMenu: End\n");
}

void FnConnSetting()
{// Setting Communication
//	int ret;

	trcS("hdlMainMenu: Beg\n");
	hdlSelect(mnuCmmCS);
	trcS("hdlMainMenu: End\n");
}

void Fn99()
{// Setting Communication
//	int ret;

	trcS("hdlMainMenu: Beg\n");
	hdlSelect(mnuFn99);
	trcS("hdlMainMenu: End\n");
}

//++BRI7
void Fn100()
{
    mapPutStr(mnuMerchant,"1 MERCHANT>       ");
    hdlSelect(mnuMerchant);
    mapPutStr(mnuMerchant,"? MERCHANT>       ");
}
//--BRI7

void FnDisableMenu()
{
	int ret = 1;
	
	MAPPUTBYTE(appIsDisable, 1,lblKO);
	//modifySubMnuLayout();
	hdlSubmenu(mnuMainMenu, mnuEnd);
	MAPPUTBYTE(appIsDisable, 0,lblKO);
	//checkSubMnuLayout();
	lblKO:
    	;
}

void FnSetMQPSVEPS()
{
	#ifdef _USE_ECR
		KonfirmasiSetMQPSVEPS();
	#endif
}

void FnSetPIN()
{
	#ifdef _USE_ECR
		KonfirmasiSetPIN();
	#endif
}


void FnSettingTID()
{
	konfirmasiSettingTIDMID();
}

void cetakVersion()
{
	printVersion();
}
#ifdef _USE_ECR
int FnSales(void)
{
	int ret = 1;
	
	trcS("hdlMainMenu: Beg\n");
	if(KonfirmasiPembayaranECR())
		hdlSelect(mnuSale);
	trcS("hdlMainMenu: End\n");

	return ret;
}

int FnPrepaid(void)
{
	int ret = 1;
	
	trcS("hdlMainMenu: Beg\n");
	if(KonfirmasiPembayaranECR())
		hdlSelect(mnuPrepaidPayment);
	trcS("hdlMainMenu: End\n");

	return ret;
}

int FnTopUp(void)
{
	int ret = 1;
	
	trcS("hdlMainMenu: Beg\n");
	if(KonfirmasiPembayaranECR())
		hdlSelect(mnuTopUpOnline);
	trcS("hdlMainMenu: End\n");

	return ret;
}



int FnPrintRef(void)
{
	int ret = 1;
	
	trcS("hdlMainMenu: Beg\n");
	hdlSelect(mnuPOS);
	trcS("hdlMainMenu: End\n");

	return ret;
}

int FnshowOption(void)
{
	int ret = 1;

	trcS("FnshowOption: Beg\n");
	KonfirmasiShowOption();
	trcS("FnshowOption: End\n");

	return ret;
}

int FnPrinthis(void)
{
	int ret = 1;
	
	trcS("hdlMainMenu: Beg\n");
	hdlSelect(mnuMLastTrx);
	trcS("hdlMainMenu: End\n");

	return ret;
}
#endif //_USE_ECR


void FnTMS()
{// Setting Communication
//	int ret;

	trcS("hdlMainMenu: Beg\n");
	hdlSelect(mnuTMS);
	trcS("hdlMainMenu: End\n");
}

void  FnSetMin()  // adrian_rx
{

	
	trcS("hdlMainMenu: Beg\n");
	hdlSelect(mnuSaldo);
	trcS("hdlMainMenu: End\n");
}


void FnTerminal()
{// Setting Terminal (baru setting date aja)
//	int ret;

	trcS("hdlMainMenu: Beg\n");
	hdlSelect(mnuTerm2);
	trcS("hdlMainMenu: End\n");
}

void viewLog()
{
	hdlSelect(mnuViewLog);
}
