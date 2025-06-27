#include "strategy.h"
#include <vector>

using namespace std;

double average_volume(const vector<double>& volumes, int end, int period)
{
    double sum = 0.0;
    for (int i = end - period + 1; i <= end; ++i)
        sum += volumes[i];
    return sum / period;
}

TradeResult run_volume_spike_strategy(const vector<Candle> &candles, double profit_threshold, double multiplier = 2.0)
{
    vector<double> closes, volumes;
    for (const auto &c : candles)
    {
        closes.push_back(c.close);
        volumes.push_back(c.volume);
    }

    vector<int> volume_positions(closes.size(), 0);
    int profitable_trades = 0, total_trades = 0;
    double total_return = 0, entry_price = 0;
    enum Position { NONE, LONG } state = NONE;

    for (size_t i = 20; i < volumes.size(); ++i)
    {
        double avg_vol = average_volume(volumes, i, 20);
        if (volumes[i] > multiplier * avg_vol)
            volume_positions[i] = 1;

        if (state == NONE && volume_positions[i] == 1)
        {
            state = LONG;
            entry_price = closes[i];
        }
        else if (state == LONG && volume_positions[i] != 1)
        {
            double ret = (closes[i] - entry_price) / entry_price;
            total_return += ret;
            if (ret >= profit_threshold) ++profitable_trades;
            ++total_trades;
            state = NONE;
        }
    }

    if (state == LONG)
    {
        double final_ret = (closes.back() - entry_price) / entry_price;
        total_return += final_ret;
        if (final_ret >= profit_threshold) ++profitable_trades;
        ++total_trades;
    }

    return {
        total_trades ? 100.0 * profitable_trades / total_trades : 0,
        total_trades ? 100.0 * total_return / total_trades : 0,
        total_trades,
        volume_positions
    };
}
