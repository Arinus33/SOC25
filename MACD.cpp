#include "strategy.h"
#include <vector>

using namespace std;

void calculate_macd(const vector<double> &closes, vector<double> &macd, vector<double> &signal)
{
    const int fast = 12, slow = 26, signal_period = 9;
    double k_fast = 2.0 / (fast + 1), k_slow = 2.0 / (slow + 1), k_signal = 2.0 / (signal_period + 1);

    vector<double> ema_fast(closes.size()), ema_slow(closes.size());
    ema_fast[0] = closes[0]; ema_slow[0] = closes[0];
    for (size_t i = 1; i < closes.size(); ++i)
    {
        ema_fast[i] = closes[i] * k_fast + ema_fast[i - 1] * (1 - k_fast);
        ema_slow[i] = closes[i] * k_slow + ema_slow[i - 1] * (1 - k_slow);
    }

    for (size_t i = 0; i < closes.size(); ++i)
        macd[i] = ema_fast[i] - ema_slow[i];

    signal[0] = macd[0];
    for (size_t i = 1; i < closes.size(); ++i)
        signal[i] = macd[i] * k_signal + signal[i - 1] * (1 - k_signal);
}

TradeResult run_macd_strategy(const vector<Candle> &candles, double profit_threshold)
{
    vector<double> closes;
    for (const auto &c : candles) closes.push_back(c.close);

    vector<double> macd(closes.size()), signal(closes.size());
    calculate_macd(closes, macd, signal);

    vector<int> macd_positions(closes.size(), 0);
    int profitable_trades = 0, total_trades = 0;
    double total_return = 0, entry_price = 0;
    enum Position { NONE, LONG, SHORT } state = NONE;

    for (size_t i = 1; i < closes.size(); ++i)
    {
        if (macd[i] > signal[i] && macd[i - 1] <= signal[i - 1])
            macd_positions[i] = 1; // Buy
        else if (macd[i] < signal[i] && macd[i - 1] >= signal[i - 1])
            macd_positions[i] = -1; // Sell

        if (state == NONE)
        {
            if (macd_positions[i] == 1)
            {
                state = LONG;
                entry_price = closes[i];
            }
            else if (macd_positions[i] == -1)
            {
                state = SHORT;
                entry_price = closes[i];
            }
        }
        else if (state == LONG && macd_positions[i] == -1)
        {
            double ret = (closes[i] - entry_price) / entry_price;
            total_return += ret;
            if (ret >= profit_threshold) ++profitable_trades;
            ++total_trades; state = NONE;
        }
        else if (state == SHORT && macd_positions[i] == 1)
        {
            double ret = (entry_price - closes[i]) / entry_price;
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
        macd_positions
    };
}
