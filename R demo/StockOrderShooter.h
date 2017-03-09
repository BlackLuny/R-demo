#pragma once
#include"Order.h"
#include"Utilities.h"

typedef bool(__stdcall* LoginDelegate)(char* IP, int Port, char* User, char* PassWord, char* TXPass, char* Yyb);//��½
typedef char*(__stdcall* QueryDataDelegate)(char* User, int Category);//��ѯ
typedef char*(__stdcall* CancelOrderDelegate)(char* User, char* Bh);//����
typedef char*(__stdcall* GetPriceDelegate)(char* Gpdm);//��ȡ�嵵����
typedef char*(__stdcall* SendOrderDelegate)(int Fx, char* User, char* Gddm, char* Gpdm, int Quantity, float Price);//�µ�
typedef std::string OrderTime;
typedef enum
{
	EXECUTE_BUY_PLAN,
	FIGHT_UNTIL_WIN
}Task;
typedef enum
{
	EXECUTED,
	NOT_EXECUTED

}ExecuteFlag;
struct CompareByValue
{
	bool operator()(const std::tuple<Price, OrderVolume, TradeVolume, _OrderID> &a, const std::tuple<Price, OrderVolume, TradeVolume, _OrderID> &b)
	{
		return std::get<0>(a) > std::get<0>(b);
	}

	bool operator()(const OrderDetailSync &a, const OrderDetailSync &b)
	{
		return a.price > b.price;
	}

	bool operator()(const PositionDetailSync &a, const PositionDetailSync &b)
	{
		return a.trade_date < b.trade_date;
	}
};
class StockOrderShooter:
	public Order
{
private:
	HMODULE hmodule = LoadLibraryA("JLAPI.dll");
	char return_char_arr[1024 * 10];
	std::string return_string;
	std::vector<std::string> tokens;
	FILE* fp;
	bool is_order_placed;//�����Ƿ񱻳ɹ��ύ
	bool all_positions_current = false;//���гֲ��Ƿ�ȫΪ���
	OrderDetailSync od;
	PositionDetailSync pd;
	int per_share;//ÿ������ķݶ�
private:
	void GenerateRandomTradeTime(std::string &trade_time);
	void GenerateRandomTradeTimeUnique(std::string &trade_time);
	int MakeVolume(double capital, double price);
#ifndef _DEBUG
	void MakeOrderPlan(double half_capital);//����һ���������ʽ���볡�ƻ�
#endif // !_DEBUG
	void PlaceLadderSellOrders(bool first_run);//���ݲ�ֵķ������ֲ��½�������
	void PlaceLadderBuyOrders(bool first_run);//�½�����
	void SplitPositions(std::string date = "");//���ڽ��������ֲֲ�ֳɽ��ݳֲ�
	void ModifyLocalPositionBasedOnServer();
	void PrintExecuteOrderPlan();
	void PrintFightUntilWin();

public:
	LoginDelegate Login = (LoginDelegate)GetProcAddress(hmodule, "JL_Login");
	QueryDataDelegate QueryData = (QueryDataDelegate)GetProcAddress(hmodule, "JL_QueryData");
	CancelOrderDelegate CancelOrder = (CancelOrderDelegate)GetProcAddress(hmodule, "JL_CancelOrder");
	GetPriceDelegate GetPrice = (GetPriceDelegate)GetProcAddress(hmodule, "JL_GetPrice");
	SendOrderDelegate SendOrder = (SendOrderDelegate)GetProcAddress(hmodule, "JL_SendOrder");
	char stock_code[7];
	double total_capital;//��ǰ���ʽ�
	double profit_rate;//Ŀ��ӯ����
	double init_capital;//��ʼͶ���ʽ�
	double available;//�����ʽ�
	double last_price;//���¼�
	double cost_price;//�ɱ���
	double yest_close_price;//�����̼�
	double upper_limit_price;//��ͣ���
	double lower_limit_price;//��ͣ���
	double price_cursor = 0.0;//�۸��α�
	unsigned int volume;//t+1��������
	int position_local = 0;//�����ֲܳ�
	int position_server = 0;//�������ֲܳ�
	double price_step;//���ռ۸񲽳�
	std::multiset<PositionDetailSync, CompareByValue> position_list;//�������ɽ�����,����������,1.ÿһ������Ҫ��ÿһ�ݳֲֶ�Ӧ2.���ɽ�����Ϊ����ʱ���������Ϊ��
	std::set<std::tuple<Price, OrderVolume, TradeVolume, _OrderID>, CompareByValue> order_list_buy_plan;//���б����ݼ۸񣬱��������ɽ���������������
	std::map<Price, OrderDetailSync> order_list_fight;//������Ϣֻ�õ������ֶΣ����ݼ۸񣬷��򣬱��������ɽ������������
	std::map<OrderTime, Capital> buy_plan;//�򵥼ƻ����µ�ʱ�䣬�µ���
	std::map<_OrderID, std::pair<Price, ExecuteFlag>> orderid_price_map;//������ţ��۸������map
	std::map<_OrderID, TradeTime> orderid_tradetime_map;//���ճɽ��Ķ�����ί�б�ź����һ�ʳɽ�ʱ�����ϼ�
	Task task;
	bool init = false;
	std::string curr_time;//��ǰʱ���ַ���
	std::string curr_date = DateTime::GetCurrentDateString();//��ǰ�����ַ���//TODO:curr_dateӦ��������������+ʱ�䣬��������ʱ������bug
public:
	StockOrderShooter(char* _stock_code, double _init_capital, double _profit_rate);
	virtual ~StockOrderShooter();
	void ConnectOrderServer();
	void Shoot(char* contract, double price, Direction direction, OpenFlag open_flag, int volume, OrderBroker* order_broker, InvestAccountSync* invest_account);
	void SellAll(double price = 0.0);
	void BuyHalf();
	void Cancel(OrderBroker* order_broker, InvestAccountSync* invest_account, OrderCredential* oc);
	void RefreshAccount();//��ѯ�ʽ�
	void RefreshLastPrice();//��ѯ�嵵����
	void RefreshVolume();//��ѯ�ɷ�
	double RefreshOrders();//��ѯ����ί��
	void RefreshTradedOrders();//��ѯ���ճɽ�
	void ComparePositionBetweenServerLocal();//�״����������ݿ��ȡ�ֲֶ��к󣬸ú����ӷ�������ȡ�ֲֶ��У��Ա�����
#ifdef _DEBUG
	void MakeOrderPlan(double half_capital);//����һ���������ʽ���볡�ƻ�
#endif // !_DEBUG
	void ExecuteOrderPlan();
	void FightUntilWin();
	void Print();//��ӡ��ǰ���ʱ���Ŀ���ʱ�������·�ɣ��ֲ��б������б��۸��α�
};
