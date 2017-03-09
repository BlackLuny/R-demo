#pragma once
#include"AccountManager.h"

class FutureAccountManager :
	public AccountManager
{
private:
public:
	bool RetrieveAccountList(OrderBrokerList* order_broker_list) {}
	void UpdateAccount(OrderBroker* order_broker, InvestAccountSync* invest_account);
	void UpdatePosition(OrderBroker* order_broker, InvestAccountSync* invest_account);
	void TransferCapitalFromBrokerToBank(OrderBroker* order_broker, InvestAccountSync* invest_account, double amount);
	void TransferCapitalFromBankToBroker(OrderBroker* order_broker, InvestAccountSync* invest_account, double amount);
	void QueryPositionDetail(OrderBroker* order_broker, InvestAccountSync* invest_account);//�����ڻ������ã�����ɾ��
};
