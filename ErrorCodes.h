
#ifndef ERROR_NONE

#define ERROR_NONE 				0

// ERRROR numbers less than 0 are NOT trappable
// and are fatal at runtime
#define ERROR_NOSUCHLABEL		-1
#define ERROR_NEXTNOFOR 		-2
#define ERROR_NOTARRAY 			-3
#define ERROR_ARGUMENTCOUNT 	-5
#define ERROR_MAXRECURSE 		-6
#define ERROR_STACKUNDERFLOW	-7


// ERRRORS and WARNINGS greater than 0 are trappable
// with the "ONERR" and TRY/CATCH/ENDTRY statements
// 
// ERRORS from 1-65535 are trapable but by default cause execution to stop
// WARNINGS 65536+ are trapable but execution will continue by default
//
// trapable ERRORS
#define ERROR_FOR1 						2
#define ERROR_FOR2 						3
#define ERROR_FILENUMBER 				5
#define ERROR_FILEOPEN 					6
#define ERROR_FILENOTOPEN 				7
#define ERROR_FILEWRITE 				8
#define ERROR_FILERESET 				9
#define ERROR_ARRAYSIZELARGE 			10
#define ERROR_ARRAYSIZESMALL 			11
#define ERROR_NOSUCHVARIABLE 			12
#define ERROR_VARNOTASSIGNED 			13
#define ERROR_ARRAYINDEX 				15
#define ERROR_STRNEGLEN 				16
#define ERROR_STRSTART 					17
#define ERROR_NONNUMERIC 				19
#define ERROR_RGB 						20
#define ERROR_PUTBITFORMAT 				21
#define ERROR_POLYARRAY 				22
#define ERROR_POLYPOINTS 				23
#define ERROR_IMAGEFILE 				24
#define ERROR_SPRITENUMBER 				25
#define ERROR_SPRITENA 					26
#define ERROR_SPRITESLICE 				27
#define ERROR_FOLDER 					28
#define ERROR_INFINITY 					29
#define ERROR_DBOPEN 					30
#define ERROR_DBQUERY 					31
#define ERROR_DBNOTOPEN 				32
#define ERROR_DBCOLNO 					33
#define ERROR_DBNOTSET 					34
#define ERROR_TYPECONV					35
#define ERROR_NETSOCK 					36
#define ERROR_NETHOST 					37
#define ERROR_NETCONN 					38
#define ERROR_NETREAD 					39
#define ERROR_NETNONE 					40
#define ERROR_NETWRITE 					41
#define ERROR_NETSOCKOPT 				42
#define ERROR_NETBIND 					43
#define ERROR_NETACCEPT 				44
#define ERROR_NETSOCKNUMBER 			45
#define ERROR_PERMISSION 				46
#define ERROR_IMAGESAVETYPE 			47
#define ERROR_DIVZERO 					50
#define ERROR_BYREF 					51
#define ERROR_BYREFTYPE 				52
#define ERROR_FREEFILE 					53
#define ERROR_FREENET 					54
#define ERROR_FREEDB 					55
#define ERROR_DBCONNNUMBER 				56
#define ERROR_FREEDBSET 				57
#define ERROR_DBSETNUMBER 				58
#define ERROR_DBNOTSETROW 				59
#define ERROR_PENWIDTH 					60
#define ERROR_COLORNUMBER 				61
#define ERROR_ARRAYINDEXMISSING			62
#define ERROR_IMAGESCALE 				63
#define ERROR_FONTSIZE 					64
#define ERROR_FONTWEIGHT 				65
#define ERROR_RADIXSTRING 				66
#define ERROR_RADIX 					67
#define ERROR_LOGRANGE 					68
#define ERROR_STRINGMAXLEN 				69
#define ERROR_NOTANUMBER				70
#define ERROR_PRINTERNOTON				71
#define ERROR_PRINTERNOTOFF				72
#define ERROR_PRINTEROPEN				73
#define ERROR_WAVFILEFORMAT				74
#define ERROR_WAVNOTOPEN				75
#define ERROR_WAVNOTSEEKABLE			76
#define ERROR_WAVNODURATION				77
#define ERROR_FILEOPERATION				78
#define ERROR_SERIALPARAMETER			79
#define ERROR_LONGRANGE					80

// Insert new error codes here


#define ERROR_NOTIMPLEMENTED 			65535


// trapable WaRNINGS (same as error with warning bit set)
#define WARNING_START					65536
#define WARNING_TYPECONV				WARNING_START + ERROR_TYPECONV
#define WARNING_WAVNOTSEEKABLE			WARNING_START + ERROR_WAVNOTSEEKABLE
#define WARNING_WAVNODURATION			WARNING_START + ERROR_WAVNODURATION
#define WARNING_VARNOTASSIGNED 			WARNING_START + ERROR_VARNOTASSIGNED
#define WARNING_LONGRANGE 				WARNING_START + ERROR_LONGRANGE


//
#endif
