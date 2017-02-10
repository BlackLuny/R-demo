#pragma once
#include"TradeStruct.h"
class AccountManager
{
protected:
	virtual bool RetrieveAccountList(BrokerInfoList* broker_info_list) = 0;
	virtual void UpdateAccount(BrokerInfo* broker_info, InvestAccountSync* invest_account) = 0;
	virtual void UpdatePosition(BrokerInfo* broker_info, InvestAccountSync* invest_account) = 0;
	virtual void TransferCapitalFromBrokerToBank(BrokerInfo* broker_info, InvestAccountSync* invest_account, double amount) = 0;//֤��������ת��
	virtual void TransferCapitalFromBankToBroker(BrokerInfo* broker_info, InvestAccountSync* invest_account, double amount) = 0;//��֤������ת��
	virtual void QueryPositionDetail(BrokerInfo* broker_info, InvestAccountSync* invest_account) = 0;//�����ڻ������ã�����ɾ��
};
