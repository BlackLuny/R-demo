#pragma once
#include"OrderListSync.h"
#include"PositionListSync.h"
#include"InvestAccountSync.h"
#include"SocketBase.h"

#pragma region struct for account
typedef InvestAccountSync QuoteAccountSync;
typedef std::vector<QuoteAccountSync> QuoteAccountList;
typedef std::string AccountID, BrokerUniqueName;//the BrokerUniqueName is a broker_name+broker_id combination
typedef std::unordered_map<AccountID, InvestAccountSync> InvestAccountList;

typedef struct
{
	std::string broker_name;
	std::string broker_id;
	AccountType account_type;
	InvestAccountList invest_account_list;
	std::vector<std::pair<OrderFront, TCPPortConnectionState>> order_front_list;
}OrderBroker;
typedef std::vector<OrderBroker> OrderBrokerList;
typedef std::unordered_map<BrokerUniqueName, OrderBroker> OrderBrokerLst;

typedef struct
{
	std::string broker_name;
	std::string broker_id;
	QuoteAccountList quote_account_list;//if one or more of the quote fronts are connectable,it is 'CONNECTABLE', otherwise 'UNCONNECTABLE'
	std::pair<std::vector<std::pair<QuoteFront, TCPPortConnectionState>>, TCPPortConnectionState> quote_front;
}QuoteBroker;
typedef std::vector<QuoteBroker> QuoteBrokerList;
#pragma endregion

#pragma region struct for contract
struct ContractID
{
	std::string contract;
	ExpireFlag expire_flag;
	bool operator==(const ContractID &id) { return contract == id.contract && expire_flag == id.expire_flag; }
};

bool operator==(const ContractID &id1, const ContractID &id2);

namespace std {
	template <>
	struct hash<ContractID>
	{
		std::size_t operator()(const ContractID& id) const
		{
			std::size_t hash1 = std::hash<std::string>()(id.contract);
			std::size_t hash2 = std::hash<int>()(id.expire_flag);
			return (hash1 ^ (hash2 << 1)) >> 1;
		}
	};
}

typedef struct
{
	std::string exchange;//����������
	double price_tick;//��Լ(��Ʊ)��С�䶯��λ
	double open_interest;//���ڻ���ָ�ֲ������ڹ�Ʊ��ָ��ͨ���� 
	/*future only*/int volume_multiple;//�ڻ�ר�У���Լ��������Ʊʼ��Ϊ1
	/*future only*/int deliver_year;//�ڻ�ר�У��������
	/*future only*/int deliver_month;//�ڻ�ר�У������·�
	/*future only*/std::string expire_date;//�ڻ�ר�У�������
	/*future only*/std::string start_deliver_date;//�ڻ�ר�У���ʼ������
	/*future only*/std::string end_deliver_date;//�ڻ�ר�У����������� 
	/*future only*/double buy_margin_ratio;//�ڻ�ר�У��򷽱�֤����
	/*future only*/double sell_margin_ratio;//�ڻ�ר�У�������֤����
}ContractCredential;
typedef std::unordered_map<ContractID, ContractCredential> ContractDictionary;
 
ContractDictionary::iterator FindContract(const std::string &contract, ContractDictionary* contract_dic);
ContractDictionary::iterator FindContractFromNormalNew(const std::string &contract, ContractDictionary* contract_dic);
ContractDictionary::iterator FindContractFromNormal(const std::string &contract, ContractDictionary* contract_dic);
ContractDictionary::iterator FindContractFromNew(const std::string &contract, ContractDictionary* contract_dic);
#pragma endregion