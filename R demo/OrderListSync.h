#pragma once
#include"DataBind.h"
#include"TradeDataType.h"
//UI synchronized order list
#pragma region  data struct for order
class OrderCredential
{
private:
public:
	std::string contract;
	int front_id;//�ڻ�ר�У�ǰ�ñ��
	int session_id;//�ڻ�ר�У��Ự���
	std::string order_ref;//�ڻ�ר�У���������
	std::string exchange;
	std::string order_sys_id;//�ڻ�ָ�����ڽ������ı�ţ���Ʊָ�������
	bool operator==(const OrderCredential &oc) { return front_id == oc.front_id && session_id == oc.session_id && order_ref == oc.order_ref; }
};

bool operator==(const OrderCredential &oc1, const OrderCredential &oc2);

namespace std {
	template <>
	struct hash<OrderCredential>
	{
		std::size_t operator()(const OrderCredential& oc) const
		{
			std::size_t hash1 = std::hash<int>()(oc.front_id);
			std::size_t hash2 = std::hash<int>()(oc.session_id);
			std::size_t hash3 = std::hash<std::string>()(oc.order_ref);
			return ((hash1 ^ (hash2 << 1)) >> 1) ^ (hash3 << 1);
		}
	};
}

class OrderDetailSync :
	public DataBind
{
private:
	int row_index;
public:
	Direction direction;
	OpenFlag open_flag;//��ƽ��־
	std::string status;//�ҵ�״̬
	int volume;//��������
	int trade_volume;//�ɽ�����
	double price;//�����۸�
	std::string insert_time;//����ʱ��
	ExpireFlag expire_flag;
	_OrderID order_id;
	const OrderCredential* oc;
	void UpdateContract();//update control only
	void UpdateDirection();
	void UpdateOpenFlag();
	void UpdateStatus();
	void UpdateStatus(const char* _status);//update control and its internal corresponding member
	void UpdateStatus(const std::string &_status);
	void UpdateVolume();
	void UpdateVolume(int _volume);
	void UpdatePrice();
	void UpdatePrice(double _price);
	void UpdateInsertTime();
	void UpdateInsertTime(const std::string &_insert_time);
	void UpdateTradeVolume();
	void UpdateTradeVolume(int _trade_volume);
	void UpdateExchange();
	void UpdateFrontID();
	void UpdateSessionID();
	void UpdateOrderRef();
	int AddDataGridRow();
};

typedef std::unordered_map<OrderCredential, OrderDetailSync> OrderListSync;
#pragma endregion
