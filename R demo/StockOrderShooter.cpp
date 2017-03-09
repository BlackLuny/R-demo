#pragma once
#include"StockOrderShooter.h"
#include"MathUtils.h"
StockOrderShooter::StockOrderShooter(char* _stock_code, double _init_capital, double _profit_rate):
	hmodule(LoadLibraryA("JLAPI.dll")),
	Login((LoginDelegate)GetProcAddress(hmodule, "JL_Login")),
	QueryData((QueryDataDelegate)GetProcAddress(hmodule, "JL_QueryData")),
	CancelOrder((CancelOrderDelegate)GetProcAddress(hmodule, "JL_CancelOrder")),
	GetPrice((GetPriceDelegate)GetProcAddress(hmodule, "JL_GetPrice")),
	SendOrder((SendOrderDelegate)GetProcAddress(hmodule, "JL_SendOrder")),
	init_capital(_init_capital),
	profit_rate(_profit_rate),
	position_local(0),
	position_server(0),
	price_cursor(0.0),
	init(false),
	all_positions_current(false),
	//TODO:curr_dateӦ��������������+ʱ�䣬��������ʱ������bug
	curr_date(DateTime::GetCurrentDateString())
{
	strcpy_s(return_char_arr, ARRAYCOUNT(return_char_arr), "");
	strcpy_s(stock_code, ARRAYCOUNT(stock_code), _stock_code);
	fopen_s(&fp, (DateTime::GetCurrentDateString() + ".log").c_str(), "a");
}
StockOrderShooter::~StockOrderShooter()
{
	FreeLibrary(hmodule);
}
void StockOrderShooter::ConnectOrderServer()
{
	bool login = false;
	login = /*�����Ϻ���ͨ*/Login("140.207.225.74", 7708, "39011807", "399884", NULL, "0");
	if (!login)
	{
		login = /*�����Ϻ�����*/Login("222.73.56.70", 7708, "39011807", "399884", NULL, "0");
	}
	if (!login)
	{
		login = /*����ɶ���ͨ*/Login("119.7.222.43", 7708, "39011807", "399884", NULL, "0");
	}
	if (!login)
	{
		login = /*����ɶ�����1*/Login("221.236.15.16", 7708, "39011807", "399884", NULL, "0");
	}
	if (!login)
	{
		login = /*����ɶ�����2*/Login("221.236.16.192", 7708, "39011807", "399884", NULL, "0");
	}
	if (!login)
	{
		login = /*�����ر�ͨ���ɶ�����1*/Login("221.236.15.16", 995, "39011807", "399884", NULL, "0");
	}
	if (!login)
	{
		login = /*�����ر�ͨ���ɶ�����2*/Login("221.236.29.103", 80, "39011807", "399884", NULL, "0");
	}
	if (!login)
	{
		login = /*�����ر�ͨ���Ϻ�����*/Login("222.73.69.88", 443, "39011807", "399884", NULL, "0");
	}
	if (!login)
	{
		login = /*�����ر�ͨ���Ϻ���ͨ*/Login("140.207.225.74", 80, "39011807", "399884", NULL, "0");
	}
}
void StockOrderShooter::Shoot(char* contract, double price, Direction direction, OpenFlag open_flag, int volume, OrderBroker* order_broker, InvestAccountSync* invest_account)
{
	//��A�ɶ����� 0153275142
	//��A�ɶ����� A408070359
	if (direction == BUY && open_flag == OPEN)
	{
		strcpy_s(return_char_arr, ARRAYCOUNT(return_char_arr), SendOrder(0, "39011807", "", contract, volume, MathUtils::RoundFloatTwoDigits(price)));
		if (atoi(return_char_arr) != 0)
		{
			is_order_placed = true;
			if (task == EXECUTE_BUY_PLAN)
			{
				order_list_buy_plan.insert(std::make_tuple(price, volume, 0, std::string(return_char_arr)));
			}
		}
		else
		{
			is_order_placed = false;
		}
		return;
	}
	if (direction == SELL && open_flag == CLOSE)
	{
		strcpy_s(return_char_arr, ARRAYCOUNT(return_char_arr), SendOrder(0, "39011807", "", contract, volume, MathUtils::RoundFloatTwoDigits(price)));
		if (atoi(return_char_arr) != 0)
		{
			is_order_placed = true;
			if (task == EXECUTE_BUY_PLAN)
			{
				order_list_buy_plan.insert(std::make_tuple(price, volume, 0, std::string(return_char_arr)));
			}
		}
		else
		{
			is_order_placed = false;
		}
	}
}
void StockOrderShooter::Cancel(OrderBroker* order_broker, InvestAccountSync* invest_account, OrderCredential* oc)
{
	CancelOrder("39011807", &oc->order_sys_id[0]);
}
void StockOrderShooter::RefreshAccount()
{
	strcpy_s(return_char_arr, ARRAYCOUNT(return_char_arr), QueryData("39011807", 104));
	if (strlen(return_char_arr) > 0)
	{
		return_string = return_char_arr;
		String::Split(&return_string, '|', &tokens);
		//���¿����ʽ�
		available = atof(tokens[14].c_str());
		//�������ʲ�
		total_capital = atof(tokens[17].c_str());
	}
}
void StockOrderShooter::RefreshLastPrice()
{
	strcpy_s(return_char_arr, ARRAYCOUNT(return_char_arr), GetPrice(stock_code));
	if (strlen(return_char_arr) > 0)
	{
		return_string = return_char_arr;
		String::Split(&return_string, '|', &tokens);
		last_price = atof(tokens[5].c_str());
		if (!init)
		{
			yest_close_price = atof(tokens[2].c_str());//���������̼�
			price_step = yest_close_price * 0.99 / 100.0;//�����̼�Ϊ��׼������۸񲽳�
			upper_limit_price = yest_close_price * 1.0995;//�����̼�Ϊ��׼��������ͣ���
			lower_limit_price = yest_close_price * 0.9005;//�����̼�Ϊ��׼�������ͣ���

			//�������̼�Ϊ��׼�����ɼ۸����
			double price_cursor_tmp = lower_limit_price;
			od.direction = UNKNOWN;
			od.volume = 0;
			od.trade_volume = 0;
			od.order_id = "";
			while (abs(price_cursor_tmp - yest_close_price) > price_step)
			{
				order_list_fight.insert(std::make_pair(price_cursor_tmp, od));
				price_cursor_tmp += price_step;
			}
			order_list_fight.insert(std::make_pair(yest_close_price, od));
			price_cursor_tmp = yest_close_price + price_step;
			while (abs(price_cursor_tmp - upper_limit_price) > price_step)
			{
				order_list_fight.insert(std::make_pair(price_cursor_tmp, od));
				price_cursor_tmp += price_step;
			}
			order_list_fight.insert(std::make_pair(upper_limit_price, od));
		}
	}
}
void StockOrderShooter::RefreshVolume()
{
	//��ѯ�ɷ�
	strcpy_s(return_char_arr, ARRAYCOUNT(return_char_arr), QueryData("39011807", 1140));
	if (strlen(return_char_arr) > 0)
	{
		return_string = return_char_arr;
		String::Split(&return_string, '|', &tokens);
		if (tokens.size() > 14)
		{
			//���¿�������
			volume = atoi(tokens[17].c_str());
			if (!init)
			{
				position_server = atoi(tokens[16].c_str());//���·������ֲܳ�
				cost_price = atof(tokens[18].c_str());//���³ɱ���
			}
		}
	}
}
void StockOrderShooter::RefreshTradedOrders()
{
	//��ѯ���ճɽ�
	strcpy_s(return_char_arr, ARRAYCOUNT(return_char_arr), QueryData("39011807", 1108));
	if (strlen(return_char_arr) > 0)
	{
		return_string = return_char_arr;
		String::Split(&return_string, '|', &tokens);
		if (tokens.size() > 17)
		{
			//���µ���ί��
			int size = tokens.size();
			size -= 17;
			int base_index = 16;//19Ϊ��ͷ��
			std::map<_OrderID, TradeTime>::iterator it;
			while (size > 0)
			{
				//orderid_tradetime_map
				it = orderid_tradetime_map.find(tokens[base_index + 11]);
				if (it == orderid_tradetime_map.end())
				{
					orderid_tradetime_map[tokens[base_index + 11]] = tokens[base_index + 1];
				}
				else
				{
					if (orderid_tradetime_map[tokens[base_index + 11]] < tokens[base_index + 1])
					{
						orderid_tradetime_map[tokens[base_index + 11]] = tokens[base_index + 1];
					}
				}
				base_index += 15;
				size -= 15;
			}
		}
	}
}
double StockOrderShooter::RefreshOrders()
{
	double trade_price = 0.0;
	RefreshTradedOrders();//�ڲ�ѯ����ί��ǰ�ȸ��µ��ճɽ�
	//��ѯ����ί�У�1������order_list_fight���ݽ�㣬���ýڵ�ȫ�ɽ���������Ϊ��ʼ״̬�������ֳɽ���������ɽ�����2�����ȫ���ɽ�ʱ����Ϊ�򵥣����õ����ɵĳֲּ���ֲ��б���Ϊ���������õ������ĳֲִӳֲ��б��Ƴ���3�����ݸ��º��orderid_tradetime_map,�ڱ���tokens��ÿһ����������󣬵õ����³ɽ���order_list_fight���ݽ��
	strcpy_s(return_char_arr, ARRAYCOUNT(return_char_arr), QueryData("39011807", 1102));
	if (strlen(return_char_arr) > 0)
	{
		return_string = return_char_arr;
		String::Split(&return_string, '|', &tokens);
		if (tokens.size() > 20)
		{
			//���µ���ί��
			int size = tokens.size();
			size -= 20;
			int base_index = 19;//19Ϊ(��0��ʼ��)��ͷ��
			std::string trade_time = "";
			double trade_price;
			while (size > 0)
			{
				if (tokens[base_index + 8] != tokens[base_index + 10])
				{
					auto it1 = orderid_price_map.find(tokens[base_index + 13]);//13Ϊorder_id��index
					if (it1 != orderid_price_map.end())
					{
						auto it2 = order_list_fight.find(it1->second.first);
						if (it2 != order_list_fight.end())
						{
							it2->second.trade_volume = atoi(tokens[base_index + 10].c_str());//���³ɽ������ɽ���������0Ҳ�����Ǳ����Ŀ�������Բ����Ǳ�����
						}
					}
				}
				else
				{
					auto it1 = orderid_price_map.find(tokens[base_index + 13]);//13Ϊorder_id��index
					if (it1 != orderid_price_map.end())
					{
						if (it1->second.second == NOT_EXECUTED)//����õ�����һ��timerδ�ɽ�������һ��timer�ѳɽ�
						{
							auto it2 = order_list_fight.find(it1->second.first);
							if (it2 != order_list_fight.end())
							{
								auto it3 = orderid_tradetime_map.find(it1->first);
								if (it3 != orderid_tradetime_map.end())
								{
									if (trade_time == "")
									{
										trade_time = orderid_tradetime_map[it1->first];
										trade_price = it1->second.first;
									}
									else if(trade_time < orderid_tradetime_map[it1->first])
									{
										trade_time = orderid_tradetime_map[it1->first];
										trade_price = it1->second.first;
									}
								}


								if (it2->second.direction == BUY)//����õ����򵥣����õ����ɵĳֲּ���ֲ��б�
								{
									pd.sell_order_id = "";
									pd.volume = it2->second.volume;
									pd.trade_date = curr_date;
									position_list.insert(pd);
								}
								else if(it2->second.direction == SELL)//����õ������������õ������ĳֲִӳֲ��б��Ƴ�
								{
									for (auto it4 = position_list.begin(); it4 != position_list.end(); ++it4)
									{
										if (it4->sell_order_id == it1->first)
										{
											position_list.erase(it4);
											break;
										}
									}
								}
								//���۸���ݽ����Ϊ��ʼ״̬
								it2->second.volume = 0;
								it2->second.order_id = "";
								it2->second.direction = UNKNOWN;
								it2->second.trade_volume = 0;
							}
							it1->second.second = EXECUTED;
						}
					}
				}
				base_index += 18;
				size -= 18;
			}
		}
	}
	return trade_price;
}
void StockOrderShooter::ComparePositionBetweenServerLocal()
{
	if (position_server == 0 && position_local != 0)
	{
		position_list.clear();
		//��������ǿ���ģ�����ֻƱ�ɵ��ף�ֱ��ӯ��Ϊֹ
		if (total_capital < init_capital)
		{
			double half_capital = available * 0.5;
			if (half_capital <= 100000.0)
			{
				//��ǰ���ʲ���50%С��100000ʱ��һ���Խ��÷ݶ�
				BuyHalf();
				//ok, ����һ����ɣ���timer���µĵ���û�б��ɽ������5s��δ�ɽ�����������
			}
			else
			{
				MakeOrderPlan(half_capital);
			}
			task = EXECUTE_BUY_PLAN;
		}
		return;
	}
	if (position_server != 0 && position_local == 0)
	{
		SplitPositions();
		price_cursor = last_price;//�۸��α�����Ϊ�ּ�
		PlaceLadderSellOrders(true);//���ݲ�ֵĳֲ��½�������
		PlaceLadderBuyOrders(true);//���ݿ����ʽ��½�����
		task = FIGHT_UNTIL_WIN;
		return;
	}
	if (position_server != 0 && position_local != 0)
	{
		ModifyLocalPositionBasedOnServer();
		price_cursor = yest_close_price;//�۸��α�����Ϊ�����̼ۣ��˼۸�Ϊ�����г�������Բ鵽����ӽ��������һ�ʳɽ��۵ļ۸��ҵ��յ��ǵ�ͣ��۵ļ����Դ˼۸�Ϊ������
		PlaceLadderSellOrders(true);//����������ĳֲ��½�������
		PlaceLadderBuyOrders(true);//���ݿ����ʽ��½�����
		task = FIGHT_UNTIL_WIN;
		return;
	}
}
void StockOrderShooter::SellAll(double price)
{
	if (price == 0.0)
	{
		Shoot(stock_code, last_price * 0.99, SELL, CLOSE, volume, NULL, NULL);
	}
	else
	{
		Shoot(stock_code, price, SELL, CLOSE, volume, NULL, NULL);
	}
}
void StockOrderShooter::BuyHalf()
{
	Shoot(stock_code, last_price * 1.03, BUY, OPEN, MakeVolume(available * 0.5, last_price * 1.03), NULL, NULL);
}
void StockOrderShooter::GenerateRandomTradeTime(std::string &trade_time)
{
	int section = MathUtils::GenerateRandomInRange(1,2);
	int hour, min, sec;
	if (section == 1)
	{
		//09:28:00 -- 11:29:59
		hour = MathUtils::GenerateRandomInRange(9, 11);
		if (hour == 9)
		{
			//range of min is 28 -- 59
			min = MathUtils::GenerateRandomInRange(28, 59);
		}
		else if (hour == 10)
		{
			//range of min is 0 -- 59
			min = MathUtils::GenerateRandomInRange(0, 59);
		}
		else
		{
			//range of min is 0 -- 29
			min = MathUtils::GenerateRandomInRange(0, 29);
		}
	}
	else
	{
		//13:00:00 -- 14:59:59
		hour = MathUtils::GenerateRandomInRange(13, 14);
		min = MathUtils::GenerateRandomInRange(0, 59);
	}
	sec = MathUtils::GenerateRandomInRange(0, 59);
	char hour_str[3];
	char min_str[3];
	char sec_str[3];
	if (hour < 10)
	{
		sprintf_s(hour_str, ARRAYCOUNT(hour_str), "0%d", hour);
	}
	else
	{
		sprintf_s(hour_str, ARRAYCOUNT(hour_str), "%d", hour);
	}

	if (min < 10)
	{
		sprintf_s(min_str, ARRAYCOUNT(min_str), "0%d", min);
	}
	else
	{
		sprintf_s(min_str, ARRAYCOUNT(min_str), "%d", min);
	}

	if (sec < 10)
	{
		sprintf_s(sec_str, ARRAYCOUNT(sec_str), "0%d", sec);
	}
	else
	{
		sprintf_s(sec_str, ARRAYCOUNT(sec_str), "%d", sec);
	}
	trade_time = std::string(hour_str) + ":" + min_str + ":" + hour_str;
}
int  StockOrderShooter::MakeVolume(double capital, double price)
{
	return (int)(capital / price / 100.0) * 100;
}
void StockOrderShooter::GenerateRandomTradeTimeUnique(std::string &trade_time)
{
	GenerateRandomTradeTime(trade_time);
	while (buy_plan.find(trade_time) != buy_plan.end())
	{
		GenerateRandomTradeTime(trade_time);
	}
}
void StockOrderShooter::MakeOrderPlan(double half_capital)
{
	//��ǰ���ʲ���50%����100000ʱ��ÿ10��һ����λ����
	std::string trade_time;
	do
	{
		GenerateRandomTradeTimeUnique(trade_time);
		buy_plan.insert(std::make_pair(trade_time, 100000.0));
		half_capital -= 100000.0;
	} while (half_capital > 100000.0);

	if (half_capital > 10000.0)
	{
		//���ʣ�µ�half_capital����10000������һ����λ������volumes��
		GenerateRandomTradeTimeUnique(trade_time);
		buy_plan.insert(std::make_pair(trade_time, half_capital));
	}
	else
	{
		//���ʣ�µ�half_capitalС�ڵ���10000�����÷ݶ���뵽volumes�ĵ�һ��iterator��
		buy_plan.begin()->second += half_capital;
	}
}
void StockOrderShooter::PlaceLadderSellOrders(bool fisrt_run)
{
	auto it1 = order_list_fight.begin();
	while (it1->first <= price_cursor)//��Ϊ������������ȡ���д��ڼ۸��α�Ľ��ݽ��
	{
		++it1;
		if (it1 == order_list_fight.end())
		{
			break;
		}
	}
	std::vector<std::multiset<PositionDetailSync, CompareByValue>::const_iterator> position_list_iterators;
	std::vector<PositionDetailSync> pds;
	if (fisrt_run)
	{
		for (auto it2 = position_list.begin(); it1 != order_list_fight.end() && it2 != position_list.end(); ++it1, ++it2)
		{
			Shoot(stock_code, it1->first, SELL, CLOSE, it2->volume, NULL, NULL);
			if (is_order_placed)//��������ύ�ɹ�
			{
				orderid_price_map[return_char_arr] = std::make_pair(it1->first, NOT_EXECUTED);//��ί�б�ţ��۸���ϲ���orderid_price_map
																							  //���¶������ݸý��ķ���ί��������ί�б��
				it1->second.direction = SELL;
				it1->second.volume = it2->volume;
				it1->second.order_id = return_char_arr;

				pd.volume = it2->volume;//all items in PositionList(a multiset container) are const, so use replacement rather than remove
				pd.trade_date = it2->trade_date;
				pd.sell_order_id = return_char_arr;
				pds.push_back(pd);
				position_list_iterators.push_back(it2);
			}
		}
	}
	else
	{
		for (auto it2 = position_list.begin(); it1 != order_list_fight.end() && it2 != position_list.end(); ++it1)
		{
			if (it2->trade_date == curr_date)
			{
				break;
			}
			if (it1->second.volume != 0)
			{
				continue;
			}
			else
			{
				Shoot(stock_code, it1->first, SELL, CLOSE, it2->volume, NULL, NULL);
				if (is_order_placed)//��������ύ�ɹ�
				{
					orderid_price_map[return_char_arr] = std::make_pair(it1->first, NOT_EXECUTED);//��ί�б�ţ��۸���ϲ���orderid_price_map
																								  //���¶������ݸý��ķ���ί��������ί�б��
					it1->second.direction = SELL;
					it1->second.volume = it2->volume;
					it1->second.order_id = return_char_arr;

					pd.volume = it2->volume;//all items in PositionList(a multiset container) are const, so use replacement rather than remove
					pd.trade_date = it2->trade_date;
					pd.sell_order_id = return_char_arr;
					pds.push_back(pd);
					position_list_iterators.push_back(it2);
				}
				++it2;
			}
		}
	}
	for (auto it3 = position_list_iterators.begin(); it3 != position_list_iterators.end(); ++it3)
	{
		position_list.erase(*it3);
	}
	for (auto it4 = pds.begin(); it4 != pds.end(); ++it4)
	{
		position_list.insert(*it4);
	}
}
void StockOrderShooter::PlaceLadderBuyOrders(bool first_run)
{
	if (first_run)
	{
		auto it1 = order_list_fight.rbegin();
		while (it1->first >= price_cursor)
		{
			++it1;
			if (it1 == order_list_fight.rend())
			{
				break;
			}
		}
		for (; it1 != order_list_fight.rend(); ++it1)
		{
			//���򵥣���ܸ�ƨ�ֲ�
			per_share = MakeVolume(available / (int)(available / 10000), it1->first);
			available -= it1->first * (double)per_share;
			if (available > 0)
			{
				Shoot(stock_code, it1->first, BUY, OPEN, per_share, NULL, NULL);
				if (is_order_placed)//��������ύ�ɹ�
				{
					orderid_price_map[return_char_arr] = std::make_pair(it1->first, NOT_EXECUTED);//��ί�б�ţ��۸���ϲ���orderid_price_map
																								  //�������ݸý��ķ���ί��������ί�б��
					it1->second.direction = BUY;
					it1->second.volume = per_share;
					it1->second.order_id = return_char_arr;
				}
			}
			else
			{
				break;
			}
			//������β��ʱ���ò�ֲ֣���Ϊÿ��timer�Ŀ�ʼ�׶ζ���RefreshAccount()
		}
	}
	else
	{

	}
}
void StockOrderShooter::SplitPositions(std::string date)
{
	//TODO:���cursor�Ѿ����ڵ�ǰ�����ո�ֻ��Ʊ����ͣ��ۻ��ͣ��ۣ���Ҫ�����޷��µ����쳣
	//���ְֲ�һ���㷨���
	double f_per_share = (double)position_server / 10;
	if (date == "")
	{
		date = DateTime::GetYesterDateString();
	}
	pd.trade_date = date;
	pd.sell_order_id = "";
	if (f_per_share < 100)//������µ���������1000
	{
		pd.volume = 100;
		do
		{
			position_list.insert(pd);
			position_server -= 100;
		} while (position_server > 0);
	}
	else
	{
		if ((int)f_per_share % 100 == 0)//����ֲ�10�ȷ��е�ÿ�ݶ���������
		{
			pd.volume = f_per_share;
			for (unsigned int i = 0; i < 10; ++i)
			{
				position_list.insert(pd);
			}
		}
		else//����ֲ�10�ȷ��е�ÿ�ݶ�����������
		{
			int mine_share = f_per_share - (int)f_per_share % 100;
			pd.volume = mine_share;
			for (unsigned int i = 0; i < 10; ++i)
			{
				position_list.insert(pd);//����������
				position_server -= mine_share;
			}
			int times = position_server / 100;
			for (unsigned int j = 0; j < times; ++j)
			{
				position_list.erase(position_list.begin());
				position_list.insert(pd);//ʣ��δ�����������ÿһ�ֲ����Ѿ����ɵķݶ���
			} 
		}
	}
}
void StockOrderShooter::ModifyLocalPositionBasedOnServer()
{
	//����position_local,���ݳֲ������������������ʽ���������
	int differ = position_local - position_server;
	auto it = position_list.begin();
	pd.sell_order_id = "";
	if (differ > 0)
	{
		differ = it->volume - differ;
		while (differ < 0)
		{
			it = position_list.erase(it);
			differ = it->volume + differ;
		}
		if (differ > 0)
		{
			pd.volume = differ;
			pd.trade_date = it->trade_date;
			position_list.insert(pd);
		}
		position_list.erase(it);
	}
	if (differ < 0)
	{
		pd.volume = it->volume - differ;
		pd.trade_date = it->trade_date;
		position_list.insert(pd);
		position_list.erase(it);
	}
}
void StockOrderShooter::ExecuteOrderPlan()
{
	if (!buy_plan.empty())
	{
		auto it = buy_plan.begin();
		if (curr_time > it->first)
		{
			Shoot(stock_code, last_price * 1.03, BUY, OPEN, MakeVolume(it->second, last_price * 1.03), NULL, NULL);
		}
		buy_plan.erase(it);
	}
	else
	{
		init = false;
		RefreshVolume();
		price_cursor = cost_price;
	}
}
void StockOrderShooter::FightUntilWin()
{
	price_cursor = RefreshOrders();
	if (price_cursor != 0.0)
	{
		if (!all_positions_current)
		{
			if (position_list.empty() || (!position_list.empty() && (position_list.begin())->trade_date == curr_date))//(1)���ڼ۸��ǣ���ʷ�ֶ�ʱ��ȫ��ƽ��(2)��ʷ��ȫ���������ֲ��б���ȫ�ǽ��
			{
				all_positions_current = true;//�����ûɶ����
			}
			//����order_list_fight
			//(1)С��price_cursor�Ľ��ݽ�㣬���δ�µ�������
			//(2)����price_cursor�Ľ��ݽ�㣬���δ�µ���������
			PlaceLadderBuyOrders(false);
			PlaceLadderSellOrders(false);
		}
	}
}
void StockOrderShooter::Print()
{
	printf("current capital: %.2f init capital: %.2f target capital: %.2f\n", total_capital, init_capital, init_capital * (1.0 + profit_rate));
	//TODO:the WriteLog function always fail, should try to solve the issue
	if (task == EXECUTE_BUY_PLAN)
	{
		PrintExecuteOrderPlan();
		return;
	}
	if (task == FIGHT_UNTIL_WIN)
	{
		PrintFightUntilWin();
		return;
	}
}
void StockOrderShooter::PrintExecuteOrderPlan()
{
	//****************************************************************************
	//|               placed orders              |        remaining plan         |
	//****************************************************************************
	//| price | order vol | trade vol | order id |   order time    |   capital   |
	//****************************************************************************
	//|   7   |    11     |    11     |   10     |      17         |     13      |

	printf("current routine: EXECUTE_BUY_PLAN\n");
	printf("****************************************************************************\n");
	printf("|             placed buy orders            |        remaining plan         |\n");
	printf("****************************************************************************\n");
	printf("| price | order vol | trade vol | order id |   order time    |   capital   |\n");
	printf("****************************************************************************\n");

	auto it1 = order_list_buy_plan.begin();
	auto it2 = buy_plan.begin();
	bool first_one_ends = true;
	bool second_one_ends = true;
	while (it1 != order_list_buy_plan.end() || it2 != buy_plan.end())
	{
		if (it1 != order_list_buy_plan.end())
		{
			printf("|%6.2f | %9d | %9d | %8s |", std::get<0>(*it1), std::get<1>(*it1), std::get<2>(*it1), std::get<3>(*it1).c_str());
			first_one_ends = false;
			++it1;
		}
		if (it2 != buy_plan.end())
		{
			if (!first_one_ends)
			{
				printf("    %8s     |%13.2f|\n", it2->first.c_str(), it2->second);
			}
			else
			{
				printf("                                           |    %8s     |%13.2f|\n", it2->first.c_str(), it2->second);
			}
			second_one_ends = false;
			++it2;
		}
		else
		{
			printf("\n");
		}

		if (!first_one_ends && !second_one_ends)
		{
			//�������Ͼ�δ����
			printf("****************************************************************************\n");
		}
		else if (!first_one_ends && second_one_ends)
		{
			//����һδ���������϶��ѽ���
			printf("********************************************\n");
		}
		else if (first_one_ends && !second_one_ends)
		{
			printf("                                           *********************************\n");
			//����һ�ѽ��������϶�δ����
		}
		first_one_ends = true;
		second_one_ends = true;
	}
}
void StockOrderShooter::PrintFightUntilWin()
{
	//********************************************************************************************************
	//|                 order ladder                 | price cursor |              position list             |
	//********************************************************************************************************
	//| price | dir | order vol |      order id      |<-------------| vol | trade date |  attached order id  |
	//********************************************************************************************************
	//|   7   |	B/S	|    11     |          4         |      17      |  4  | 2016-08-15 |          8          |

	printf("current routine: EXECUTE_FIGHT_UNTIL_WIN\n");
	printf("********************************************************************************************************\n");
	printf("|                 order ladder                 | price cursor |              position list             |\n");
	printf("********************************************************************************************************\n");
	printf("| price | dir | order vol |      order id      |<-------------| vol | trade date |  attached order id  |\n");
	printf("********************************************************************************************************\n");

	auto it1 = order_list_fight.begin();
	auto it2 = position_list.begin();
	bool first_one_ends = true;
	bool second_one_ends = true;
	static char direction[2];
	while (it1 != order_list_fight.end() || it2 != position_list.end())
	{
		if (it1 != order_list_fight.end())
		{
			if (it1->second.direction == BUY)
			{
				strcpy_s(direction, ARRAYCOUNT(direction), "B");
			}
			else if (it1->second.direction == SELL)
			{
				strcpy_s(direction, ARRAYCOUNT(direction), "S");
			}

			printf("|%6.2f |  %s  |%11d|%20s|", it1->first, direction, it1->second.volume, it1->second.order_id);
			
			if (price_cursor == it1->first)
			{
				printf("<-------------|");
			}
			else
			{
				printf("              |");
			}
			
			first_one_ends = false;
			++it1;
		}
		if (it2 != position_list.end())
		{
			if (!first_one_ends)
			{
				printf("%5d|%12s|%21s\n", it2->volume, it2->trade_date.c_str(),it2->sell_order_id.c_str());
			}
			else
			{
				printf("                                                               %5d|%12s|%21s\n", it2->volume, it2->trade_date.c_str(), it2->sell_order_id.c_str());
			}
			second_one_ends = false;
			++it2;
		}
		else
		{
			printf("\n");
		}

		if (!first_one_ends && !second_one_ends)
		{
			//�������Ͼ�δ����
			printf("********************************************************************************************************\n");
		}

		else if (!first_one_ends && second_one_ends)
		{
			//����һδ���������϶��ѽ���
			printf("***************************************************************\n");
		}
		else if (first_one_ends && !second_one_ends)
		{
			printf("                                                               *********************************\n");
			//����һ�ѽ��������϶�δ����
		}
		first_one_ends = true;
		second_one_ends = true;
	}
}