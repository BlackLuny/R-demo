#pragma once
#include"preinclude.h"
#include"TradeDataType.h"
#include"ThostFtdcUserApiStruct.h"
//��������ˢ�½ṹ
typedef struct
{
	//TODO:modify here, use InstrumentID and LastPrice only
	CThostFtdcDepthMarketDataField depth_market_data;
	Direction direction;
}RefreshQuote;
