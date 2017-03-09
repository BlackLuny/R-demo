#pragma once
#include"Tickache.h"
#include"FutureQuoteFeedForCache.h"
#include"Utilities.h"

Tickache* Tickache::local_tickache = NULL;

static DWORD WINAPI ThreadTickCache(LPVOID lpParam)
{
	while (::WaitForSingleObject(*Tickache::local_tickache->quote_feed->event, INFINITE) == WAIT_OBJECT_0)
	{
		Tickache::local_tickache->TickFile();
	}
#pragma endregion
	return 0;
}

Tickache::Tickache(FutureQuoteFeedForCache* _quote_feed, ContractDictionary* _contract_dic):
	quote_feed(_quote_feed),
	contract_dic(_contract_dic)
{
	local_tickache = this;

	std::string current_year(DateTime::GetCurrentYearString());
	std::string current_month(DateTime::GetCurrentMonthString());
	std::string current_day(DateTime::GetCurrentDayString());
	tick_store_dir = "Tick\\Future\\" + current_year + "\\" + current_month + "\\" + current_day + "\\";

	*quote_feed->event = ::CreateEvent(NULL, false, false, L"TickCache");
	::CloseHandle(::CreateThread(NULL, 0, ThreadTickCache, NULL, 0, NULL));
}

void Tickache::TickFile()
{
	if (quote_feed->tick_cache_index == 2)
	{
		TickFile(quote_feed->market_data_vec1);
	}
	else if (quote_feed->tick_cache_index == 1)
	{
		TickFile(quote_feed->market_data_vec2);
	}
}

std::vector<CThostFtdcDepthMarketDataField>::iterator it;
ContractDictionary::iterator it1;
std::unordered_map<Contract, std::pair<FILE*, int>>::iterator it2;
char buffer[200];
FILE* fp;
Contract contract;
void Tickache::TickFile(std::vector<CThostFtdcDepthMarketDataField> &market_data_vec)
{
	//������ + ����޸�ʱ��
	//TThostFtdcDateType	TradingDay + TThostFtdcTimeType	UpdateTime;
	//��Լ����
	//TThostFtdcInstrumentIDType	InstrumentID;
	//���¼�
	//TThostFtdcPriceType	LastPrice;
	//�ɽ�����
	//TThostFtdcVolumeType	Volume;
	//�ɽ����
	//TThostFtdcMoneyType	Turnover;
	//�ֲ���
	//TThostFtdcLargeVolumeType	OpenInterest;
	//�����һ
	//TThostFtdcPriceType	BidPrice1;
	//������һ
	//TThostFtdcVolumeType	BidVolume1;
	//������һ
	//TThostFtdcPriceType	AskPrice1;
	//������һ
	//TThostFtdcVolumeType	AskVolume1;
	for (it = quote_feed->market_data_vec1.begin(); it != quote_feed->market_data_vec1.end(); )
	{
		//modifications
		if (it->AskPrice1 > it->UpperLimitPrice)	//�����йұߵ����
		{
			it->AskPrice1 = it->LastPrice;
		}
		if (it->BidPrice1 > it->UpperLimitPrice)
		{
			it->BidPrice1 = it->LastPrice;
		}
			
		contract = it->InstrumentID;
		it1 = FindContractFromNormalNew(contract, contract_dic);
		if (it1 != contract_dic->end())
		{
			//strcpy(f.TradingDay, tradingDay);			//����
			strcpy_s(it->ExchangeID, ARRAYCOUNT(it->ExchangeID), it1->second.exchange.c_str());	//������
			if (it1->second.exchange == "CZCE") //�ɽ��������
			{
				//֣������ǰtick�ɽ��� = ����ṹ�ĳɽ���*��Լ����
				it->Turnover *= it1->second.volume_multiple;
			}
			else
			{
				//��֣�����ɽ����� = ����ṹ�ĳɽ�����/��Լ����
				it->AveragePrice /= it1->second.volume_multiple;
			}	
		}

		//format tick to string and save to disk
		sprintf_s(buffer, ARRAYCOUNT(buffer), "%s %s,%s,%.3f,%d,%.2f,%.2f,%.3f,%d,%.3f,%d", it->TradingDay,it->UpdateTime,it->InstrumentID,it->LastPrice,it->Volume,it->Turnover,it->OpenInterest,it->BidPrice1,it->BidVolume1,it->AskPrice1,it->AskVolume1);

		it2 = contract_file_handle_map.find(contract);
		if (it2 != contract_file_handle_map.end())
		{
			if (it2->second.second < start_count)
			{
				fopen_s(&fp, (tick_store_dir + contract + ".tk").c_str(), "a");;
				contract_file_handle_map[contract].second = start_count + 1;
			}
			else
			{
				fp = contract_file_handle_map[contract].first;
				++contract_file_handle_map[contract].second;
			}
		}
		else
		{
			fopen_s(&fp, (tick_store_dir + contract + ".tk").c_str(), "a");
			contract_file_handle_map[contract] = std::make_pair(fp, start_count + 1);
		}

		FileIO::AppendText(fp, buffer);

		it = quote_feed->market_data_vec1.erase(it);
		++start_count;
	}

	//release file handles haven't used in current loop
	for (it2 = contract_file_handle_map.begin(); it2 != contract_file_handle_map.end(); ++it2)
	{
		if (it2->second.second < start_count)
		{
			fclose(it2->second.first);
		}
	}
}


	
	
	
	
	
	
	
	
	
	
