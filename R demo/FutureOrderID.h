#pragma once
#include"preinclude.h"

struct OrderID
{
	std::string  contract;
	std::string exchange;
	std::string order_sys_id;//�ڻ�ר�У��������������
	std::string broker_order_seq;//�ڻ�ר�У����͹�˾�������
	bool operator==(const OrderID &id) { return contract == id.contract && exchange == id.exchange && broker_order_seq == id.broker_order_seq; }
};

bool operator==(const OrderID &id1, const OrderID &id2);
