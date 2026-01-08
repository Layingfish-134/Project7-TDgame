#pragma once
#include "manager.h"

class CoinManager : public Manager<CoinManager>
{
	friend class Manager<CoinManager>;
public:
	CoinManager() = default;
	~CoinManager() = default;
public:
	void increase_coin(double val)
	{
		num_coin += val;
	}

	void decrease_coin(double val)
	{
		num_coin = num_coin - val;
		if (num_coin < 0)
			num_coin = 0;
	}
private:
	double num_coin = 0;
};

