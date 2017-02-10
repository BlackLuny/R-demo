#pragma once
#include"FutureQuoteProcessorForCache.h"
#include"FutureQuoteFeedForCache.h"

FutureQuoteProcessorForCache::FutureQuoteProcessorForCache(FutureQuoteFeedForCache* _quote_feed, BrokerInfo* _broker_info, InvestAccountSync* _invest_account, unsigned int _index) :
	FutureQuoteProcessor(_broker_info, _invest_account, _index)
{
	quote_feed = _quote_feed;
}

///���ͻ����뽻�׺�̨������ͨ������ʱ����δ��¼ǰ�����÷��������á�
void FutureQuoteProcessorForCache::OnFrontConnected()
{
	CThostFtdcReqUserLoginField field;
	::ZeroMemory(&field, sizeof(field));
	strcpy_s(field.BrokerID, ARRAYCOUNT(field.BrokerID), broker_info->broker_id.c_str());
	strcpy_s(field.UserID, ARRAYCOUNT(field.UserID), invest_account->user_id.c_str());
	strcpy_s(field.Password, ARRAYCOUNT(field.Password), invest_account->investor_id.c_str());
	strcpy_s(field.UserProductInfo, ARRAYCOUNT(field.UserProductInfo), "EasyTrade2");
	switch (quote_feed->GetMdApiInstance((size_t)index)->ReqUserLogin(&field, ++invest_account->ctp_quote_request_id))
	{
	case 0:
		break;
	case -1:
	{
		msg = "failed to log in " + broker_info->broker_name + " ctp quote server. network connection failed";
		(*quote_feed->handler)(msg.c_str());
	}
	break;
	case -2:
	{
		msg = "failed to log in " + broker_info->broker_name + " ctp quote server. unprocessed requests exceed limit";
		(*quote_feed->handler)(msg.c_str());
	}
	break;
	case -3:
	{
		msg = "failed to log in " + broker_info->broker_name + " ctp quote server. requests sent per second exceed limit";
		(*quote_feed->handler)(msg.c_str());
	}
	break;
	default:
		break;
	}
}

///���ͻ����뽻�׺�̨ͨ�����ӶϿ�ʱ���÷��������á���������������API���Զ��������ӣ��ͻ��˿ɲ�������
///@param nReason ����ԭ��
///        0x1001 �����ʧ��
///        0x1002 ����дʧ��
///        0x2001 ����������ʱ
///        0x2002 ��������ʧ��
///        0x2003 �յ�������
///this function will be invoked only when there is a created connection 
void FutureQuoteProcessorForCache::OnFrontDisconnected(int nReason)
{
	switch (nReason)
	{
	case 0x1001:
	{
		msg = "failed to keep connection to " + broker_info->broker_name + " ctp quote server. failed to read on network";
		(*quote_feed->handler)(msg.c_str());
	}
	break;
	case 0x1002:
	{
		msg = "failed to keep connection to " + broker_info->broker_name + " ctp quote server. failed to write on network";
		(*quote_feed->handler)(msg.c_str());
	}
	break;
	case 0x2001:
	{
		msg = "failed to keep connection to " + broker_info->broker_name + " ctp quote server. receive heartbeat timeout";
		(*quote_feed->handler)(msg.c_str());
	}
	break;
	case 0x2002:
	{
		msg = "failed to keep connection to " + broker_info->broker_name + " ctp quote server. send heartbeat failed";
		(*quote_feed->handler)(msg.c_str());
	}
	break;
	case 0x2003:
	{
		msg = "failed to keep connection to " + broker_info->broker_name + " ctp quote server. received errorneous packets";
		(*quote_feed->handler)(msg.c_str());
	}
	break;
	default:
		break;
	}
}

///��¼������Ӧ
void FutureQuoteProcessorForCache::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (IsErrorRspInfo(pRspInfo))
	{
		msg = "failed to log in " + broker_info->broker_name + " CTP future quote server";
		CTPShared::FormatRspInfo(pRspInfo, &msg);
		(*quote_feed->handler)(msg.c_str());
	}
	else
	{
		//TODO:no need to write succeed message to disk log file
		msg = "succeeded in logging in " + broker_info->broker_name + " ctp quote server";
		(*quote_feed->handler)(msg.c_str());
		if (index == 0)
		{
			::SetEvent(*quote_feed->event);
		}
	}
}

///����Ӧ��
void FutureQuoteProcessorForCache::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (IsErrorRspInfo(pRspInfo))
	{
		msg = "you've passed errorneous arguments to CTP functions";
		CTPShared::FormatRspInfo(pRspInfo, &msg);
		(*quote_feed->handler)(msg.c_str());
	}
}

///��������Ӧ��
void FutureQuoteProcessorForCache::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (IsErrorRspInfo(pRspInfo))
	{
		FormatRspInfo(pRspInfo, pSpecificInstrument, SUBSCRIBE, &msg);
		(*quote_feed->handler)(msg.c_str());
	}
}

///ȡ����������Ӧ��
void FutureQuoteProcessorForCache::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (IsErrorRspInfo(pRspInfo))
	{
		FormatRspInfo(pRspInfo, pSpecificInstrument, UNSUBSCRIBE, &msg);
		(*quote_feed->handler)(msg.c_str());
	}
}

///�������֪ͨ
void FutureQuoteProcessorForCache::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	if (quote_feed->tick_cache_index == 1)
	{
		if (pDepthMarketData->LastPrice < pDepthMarketData->UpperLimitPrice)
		{
			quote_feed->market_data_vec1.push_back(*pDepthMarketData);
			if (quote_feed->market_data_vec1.size() == 1000)
			{
				quote_feed->tick_cache_index = 2;
				::SetEvent(*quote_feed->event);
			}
		}
	}
	else if (quote_feed->tick_cache_index == 2)
	{
		if (pDepthMarketData->LastPrice < pDepthMarketData->UpperLimitPrice)
		{
			quote_feed->market_data_vec2.push_back(*pDepthMarketData);
			if (quote_feed->market_data_vec1.size() == 1000)
			{
				quote_feed->tick_cache_index = 1;
				::SetEvent(*quote_feed->event);
			}
		}
	}
}
