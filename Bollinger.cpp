#include "strategy.h"
#include <vector>
#include <cmath>

using namespace std;

double sma(const vector<double>& data, int end, int period)
{
    double sum = 0.0;
    for (int i = end - period + 1; i <= end; ++i)
        sum += data[i];
    return sum / period;
}

double stddev(const vector<double>& data, int end, int period)
{
    double mean = sma(data, end, period);
    double variance = 0.0;
    for (int i = end - period + 1; i <= end; ++i)
        variance += (data[i] - mean) * (data[i] - mean);
    return sqrt(variance / period);
}

TradeResult run_bollinger_strategy(const vector<Candle> &candles, double profit_threshold)
{
    vector<double> closes;
    for (const auto &c : candles) closes.push_back(c.close);

    vector<int> boll_positions(closes.size(), 0);
    int profitable_trades = 0, total_trades = 0;
    double total_return = 0, entry_price = 0;
    enum Position { NONE, LONG, SHORT } state = NONE;

    for (size_t i = 20; i < closes.size(); ++i)
    {
        double mid = sma(closes, i, 20);
        double std = stddev(closes, i, 20);
        double upper = mid + 2 * std;
        double lower = mid - 2 * std;

        if (closes[i] > upper)
            boll_positions[i] = -1;
        else if (closes[i] < lower)
            boll_positions[i] = 1;

        if (state == NONE)
        {
            if (boll_positions[i] == 1)
            {
                state = LONG;
                entry_price = closes[i];
            }
            else if (boll_positions[i] == -1)
            {
                state = SHORT;
                entry_price = closes[i];
            }
        }
        else if ((state == LONG && boll_positions[i] == -1) || (state == SHORT && boll_positions[i] == 1))
        {
            double ret = (state == LONG) ?
                (closes[i] - entry_price) / entry_price :
                (entry_price - closes[i]) / entry_price;
            total_return += ret;
            if (ret >= profit_threshold) ++profitable_trades;
            ++total_trades; state = NONE;
        }
    }

    if (state != NONE)
    {
        double final_ret = (state == LONG) ?
            (closes.back() - entry_price) / entry_price :
            (entry_price - closes.back()) / entry_price;
        total_return += final_ret;
        if (final_ret >= profit_threshold) ++profitable_trades;
        ++total_trades;
    }

    return {
        total_trades ? 100.0 * profitable_trades / total_trades : 0,
        total_trades ? 100.0 * total_return / total_trades : 0,
        total_trades,
        boll_positions
    };
}
