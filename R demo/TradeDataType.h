#pragma once
#include"preinclude.h"

const std::string trade_first_start_time = "08:50:00";
const std::string trade_first_end_time = "16:00:00";
const std::string trade_second_start_time = "20:50:00";
const std::string trade_second_end_time = "02:40:00";

//������������
typedef enum
{
	BUY,
	SELL,
	UNKNOWN
}Direction;

//���忪ƽ����
typedef enum
{
	OPEN,
	CLOSE,
	/*future only*/FORCE_CLOSE,//�ڻ�ר�У�ǿƽ
	/*future only*/FORCE_OFF,//�ڻ�ר�У�ǿ��
	/*future only*/LOCAL_FORCE_CLOSE//�ڻ�ר�У�����ǿƽ
}OpenFlag;

//�����ڻ�Ͷ���ױ���ʶ
typedef enum
{
	SPECULATION,//Ͷ��
	ARBITRAGE,//����
	HEDGE//�ױ�
}HedgeFlag;//�ױ���־

//�����Լ���ڱ�־
typedef enum
{
	NORMAL,//���ݿ�ȡ�ҽ�����δ����
	EXPIRE,//���ݿ�ȡ�ҽ���������
	NEW//����ʱ�����ҽ�����δ����
}ExpireFlag;

//�����˻�����
typedef enum
{
	REAL,//ʵ���˻�
	SIMULATE//ģ���˻�
}AccountType;

//���嶩�ı�ʶ
typedef enum
{
	SUBSCRIBE,//���ı�ʶ
	UNSUBSCRIBE,//ȡ�����ı�ʶ
} SubscribeFlag;

typedef std::string OrderFront, QuoteFront, Front, TradeID, TradeTime, TradeDate, _OrderID/*��Ʊר�ã�ί���ַ���*/;
typedef int Volume, OrderVolume, TradeVolume;
typedef double OpenPrice, Margin, Price, Capital;

typedef void(*AccountSpecificMessageHandler)(const char* msg, const char* table_name, const char* log_path);
typedef void(*GeneralMessageHandler)(const char* msg);




