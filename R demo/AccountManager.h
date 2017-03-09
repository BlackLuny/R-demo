#pragma once
#include"TradeStruct.h"
class AccountManager
{
protected:
	virtual bool RetrieveAccountList(OrderBrokerList* order_broker_list) = 0;
	virtual void UpdateAccount(OrderBroker* order_broker, InvestAccountSync* invest_account) = 0;
	virtual void UpdatePosition(OrderBroker* order_broker, InvestAccountSync* invest_account) = 0;
	virtual void TransferCapitalFromBrokerToBank(OrderBroker* order_broker, InvestAccountSync* invest_account, double amount) = 0;//֤��������ת��
	virtual void TransferCapitalFromBankToBroker(OrderBroker* order_broker, InvestAccountSync* invest_account, double amount) = 0;//��֤������ת��
	virtual void QueryPositionDetail(OrderBroker* order_broker, InvestAccountSync* invest_account) = 0;//�����ڻ������ã�����ɾ��
};
