#pragma once
#include"DataBind.h"
#include"OrderListSync.h"
#include"PositionListSync.h"

//UI synchronized invest account list
class CThostFtdcTraderApi;
class FutureOrderProcessor;
class InvestAccountSync :
	public DataBind
{
public:
	std::string investor_id;
	std::string investor_password;
	double balance = 0.0;//���ڻ���ָ��̬Ȩ��
	double position_profit = 0.0;//�ֲ�ӯ��
	double close_profit = 0.0;//ƽ��ӯ��
	double frozen = 0.0;//�µ�����
	double available = 0.0;//�����ʽ�
	OrderListSync _order_list;
	PositionListSync position_list;
	std::string bound_bank_name;//���������������
	std::string bound_bank_account;//������������˻�
	double bound_bank_balance = 0.0;//����������л������
//TODO:please solve the problem:close the app in initialization phase will crash
//TODO:use file system to replace database
	/*future only*/int order_ref = 0;//�ڻ�ר�У���������
	/*future only*/double margin = 0.0;//�ڻ�ר�У�ռ�ñ�֤��
	/*future only*/std::string user_id;//�ڻ�ר��
	//*future only*//double prev_balance;//�ڻ�ר�У���̬Ȩ��
	/*future only*/int ctp_order_request_id = 0;
	/*future only*/int ctp_quote_request_id = 0;
	/*future only*/std::pair<CThostFtdcTraderApi*, FutureOrderProcessor* > orderapi_spi_pair;//�ڻ�ר�У�����ʵ��
	void UpdateAccountInfo();
	void UpdateErrorMessagePane(const std::string & err_msg);
	void UpdateErrorMessagePane(const char* err_msg);
};



