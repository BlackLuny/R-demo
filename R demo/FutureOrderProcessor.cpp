#pragma once
#include"FutureOrderProcessor.h"
#include"FutureOrderShooter.h"
#include"FutureOrderManager.h"
#include"GlobalObject.h"

FutureOrderProcessor::FutureOrderProcessor(FutureOrderShooter* _order_shooter, OrderBroker* _order_broker, InvestAccountSync* _invest_account, unsigned int _index):
	CTPShared(_invest_account, _index),
	order_shooter(_order_shooter),
	order_broker(_order_broker)
	
{
}

FutureOrderProcessor::~FutureOrderProcessor()
{
}
																				
///���ͻ����뽻�׺�̨������ͨ������ʱ����δ��¼ǰ�����÷��������á�
void FutureOrderProcessor::OnFrontConnected()
{
	CThostFtdcReqUserLoginField field;
	ZeroMemory(&field, sizeof(field));
	strcpy_s(field.BrokerID, ARRAYCOUNT(field.BrokerID), order_broker->broker_id.c_str());
	strcpy_s(field.UserID, ARRAYCOUNT(field.UserID), invest_account->user_id.c_str());
	strcpy_s(field.Password, ARRAYCOUNT(field.Password), invest_account->investor_password.c_str());
	strcpy_s(field.UserProductInfo, ARRAYCOUNT(field.UserProductInfo), "EasyTrade2");
	switch (invest_account->orderapi_spi_pair.first->ReqUserLogin(&field, ++invest_account->ctp_order_request_id))
	{
	case 0:
		break;
	case -1:
	{
		msg = "failed to log in " + order_broker->broker_name + " ctp order server. network connection failed";
		(*order_shooter->handler)(msg.c_str(), /*(size_t)index*/"", NULL);	
	}
		break;
	case -2:
	{
		msg = "failed to log in " + order_broker->broker_name + " ctp order server. unprocessed requests exceed limit";
		(*order_shooter->handler)(msg.c_str(), /*(size_t)index*/"", NULL);
	}
		break;
	case -3:
	{
		msg = "failed to log in " + order_broker->broker_name + " ctp order server. requests sent per second exceed limit";
		(*order_shooter->handler)(msg.c_str(), /*(size_t)index*/"", NULL);
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
void FutureOrderProcessor::OnFrontDisconnected(int nReason)
{
	switch (nReason)
	{
	case 0x1001:
	{
		msg = "failed to keep connection to " + order_broker->broker_name + " ctp order server. failed to read on network";
		(*order_shooter->handler)(msg.c_str(), /*(size_t)index*/"", NULL);
	}
	break;
	case 0x1002:
	{
		msg = "failed to keep connection to " + order_broker->broker_name + " ctp order server. failed to write on network";
		(*order_shooter->handler)(msg.c_str(), /*(size_t)index*/"", NULL);
	}
	break;
	case 0x2001:
	{
		msg = "failed to keep connection to " + order_broker->broker_name + " ctp order server. receive heartbeat timeout";
		(*order_shooter->handler)(msg.c_str(), /*(size_t)index*/"", NULL);
	}
	break;
	case 0x2002:
	{
		msg = "failed to keep connection to " + order_broker->broker_name + " ctp order server. send heartbeat failed";
		(*order_shooter->handler)(msg.c_str(), /*(size_t)index*/"", NULL);
	}
	break;
	case 0x2003:
	{
		msg = "failed to keep connection to " + order_broker->broker_name + " ctp order server. received errorneous packets";
		(*order_shooter->handler)(msg.c_str(), /*(size_t)index*/"", NULL);
	}
	break;
	default:
		break;
	}
}

///��¼������Ӧ,�ڵ�½���Զ��������ȷ��
void FutureOrderProcessor::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (CTPShared::IsErrorRspInfo(pRspInfo))
	{
		msg = "failed to log in " + order_broker->broker_name + " ctp order server";
		CTPShared::FormatRspInfo(pRspInfo, &msg);
		(*order_shooter->handler)(msg.c_str(), /*(size_t)index*/"", NULL);
	}
	else if (pRspUserLogin)
	{
		//TODO:no need to write succeed message to disk log file
		msg = "succeeded in logging in " + order_broker->broker_name + " ctp order server";
		(*order_shooter->handler)(msg.c_str(), /*(size_t)index*/"", NULL);

		invest_account->order_ref = atoi(pRspUserLogin->MaxOrderRef);

		if (strlen(order_shooter->ctp_trading_day) <= 0)
		{
			strcpy_s(order_shooter->ctp_trading_day, ARRAYCOUNT(order_shooter->ctp_trading_day), invest_account->orderapi_spi_pair.first->GetTradingDay());
		}

#pragma region ��ѯ�����Ƿ��Ѿ�ȷ�Ͻ���
		CThostFtdcQrySettlementInfoConfirmField field;
		ZeroMemory(&field, sizeof(field));
		strcpy_s(field.BrokerID, ARRAYCOUNT(field.BrokerID), order_broker->broker_id.c_str());
		strcpy_s(field.InvestorID, ARRAYCOUNT(field.InvestorID), invest_account->investor_id.c_str());
		invest_account->orderapi_spi_pair.first->ReqQrySettlementInfoConfirm(&field, ++invest_account->ctp_order_request_id);
#pragma endregion	
	}
}

///����¼��������Ӧ(CTP�յ�����ָ����û��ͨ������У�飬�ܾ����ܱ���ָ��)
void FutureOrderProcessor::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	char request_id[50];
	sprintf_s(request_id, ARRAYCOUNT(request_id), "%d", nRequestID);
	if (CTPShared::IsErrorRspInfo(pRspInfo))
	{
		msg = "failed to insert order to CTP.";
		CTPShared::FormatRspInfo(pRspInfo, &msg);
		(*order_shooter->handler)(msg.c_str(), /*(size_t)index*/"", NULL);
	}
	else if (pInputOrder)
	{
		msg = "succeeded in inserting order ";
		(*order_shooter->handler)(msg.c_str(), /*(size_t)index*/"", NULL);
	}
}

///��������������Ӧ(CTP��Ϊ����ָ������)
void FutureOrderProcessor::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (CTPShared::IsErrorRspInfo(pRspInfo))
	{
		msg ="failed to cancel order by CTP";
		CTPShared::FormatRspInfo(pRspInfo, &msg);
		(*order_shooter->handler)(msg.c_str(), /*(size_t)index*/"", NULL);
	}
	else if (pInputOrderAction)
	{

	}
}

///Ͷ���߽�����ȷ����Ӧ
void FutureOrderProcessor::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (CTPShared::IsErrorRspInfo(pRspInfo))
	{
		msg = "failed to confirm settlement info of" + order_broker->broker_name + " " + invest_account->investor_id;
		CTPShared::FormatRspInfo(pRspInfo, &msg);
		(*order_shooter->handler)(msg.c_str(), /*(size_t)index*/"", NULL);
	}
	else
	{
		if (index == 0)
		{
			::SetEvent(Global::event);
		}
	}
}

///�����ѯ�ʽ��˻���Ӧ
void FutureOrderProcessor::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (CTPShared::IsErrorRspInfo(pRspInfo))
	{
	    msg = "failed to query account info of " + order_broker->broker_name + " " + invest_account->investor_id;
		CTPShared::FormatRspInfo(pRspInfo, &msg);
		(*order_shooter->handler)(msg.c_str(), /*(size_t)index*/"", NULL);
	}
	else if (pTradingAccount)
	{
		invest_account->available = pTradingAccount->Available;
		invest_account->close_profit = pTradingAccount->CloseProfit;
		//��̬Ȩ��=���ս���-�ϴ����ö��-�ϴ���Ѻ���+��Ѻ���-������+�����
		double prev_balance(pTradingAccount->PreBalance - pTradingAccount->PreCredit - pTradingAccount->PreMortgage + pTradingAccount->Mortgage - pTradingAccount->Withdraw + pTradingAccount->Deposit);
		//��̬Ȩ��=��̬Ȩ��+ ƽ��ӯ��+ �ֲ�ӯ�� +�ʽ���- ������
		invest_account->balance = prev_balance
			+ pTradingAccount->CloseProfit + pTradingAccount->PositionProfit + pTradingAccount->CashIn - pTradingAccount->Commission;
		invest_account->margin = pTradingAccount->CurrMargin;
		invest_account->frozen = pTradingAccount->FrozenMargin + pTradingAccount->FrozenCommission;
		invest_account->position_profit = pTradingAccount->PositionProfit;
		invest_account->UpdateAccountInfo();
	}
}

///�����ѯ��Լ��֤������Ӧ
void FutureOrderProcessor::OnRspQryInstrumentMarginRate(CThostFtdcInstrumentMarginRateField *pInstrumentMarginRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
}

///�����ѯ��Լ����������Ӧ
void FutureOrderProcessor::OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
}

///�����ѯ��Լ��Ӧ
void FutureOrderProcessor::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (CTPShared::IsErrorRspInfo(pRspInfo))
	{
		msg = "failed to query instrument";
		CTPShared::FormatRspInfo(pRspInfo, &msg);
		(*order_shooter->handler)(msg.c_str(), /*(size_t)index*/"", NULL);
	}
	else if (pInstrument)
	{
		if (pInstrument->ProductClass == THOST_FTDC_PC_Futures &&
			(pInstrument->IsTrading))
		{
			if (FindContractFromNormal(pInstrument->InstrumentID, &Global::contract_dic) == Global::contract_dic.end())
			{
				ContractCredential contract_credential;
				contract_credential.exchange = pInstrument->ExchangeID;
				contract_credential.deliver_year = pInstrument->DeliveryYear;
				contract_credential.deliver_month = pInstrument->DeliveryMonth;
				contract_credential.volume_multiple = pInstrument->VolumeMultiple;
				contract_credential.price_tick = pInstrument->PriceTick;
				contract_credential.expire_date = pInstrument->ExpireDate;
				contract_credential.start_deliver_date = pInstrument->StartDelivDate;
				contract_credential.end_deliver_date = pInstrument->EndDelivDate;
				contract_credential.buy_margin_ratio = pInstrument->LongMarginRatio;
				contract_credential.sell_margin_ratio = pInstrument->ShortMarginRatio;

				ContractID contract_id;
				contract_id.contract = pInstrument->InstrumentID;
				contract_id.expire_flag = NEW;
				Global::contract_dic[contract_id] = contract_credential;//so the full contract dictionary is normal + new
			}
		}
	}
	if (bIsLast)
	{
		::SetEvent(Global::event);
	}
}

///�����ѯͶ���߽�������Ӧ
void FutureOrderProcessor::OnRspQrySettlementInfo(CThostFtdcSettlementInfoField *pSettlementInfo, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (CTPShared::IsErrorRspInfo(pRspInfo))
	{
		msg = "failed to query " + order_broker->broker_name + " " + invest_account->investor_id + " " + "settlement info";
		CTPShared::FormatRspInfo(pRspInfo, &msg);
		(*order_shooter->handler)(msg.c_str(), /*(size_t)index*/"", NULL);
	}
	else if (pSettlementInfo)
	{
	}
}

///�����ѯת��������Ӧ
void FutureOrderProcessor::OnRspQryTransferBank(CThostFtdcTransferBankField *pTransferBank, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
}

///�����ѯͶ���ֲ߳���Ӧ
void FutureOrderProcessor::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	PositionCredential pc;
	if (CTPShared::IsErrorRspInfo(pRspInfo))
	{
		msg = "failed to query position of" + order_broker->broker_name + " " + invest_account->investor_id;
		CTPShared::FormatRspInfo(pRspInfo, &msg);
		(*order_shooter->handler)(msg.c_str(), /*(size_t)index*/"", NULL);
	}
	else if (pInvestorPosition && pInvestorPosition->Position > 0)
	{
		pc.contract = pInvestorPosition->InstrumentID;
		if (pInvestorPosition->PosiDirection == THOST_FTDC_PD_Long)
		{
			pc.direction = BUY;
		}
		else if (pInvestorPosition->PosiDirection == THOST_FTDC_PD_Short)
		{
			pc.direction = SELL;
		}

		std::string exchange;
		auto it2 = FindContractFromNormalNew(pc.contract, &Global::contract_dic);
		if (it2 != Global::contract_dic.end())
		{
			exchange = it2->second.exchange;
		}
		auto it = FindPositionFromNormalNew(&pc, &invest_account->position_list);
		if (it != invest_account->position_list.end())
		{
			it->second.is_server_contained = true;
			//update profit
			if (it->second.profit != pInvestorPosition->PositionProfit)
			{
				it->second.UpdateProfit(pInvestorPosition->PositionProfit);
			}
			//update historical volume
			if (it->second.historical_volume != pInvestorPosition->YdPosition)
			{
				it->second.historical_volume = pInvestorPosition->Position - pInvestorPosition->TodayPosition;
			}
			//update volume
			if (it->second.historical_volume > 0)
			{
				it->second.UpdateVolume(it->second.volume + it->second.historical_volume);
			}
			//insert into subpositions
			if (it->second.historical_volume > 0)
			{
				it->second.sub_positions.push_back(std::make_pair(it->second.historical_volume, pInvestorPosition->PreSettlementPrice));
			}
			//update average price
			if (it->second.historical_volume > 0)
			{
				double volume_x_price = 0.0;
				for (auto it1 = it->second.sub_positions.begin(); it1 != it->second.sub_positions.end(); ++it1)
				{
					volume_x_price += (double)it1->first * it1->second;
				}
				it->second.UpdateAveragePrice(volume_x_price / it->second.volume);
			}
			//update margin
			if (it->second.margin != pInvestorPosition->UseMargin)
			{
				it->second.UpdateMargin(pInvestorPosition->UseMargin);
			}
			//update exchange
			it->second.exchange = exchange;
			if (it->first.expire_flag == NORMAL)
			{
				it->second.AddDataGridRow();
			}
		}
		else
		{
			pc.expire_flag = NEW;
			PositionDetailSync pd; 
			if (pd.AddDataGridRow() != -1)
			{
				pd.is_server_contained = true;

				pd.UpdateContract();
				pd.UpdateProfit(pInvestorPosition->PositionProfit);
				pd.UpdateDirection();
				//insert historical volume
				pd.historical_volume = pInvestorPosition->Position - pInvestorPosition->TodayPosition;
				//insert average price
				if (pd.historical_volume > 0)
				{
					pd.UpdateAveragePrice(pInvestorPosition->PreSettlementPrice);
				}
				//insert volume
				pd.volume = 0;
				if (pd.historical_volume > 0)
				{
					pd.UpdateVolume(pd.volume + pd.historical_volume);
				}
				pd.UpdateMargin(pInvestorPosition->UseMargin);
				//insert exchange
				pd.exchange = exchange;
				//insert into subpositions
				if (pd.historical_volume > 0)
				{
					pd.sub_positions.push_back(std::make_pair(pd.historical_volume, pInvestorPosition->PreSettlementPrice));
				}
				invest_account->position_list[pc] = pd;
			}
			
#pragma endregion
		}
	}
	if (bIsLast)
	{
		for (auto it2 = invest_account->position_list.begin(); it2 != invest_account->position_list.end();)
		{
			if (!it2->second.is_server_contained && it2->first.expire_flag != EXPIRE)
			{
				pc = it2->first;
				pc.expire_flag = EXPIRE;
				std::swap(invest_account->position_list[pc], it2->second);


				it2->second.DeleteDataGridRow();
				for (auto it3 = invest_account->position_list.begin(); it3 != invest_account->position_list.end(); ++it3)
				{
					if (it3->second.GetRowIndex() > it2->second.GetRowIndex())
					{
						it3->second.DecrementRowIndex();
					}
				}
				Global::RemoveContractForUnsubscribe(it2->first.contract.c_str());
				
				it2 = invest_account->position_list.erase(it2);
			}
			else
			{
				++it2;
			}
		}
	}
}

///�����ѯ������Ϣȷ����Ӧ
void FutureOrderProcessor::OnRspQrySettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (CTPShared::IsErrorRspInfo(pRspInfo))
	{
		msg = "failed to query " + order_broker->broker_name + " " + invest_account->investor_id + " " + "settlement confirm";
		CTPShared::FormatRspInfo(pRspInfo, &msg);
		(*order_shooter->handler)(msg.c_str(), /*(size_t)index*/"", NULL);
	}
	else if (pSettlementInfoConfirm)
	{
		if (strcmp(pSettlementInfoConfirm->ConfirmDate, invest_account->orderapi_spi_pair.first->GetTradingDay()) == 0)
		{
			//have confirmed settlement today
			if (index == 0)
			{
				::SetEvent(Global::event);
			}
		}
		else
		{
			CThostFtdcSettlementInfoConfirmField field;
			ZeroMemory(&field, sizeof(field));
			strcpy_s(field.BrokerID, ARRAYCOUNT(field.BrokerID), order_broker->broker_id.c_str());
			strcpy_s(field.InvestorID, ARRAYCOUNT(field.InvestorID), invest_account->investor_id.c_str());
			invest_account->orderapi_spi_pair.first->ReqSettlementInfoConfirm(&field, ++invest_account->ctp_order_request_id);
		}
	}
	else
	{
		CThostFtdcSettlementInfoConfirmField field;
		ZeroMemory(&field, sizeof(field));
		strcpy_s(field.BrokerID, ARRAYCOUNT(field.BrokerID), order_broker->broker_id.c_str());
		strcpy_s(field.InvestorID, ARRAYCOUNT(field.InvestorID), invest_account->investor_id.c_str());
		invest_account->orderapi_spi_pair.first->ReqSettlementInfoConfirm(&field, ++invest_account->ctp_order_request_id);
	}
}

///��ѯ��֤����ϵͳ���͹�˾�ʽ��˻���Կ��Ӧ
void FutureOrderProcessor::OnRspQryCFMMCTradingAccountKey(CThostFtdcCFMMCTradingAccountKeyField *pCFMMCTradingAccountKey, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

}

///�����ѯͶ����Ʒ��/��Ʒ�ֱ�֤����Ӧ
void FutureOrderProcessor::OnRspQryInvestorProductGroupMargin(CThostFtdcInvestorProductGroupMarginField *pInvestorProductGroupMargin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

}

///�����ѯ��������֤������Ӧ
void FutureOrderProcessor::OnRspQryExchangeMarginRate(CThostFtdcExchangeMarginRateField *pExchangeMarginRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
}

///�����ѯ������������֤������Ӧ
void FutureOrderProcessor::OnRspQryExchangeMarginRateAdjust(CThostFtdcExchangeMarginRateAdjustField *pExchangeMarginRateAdjust, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
}

///�����ѯת����ˮ��Ӧ
void FutureOrderProcessor::OnRspQryTransferSerial(CThostFtdcTransferSerialField *pTransferSerial, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
}

///�����ѯ����ǩԼ��ϵ��Ӧ
void FutureOrderProcessor::OnRspQryAccountregister(CThostFtdcAccountregisterField *pAccountregister, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (CTPShared::IsErrorRspInfo(pRspInfo))
	{
		msg = "failed to query bound bank account";
		CTPShared::FormatRspInfo(pRspInfo, &msg);
		(*order_shooter->handler)(msg.c_str(), /*(size_t)index*/"", NULL);
	}
	else if (pAccountregister)
	{
		if (pAccountregister->BankID[0] == THOST_FTDC_PID_ABCProtocal)
			invest_account->bound_bank_name = "ũҵ����";

		else if (pAccountregister->BankID[0] == THOST_FTDC_PID_CBCProtocal)
			invest_account->bound_bank_name = "�й�����";
		else if (pAccountregister->BankID[0] == THOST_FTDC_PID_BOCOMProtocal)
			invest_account->bound_bank_name = "��ͨ����";
		else if (pAccountregister->BankID[0] == THOST_FTDC_PID_CCBProtocal)
			invest_account->bound_bank_name = "��������";
		else if (pAccountregister->BankID[0] == THOST_FTDC_PID_ICBCProtocal)
			invest_account->bound_bank_name = "��������";
		else
			invest_account->bound_bank_name = "��������";

		invest_account->bound_bank_account = pAccountregister->BankAccount;

		/*bankID = bankID.substr(strlen(pAccountregister->BankAccount) - 4, 4);*/

	}
	else
	{
		invest_account->bound_bank_name = "no bank account bound";
		invest_account->bound_bank_account = "no bank account bound";
	}
}

///����Ӧ��
void FutureOrderProcessor::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo)
	{
		char request_id[20];
		sprintf_s(request_id, ARRAYCOUNT(request_id), "%d", nRequestID);
		msg = std::string("failed to request ") + request_id;
		CTPShared::FormatRspInfo(pRspInfo, &msg);
		(*order_shooter->handler)(msg.c_str(), /*(size_t)index*/"", NULL);
	}
}

///����֪ͨ
void FutureOrderProcessor::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
	//update order
	if (pOrder)
	{
		OrderCredential oc;
		oc.contract = pOrder->InstrumentID;
		oc.exchange = pOrder->ExchangeID;
		oc.order_sys_id = pOrder->OrderSysID;
		oc.front_id = pOrder->FrontID;
		oc.session_id = pOrder->SessionID;
		oc.order_ref = pOrder->OrderRef;

		auto it1 = invest_account->_order_list.find(oc);
		if (it1 != invest_account->_order_list.end())
		{
			int index1 = std::distance(invest_account->_order_list.begin(), it1);
			it1->second.UpdateTradeVolume(pOrder->VolumeTraded);
			it1->second.UpdateStatus(pOrder->StatusMsg);
		}
		else
		{
			size_t index2 = invest_account->_order_list.size();
			OrderDetailSync od;
			od.expire_flag = NEW;

			if (od.AddDataGridRow() != -1)
			{
				od.UpdateContract();
				//insert direction
				if (pOrder->Direction == THOST_FTDC_D_Sell)
				{
					od.direction = SELL;
				}
				else if (pOrder->Direction == THOST_FTDC_D_Buy)
				{
					od.direction = BUY;
				}
				od.UpdateDirection();

#pragma region insert open_flag
				if (pOrder->CombOffsetFlag[0] == THOST_FTDC_OF_Open)
				{
					od.open_flag = OPEN;//open position
				}
				else if (pOrder->CombOffsetFlag[0] == THOST_FTDC_OF_Close || pOrder->CombOffsetFlag[0] == THOST_FTDC_OF_CloseToday || pOrder->CombOffsetFlag[0] == THOST_FTDC_OF_CloseYesterday)
				{
					od.open_flag = CLOSE;//close position
				}
				else if (pOrder->CombOffsetFlag[0] == THOST_FTDC_OF_ForceClose)
				{
					od.open_flag = FORCE_CLOSE;//ǿƽ
				}
				else if (pOrder->CombOffsetFlag[0] == THOST_FTDC_OF_ForceOff)
				{
					od.open_flag = FORCE_OFF;//ǿ��
				}
				else if (pOrder->CombOffsetFlag[0] == THOST_FTDC_OF_LocalForceClose)
				{
					od.open_flag = LOCAL_FORCE_CLOSE;//����ǿƽ
				}
				od.UpdateOpenFlag();
#pragma endregion

				od.UpdateStatus(pOrder->StatusMsg);
				od.UpdateVolume(pOrder->VolumeTotalOriginal);
				od.UpdatePrice(pOrder->LimitPrice);
				od.UpdateInsertTime(pOrder->InsertDate + std::string(" ") + pOrder->InsertTime);
				od.UpdateTradeVolume(pOrder->VolumeTraded);
				od.UpdateExchange();
				od.UpdateFrontID();
				od.UpdateSessionID();
				od.UpdateOrderRef();

				invest_account->_order_list[oc] = od;
				if (pOrder->CombOffsetFlag[0] == THOST_FTDC_OF_Open)
				{
					//5s��δ�ɽ��Զ�����
					order_shooter->future_order_manager.RegisterOpenOrder(order_broker, invest_account, &oc);
				}
			}
		}
	}
}

///�ɽ�֪ͨ
void FutureOrderProcessor::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
	if (pTrade)
	{
		if (pTrade->TradeSource == THOST_FTDC_TSRC_NORMAL)
		{
			PositionCredential pc;
			pc.contract = pTrade->InstrumentID;
			PositionListSync::iterator it;
#pragma region open position
			if (pTrade->OffsetFlag == THOST_FTDC_OF_Open)
			{
				pc.direction = pTrade->Direction == THOST_FTDC_D_Sell ? SELL : BUY;
				it = FindPositionFromNormalNew(&pc, &invest_account->position_list);
				if (it != invest_account->position_list.end())
				{
					//insert into subpositions
					it->second.sub_positions.push_back(std::make_pair(pTrade->Volume, pTrade->Price));
					it->second.UpdateVolume(it->second.volume + pTrade->Volume);
					//update average_price
					double volume_x_price = 0.0;
					for (auto it1 = it->second.sub_positions.begin(); it1 != it->second.sub_positions.end(); ++it1)
					{
						volume_x_price += (double)it1->first * it1->second;
					}
					it->second.UpdateAveragePrice(volume_x_price / it->second.volume);
					//TODO calculate margin and render
				}
				else
				{
					pc.expire_flag = NEW;
					PositionDetailSync pd(&pc);
					if (pd.AddDataGridRow() != -1)
					{
						pd.UpdateContract();
						pd.UpdateDirection();
						pd.UpdateAveragePrice(pTrade->Price);
						pd.UpdateVolume(pTrade->Volume);
						//TODO:insert profit
						pd.profit = 0.0;
						//TODO: calculate margin and render
						pd.margin = 0.0;
						//insert into subpositions
						pd.sub_positions.push_back(std::make_pair(pd.volume, pd.average_price));

						invest_account->position_list[pc] = pd;
						it = invest_account->position_list.find(pc);
						it->second.pc = &(it->first);

						Global::AddContractForSubscribe(pTrade->InstrumentID);
					}
				}
			}
#pragma endregion
			
#pragma region close position
			else if (pTrade->OffsetFlag == THOST_FTDC_OF_Close || pTrade->OffsetFlag == THOST_FTDC_OF_CloseToday || pTrade->OffsetFlag == THOST_FTDC_OF_CloseYesterday || pTrade->OffsetFlag == THOST_FTDC_OF_ForceClose || pTrade->OffsetFlag == THOST_FTDC_OF_ForceOff || pTrade->OffsetFlag == THOST_FTDC_OF_LocalForceClose)
			{
				pc.direction = pTrade->Direction == THOST_FTDC_D_Sell ? BUY : SELL;
				it = FindPositionFromNormalNew(&pc, &invest_account->position_list);
				if (it != invest_account->position_list.end())
				{
					//update volume
					if ((pTrade->OffsetFlag == THOST_FTDC_OF_CloseYesterday || pTrade->OffsetFlag == THOST_FTDC_OF_Close) && it->second.exchange == "SHFE")
					{
						it->second.historical_volume -= pTrade->Volume;
					}
					it->second.volume -= pTrade->Volume;

					if (it->second.volume > 0)
					{
						it->second.UpdateVolume();
					}
					else
					{
						pc = it->first;
						pc.expire_flag = EXPIRE;
						std::swap(invest_account->position_list[pc], it->second);

						it->second.DeleteDataGridRow();
						for (auto it1 = invest_account->position_list.begin(); it1 != invest_account->position_list.end(); ++it1)
						{
							if (it1->second.GetRowIndex() > it->second.GetRowIndex())
							{
								it1->second.DecrementRowIndex();
							}
						}
						Global::RemoveContractForUnsubscribe(pTrade->InstrumentID);

						invest_account->position_list.erase(it);
					}
				}
			}
#pragma endregion
		}
	}
}

///����¼�����ر����������յ���������Ϊ��������
void FutureOrderProcessor::OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo)
{
	if (CTPShared::IsErrorRspInfo(pRspInfo))
	{
		msg = "failed to insert order to exchange.";
		CTPShared::FormatRspInfo(pRspInfo, &msg);
		(*order_shooter->handler)(msg.c_str(), /*(size_t)index*/"", NULL);
	}
	else if (pInputOrder)
	{

	}
}

///������������ر����������յ������������Ϊ��������
void FutureOrderProcessor::OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo)
{
	if (CTPShared::IsErrorRspInfo(pRspInfo))
	{
		msg = "failed to cancel by exchange.";
		CTPShared::FormatRspInfo(pRspInfo, &msg);
		(*order_shooter->handler)(msg.c_str(), /*(size_t)index*/"", NULL);
	}
	else if (pOrderAction)
	{

	}
}

///��Լ����״̬֪ͨ
void FutureOrderProcessor::OnRtnInstrumentStatus(CThostFtdcInstrumentStatusField *pInstrumentStatus)
{
}

///��֤���������û�����
void FutureOrderProcessor::OnRtnCFMMCTradingAccountToken(CThostFtdcCFMMCTradingAccountTokenField *pCFMMCTradingAccountToken)
{
}

///�����ѯǩԼ������Ӧ
void FutureOrderProcessor::OnRspQryContractBank(CThostFtdcContractBankField *pContractBank, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
}

///�����ѯ��������û�����
void FutureOrderProcessor::OnRspQueryCFMMCTradingAccountToken(CThostFtdcQueryCFMMCTradingAccountTokenField *pQueryCFMMCTradingAccountToken, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
}

///�ڻ����������ʽ�ת�ڻ�֪ͨ
void FutureOrderProcessor::OnRtnFromBankToFutureByFuture(CThostFtdcRspTransferField *pRspTransfer)
{
	if (IsErrorRspInfo(pRspTransfer))
	{
		msg = "failed to transfer fund from bank to future";
		FormatRspInfo(pRspTransfer, &msg);
		(*order_shooter->handler)(msg.c_str(), /*(size_t)index*/"", NULL);
	}
}

///�ڻ������ڻ��ʽ�ת����֪ͨ
void FutureOrderProcessor::OnRtnFromFutureToBankByFuture(CThostFtdcRspTransferField *pRspTransfer)
{
	if (IsErrorRspInfo(pRspTransfer))
	{
		msg = "failed to transfer fund from future to bank";
		FormatRspInfo(pRspTransfer, &msg);
		(*order_shooter->handler)(msg.c_str(), /*(size_t)index*/"", NULL);
	}
	else if (pRspTransfer)
	{

	}
}

///�ڻ������ѯ�������֪ͨ
void FutureOrderProcessor::OnRtnQueryBankBalanceByFuture(CThostFtdcNotifyQueryAccountField *pNotifyQueryAccount)
{
	invest_account->bound_bank_balance = max(pNotifyQueryAccount->BankUseAmount, pNotifyQueryAccount->BankFetchAmount);
}

///�ڻ����������ʽ�ת�ڻ�����ر�
void FutureOrderProcessor::OnErrRtnBankToFutureByFuture(CThostFtdcReqTransferField *pReqTransfer, CThostFtdcRspInfoField *pRspInfo)
{
	msg = "failed to transfer fund from bank to future";
	CTPShared::FormatRspInfo(pRspInfo, &msg);
	(*order_shooter->handler)(msg.c_str(), /*(size_t)index*/"", NULL);
}

///�ڻ������ڻ��ʽ�ת���д���ر�
void FutureOrderProcessor::OnErrRtnFutureToBankByFuture(CThostFtdcReqTransferField *pReqTransfer, CThostFtdcRspInfoField *pRspInfo)
{
	msg = "failed to transfer fund from future to bank";
	CTPShared::FormatRspInfo(pRspInfo, &msg);
	(*order_shooter->handler)(msg.c_str(), /*(size_t)index*/"", NULL);
}

///�ڻ������ѯ����������ر�
void FutureOrderProcessor::OnErrRtnQueryBankBalanceByFuture(CThostFtdcReqQueryAccountField *pReqQueryAccount, CThostFtdcRspInfoField *pRspInfo)
{
	msg = "failed to query bound bank account balance";
	CTPShared::FormatRspInfo(pRspInfo, &msg);
	(*order_shooter->handler)(msg.c_str(), /*(size_t)index*/"", NULL);
}

///�ڻ����������ʽ�ת�ڻ�Ӧ��
void FutureOrderProcessor::OnRspFromBankToFutureByFuture(CThostFtdcReqTransferField *pReqTransfer, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	msg = "failed to transfer fund from bank to future";
	CTPShared::FormatRspInfo(pRspInfo, &msg);
	(*order_shooter->handler)(msg.c_str(), /*(size_t)index*/"", NULL);
}

///�ڻ������ڻ��ʽ�ת����Ӧ��
void FutureOrderProcessor::OnRspFromFutureToBankByFuture(CThostFtdcReqTransferField *pReqTransfer, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	msg = "failed to transfer fund from future to bank";
	CTPShared::FormatRspInfo(pRspInfo, &msg);
	(*order_shooter->handler)(msg.c_str(), /*(size_t)index*/"", NULL);
}

///�ڻ������ѯ�������Ӧ��
void FutureOrderProcessor::OnRspQueryBankAccountMoneyByFuture(CThostFtdcReqQueryAccountField *pReqQueryAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
}

bool FutureOrderProcessor::IsErrorRspInfo(CThostFtdcRspTransferField*pRspTransfer)
{
	bool IsError(((pRspTransfer) && (pRspTransfer->ErrorID != 0)));
	return IsError;
}

void FutureOrderProcessor::FormatRspInfo(CThostFtdcRspTransferField* pRspTransfer, std::string* entry_msg)
{
	*entry_msg += std::string(" error message ") + pRspTransfer->ErrorMsg + ".";
}