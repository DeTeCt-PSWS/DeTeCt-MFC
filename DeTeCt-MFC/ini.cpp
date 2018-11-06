#include "ini.h"
#include <iostream>

void update_options(std::string filename) {
	std::cout << filename << std::endl;
	TCHAR outbuf[1000];
	opts.filename = opts.ofilename = opts.darkfilename = opts.ovfname = opts.sfname = (char *) GetPrivateProfileString(L"all", L"filename", L"", outbuf, sizeof(outbuf) / sizeof(outbuf[0]), L".\\options.ini");
	opts.nsaveframe = GetPrivateProfileString(L"all", L"nsaveframe", L"0", outbuf, 32, L".\\options.ini");
	//opts.ostype = OTYPE_ADU;
	opts.ostype = OTYPE_NO;
	opts.ovtype = OTYPE_NO;
	std::cout << opts.timeImpact << std::endl;
	opts.timeImpact = GetPrivateProfileInt(L"all", L"timeImpact", 4, L".\\options.ini");
	std::cout << opts.timeImpact << std::endl;
	opts.incrLumImpact = (double) GetPrivateProfileInt(L"all", L"incrLumImpact", 0.9, L".\\options.ini");
	std::cout << opts.incrLumImpact << std::endl;
	opts.incrFrameImpact = GetPrivateProfileString(L"all", L"incrFrameImpact", L"10", outbuf, sizeof(outbuf) / sizeof(outbuf[0]), L".\\options.ini");
	std::cout << opts.incrLumImpact << std::endl;
	opts.radius = GetPrivateProfileString(L"all", L"radius", L"10", outbuf, sizeof(outbuf) / sizeof(outbuf[0]), L".\\options.ini");
	opts.nframesROI = GetPrivateProfileString(L"all", L"nframesROI", L"1", outbuf, sizeof(outbuf) / sizeof(outbuf[0]), L".\\options.ini");
	opts.bayer = GetPrivateProfileString(L"all", L"bayer", L"-1", outbuf, sizeof(outbuf) / sizeof(outbuf[0]), L".\\options.ini");
	opts.medSize = GetPrivateProfileString(L"all", L"medSize", L"5", outbuf, sizeof(outbuf) / sizeof(outbuf[0]), L".\\options.ini");
	opts.wait = GetPrivateProfileString(L"all", L"wait", L"1", outbuf, sizeof(outbuf) / sizeof(outbuf[0]), L".\\options.ini");
	opts.facSize = GetPrivateProfileString(L"all", L"facSize", L"0.9", outbuf, sizeof(outbuf) / sizeof(outbuf[0]), L".\\options.ini");
	opts.secSize = GetPrivateProfileString(L"all", L"secSize", L"1.05", outbuf, sizeof(outbuf) / sizeof(outbuf[0]), L".\\options.ini");
	opts.threshold = GetPrivateProfileString(L"all", L"threshold", L"30", outbuf, sizeof(outbuf) / sizeof(outbuf[0]), L".\\options.ini");
	opts.learningRate = GetPrivateProfileString(L"all", L"learningRate", L"0.8", outbuf, sizeof(outbuf) / sizeof(outbuf[0]), L".\\options.ini");
	opts.thrWithMask = GetPrivateProfileString(L"all", L"thrWithMask", L"0", outbuf, sizeof(outbuf) / sizeof(outbuf[0]), L".\\options.ini");
	opts.histScale = GetPrivateProfileString(L"all", L"histScale", L"0.8", outbuf, sizeof(outbuf) / sizeof(outbuf[0]), L".\\options.ini");
	opts.viewROI = GetPrivateProfileString(L"all", L"viewROI", L"0", outbuf, sizeof(outbuf) / sizeof(outbuf[0]), L".\\options.ini");
	opts.viewTrk = GetPrivateProfileString(L"all", L"viewTrk", L"0", outbuf, sizeof(outbuf) / sizeof(outbuf[0]), L".\\options.ini");
	opts.viewDif = GetPrivateProfileString(L"all", L"viewDif", L"0", outbuf, sizeof(outbuf) / sizeof(outbuf[0]), L".\\options.ini");
	opts.viewRef = GetPrivateProfileString(L"all", L"viewRef", L"0", outbuf, sizeof(outbuf) / sizeof(outbuf[0]), L".\\options.ini");
	opts.viewThr = GetPrivateProfileString(L"all", L"viewThr", L"0", outbuf, sizeof(outbuf) / sizeof(outbuf[0]), L".\\options.ini");
	opts.viewSmo = GetPrivateProfileString(L"all", L"viewSmo", L"0", outbuf, sizeof(outbuf) / sizeof(outbuf[0]), L".\\options.ini");
	opts.viewRes = GetPrivateProfileString(L"all", L"viewRes", L"0", outbuf, sizeof(outbuf) / sizeof(outbuf[0]), L".\\options.ini");
	opts.viewHis = GetPrivateProfileString(L"all", L"viewHis", L"0", outbuf, sizeof(outbuf) / sizeof(outbuf[0]), L".\\options.ini");
	opts.viewMsk = GetPrivateProfileString(L"all", L"viewMsk", L"0", outbuf, sizeof(outbuf) / sizeof(outbuf[0]), L".\\options.ini");
	opts.verbose = GetPrivateProfileString(L"all", L"verbose", L"0", outbuf, sizeof(outbuf) / sizeof(outbuf[0]), L".\\options.ini");
	opts.filter.type = GetPrivateProfileString(L"all", L"filterType", L"-1", outbuf, sizeof(outbuf) / sizeof(outbuf[0]), L".\\options.ini");
	opts.filter.param[0] = GetPrivateProfileString(L"all", L"filterParam0", L"3", outbuf, sizeof(outbuf) / sizeof(outbuf[0]), L".\\options.ini");
	opts.filter.param[1] = GetPrivateProfileString(L"all", L"filterParam1", L"3", outbuf, sizeof(outbuf) / sizeof(outbuf[0]), L".\\options.ini");
	opts.filter.param[2] = GetPrivateProfileString(L"all", L"filterParam2", L"0", outbuf, sizeof(outbuf) / sizeof(outbuf[0]), L".\\options.ini");
	opts.filter.param[3] = GetPrivateProfileString(L"all", L"filterParam3", L"0", outbuf, sizeof(outbuf) / sizeof(outbuf[0]), L".\\options.ini");
	opts.debug = GetPrivateProfileString(L"all", L"debug", L"0", outbuf, sizeof(outbuf) / sizeof(outbuf[0]), L".\\options.ini");
	opts.ADUdtconly = GetPrivateProfileString(L"all", L"ADUdtconly", L"0", outbuf, sizeof(outbuf) / sizeof(outbuf[0]), L".\\options.ini");
	opts.detail = GetPrivateProfileString(L"all", L"detail", L"0", outbuf, sizeof(outbuf) / sizeof(outbuf[0]), L".\\options.ini");
	opts.allframes = GetPrivateProfileString(L"all", L"allframes", L"0", outbuf, sizeof(outbuf) / sizeof(outbuf[0]), L".\\options.ini");
	opts.minframes = GetPrivateProfileString(L"all", L"minframes", L"3", outbuf, sizeof(outbuf) / sizeof(outbuf[0]), L".\\options.ini");
	opts.dateonly = GetPrivateProfileString(L"all", L"dateonly", L"0", outbuf, sizeof(outbuf) / sizeof(outbuf[0]), L".\\options.ini");
	opts.ignore = GetPrivateProfileString(L"all", L"ignore", L"0", outbuf, sizeof(outbuf) / sizeof(outbuf[0]), L".\\options.ini");
	opts.videotest = GetPrivateProfileString(L"all", L"videotest", L"0", outbuf, sizeof(outbuf) / sizeof(outbuf[0]), L".\\options.ini");
	opts.wROI = GetPrivateProfileString(L"all", L"wROI", L"0", outbuf, sizeof(outbuf) / sizeof(outbuf[0]), L".\\options.ini");
	opts.hROI = GetPrivateProfileString(L"all", L"hROI", L"0", outbuf, sizeof(outbuf) / sizeof(outbuf[0]), L".\\options.ini");
}